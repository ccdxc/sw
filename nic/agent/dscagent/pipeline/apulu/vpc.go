// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"encoding/binary"
	"fmt"
	"strings"

	"github.com/satori/go.uuid"

	"github.com/pkg/errors"

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
	vpcReq := convertVrfToVPC(vrf)
	resp, err := client.VPCCreate(context.Background(), vpcReq)
	log.Infof("createVPCHandler Response: %v. Err: %v", resp, err)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
			return err
		}
	}
	if vrf.Spec.VrfType != "CUSTOMER" {
		return nil
	}

	uid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	// Create Metaswitch related objects
	evrfReq := msTypes.EvpnIpVrfRequest{
		Request: []*msTypes.EvpnIpVrfSpec{
			{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				VNI:     vrf.Spec.VxLANVNI,
				AutoRD:  msTypes.EvpnCfg_EVPN_CFG_AUTO,
				VRFName: vrf.Name,
			},
		},
	}
	ctx := context.TODO()
	eVrfResp, err := msc.EvpnIpVrfSpecCreate(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring EVPN VRF (%s)", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF Config: %s | Err: Configuring EVPN VRF status(%s)", eVrfResp.ApiStatus)
	}
	log.Infof("VRF Config: %s: got response [%v/%v]", vrf.Name, eVrfResp.ApiStatus, eVrfResp.Response)

	rtReq := msTypes.EvpnIpVrfRtRequest{}
	for _, rt := range vrf.Spec.RouteImportExport.ExportRTs {
		ert := msTypes.EvpnIpVrfRtSpec{
			Id:      uid.Bytes(),
			VPCId:   uid.Bytes(),
			RT:      utils.RDToBytes(rt),
			RTType:  msTypes.EvpnRtType_EVPN_RT_EXPORT,
			VRFName: vrf.Name,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	for _, rt := range vrf.Spec.RouteImportExport.ImportRTs {
		ert := msTypes.EvpnIpVrfRtSpec{
			Id:      uid.Bytes(),
			VPCId:   uid.Bytes(),
			RT:      utils.RDToBytes(rt),
			RTType:  msTypes.EvpnRtType_EVPN_RT_IMPORT,
			VRFName: vrf.Name,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	eVrfRTResp, err := msc.EvpnIpVrfRtSpecCreate(ctx, &rtReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring  EVPN VRF RT (%s)", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF RT Config: %s | Err: Configuring EVPN VRF RT status(%s)", eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Config: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err)
	}
	return nil
}

func updateVPCHandler(infraAPI types.InfraAPI, client halapi.VPCSvcClient, msc msTypes.EvpnSvcClient, vrf netproto.Vrf) error {
	curVrfB, err := infraAPI.Read(vrf.Kind, vrf.GetKey())
	if err != nil {
		return err
	}
	vpcReq := convertVrfToVPC(vrf)
	resp, err := client.VPCUpdate(context.Background(), vpcReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", vrf.GetKind(), vrf.GetKey())); err != nil {
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
		return errors.Wrapf(types.ErrBadRequest, "UUID change on update [%v][%v/%v]", vrf.Name, curVrf.UUID, vrf.UUID)
	}
	if curVrf.Spec.VxLANVNI != vrf.Spec.VxLANVNI {
		evrfReq := msTypes.EvpnIpVrfRequest{
			Request: []*msTypes.EvpnIpVrfSpec{
				{
					Id:      uid.Bytes(),
					VPCId:   uid.Bytes(),
					VNI:     vrf.Spec.VxLANVNI,
					AutoRD:  msTypes.EvpnCfg_EVPN_CFG_AUTO,
					VRFName: vrf.Name,
				},
			},
		}

		eVrfResp, err := msc.EvpnIpVrfSpecUpdate(ctx, &evrfReq)
		if err != nil {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring  EVPN VRF (%s)", vrf.GetKey(), err)
		}
		if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF Config: %s | Err: Configuring EVPN VRF status(%s)", eVrfResp.ApiStatus)
		}
	}

	var rtDelReq, rtAddReq msTypes.EvpnIpVrfRtRequest
	for _, r := range curVrf.Spec.RouteImportExport.ExportRTs {
		found := false
		for _, r1 := range vrf.Spec.RouteImportExport.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				RT:      utils.RDToBytes(r),
				RTType:  msTypes.EvpnRtType_EVPN_RT_EXPORT,
				VRFName: vrf.Name,
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range vrf.Spec.RouteImportExport.ExportRTs {
		found := false
		for _, r1 := range curVrf.Spec.RouteImportExport.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				RT:      utils.RDToBytes(r),
				RTType:  msTypes.EvpnRtType_EVPN_RT_EXPORT,
				VRFName: vrf.Name,
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}

	for _, r := range curVrf.Spec.RouteImportExport.ImportRTs {
		found := false
		for _, r1 := range vrf.Spec.RouteImportExport.ImportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				RT:      utils.RDToBytes(r),
				RTType:  msTypes.EvpnRtType_EVPN_RT_IMPORT,
				VRFName: vrf.Name,
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range vrf.Spec.RouteImportExport.ImportRTs {
		found := false
		for _, r1 := range curVrf.Spec.RouteImportExport.ImportRTs {
			if r == r1 {

				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				RT:      utils.RDToBytes(r),
				RTType:  msTypes.EvpnRtType_EVPN_RT_IMPORT,
				VRFName: vrf.Name,
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}
	eVrfRTResp, err := msc.EvpnIpVrfRtSpecCreate(ctx, &rtAddReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring  EVPN VRF RT (%s)", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF RT Config: %s | Err: Configuring EVPN VRF RT status(%s)", eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Config: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	eVrfRTResp, err = msc.EvpnIpVrfRtSpecDelete(ctx, &rtDelReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting  EVPN VRF RT (%s)", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF RT Config: %s | Err: Deleting EVPN VRF RT status(%s)", eVrfRTResp.ApiStatus)
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
	vpcID := make([]byte, 8)
	binary.LittleEndian.PutUint64(vpcID, vrf.Status.VrfID)

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
			Id:      uid.Bytes(),
			VPCId:   uid.Bytes(),
			RT:      utils.RDToBytes(rt),
			RTType:  msTypes.EvpnRtType_EVPN_RT_EXPORT,
			VRFName: vrf.Name,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	for _, rt := range vrf.Spec.RouteImportExport.ImportRTs {
		ert := msTypes.EvpnIpVrfRtSpec{
			Id:      uid.Bytes(),
			VPCId:   uid.Bytes(),
			RT:      utils.RDToBytes(rt),
			RTType:  msTypes.EvpnRtType_EVPN_RT_IMPORT,
			VRFName: vrf.Name,
		}
		rtReq.Request = append(rtReq.Request, &ert)
	}
	eVrfRTResp, err := msc.EvpnIpVrfRtSpecDelete(ctx, &rtReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting  EVPN VRF RT (%s)", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Delete received resp (%v)[%v, %v]", err, eVrfRTResp.ApiStatus, eVrfRTResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF RT Config: %s | Err: Deleting EVPN VRF RT status(%s)", eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Delete: %s: got response [%v/%v]", vrf.Name, eVrfRTResp.ApiStatus, eVrfRTResp.Response)

	evrfReq := msTypes.EvpnIpVrfRequest{
		Request: []*msTypes.EvpnIpVrfSpec{
			{
				Id:      uid.Bytes(),
				VPCId:   uid.Bytes(),
				VNI:     vrf.Spec.VxLANVNI,
				AutoRD:  msTypes.EvpnCfg_EVPN_CFG_AUTO,
				VRFName: vrf.Name,
			},
		},
	}
	eVrfResp, err := msc.EvpnIpVrfSpecDelete(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting EVPN VRF (%s)", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%v, %v]", err, eVrfResp.ApiStatus, eVrfResp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "VRF Config: %s | Err: Deleting EVPN VRF status(%s)", eVrfResp.ApiStatus)
	}
	log.Infof("VRF Delete: %s: got response [%v/%v]", vrf.Name, eVrfResp.ApiStatus, eVrfResp.Response)

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
