// +build iris

package iris

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	commonUtils "github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/log"
)

type mirrorIDs struct {
	sessionID  uint64
	MirrorKeys []string
}

// Collector represents a single collector with respect to HAL
type Collector struct {
	Name         string
	VrfName      string
	Destination  string
	PacketSize   uint32
	Gateway      string
	Type         string
	StripVlanHdr bool
	SessionID    uint64
}

// MirrorDestToIDMapping maps the unique tuple of vrfname-destinationIP to mirror session IDs for hal and referenced objects.
var MirrorDestToIDMapping = map[string]*mirrorIDs{}

// HandleCol handles crud operations on collector
func HandleCol(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, col Collector, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createColHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID)
	case types.Delete:
		return deleteColHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createColHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col Collector, vrfID uint64) error {
	foundCollector := false
	var mirrorSessionID uint64
	dstIP := col.Destination
	// Create the unique key for collector dest IP
	destKey := commonUtils.BuildDestKey(col.VrfName, dstIP)
	_, ok := MirrorDestToIDMapping[destKey]
	if ok {
		mirrorSessionID = MirrorDestToIDMapping[destKey].sessionID
		MirrorDestToIDMapping[destKey].MirrorKeys = append(MirrorDestToIDMapping[destKey].MirrorKeys, col.Name)
		foundCollector = true
	} else {
		// Update Mirror session ID to be under 8. TODO remove this once HAL doesn't rely on agents to provide its hardware ID
		mirrorSessionID = infraAPI.AllocateID(types.MirrorSessionID, 0)
	}
	col.SessionID = mirrorSessionID

	if !foundCollector {
		compositeKey := fmt.Sprintf("Collector/%s", destKey)
		mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
		if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, mgmtIP, dstIP, col.Gateway, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorCreateLateralObjects, "Collector: %s | Err: %v", col.Name, err))
			return errors.Wrapf(types.ErrMirrorCreateLateralObjects, "Collector: %s | Err: %v", col.Name, err)
		}

		// Create MirrorSession
		mirrorReqMsg := convertHalMirrorSession(col, vrfID)
		resp, err := telemetryClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for MirrorSession | %s", col.Name)); err != nil {
				return err
			}
		}

		// Add to mappings
		MirrorDestToIDMapping[destKey] = &mirrorIDs{
			sessionID:  mirrorSessionID,
			MirrorKeys: []string{col.Name},
		}
	}
	return nil
}

func deleteColHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col Collector, vrfID uint64) error {
	dstIP := col.Destination
	// Create the unique key for collector dest IP
	destKey := commonUtils.BuildDestKey(col.VrfName, dstIP)
	sessionKeys, ok := MirrorDestToIDMapping[destKey]
	if !ok {
		log.Error(errors.Wrapf(types.ErrCollectorAlreadyDeleted, "Collector: %s | DestKey: %s", col.Name, destKey))
		return errors.Wrapf(types.ErrCollectorAlreadyDeleted, "Collector: %s | DestKey: %s", col.Name, destKey)
	}
	// Remove the mirror key from the map
	length := len(sessionKeys.MirrorKeys)
	index := -1
	for idx, m := range sessionKeys.MirrorKeys {
		if m == col.Name {
			index = idx
			break
		}
	}
	if index != -1 {
		sessionKeys.MirrorKeys[index] = sessionKeys.MirrorKeys[length-1]
		sessionKeys.MirrorKeys = sessionKeys.MirrorKeys[:length-1]
	}

	if len(sessionKeys.MirrorKeys) == 0 {
		col.SessionID = sessionKeys.sessionID
		mirrorSessionDeleteReq := &halapi.MirrorSessionDeleteRequestMsg{
			Request: []*halapi.MirrorSessionDeleteRequest{
				{
					KeyOrHandle: convertMirrorSessionKeyHandle(col.SessionID),
				},
			},
		}

		mResp, err := telemetryClient.MirrorSessionDelete(context.Background(), mirrorSessionDeleteReq)
		if mResp != nil {
			if err := utils.HandleErr(types.Delete, mResp.Response[0].ApiStatus, err, fmt.Sprintf("Delete Failed for MirrorSession | %s", col.Name)); err != nil {
				return err
			}
		}
		delete(MirrorDestToIDMapping, destKey)
		compositeKey := fmt.Sprintf("Collector/%s", destKey)
		if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, dstIP, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.Name, err))
			return errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.Name, err)
		}
	} else {
		log.Infof("Collector: %s | DstIP: %s | VrfName: %s still referenced by %s", col.Name, dstIP, col.VrfName, strings.Join(sessionKeys.MirrorKeys, " "))
	}
	return nil
}

func convertHalMirrorSession(col Collector, vrfID uint64) *halapi.MirrorSessionRequestMsg {
	return &halapi.MirrorSessionRequestMsg{
		Request: []*halapi.MirrorSessionSpec{
			{
				KeyOrHandle:  convertMirrorSessionKeyHandle(col.SessionID),
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				Snaplen:      col.PacketSize,
				Destination: &halapi.MirrorSessionSpec_ErspanSpec{ // TODO Fix Destination when more than one collector per MirrorSession is supported
					ErspanSpec: &halapi.ERSpanSpec{
						DestIp:      utils.ConvertIPAddresses(col.Destination)[0],
						SpanId:      uint32(col.SessionID),
						Type:        halapi.ERSpanType(halapi.ERSpanType_value[strings.ToUpper(col.Type)]),
						VlanStripEn: col.StripVlanHdr,
					},
				},
			},
		},
	}
}
