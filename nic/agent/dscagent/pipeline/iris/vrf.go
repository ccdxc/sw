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

// HandleVrf handles crud operations on vrf
func HandleVrf(infraAPI types.InfraAPI, client halapi.VrfClient, oper types.Operation, vrf netproto.Vrf) error {
	switch oper {
	case types.Create:
		return createVrfHandler(infraAPI, client, vrf)
	case types.Update:
		return updateVrfHandler(infraAPI, client, vrf)
	case types.Delete:
		return deleteVrfHandler(infraAPI, client, vrf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createVrfHandler(infraAPI types.InfraAPI, client halapi.VrfClient, vrf netproto.Vrf) error {
	vrfReqMsg := convertVrf(vrf)
	resp, err := client.VrfCreate(context.Background(), vrfReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func updateVrfHandler(infraAPI types.InfraAPI, client halapi.VrfClient, vrf netproto.Vrf) error {
	vrfReqMsg := convertVrf(vrf)
	resp, err := client.VrfUpdate(context.Background(), vrfReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Vrf: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func deleteVrfHandler(infraAPI types.InfraAPI, client halapi.VrfClient, vrf netproto.Vrf) error {
	vrfDelReq := &halapi.VrfDeleteRequestMsg{
		Request: []*halapi.VrfDeleteRequest{
			{
				KeyOrHandle: &halapi.VrfKeyHandle{
					KeyOrHandle: &halapi.VrfKeyHandle_VrfId{
						VrfId: vrf.Status.VrfID,
					},
				},
			},
		},
	}

	resp, err := client.VrfDelete(context.Background(), vrfDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("Vrf: %s", vrf.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(vrf.Kind, vrf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func convertVrf(vrf netproto.Vrf) *halapi.VrfRequestMsg {
	var vrfType halapi.VrfType
	if strings.ToLower(vrf.Spec.VrfType) == "infra" {
		vrfType = halapi.VrfType_VRF_TYPE_INFRA
	} else {
		vrfType = halapi.VrfType_VRF_TYPE_CUSTOMER
	}

	return &halapi.VrfRequestMsg{
		Request: []*halapi.VrfSpec{
			{
				KeyOrHandle: convertVrfKeyHandle(vrf.Status.VrfID),
				VrfType:     vrfType,
			},
		},
	}
}

func convertVrfKeyHandle(vrfID uint64) *halapi.VrfKeyHandle {
	return &halapi.VrfKeyHandle{
		KeyOrHandle: &halapi.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}
}

func convertL2SegKeyHandle(l2SegID uint64) *halapi.L2SegmentKeyHandle {
	return &halapi.L2SegmentKeyHandle{
		KeyOrHandle: &halapi.L2SegmentKeyHandle_SegmentId{
			SegmentId: l2SegID,
		},
	}
}
