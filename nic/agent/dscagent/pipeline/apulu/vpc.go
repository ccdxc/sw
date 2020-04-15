// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleVPC handles crud operations on vrf
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

	if vrf.Spec.VrfType != "INFRA" && vrf.Spec.VxLANVNI == 0 {
		dat, _ := vrf.Marshal()
		if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
			return errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err)
		}
		return nil
	}
	var success bool
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

	defer func() {
		if !success {
			delreq := halapi.VPCDeleteRequest{
				Id: [][]byte{vpcReq.Request[0].Id},
			}
			delresp, err := client.VPCDelete(context.TODO(), &delreq)
			if err != nil {
				log.Errorf("Vrf: %s could not cleanup vpc from datapath: Err: %s", vrf.GetKey(), err)
				return
			}
			if delresp.ApiStatus[0] != halapi.ApiStatus_API_STATUS_OK {
				log.Errorf("Vrf: %s could not cleanup vpc from datapath: Status: %s", vrf.GetKey(), delresp.ApiStatus)
			}
			log.Infof("Vrf: %s delete returned | Status: %s Err %v", vrf.GetKey(), delresp.ApiStatus, err)
		}
	}()

	uid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("Vrf: %s failed to parse UUID %s | Err: %v", vrf.GetKey(), vrf.UUID, err)
		return err
	}

	// Create Metaswitch related objects
	evrfReq := msTypes.EvpnIpVrfRequest{
		Request: []*msTypes.EvpnIpVrfSpec{
			{
				Id:    uid.Bytes(),
				VPCId: uid.Bytes(),
				VNI:   vrf.Spec.VxLANVNI,
			},
		},
	}
	log.Infof("Creating IP Vrf EVI [%+v]", evrfReq.Request[0])
	ctx := context.TODO()
	eVrfResp, err := msc.EvpnIpVrfCreate(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF | Err: %v", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Create received resp (%v)[%v]", err, eVrfResp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF | Status: %v", vrf.GetKey(), eVrfResp.ApiStatus)
	}
	log.Infof("VRF Config: %s: got response [%v]", vrf.Name, eVrfResp.ApiStatus)

	defer func() {
		if !success {
			// Create Metaswitch related objects
			key := &msTypes.EvpnIpVrfKey{
				VPCId: uid.Bytes(),
			}
			evrfReq := msTypes.EvpnIpVrfDeleteRequest{
				Request: []*msTypes.EvpnIpVrfKeyHandle{
					{
						IdOrKey: &msTypes.EvpnIpVrfKeyHandle_Key{key},
					},
				},
			}
			delresp, err := msc.EvpnIpVrfDelete(ctx, &evrfReq)
			if err != nil {
				log.Infof("EVPN VRF Spec cleanup received resp (%v)[%+v]", err, delresp)
			}
			if delresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Infof("EVPN VRF Spec cleanup received resp (%v)[%v]", err, delresp.ApiStatus)
			}
		}
	}()

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
				RT:     utils.RTToBytes(rt.rt),
				RTType: rtype,
			}
			log.Infof("add Evi VRF RT [%+v]", evirt)
			rtReq.Request = append(rtReq.Request, &evirt)
		}

		rtDelReq := msTypes.EvpnIpVrfRtDeleteRequest{}
		for _, rt := range rts {
			key := &msTypes.EvpnIpVrfRtKey{
				RT:    utils.RTToBytes(rt.rt),
				VPCId: uid.Bytes(),
			}
			evirt := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}
			log.Infof("add Evi VRF RT [%+v]", evirt)
			rtDelReq.Request = append(rtDelReq.Request, &evirt)
		}
		eVrfRTResp, err := msc.EvpnIpVrfRtCreate(ctx, &rtReq)
		if err != nil {
			log.Infof("EVPN VRF RT Create received resp (%v)[%+v]", err, eVrfRTResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF RT | Err: %v", vrf.GetKey(), err)
		}
		if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF RT Create received resp (%v)[%v] req[%+v]", err, eVrfRTResp.ApiStatus, rtReq)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring EVPN VRF RT  | status: %s", vrf.GetKey(), eVrfRTResp.ApiStatus)
		}
		log.Infof("VRF RT Config: %s: got response [%v]", vrf.Name, eVrfRTResp.ApiStatus)
		defer func() {
			if !success {
				delresp, err := msc.EvpnIpVrfRtDelete(ctx, &rtDelReq)
				if err != nil {
					log.Infof("EVPN VRF RT cleanup received resp (%v)[%+v]", err, delresp)
					return
				}
				if delresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
					log.Infof("EVPN VRF RT cleanup received resp (%v)[%v]", err, delresp.ApiStatus)
				}
			}
		}()
	}

	dat, _ := vrf.Marshal()

	if err := infraAPI.Store(vrf.Kind, vrf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "VPC: %s | Err: %v", vrf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}
	success = true
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
					Id:    uid.Bytes(),
					VPCId: uid.Bytes(),
					VNI:   vrf.Spec.VxLANVNI,
				},
			},
		}

		eVrfResp, err := msc.EvpnIpVrfUpdate(ctx, &evrfReq)
		if err != nil {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%+v]", err, eVrfResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF | Err: %v", vrf.GetKey(), err)
		}
		if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF Spec Create received resp (%v)[%v]", err, eVrfResp.ApiStatus)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s  Configuring EVPN VRF | status(%s)", vrf.GetKey(), eVrfResp.ApiStatus)
		}
	}

	var rtAddReq msTypes.EvpnIpVrfRtRequest
	var rtDelReq msTypes.EvpnIpVrfRtDeleteRequest
	curRts := curVrf.Spec.RouteImportExport
	newRts := vrf.Spec.RouteImportExport
	if curRts == nil {
		curRts = &netproto.RDSpec{}
	}
	if newRts == nil {
		newRts = &netproto.RDSpec{}
	}

	isEqual := func(a, b *netproto.RouteDistinguisher) bool {
		return a.Type == b.Type && a.AssignedValue == b.AssignedValue && a.AdminValue == b.AdminValue
	}

	curImportRts, curExportRts, curImportExportRts := classifyRouteTargets(curRts)
	newImportRts, newExportRts, newImportExportRts := classifyRouteTargets(newRts)
	for _, r := range newExportRts {
		found := false
		for _, r1 := range curExportRts {
			if isEqual(r, r1) {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RTToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_EXPORT,
			}
			log.Infof("adding export add RT to req [%v]", ert)
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range curExportRts {
		found := false
		for _, r1 := range newExportRts {
			if isEqual(r, r1) {
				found = true
			}
		}
		if !found {
			key := &msTypes.EvpnIpVrfRtKey{
				VPCId: uid.Bytes(),
				RT:    utils.RTToBytes(r),
			}
			ert := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}

			log.Infof("adding export del RT to req [%v]", ert)
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}

	for _, r := range newImportRts {
		found := false
		for _, r1 := range curImportRts {
			if isEqual(r, r1) {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RTToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_IMPORT,
			}
			log.Infof("adding import add RT to req [%v]", ert)
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range curImportRts {
		found := false
		for _, r1 := range newImportRts {
			if isEqual(r, r1) {

				found = true
			}
		}
		if !found {
			key := &msTypes.EvpnIpVrfRtKey{
				VPCId: uid.Bytes(),
				RT:    utils.RTToBytes(r),
			}
			ert := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}
			log.Infof("adding import del RT to req [%v]", ert)
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}

	for _, r := range newImportExportRts {
		found := false
		for _, r1 := range curImportExportRts {
			if isEqual(r, r1) {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnIpVrfRtSpec{
				Id:     uid.Bytes(),
				VPCId:  uid.Bytes(),
				RT:     utils.RTToBytes(r),
				RTType: msTypes.EvpnRtType_EVPN_RT_IMPORT_EXPORT,
			}
			log.Infof("adding import_export add RT to req [%v]", ert)
			rtAddReq.Request = append(rtAddReq.Request, &ert)
		}
	}
	for _, r := range curImportExportRts {
		found := false
		for _, r1 := range newImportExportRts {
			if isEqual(r, r1) {
				found = true
			}
		}
		if !found {
			key := &msTypes.EvpnIpVrfRtKey{
				VPCId: uid.Bytes(),
				RT:    utils.RTToBytes(r),
			}
			ert := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}
			log.Infof("adding import_export del RT to req [%v]", ert)
			rtDelReq.Request = append(rtDelReq.Request, &ert)
		}
	}

	eVrfRTResp, err := msc.EvpnIpVrfRtCreate(ctx, &rtAddReq)
	if err != nil {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%+v]", err, eVrfRTResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Configuring  EVPN VRF RT | Err: %v", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Spec Create received resp (%v)[%v]", err, eVrfRTResp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s  Configuring EVPN VRF RT  | Status: %s", vrf.GetKey(), eVrfRTResp.ApiStatus)
	}
	log.Infof("VRF RT Config: %s: got response [%v]", vrf.Name, eVrfRTResp.ApiStatus)

	eVrfRTDResp, err := msc.EvpnIpVrfRtDelete(ctx, &rtDelReq)
	if err != nil {
		log.Infof("EVPN VRF RT Delete received resp (%v)[%+v]", err, eVrfRTDResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting  EVPN VRF RT| Err: %v", vrf.GetKey(), err)
	}
	if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF RT Delete received resp (%v)[%v]", err, eVrfRTDResp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF RT | Status: %s", vrf.GetKey(), eVrfRTDResp.ApiStatus)
	}
	log.Infof("VRF RT Delete: %s: got response [%v]", vrf.Name, eVrfRTDResp.ApiStatus)

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

	if curVrf.Spec.RouteImportExport != nil {
		rtReq := msTypes.EvpnIpVrfRtDeleteRequest{}
		for _, rt := range vrf.Spec.RouteImportExport.ExportRTs {
			// Create Metaswitch related objects
			key := &msTypes.EvpnIpVrfRtKey{
				VPCId: uid.Bytes(),
				RT:    utils.RTToBytes(rt),
			}
			ert := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}
			rtReq.Request = append(rtReq.Request, &ert)
		}
		for _, rt := range vrf.Spec.RouteImportExport.ImportRTs {
			key := &msTypes.EvpnIpVrfRtKey{
				VPCId: uid.Bytes(),
				RT:    utils.RTToBytes(rt),
			}
			ert := msTypes.EvpnIpVrfRtKeyHandle{
				IdOrKey: &msTypes.EvpnIpVrfRtKeyHandle_Key{key},
			}
			rtReq.Request = append(rtReq.Request, &ert)
		}
		eVrfRTResp, err := msc.EvpnIpVrfRtDelete(ctx, &rtReq)
		if err != nil {
			log.Infof("EVPN VRF RT Delete received resp (%v)[%+v]", err, eVrfRTResp)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting  EVPN VRF RT| Err: %v", vrf.GetKey(), err)
		}
		if eVrfRTResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Infof("EVPN VRF RT Delete received resp (%v)[%v]", err, eVrfRTResp.ApiStatus)
			return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF RT | Status: %v", vrf.GetKey(), eVrfRTResp.ApiStatus)
		}
		log.Infof("VRF RT Delete: %s: got response [%v]", vrf.Name, eVrfRTResp.ApiStatus)
	}

	key := &msTypes.EvpnIpVrfKey{
		VPCId: uid.Bytes(),
	}
	evrfReq := msTypes.EvpnIpVrfDeleteRequest{
		Request: []*msTypes.EvpnIpVrfKeyHandle{
			{
				IdOrKey: &msTypes.EvpnIpVrfKeyHandle_Key{key},
			},
		},
	}

	eVrfResp, err := msc.EvpnIpVrfDelete(ctx, &evrfReq)
	if err != nil {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%+v]", err, eVrfResp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF | Err: %v", vrf.GetKey(), err)
	}
	if eVrfResp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Infof("EVPN VRF Spec Delete received resp (%v)[%v]", err, eVrfResp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "Vrf: %s Deleting EVPN VRF | Status: %s", vrf.GetKey(), eVrfResp.ApiStatus)
	}
	log.Infof("VRF Delete: %s: got response [%v]", vrf.Name, eVrfResp.ApiStatus)

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

func classifyRouteTargets(vrfrt *netproto.RDSpec) (importRts, exportRts, importExportRts []*netproto.RouteDistinguisher) {
	var rts []*rtImpExp
	isEqual := func(a, b *netproto.RouteDistinguisher) bool {
		return a.Type == b.Type && a.AssignedValue == b.AssignedValue && a.AdminValue == b.AdminValue
	}

outerLoop1:
	for _, rt := range vrfrt.ExportRTs {
		for _, r := range rts {
			if isEqual(r.rt, rt) {
				r.exp = true
				continue outerLoop1
			}
		}
		rts = append(rts, &rtImpExp{rt: rt, exp: true})
	}

outerLoop2:
	for _, rt := range vrfrt.ImportRTs {
		for _, r := range rts {
			if isEqual(r.rt, rt) {
				r.imp = true
				continue outerLoop2
			}
		}
		rts = append(rts, &rtImpExp{rt: rt, imp: true})
	}

	for _, rt := range rts {
		switch {
		case rt.imp && rt.exp:
			importExportRts = append(importExportRts, rt.rt)
		case rt.exp:
			exportRts = append(exportRts, rt.rt)
		case rt.imp:
			importRts = append(importRts, rt.rt)
		}
	}
	return
}
