// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleVPC handles crud operations on vrf TODO use VPCClient here
func HandleVPC(infraAPI types.InfraAPI, client halapi.VPCSvcClient, msc msTypes.EvpnSvcClient, oper types.Operation, vrf netproto.Vrf) error {
	switch oper {
	case types.Create:
		return createVPCHandler(infraAPI, client, msc, vrf)
	case types.Update:
		return updateVPCHandler(infraAPI, client, msc, vrf)
	case types.Delete:
		return deleteVPCHandler(infraAPI, client, msc, vrf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, msc msTypes.EvpnSvcClient, vrf netproto.Vrf) error {
	vpcReq, err := convertVrfToVPC(infraAPI, vrf)
	if err != nil {
		log.Errorf("Vrf: %s could not convert to VPC: Err: %s", vrf.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, fmt.Sprintf("Vrf: %s | Err: %s", vrf.GetKey(), err))
	}
	resp, err := client.VPCCreate(context.Background(), vpcReq)
	log.Infof("createVPCHandler Response: %v. Err: %v", resp, err)
	if err != nil {
		log.Errorf("Vrf: %s could not create datapath vpc: Err: %s", vrf.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, fmt.Sprintf("Vrf: %s | Err: %s", vrf.GetKey(), err))
	}
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Vrf: %s Create Failed for", vrf.GetKey())); err != nil {
			return err
		}
	}
	log.Infof("Vrf: %s Create returned | Status: %s | Response: %+v", vrf.GetKey(), resp.ApiStatus, resp.Response)

	if vrf.Spec.VrfType != "CUSTOMER" {
		dat, _ := vrf.Marshal()
		if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
			return errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err)
		}
		return nil
	}

	uid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("Vrf: %s failed to parse UUID %s | Err: %v", vrf.GetKey(), vrf.UUID, err)
		return err
	}

	// Create Metaswitch related objects
	evrfReq := msTypes.EvpnIpVrfRequest{
		Request: []*msTypes.EvpnIpVrfSpec{
			{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				VNI:    vrf.Spec.VxLANVNI,
				AutoRD: msTypes.EvpnCfg_EVPN_CFG_AUTO,
			},
		},
	}
	log.Infof("Creating IP Vrf EVI [%+v]", evrfReq.Request[0])
	ctx := context.TODO()
	eVrfResp, err := msc.EvpnIpVrfSpecCreate(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF | Err: %v", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF | Status: %v", vrf.GetKey(), eVrfResp.ApiStatus)
	}
	log.Infof("VRF Config: %s: got response [%v/%v]", vrf.Name, eVrfResp.ApiStatus, eVrfResp.Response)

	if vrf.Spec.RouteImportExport != nil {
		var rts []*rtImpExp
		isEqual := func(a, b *netproto.RouteDistinguisher) bool {
			return a.Type == b.Type && a.AssignedValue == b.AssignedValue && a.AdminValue == b.AdminValue
		}
	outerLoop1:
		for _, rt := range vrf.Spec.RouteImportExport.ExportRTs {
			for _, r := range rts {
				if isEqual(r.rt, rt) {
					r.exp = true
					continue outerLoop1
				}
			}
			rts = append(rts, &rtImpExp{rt: rt, exp: true})
		}

	outerLoop2:
		for _, rt := range vrf.Spec.RouteImportExport.ImportRTs {
			for _, r := range rts {
				if isEqual(r.rt, rt) {
					r.imp = true
					continue outerLoop2
				}
			}
			rts = append(rts, &rtImpExp{rt: rt, imp: true})
		}

		rtReq := msTypes.EvpnIpVrfRtRequest{}
		for _, rt := range rts {
			var rtype msTypes.EvpnRtType
			switch {
			case rt.imp && rt.exp:
				rtype = msTypes.EvpnRtType_EVPN_RT_IMPORT_EXPORT
			case rt.exp:
				rtype = msTypes.EvpnRtType_EVPN_RT_EXPORT
			case rt.imp:
				rtype = msTypes.EvpnRtType_EVPN_RT_IMPORT
			}
			evirt := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RDToBytes(rt.rt),
				RTType: rtype,
			}
			log.Infof("add Evi VRF RT [%+v]", evirt)
			rtReq.Request = append(rtReq.Request, &evirt)
		}
		eVrfRTResp, err := msc.EvpnIpVrfRtSpecCreate(ctx, &rtReq)
		if err != nil {
			log.Infof("EVPN VRF RT Spec Create received resp (%v)[%+v]", err, eVrfRTResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF RT | Err: %v", vrf.GetKey(), err)
		}
		if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF RT Spec Create received resp (%v)[%v, %v] req[%+v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response, rtReq)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF RT  | status: %s", vrf.GetKey(), eVrfRTResp.ApiStatus)
		}
		log.Infof("VRF RT Config: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
	}

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func updateVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, msc msTypes.EvpnSvcClient, vrf netproto.Vrf) error {
	curVrfB, err := infraAPI.Read(vrf.Kind, vrf.GetKey())
	if err != nil {
		return err
	}
	vpcReq, err := convertVrfToVPC(infraAPI, vrf)
	if err != nil {
		log.Errorf("Vrf: %s could not convert to VPC: Err: %v", vrf.GetKey(), err)
		return err
	}
	resp, err := client.VPCUpdate(context.Background(), vpcReq)
	if err != nil {
		log.Errorf("Vrf: %s could not update datapath: Err: %s", vrf.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, fmt.Sprintf("Vrf: %s | Err: %s", vrf.GetKey(), err))
	}
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Vrf: %s Update Failed ", vrf.GetKey())); err != nil {
			return err
		}
	}
	ctx := context.TODO()
	curVrf := netproto.Vrf{}
	curVrf.Unmarshal(curVrfB)
	uid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	// only the VNI or RTs could change in update
	if curVrf.UUID != vrf.UUID {
		log.Errorf("UUID change on update [%v][%v/%v]", vrf.Name, curVrf.UUID, vrf.UUID)
		return errors.Wrapf(types.ErrBadRequest, "Vrf: %s UUID change on update", vrf.GetKey())
	}
	if curVrf.Spec.VxLANVNI != vrf.Spec.VxLANVNI {
		evrfReq := msTypes.EvpnIpVrfRequest{
			Request: []*msTypes.EvpnIpVrfSpec{
				{
					Id:     uid.Bytes(),
					VPCId:  uid.Bytes(),
					VNI:    vrf.Spec.VxLANVNI,
					AutoRD: msTypes.EvpnCfg_EVPN_CFG_AUTO,
				},
			},
		}

		eVrfResp, err := msc.EvpnIpVrfSpecUpdate(ctx, &evrfReq)
		if err != nil {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF | Err: %v", vrf.GetKey(), err)
		}
		if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s  Configuring EVPN VRF | status(%s)", vrf.GetKey(), eVrfResp.ApiStatus)
		}
	}

	var rtDelReq, rtAddReq msTypes.EvpnIpVrfRtRequest
	curRts := curVrf.Spec.RouteImportExport
	newRts := vrf.Spec.RouteImportExport
	if curRts == nil {
		curRts = &netproto.RDSpec{}
	}
	if newRts == nil {
		newRts = &netproto.RDSpec{}
	}
	for _, r := range curRts.ExportRTs {
		found := false
		for _, r1 := range newRts.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RDToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_EXPORT,
			}
			log.Infof("adding RT to req [%v]", ert)
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range newRts.ExportRTs {
		found := false
		for _, r1 := range curRts.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RDToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_EXPORT,
			}
			log.Infof("adding RT to req [%v]", ert)
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}

	for _, r := range curRts.ImportRTs {
		found := false
		for _, r1 := range newRts.ImportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RDToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_IMPORT,
			}
			log.Infof("adding RT to req [%v]", ert)
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range newRts.ImportRTs {
		found := false
		for _, r1 := range curRts.ImportRTs {
			if r == r1 {

				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RDToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_IMPORT,
			}
			log.Infof("adding RT to req [%v]", ert)
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}
	eVrfRTResp, err := msc.EvpnIpVrfRtSpecCreate(ctx, &rtAddReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF RT | Err: %v", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s  Configuring EVPN VRF RT  | Status: %s", vrf.GetKey(), eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Config: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	eVrfRTResp, err = msc.EvpnIpVrfRtSpecDelete(ctx, &rtDelReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting  EVPN VRF RT| Err: %v", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF RT | Status: %s", vrf.GetKey(), eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Delete: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "VPC: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func deleteVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, msc msTypes.EvpnSvcClient, vrf netproto.Vrf) error {
	curVrfB, err := infraAPI.Read(vrf.Kind, vrf.GetKey())
	if err != nil {
		return err
	}
	ctx := context.TODO()
	curVrf := netproto.Vrf{}
	curVrf.Unmarshal(curVrfB)
	uid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	rtReq := msTypes.EvpnIpVrfRtRequest{}
	for _, rt := range vrf.Spec.RouteImportExport.ExportRTs {
		ert := msTypes.EvpnIpVrfRtSpec{
			Id:     uid.Bytes(),
			VPCId:  uid.Bytes(),
			RT:     utils.RDToBytes(rt),
			RTType: msTypes.EvpnRtType_EVPN_RT_EXPORT,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	for _, rt := range vrf.Spec.RouteImportExport.ImportRTs {
		ert := msTypes.EvpnIpVrfRtSpec{
			Id:     uid.Bytes(),
			VPCId:  uid.Bytes(),
			RT:     utils.RDToBytes(rt),
			RTType: msTypes.EvpnRtType_EVPN_RT_IMPORT,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	eVrfRTResp, err := msc.EvpnIpVrfRtSpecDelete(ctx, &rtReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting  EVPN VRF RT| Err: %v", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF RT | Status: %v", vrf.GetKey(), eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Delete: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	evrfReq := msTypes.EvpnIpVrfRequest{
		Request: []*msTypes.EvpnIpVrfSpec{
			{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				VNI:    vrf.Spec.VxLANVNI,
				AutoRD: msTypes.EvpnCfg_EVPN_CFG_AUTO,
			},
		},
	}
	eVrfResp, err := msc.EvpnIpVrfSpecDelete(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF | Err: %v", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF | Status: %s", vrf.GetKey(), eVrfResp.ApiStatus)
	}
	log.Infof("VRF Delete: %s: got response [%v/%v]", vrf.Name, eVrfResp.ApiStatus, eVrfResp.Response)

	vpcDelReq := &halapi.VPCDeleteRequest{
		Id: [][]byte{uid.Bytes()},
	}

	resp, err := client.VPCDelete(context.Background(), vpcDelReq)
	if err != nil {
		log.Errorf("Vrf: %s delete failed | Err: %v", vrf.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, "Vrf: %s delete failed | Err: %v", vrf.GetKey(), err)
	}
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("VPC: %s", vrf.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(vrf.Kind, vrf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Vrf: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func convertVrfToVPC(infraAPI types.InfraAPI, vrf netproto.Vrf) (*halapi.VPCRequest, error) {
	var vpcType halapi.VPCType
	if strings.ToLower(vrf.Spec.VrfType) == "infra" {
		vpcType = halapi.VPCType_VPC_TYPE_UNDERLAY
	} else {
		vpcType = halapi.VPCType_VPC_TYPE_TENANT
	}

	rt := netproto.RouteTable{
		ObjectMeta: api.ObjectMeta{
			Name:      vrf.Spec.V4RouteTable,
			Tenant:    vrf.Tenant,
			Namespace: vrf.Namespace,
		},
	}
	var uid uuid.UUID
	// Find the RouteTable for the VPC
	rtb, err := infraAPI.Read("RouteTable", rt.GetKey())
	if err != nil && vpcType == halapi.VPCType_VPC_TYPE_TENANT {
		log.Errorf("could not find route-table %s", rt.GetKey())
		return nil, err
	}

	err = rt.Unmarshal(rtb)
	if err != nil {
		log.Errorf("failed to unmarshal RouteTable %s | Err: %s", rt.GetKey(), err)
		return nil, err
	}
	uid, err = uuid.FromString(rt.UUID)

	uidvrf, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("Vrf: %s | could not parse UUID | Err: %v", vrf.GetKey(), err)
		return nil, err
	}
	return &halapi.VPCRequest{
		BatchCtxt: nil,
		Request: []*halapi.VPCSpec{
			{
				Id:               uidvrf.Bytes(),
				Type:             vpcType,
				V4RouteTableId:   uid.Bytes(),
				V6RouteTableId:   nil,
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
	}, nil
}
