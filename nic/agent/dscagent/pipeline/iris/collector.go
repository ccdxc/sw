// +build iris

package iris

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

type mirrorIDs struct {
	sessionID  uint64
	mirrorKeys []string
}

var mirrorDestToIDMapping = map[string]*mirrorIDs{}

// HandleCollector handles crud operations on collector
func HandleCollector(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, oper types.Operation, col netproto.Collector, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createCollectorHandler(infraAPI, telemetryClient, col, vrfID)
	case types.Update:
		return updateCollectorHandler(infraAPI, telemetryClient, col, vrfID)
	case types.Delete:
		return deleteCollectorHandler(infraAPI, telemetryClient, col, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, col netproto.Collector, vrfID uint64) error {
	foundCollector := false
	var mirrorSessionID uint64
	dstIP := col.Spec.Destination
	// Create the unique key for collector dest IP
	destKey := col.Spec.VrfName + "-" + dstIP
	_, ok := mirrorDestToIDMapping[destKey]
	if ok {
		mirrorSessionID = mirrorDestToIDMapping[destKey].sessionID
		foundCollector = true
	} else {
		// Update Mirror session ID to be under 8. TODO remove this once HAL doesn't rely on agents to provide its hardware ID
		mirrorSessionID = infraAPI.AllocateID(types.MirrorSessionID, 0)
		mirrorSessionID = mirrorSessionID % types.MaxMirrorSessions
	}
	col.Status.Collector = mirrorSessionID

	if !foundCollector {
		// Create MirrorSession
		mirrorReqMsg := convertMirrorSession(col, dstIP, vrfID)
		resp, err := telemetryClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", col.GetKind(), col.GetKey())); err != nil {
				return err
			}
		}

		// Add to mappings
		mirrorDestToIDMapping[destKey] = &mirrorIDs{
			sessionID: mirrorSessionID,
		}
	}

	dat, _ := col.Marshal()

	if err := infraAPI.Store(col.Kind, col.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Collector: %s | Collector: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Collector: %s | Collector: %v", col.GetKey(), err)
	}
	return nil
}

func updateCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, col netproto.Collector, vrfID uint64) error {
	if err := deleteCollectorHandler(infraAPI, telemetryClient, col, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrCollectorDeleteDuringUpdate, "Collector: %s | Collector: %v", col.GetKey(), err))
	}
	if err := createCollectorHandler(infraAPI, telemetryClient, col, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrCollectorCreateDuringUpdate, "Collector: %s | Collector: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrCollectorCreateDuringUpdate, "Collector: %s | Collector: %v", col.GetKey(), err)
	}
	return nil
}

func deleteCollectorHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, col netproto.Collector, vrfID uint64) error {
	dstIP := col.Spec.Destination
	// Create the unique key for collector dest IP
	destKey := col.Spec.VrfName + "-" + dstIP
	sessionKeys := mirrorDestToIDMapping[destKey]

	if len(sessionKeys.mirrorKeys) > 0 {
		log.Error(errors.Wrapf(types.ErrCollectorStillReferenced, "Collector: %s | DstIP: %s | VrfName: %s still referenced by %s", col.GetKey(), dstIP, col.Spec.VrfName, strings.Join(sessionKeys.mirrorKeys, " ")))
		return errors.Wrapf(types.ErrCollectorStillReferenced, "Collector: %s | DstIP: %s | VrfName: %s still referenced by %s", col.GetKey(), dstIP, col.Spec.VrfName, strings.Join(sessionKeys.mirrorKeys, " "))
	}

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
	delete(mirrorDestToIDMapping, destKey)

	if err := infraAPI.Delete(col.Kind, col.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", col.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", col.GetKey(), err)
	}
	return nil
}

func convertMirrorSession(col netproto.Collector, destIP string, vrfID uint64) *halapi.MirrorSessionRequestMsg {
	return &halapi.MirrorSessionRequestMsg{
		Request: []*halapi.MirrorSessionSpec{
			{
				KeyOrHandle:  convertMirrorSessionKeyHandle(col.Status.Collector),
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				Destination: &halapi.MirrorSessionSpec_ErspanSpec{ // TODO Fix Destination when more than one collector per MirrorSession is supported
					ErspanSpec: &halapi.ERSpanSpec{
						DestIp: utils.ConvertIPAddresses(destIP)[0],
						SpanId: uint32(col.Status.Collector),
					},
				},
			},
		},
	}
}
