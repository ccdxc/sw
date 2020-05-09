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

// Collector represents a single collector with respect to HAL
type Collector struct {
	Name         string
	Destination  string
	PacketSize   uint32
	Gateway      string
	Type         string
	StripVlanHdr bool
	SessionID    uint64
	SpanID       uint32
}

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
	mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
	if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, col.Name, mgmtIP, col.Destination, col.Gateway, true); err != nil {
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

	return nil
}

func deleteColHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, col Collector, vrfID uint64) error {
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
	if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, col.Name, col.Destination, true); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.Name, err))
		return errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "Collector: %s | Err: %v", col.Name, err)
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
						SpanId:      col.SpanID,
						Type:        halapi.ERSpanType(halapi.ERSpanType_value[strings.ToUpper(col.Type)]),
						VlanStripEn: col.StripVlanHdr,
					},
				},
			},
		},
	}
}
