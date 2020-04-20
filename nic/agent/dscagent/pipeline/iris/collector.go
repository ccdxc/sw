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
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

type mirrorIDs struct {
	sessionID  uint64
	MirrorKeys []string
}

// MirrorDestToIDMapping maps the unique tuple of vrfname-destinationIP to mirror session IDs for hal and referenced objects.
var MirrorDestToIDMapping = map[string]*mirrorIDs{}

// HandleCollector handles crud operations on collector
func HandleCollector(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, col netproto.Collector, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createCollectorHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID)
	case types.Update:
		return updateCollectorHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID)
	case types.Delete:
		return deleteCollectorHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col netproto.Collector, vrfID uint64) error {
	foundCollector := false
	var mirrorSessionID uint64
	dstIP := col.Spec.Destination
	// Create the unique key for collector dest IP
	destKey := commonUtils.BuildDestKey(col.Spec.VrfName, dstIP)
	_, ok := MirrorDestToIDMapping[destKey]
	if ok {
		mirrorSessionID = MirrorDestToIDMapping[destKey].sessionID
		MirrorDestToIDMapping[destKey].MirrorKeys = append(MirrorDestToIDMapping[destKey].MirrorKeys, col.GetKey())
		foundCollector = true
	} else {
		// Update Mirror session ID to be under 8. TODO remove this once HAL doesn't rely on agents to provide its hardware ID
		mirrorSessionID = infraAPI.AllocateID(types.MirrorSessionID, 0)
	}
	col.Status.Collector = mirrorSessionID

	if !foundCollector {
		compositeKey := fmt.Sprintf("%s/%s", col.GetKind(), destKey)
		mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
		if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, mgmtIP, dstIP, col.Spec.Gateway, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorCreateLateralObjects, "Collector: %s | Err: %v", col.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorCreateLateralObjects, "Collector: %s | Err: %v", col.GetKey(), err)
		}

		// Create MirrorSession
		mirrorReqMsg := convertMirrorSession(col, dstIP, vrfID)
		resp, err := telemetryClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", col.GetKind(), col.GetKey())); err != nil {
				return err
			}
		}

		// Add to mappings
		MirrorDestToIDMapping[destKey] = &mirrorIDs{
			sessionID:  mirrorSessionID,
			MirrorKeys: []string{col.GetKey()},
		}
	}

	dat, _ := col.Marshal()

	if err := infraAPI.Store(col.Kind, col.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Collector: %s | Collector: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Collector: %s | Collector: %v", col.GetKey(), err)
	}
	return nil
}

func updateCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col netproto.Collector, vrfID uint64) error {
	var existingCol netproto.Collector
	dat, err := infraAPI.Read(col.Kind, col.GetKey())
	if err != nil {
		return err
	}
	err = existingCol.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err)
	}
	if err := deleteCollectorHandler(infraAPI, telemetryClient, intfClient, epClient, existingCol, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrCollectorDeleteDuringUpdate, "Collector: %s | Collector: %v", existingCol.GetKey(), err))
	}
	if err := createCollectorHandler(infraAPI, telemetryClient, intfClient, epClient, col, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrCollectorCreateDuringUpdate, "Collector: %s | Collector: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrCollectorCreateDuringUpdate, "Collector: %s | Collector: %v", col.GetKey(), err)
	}
	return nil
}

func deleteCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col netproto.Collector, vrfID uint64) error {
	dstIP := col.Spec.Destination
	// Create the unique key for collector dest IP
	destKey := commonUtils.BuildDestKey(col.Spec.VrfName, dstIP)
	sessionKeys, ok := MirrorDestToIDMapping[destKey]
	if !ok {
		log.Error(errors.Wrapf(types.ErrCollectorAlreadyDeleted, "Collector: %s | DestKey: %s", col.GetKey(), destKey))
		return errors.Wrapf(types.ErrCollectorAlreadyDeleted, "Collector: %s | DestKey: %s", col.GetKey(), destKey)
	}
	// Remove the mirror key from the map
	length := len(sessionKeys.MirrorKeys)
	index := -1
	for idx, m := range sessionKeys.MirrorKeys {
		if m == col.GetKey() {
			index = idx
			break
		}
	}
	if index != -1 {
		sessionKeys.MirrorKeys[index] = sessionKeys.MirrorKeys[length-1]
		sessionKeys.MirrorKeys = sessionKeys.MirrorKeys[:length-1]
	}

	if len(sessionKeys.MirrorKeys) == 0 {
		col.Status.Collector = sessionKeys.sessionID
		mirrorSessionDeleteReq := &halapi.MirrorSessionDeleteRequestMsg{
			Request: []*halapi.MirrorSessionDeleteRequest{
				{
					KeyOrHandle: convertMirrorSessionKeyHandle(col.Status.Collector),
				},
			},
		}

		mResp, err := telemetryClient.MirrorSessionDelete(context.Background(), mirrorSessionDeleteReq)
		if mResp != nil {
			if err := utils.HandleErr(types.Delete, mResp.Response[0].ApiStatus, err, fmt.Sprintf("Delete Failed for %s | %s", col.GetKind(), col.GetKey())); err != nil {
				return err
			}
		}
		delete(MirrorDestToIDMapping, destKey)
		compositeKey := fmt.Sprintf("%s/%s", col.GetKind(), destKey)
		if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, dstIP, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.GetKey(), err)
		}
	} else {
		log.Infof("Collector: %s | DstIP: %s | VrfName: %s still referenced by %s", col.GetKey(), dstIP, col.Spec.VrfName, strings.Join(sessionKeys.MirrorKeys, " "))
	}

	if err := infraAPI.Delete(col.Kind, col.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", col.GetKey(), err)
	}
	return nil
}

func convertMirrorSession(col netproto.Collector, destIP string, vrfID uint64) *halapi.MirrorSessionRequestMsg {
	ctype := strings.ToUpper(col.Spec.Type)
	if _, ok := halapi.ERSpanType_value[ctype]; !ok {
		// erspan type 3 by default
		ctype = halapi.ERSpanType_ERSPAN_TYPE_3.String()
	}

	return &halapi.MirrorSessionRequestMsg{
		Request: []*halapi.MirrorSessionSpec{
			{
				KeyOrHandle:  convertMirrorSessionKeyHandle(col.Status.Collector),
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				Snaplen:      col.Spec.PacketSize,
				Destination: &halapi.MirrorSessionSpec_ErspanSpec{ // TODO Fix Destination when more than one collector per MirrorSession is supported
					ErspanSpec: &halapi.ERSpanSpec{
						DestIp:      utils.ConvertIPAddresses(destIP)[0],
						SpanId:      uint32(col.Status.Collector),
						Type:        halapi.ERSpanType(halapi.ERSpanType_value[ctype]),
						VlanStripEn: col.Spec.StripVlanHdr,
					},
				},
			},
		},
	}
}
