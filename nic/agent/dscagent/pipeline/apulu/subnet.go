// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleSubnet handles crud operations on subnet TODO use SubnetClient here
func HandleSubnet(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	oper types.Operation, nw netproto.Network,
	vpcID uint64, uplinkIDs []uint64) error {
	switch oper {
	case types.Create:
		return createSubnetHandler(infraAPI, client, nw, vpcID, uplinkIDs)
	case types.Update:
		return updateSubnetHandler(infraAPI, client, nw, vpcID, uplinkIDs)
	case types.Delete:
		return deleteSubnetHandler(infraAPI, client, nw)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {
	subnetReq := convertNetworkToSubnet(nw, vpcID, uplinkIDs)
	log.Infof("SubnetReq: %+v", subnetReq)
	resp, err := client.SubnetCreate(context.Background(), subnetReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", nw.GetKind(), nw.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := nw.Marshal()

	if err := infraAPI.Store(nw.Kind, nw.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err)
	}
	return nil
}

func updateSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {
	subnetReq := convertNetworkToSubnet(nw, vpcID, uplinkIDs)
	resp, err := client.SubnetUpdate(context.Background(), subnetReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", nw.GetKind(), nw.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := nw.Marshal()

	if err := infraAPI.Store(nw.Kind, nw.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Subnet: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Subnet: %s | Err: %v", nw.GetKey(), err)
	}
	return nil
}

func deleteSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	nw netproto.Network) error {
	subnetDelReq := &halapi.SubnetDeleteRequest{
		Id: utils.ConvertID64(nw.Status.NetworkID),
	}

	resp, err := client.SubnetDelete(context.Background(), subnetDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("Subnet: %s", nw.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(nw.Kind, nw.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", nw.GetKey(), err)
	}
	return nil
}

func convertNetworkToSubnet(nw netproto.Network, vpcID uint64, uplinkIDs []uint64) *halapi.SubnetRequest {
	var v6Prefix *halapi.IPv6Prefix
	var v4Prefix *halapi.IPv4Prefix
	var v4VrIP uint32
	var v6VrIP []byte

	v6Prefix = nil
	v4Prefix = nil
	v4VrIP = 0
	v6VrIP = nil
	if nw.Spec.V6Address != nil {
		v6Prefix = &halapi.IPv6Prefix{
			Len:  nw.Spec.V6Address[0].PrefixLen,
			Addr: nw.Spec.V6Address[0].Address.V6Address,
		}
		v6VrIP = nw.Spec.V6Address[0].Address.V6Address
	}
	if nw.Spec.V4Address != nil {
		v4Prefix = &halapi.IPv4Prefix{
			Len:  nw.Spec.V4Address[0].PrefixLen,
			Addr: nw.Spec.V4Address[0].Address.V4Address,
		}
		v4VrIP = nw.Spec.V4Address[0].Address.V4Address
	}

	return &halapi.SubnetRequest{
		BatchCtxt: nil,
		Request: []*halapi.SubnetSpec{
			{
				Id:                  utils.ConvertID64(nw.Status.NetworkID)[0],
				VPCId:               utils.ConvertID64(vpcID)[0],
				IPv4VirtualRouterIP: v4VrIP,
				IPv6VirtualRouterIP: v6VrIP,
				VirtualRouterMac:    utils.MacStrtoUint64(nw.Spec.RouterMAC),
				FabricEncap: &halapi.Encap{
					Type: halapi.EncapType_ENCAP_TYPE_VXLAN,
					Value: &halapi.EncapVal{
						Val: &halapi.EncapVal_Vnid{
							Vnid: nw.Spec.VxLANVNI,
						},
					},
				},
				V4Prefix:              v4Prefix,
				V6Prefix:              v6Prefix,
				V4RouteTableId:        utils.ConvertID32(nw.Spec.V4RouteTableID)[0],
				V6RouteTableId:        utils.ConvertID32(nw.Spec.V6RouteTableID)[0],
				IngV4SecurityPolicyId: utils.ConvertID32(nw.Spec.IngV4SecurityPolicyID...),
				EgV4SecurityPolicyId:  utils.ConvertID32(nw.Spec.EgV4SecurityPolicyID...),
				IngV6SecurityPolicyId: utils.ConvertID32(nw.Spec.IngV6SecurityPolicyID...),
				EgV6SecurityPolicyId:  utils.ConvertID32(nw.Spec.EgV6SecurityPolicyID...),
			},
		},
	}
}
