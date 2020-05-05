// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"net"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleInterface handles crud operations on interface
func HandleInterface(infraAPI types.InfraAPI, client halapi.IfSvcClient, subnetcl halapi.SubnetSvcClient, oper types.Operation, intf netproto.Interface) error {
	switch oper {
	case types.Create:
		return createInterfaceHandler(infraAPI, client, subnetcl, intf)
	case types.Update:
		return updateInterfaceHandler(infraAPI, client, subnetcl, intf)
	case types.Delete:
		return deleteInterfaceHandler(infraAPI, client, subnetcl, intf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, subnetcl halapi.SubnetSvcClient, intf netproto.Interface) error {
	intfReq, err := convertInterface(infraAPI, intf)
	if err != nil {
		log.Errorf("Interface: %s convert failed | Err: %v", intf.GetKey(), err)
		return errors.Wrapf(types.ErrBadRequest, "Interface: %s convert failed | Err: %v", intf.GetKey(), err)
	}
	uid, _ := uuid.FromBytes(intfReq.Request[0].Id)
	log.Infof("Creating Inteface [%v] UUID [%v][%v]", intf.GetKey(), uid.String(), intfReq.Request[0].Id)
	resp, err := client.InterfaceCreate(context.Background(), intfReq)
	if err != nil {
		log.Errorf("Interface: %s create failed | Err: %v", intf.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, "Interface: %s create failed | Err: %v", intf.GetKey(), err)
	}
	if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("Interface: %s Create failed  | Status: %s", intf.GetKey(), resp.ApiStatus)
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s Create failed | Status: %s", intf.GetKey(), resp.ApiStatus)
	}
	log.Infof("Inteface: %s create | Status: %s | Resp: %v", intf.GetKey(), resp.ApiStatus, resp.Response)

	dat, _ := intf.Marshal()
	if intf.Spec.Type == netproto.InterfaceSpec_LOOPBACK.String() {
		cfg := infraAPI.GetConfig()
		log.Infof("updating loopback TEP IP to [%s]", intf.Spec.IPAddress)
		ip, _, err := net.ParseCIDR(intf.Spec.IPAddress)
		if err != nil {
			log.Errorf("could not parse loopback IP (%s)", err)
		} else {
			cfg.LoopbackIP = ip.String()
		}
		infraAPI.StoreConfig(cfg)
	}
	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func updateInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, subnetcl halapi.SubnetSvcClient, intf netproto.Interface) error {
	intfReq, err := convertInterface(infraAPI, intf)
	if err != nil {
		log.Errorf("Interface: %s convert failed | Err: %v", intf.GetKey(), err)
		return errors.Wrapf(types.ErrBadRequest, "Interface: %s convert failed | Err: %v", intf.GetKey(), err)
	}

	// Check if there is a subnet association and update the subnet accordingly
	oldIntf := netproto.Interface{
		TypeMeta: api.TypeMeta{
			Kind: "Interface",
		},
		ObjectMeta: api.ObjectMeta{
			Name:      intf.Name,
			Namespace: intf.Namespace,
			Tenant:    intf.Tenant,
		},
	}

	o, err := infraAPI.Read(intf.Kind, oldIntf.GetKey())
	if err != nil {
		log.Errorf("Interface: %s not found during update | Err: %s", intf.GetKey(), err)
		return errors.Wrapf(types.ErrObjNotFound, "Interface: %s not found during update | Err: %s", intf.GetKey(), err)
	}
	err = oldIntf.Unmarshal(o)
	if err != nil {
		log.Errorf("Interface: %s could not unmarshal | Err: %s", intf.GetKey(), err)
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s could not unmarshal | Err: %s", intf.GetKey(), err)
	}

	uid, err := uuid.FromString(intf.UUID)
	if err != nil {
		log.Errorf("Interface: %s could not get UUID [%v] | Err: %s", intf.GetKey(), intf.UUID, err)
		return errors.Wrapf(types.ErrBadRequest, "Interface: %s could not get UUID [%v] | Err: %s", intf.GetKey(), intf.UUID, err)
	}

	if intf.Spec.Type == netproto.InterfaceSpec_HOST_PF.String() && (oldIntf.Spec.VrfName != intf.Spec.VrfName || oldIntf.Spec.Network != intf.Spec.Network) {
		log.Infof("Interface: %v mapping changed [%v/%v] -> [%v/%v]",
			intf.UUID, oldIntf.Spec.VrfName, oldIntf.Spec.Network,
			intf.Spec.VrfName, intf.Spec.Network)
		updateSubnet := func(tenant, netw string, uid []byte) error {
			log.Infof("Subnet: %v/%v attach interface %v | begin", tenant, netw, string(uid))
			nw := netproto.Network{
				TypeMeta: api.TypeMeta{
					Kind: "Network",
				},
				ObjectMeta: api.ObjectMeta{
					Name:      netw,
					Namespace: "default",
					Tenant:    tenant,
				},
			}
			s, err := infraAPI.Read("Network", nw.GetKey())
			if err != nil {
				log.Errorf("Network: %s not found during update | Err: %s", nw.GetKey(), err)
				if uid == nil {
					// in case of subnet being detached from host-pf, if subnet lookup fails, it is harmless
					// because the subnet must have been already cleared up, don't return an error
					return nil
				}
				return errors.Wrapf(types.ErrObjNotFound, "Network: %s not found during update | Err: %s", nw.GetKey(), err)
			}
			err = nw.Unmarshal(s)
			if err != nil {
				log.Errorf("Network: %s could not unmarshal | Err: %s", nw.GetKey(), err)
				return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Network: %s could not unmarshal | Err: %s", nw.GetKey(), err)
			}
			updsubnet, err := convertNetworkToSubnet(infraAPI, nw, nil)
			if err != nil {
				log.Errorf("Network: %s could not convert | Err: %s", nw.GetKey(), err)
				return errors.Wrapf(types.ErrDatapathHandling, "Network: %s could not Convert | Err: %s", nw.GetKey(), err)
			}
			updsubnet.Request[0].HostIf[0] = uid
			if uid != nil {
				// since boltDB is not updated for the interface yet, convertNetworkToSubnet doesn't populate the polciy IDs
				updsubnet.Request[0].IngV4SecurityPolicyId = utils.ConvertIDs(getPolicyUuid(nw.Spec.IngV4SecurityPolicies, true, nw, infraAPI)...)
				updsubnet.Request[0].EgV4SecurityPolicyId = utils.ConvertIDs(getPolicyUuid(nw.Spec.EgV4SecurityPolicies, true, nw, infraAPI)...)
			}

			resp, err := subnetcl.SubnetUpdate(context.TODO(), updsubnet)
			if err != nil {
				log.Errorf("Network: %s update failed | Err: %v", nw.GetKey(), err)
				return errors.Wrapf(types.ErrDatapathHandling, "Subnet: %s update failed | Err: %v", intf.GetKey(), err)
			}
			if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Errorf("Network: %s update failed  | Status: %s", nw.GetKey(), resp.ApiStatus)
				return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Subnet: %s update failed | Status: %s", intf.GetKey(), resp.ApiStatus)
			}
			log.Infof("Network: %s update | Err: %v | Status : %v | Resp: %v", nw.GetKey(), err, resp.ApiStatus, resp.Response)
			return nil
		}
		if oldIntf.Spec.Network != "" {
			if err = updateSubnet(oldIntf.Spec.VrfName, oldIntf.Spec.Network, nil); err != nil {
				return errors.Wrapf(types.ErrDatapathHandling, "Interface: %s updating old subnet| Err: %s", oldIntf.GetKey(), err)
			}
		}

		if intf.Spec.Network != "" {
			log.Infof("Updating Subnet [%v/%v] with Interface binding - [%s]", intf.Spec.VrfName, intf.Spec.Network, uid)
			if err = updateSubnet(intf.Spec.VrfName, intf.Spec.Network, uid.Bytes()); err != nil {
				return errors.Wrapf(types.ErrDatapathHandling, "Interface: %s updating new subnet| Err: %s", oldIntf.GetKey(), err)
			}
		}
	}
	if intf.Spec.Type == netproto.InterfaceSpec_LOOPBACK.String() {
		cfg := infraAPI.GetConfig()
		log.Infof("updating loopback TEP IP to [%s]", intf.Spec.IPAddress)
		ip, _, err := net.ParseCIDR(intf.Spec.IPAddress)
		if err != nil {
			log.Errorf("could not parse loopback IP (%s)", err)
			// reset the loopback IP
			cfg.LoopbackIP = ""
		} else {
			cfg.LoopbackIP = ip.String()
		}

		infraAPI.StoreConfig(cfg)
	}
	if intf.Spec.Type != netproto.InterfaceSpec_HOST_PF.String() {
		resp, err := client.InterfaceUpdate(context.Background(), intfReq)
		if err != nil {
			log.Errorf("Interface: %s update failed | Err: %v", intf.GetKey(), err)
			return errors.Wrapf(types.ErrDatapathHandling, "Interface: %s update failed | Err: %v", intf.GetKey(), err)
		}
		if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Errorf("Interface: %s update failed  | Status: %s", intf.GetKey(), resp.ApiStatus)
			return errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s update failed | Status: %s", intf.GetKey(), resp.ApiStatus)
		}
		log.Infof("Interface: %s update | Status: %s | Resp: %v", intf.GetKey(), resp.ApiStatus, resp.Response)

	}

	dat, _ := intf.Marshal()
	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func deleteInterfaceHandler(infraAPI types.InfraAPI, client halapi.IfSvcClient, subnetcl halapi.SubnetSvcClient, intf netproto.Interface) error {
	uid, err := uuid.FromString(intf.UUID)
	if err != nil {
		log.Errorf("Interface: %s uuid parse failed | [%v] | Err: %v", intf.GetKey(), intf.UUID, err)
		return err
	}
	intfDelReq := &halapi.InterfaceDeleteRequest{
		Id: [][]byte{uid.Bytes()},
	}

	resp, err := client.InterfaceDelete(context.Background(), intfDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("Interface: %s", intf.GetKey())); err != nil {
			return err
		}
	}
	if intf.Spec.Type == netproto.InterfaceSpec_LOOPBACK.String() {
		cfg := infraAPI.GetConfig()
		log.Infof("updating loopback TEP IP to []")
		cfg.LoopbackIP = ""
		infraAPI.StoreConfig(cfg)
	}
	log.Infof("Inteface: %s delete | Status: %s", intf.GetKey(), resp.ApiStatus)

	if err := infraAPI.Delete(intf.Kind, intf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func convertInterface(infraAPI types.InfraAPI, intf netproto.Interface) (*halapi.InterfaceRequest, error) {
	var ifStatus halapi.IfStatus

	uid, err := uuid.FromString(intf.UUID)
	if err != nil {
		log.Errorf("Interface: %s uuid parse failed | [%v] | Err: %v", intf.GetKey(), intf.UUID, err)
		return nil, err
	}

	if intf.Spec.AdminStatus == "UP" {
		ifStatus = halapi.IfStatus_IF_STATUS_UP
	} else {
		ifStatus = halapi.IfStatus_IF_STATUS_DOWN
	}

	var prefix *halapi.IPPrefix
	if intf.Spec.IPAddress != "" {
		prefix, err = utils.ConvertIPPrefix(intf.Spec.IPAddress)
		if err != nil {
			prefix = nil
		}
	}
	switch intf.Spec.Type {
	case netproto.InterfaceSpec_L3.String():
		portuid, err := uuid.FromString(intf.Status.InterfaceUUID)
		if err != nil {
			log.Errorf("failed to parse port UUID (%v)", err)
			return nil, err
		}

		vDat, err := infraAPI.List("Vrf")
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
			return nil, err
		}

		var vrf *netproto.Vrf
		for _, v := range vDat {
			var curVrf netproto.Vrf
			err = curVrf.Unmarshal(v)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", curVrf.GetKey(), err))
				continue
			}
			if curVrf.ObjectMeta.Name == intf.Spec.VrfName {
				vrf = &curVrf
				break
			}
		}

		if vrf == nil {
			log.Errorf("Failed to find vrf %v", intf.Spec.VrfName)
			return nil, fmt.Errorf("failed to find vrf")
		}

		vpcuid, err := uuid.FromString(vrf.ObjectMeta.UUID)
		if err != nil {
			log.Errorf("failed to parse port UUID (%v)", err)
			return nil, err
		}

		// TODO: no sub interface support yet
		return &halapi.InterfaceRequest{
			BatchCtxt: nil,
			Request: []*halapi.InterfaceSpec{
				{
					Id:          uid.Bytes(),
					Type:        halapi.IfType_IF_TYPE_L3,
					AdminStatus: ifStatus,
					Ifinfo: &halapi.InterfaceSpec_L3IfSpec{
						L3IfSpec: &halapi.L3IfSpec{
							VpcId:  vpcuid.Bytes(),
							Prefix: prefix,
							PortId: portuid.Bytes(),
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
		}, nil

	case netproto.InterfaceSpec_HOST_PF.String():
		// No Convert needed for HOST PF
		return nil, nil
	case netproto.InterfaceSpec_LOOPBACK.String():
		return &halapi.InterfaceRequest{
			BatchCtxt: nil,
			Request: []*halapi.InterfaceSpec{
				{
					Id:          uid.Bytes(),
					Type:        halapi.IfType_IF_TYPE_LOOPBACK,
					AdminStatus: ifStatus,
					Ifinfo: &halapi.InterfaceSpec_LoopbackIfSpec{
						LoopbackIfSpec: &halapi.LoopbackIfSpec{
							Prefix: prefix,
						},
					},
				},
			},
		}, nil

	}
	return nil, fmt.Errorf("unsupported type [%v]", intf.Spec.Type)
}
