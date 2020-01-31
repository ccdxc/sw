// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleSubnet handles crud operations on subnet TODO use SubnetClient here
func HandleSubnet(infraAPI types.InfraAPI, client halapi.SubnetSvcClient, msc msTypes.EvpnSvcClient,
	oper types.Operation, nw netproto.Network,
	vpcID uint64, uplinkIDs []uint64) error {
	switch oper {
	case types.Create:
		return createSubnetHandler(infraAPI, client, msc, nw, vpcID, uplinkIDs)
	case types.Update:
		return updateSubnetHandler(infraAPI, client, msc, nw, vpcID, uplinkIDs)
	case types.Delete:
		return deleteSubnetHandler(infraAPI, client, msc, nw)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	msc msTypes.EvpnSvcClient, nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {
	subnetReq := convertNetworkToSubnet(nw, vpcID, uplinkIDs)
	log.Infof("SubnetReq: %+v", subnetReq)

	ctx := context.TODO()
	uid, err := uuid.FromString(nw.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	resp, err := client.SubnetCreate(context.Background(), subnetReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", nw.GetKind(), nw.GetKey())); err != nil {
			return err
		}
	}

	eviReq := msTypes.EvpnEviRequest{
		Request: []*msTypes.EvpnEviSpec{
			{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				AutoRD:   msTypes.EvpnCfg_EVPN_CFG_AUTO,
				RTType:   msTypes.EvpnRtType_EVPN_RT_NONE,
				Encap:    msTypes.EvpnEncaps_EVPN_ENCAP_VXLAN,
				EVIId:    uint32(nw.Status.NetworkID),
			},
		},
	}

	evresp, err := msc.EvpnEviSpecCreate(ctx, &eviReq)
	if err != nil {
		log.Errorf("failed to create EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	log.Infof("got EVPN EVI create response [%v/%v]", evresp.ApiStatus, evresp.Response)

	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}
	// Create the RT import and export
	eviRtReq := &msTypes.EvpnEviRtRequest{}
	for _, rt := range nw.Spec.RouteImportExport.ExportRTs {
		evirt := msTypes.EvpnEviRtSpec{
			Id:       uid.Bytes(),
			SubnetId: uid.Bytes(),
			RT:       utils.RDToBytes(rt),
			RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
			EVIId:    uint32(nw.Status.NetworkID),
		}
		eviRtReq.Request = append(eviRtReq.Request, &evirt)
	}
	for _, rt := range nw.Spec.RouteImportExport.ImportRTs {
		evirt := msTypes.EvpnEviRtSpec{
			Id:       uid.Bytes(),
			SubnetId: uid.Bytes(),
			RT:       utils.RDToBytes(rt),
			RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
			EVIId:    uint32(nw.Status.NetworkID),
		}
		eviRtReq.Request = append(eviRtReq.Request, &evirt)
		log.Infof("add EVI RT")
	}
	evrtresp, err := msc.EvpnEviRtSpecCreate(ctx, eviRtReq)
	if err != nil {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet create [%v/%v] got response [%v/%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus, evrtresp.Response)

	dat, _ := nw.Marshal()

	if err := infraAPI.Store(nw.Kind, nw.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err)
	}
	return nil
}

func updateSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	msc msTypes.EvpnSvcClient, nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {
	subnetReq := convertNetworkToSubnet(nw, vpcID, uplinkIDs)
	resp, err := client.SubnetUpdate(context.Background(), subnetReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", nw.GetKind(), nw.GetKey())); err != nil {
			return err
		}
	}

	ctx := context.TODO()
	curNwB, err := infraAPI.Read(nw.Kind, nw.GetKey())
	if err != nil {
		return err
	}
	curNw := netproto.Network{}
	curNw.Unmarshal(curNwB)

	uid, err := uuid.FromString(nw.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	eviReq := msTypes.EvpnEviRequest{
		Request: []*msTypes.EvpnEviSpec{
			{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				AutoRD:   msTypes.EvpnCfg_EVPN_CFG_AUTO,
				RTType:   msTypes.EvpnRtType_EVPN_RT_NONE,
				Encap:    msTypes.EvpnEncaps_EVPN_ENCAP_VXLAN,
				EVIId:    uint32(nw.Status.NetworkID),
			},
		},
	}

	evresp, err := msc.EvpnEviSpecUpdate(ctx, &eviReq)
	if err != nil {
		log.Errorf("failed to update EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	log.Infof("got EVPN EVI update response [%v/%v]", evresp.ApiStatus, evresp.Response)

	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to update EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}

	var rtDelReq, rtAddReq msTypes.EvpnEviRtRequest
	for _, r := range curNw.Spec.RouteImportExport.ExportRTs {
		found := false
		for _, r1 := range nw.Spec.RouteImportExport.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RDToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
				EVIId:    uint32(nw.Status.NetworkID),
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
			log.Infof("add EVI RT export [%v]", ert)
		}
	}
	for _, r := range nw.Spec.RouteImportExport.ExportRTs {
		found := false
		for _, r1 := range curNw.Spec.RouteImportExport.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RDToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
				EVIId:    uint32(nw.Status.NetworkID),
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
			log.Infof("del EVI RT export [%v]", ert)
		}
	}
	for _, r := range curNw.Spec.RouteImportExport.ImportRTs {
		found := false
		for _, r1 := range nw.Spec.RouteImportExport.ImportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RDToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
				EVIId:    uint32(nw.Status.NetworkID),
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
			log.Infof("add EVI RT import [%v]", ert)
		}
	}
	for _, r := range nw.Spec.RouteImportExport.ImportRTs {
		found := false
		for _, r1 := range curNw.Spec.RouteImportExport.ImportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RDToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
				EVIId:    uint32(nw.Status.NetworkID),
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
			log.Infof("del EVI RT import [%v]", ert)
		}
	}

	evrtresp, err := msc.EvpnEviRtSpecCreate(ctx, &rtAddReq)
	if err != nil {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet update [%v/%v] RT create  got response [%v/%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus, evrtresp.Response)

	evrtresp, err = msc.EvpnEviRtSpecCreate(ctx, &rtDelReq)
	if err != nil {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet update [%v/%v] delete RT got response [%v/%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus, evrtresp.Response)

	dat, _ := nw.Marshal()

	if err := infraAPI.Store(nw.Kind, nw.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Subnet: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Subnet: %s | Err: %v", nw.GetKey(), err)
	}
	return nil
}

func deleteSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	msc msTypes.EvpnSvcClient, nw netproto.Network) error {

	ctx := context.TODO()
	curNwB, err := infraAPI.Read(nw.Kind, nw.GetKey())
	if err != nil {
		return err
	}
	curNw := netproto.Network{}
	curNw.Unmarshal(curNwB)
	uid, err := uuid.FromString(curNw.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	eviRtReq := &msTypes.EvpnEviRtRequest{}
	for _, rt := range curNw.Spec.RouteImportExport.ExportRTs {
		evirt := msTypes.EvpnEviRtSpec{
			Id:       uid.Bytes(),
			SubnetId: uid.Bytes(),
			RT:       utils.RDToBytes(rt),
			RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
			EVIId:    uint32(nw.Status.NetworkID),
		}
		eviRtReq.Request = append(eviRtReq.Request, &evirt)
		log.Infof("del EVI RT export [%v]", evirt)
	}
	for _, rt := range curNw.Spec.RouteImportExport.ImportRTs {
		evirt := msTypes.EvpnEviRtSpec{
			Id:       uid.Bytes(),
			SubnetId: uid.Bytes(),
			RT:       utils.RDToBytes(rt),
			RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
			EVIId:    uint32(nw.Status.NetworkID),
		}
		eviRtReq.Request = append(eviRtReq.Request, &evirt)
		log.Infof("del EVI RT import [%v]", evirt)
	}
	evrtresp, err := msc.EvpnEviRtSpecDelete(ctx, eviRtReq)
	if err != nil {
		log.Errorf("failed to delete EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet delete RT [%v/%v] got response [%v/%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus, evrtresp.Response)

	evireq := msTypes.EvpnEviRequest{
		Request: []*msTypes.EvpnEviSpec{
			{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				AutoRD:   msTypes.EvpnCfg_EVPN_CFG_AUTO,
				RTType:   msTypes.EvpnRtType_EVPN_RT_NONE,
				Encap:    msTypes.EvpnEncaps_EVPN_ENCAP_VXLAN,
				EVIId:    uint32(curNw.Status.NetworkID),
			},
		},
	}
	evresp, err := msc.EvpnEviSpecDelete(ctx, &evireq)
	if err != nil {
		log.Errorf("failed to delete EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	log.Infof("got EVPN EVI delete response [%v/%v]", evresp.ApiStatus, evresp.Response)

	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}

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
