// +build iris

package iris

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	irisUtils "github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleL2Segment handles crud operations on l2Seg
func HandleL2Segment(infraAPI types.InfraAPI, client halapi.L2SegmentClient, oper types.Operation, l2Seg netproto.Network, vrfID uint64, uplinkIDs []uint64) error {
	switch oper {
	case types.Create:
		return createL2SegmentHandler(infraAPI, client, l2Seg, vrfID, uplinkIDs)
	case types.Update:
		return updateL2SegmentHandler(infraAPI, client, l2Seg, vrfID, uplinkIDs)
	case types.Delete:
		return deleteL2SegmentHandler(infraAPI, client, l2Seg)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createL2SegmentHandler(infraAPI types.InfraAPI, client halapi.L2SegmentClient, l2Seg netproto.Network, vrfID uint64, uplinkIDs []uint64) error {
	l2SegReqMsg := convertL2Segment(l2Seg, vrfID, uplinkIDs)
	log.Infof("L2Seg Msg: %v", l2SegReqMsg.String())

	resp, err := client.L2SegmentCreate(context.Background(), l2SegReqMsg)
	if resp != nil {
		if err := irisUtils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", l2Seg.GetKind(), l2Seg.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := l2Seg.Marshal()

	if err := infraAPI.Store(l2Seg.Kind, l2Seg.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Network: %v", l2Seg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Network: %v", l2Seg.GetKey(), err)
	}
	return nil
}

func updateL2SegmentHandler(infraAPI types.InfraAPI, client halapi.L2SegmentClient, l2Seg netproto.Network, vrfID uint64, uplinkIDs []uint64) error {
	l2SegReqMsg := convertL2Segment(l2Seg, vrfID, uplinkIDs)
	log.Infof("L2Seg Msg: %v", l2SegReqMsg.String())

	resp, err := client.L2SegmentUpdate(context.Background(), l2SegReqMsg)
	if resp != nil {
		if err := irisUtils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", l2Seg.GetKind(), l2Seg.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := l2Seg.Marshal()

	if err := infraAPI.Store(l2Seg.Kind, l2Seg.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Network: %s | Network: %v", l2Seg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Network: %s | Network: %v", l2Seg.GetKey(), err)
	}
	return nil
}

func deleteL2SegmentHandler(infraAPI types.InfraAPI, client halapi.L2SegmentClient, l2Seg netproto.Network) error {

	l2SegDelReq := &halapi.L2SegmentDeleteRequestMsg{
		Request: []*halapi.L2SegmentDeleteRequest{
			{
				KeyOrHandle: convertL2SegKeyHandle(l2Seg.Status.NetworkID),
			},
		},
	}

	log.Infof("L2Seg Msg: %v", l2SegDelReq.String())

	resp, err := client.L2SegmentDelete(context.Background(), l2SegDelReq)
	if resp != nil {
		if err := irisUtils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("Network: %s", l2Seg.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(l2Seg.Kind, l2Seg.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", l2Seg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", l2Seg.GetKey(), err)
	}
	return nil
}

func convertL2Segment(l2Seg netproto.Network, vrfID uint64, uplinkIDs []uint64) *halapi.L2SegmentRequestMsg {
	var uplinkIfIndices []uint64
	for _, u := range uplinkIDs {
		uplinkIfIndices = append(uplinkIfIndices, utils.EthIfIndexToUplinkIfIndex(u))
	}
	ifKeyHandles := convertIfKeyHandles(l2Seg.Spec.VlanID, uplinkIfIndices...)

	return &halapi.L2SegmentRequestMsg{
		Request: []*halapi.L2SegmentSpec{
			{
				KeyOrHandle:    convertL2SegKeyHandle(l2Seg.Status.NetworkID),
				McastFwdPolicy: halapi.MulticastFwdPolicy_MULTICAST_FWD_POLICY_NONE,
				BcastFwdPolicy: halapi.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
				WireEncap: &halapi.EncapInfo{
					EncapType:  halapi.EncapType_ENCAP_TYPE_DOT1Q,
					EncapValue: l2Seg.Spec.VlanID,
				},
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				IfKeyHandle:  ifKeyHandles,
			},
		},
	}
}
