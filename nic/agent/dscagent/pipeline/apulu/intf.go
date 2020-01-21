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

// HandleInterface handles crud operations on interface
func HandleInterface(infraAPI types.InfraAPI, client halapi.IfSvcClient, oper types.Operation, intf netproto.Interface) error {
	switch oper {
	case types.Create:
		return createInterfaceHandler(infraAPI, client, intf)
	case types.Update:
		return updateInterfaceHandler(infraAPI, client, intf)
	case types.Delete:
		return deleteInterfaceHandler(infraAPI, client, intf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, intf netproto.Interface) error {
	intfReq := convertInterface(intf)
	resp, err := client.InterfaceCreate(context.Background(), intfReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", intf.GetKind(), intf.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := intf.Marshal()

	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func updateInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, intf netproto.Interface) error {
	intfReq := convertInterface(intf)
	resp, err := client.InterfaceUpdate(context.Background(), intfReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", intf.GetKind(), intf.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := intf.Marshal()

	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func deleteInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, intf netproto.Interface) error {
	// TODO: use uuid here
	intfDelReq := &halapi.InterfaceDeleteRequest{
		Id: utils.ConvertID64(intf.Status.InterfaceID),
	}

	resp, err := client.InterfaceDelete(context.Background(), intfDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("Interface: %s", intf.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(intf.Kind, intf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func convertInterface(intf netproto.Interface) *halapi.InterfaceRequest {
	var ifStatus halapi.IfStatus

	if intf.Spec.AdminStatus == "UP" {
		ifStatus = halapi.IfStatus_IF_STATUS_UP
	} else {
		ifStatus = halapi.IfStatus_IF_STATUS_DOWN
	}

	switch intf.Spec.Type {
	case "L3":
		// TODO: no sub interface support yet
		return &halapi.InterfaceRequest{
			BatchCtxt: nil,
			Request: []*halapi.InterfaceSpec{
				{
					Id:          utils.ConvertID64(intf.Status.InterfaceID)[0],
					Type:        halapi.IfType_IF_TYPE_L3,
					AdminStatus: ifStatus,
					Ifinfo: &halapi.InterfaceSpec_L3IfSpec{
						L3IfSpec: &halapi.L3IfSpec{
							VpcId:      utils.ConvertID64(0)[0], // TODO get the object references sorted out here
							Prefix:     nil,
							EthIfIndex: 0,
							Encap: &halapi.Encap{
								Type: halapi.EncapType_ENCAP_TYPE_NONE,
								Value: &halapi.EncapVal{
									Val: &halapi.EncapVal_VlanId{
										VlanId: 0,
									},
								},
							},
							MACAddress: 0,
						},
					},
				},
			},
		}
		/*
			case "LOOPBACK":
				return &halapi.InterfaceRequest{
					BatchCtxt: nil,
					Request: []*halapi.InterfaceSpec{
						{
							Id:          uint32(intf.Status.InterfaceID),
							Type:        halapi.IfType_IF_TYPE_LOOPBACK,
							AdminStatus: intfStatus,
							Ifinfo: &halapi.InterfaceSpec_LoopbackIfSpec{
								LoopbackIfSpec: &halapi.LoopbackIfSpec{
									Prefix: nil,
								},
							},
						},
					},
				}
		*/
	}
	return nil
}
