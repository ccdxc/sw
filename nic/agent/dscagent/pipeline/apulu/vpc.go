// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleVPC handles crud operations on vrf
func HandleVPC(infraAPI types.InfraAPI, client halapi.VPCSvcClient, oper types.Operation, vrf netproto.Vrf) error {
	switch oper {
	case types.Create:
		return createVPCHandler(infraAPI, client, vrf)
	case types.Update:
		return updateVPCHandler(infraAPI, client, vrf)
	case types.Delete:
		return deleteVPCHandler(infraAPI, client, vrf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, vrf netproto.Vrf) error {
	vpcReq := convertVrfToVPC(vrf)
	resp, err := client.VPCCreate(context.Background(), vpcReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func updateVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, vrf netproto.Vrf) error {
	vpcReq := convertVrfToVPC(vrf)
	resp, err := client.VPCUpdate(context.Background(), vpcReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "VPC: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func deleteVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, vrf netproto.Vrf) error {
	vpcDelReq := &halapi.VPCDeleteRequest{
		Id: utils.ConvertID64(vrf.Status.VrfID),
	}

	resp, err := client.VPCDelete(context.Background(), vpcDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("VPC: %s", vrf.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(vrf.Kind, vrf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func convertVrfToVPC(vrf netproto.Vrf) *halapi.VPCRequest {
	var vpcType halapi.VPCType
	if strings.ToLower(vrf.Spec.VrfType) == "infra" {
		vpcType = halapi.VPCType_VPC_TYPE_UNDERLAY
	} else {
		vpcType = halapi.VPCType_VPC_TYPE_TENANT
	}

	return &halapi.VPCRequest{
		BatchCtxt: nil,
		Request: []*halapi.VPCSpec{
			{
				Id:               utils.ConvertID64(vrf.Status.VrfID)[0],
				Type:             vpcType,
				V4RouteTableId:   utils.ConvertID32(vrf.Spec.V4RouteTableID)[0],
				V6RouteTableId:   utils.ConvertID32(vrf.Spec.V6RouteTableID)[0],
				VirtualRouterMac: utils.MacStrtoUint64(vrf.Spec.RouterMAC),
				FabricEncap: &halapi.Encap{
					Type: halapi.EncapType_ENCAP_TYPE_VXLAN,
					Value: &halapi.EncapVal{
						Val: &halapi.EncapVal_Vnid{
							Vnid: vrf.Spec.VxLANVNI,
						},
					},
				},
			},
		},
	}
}
