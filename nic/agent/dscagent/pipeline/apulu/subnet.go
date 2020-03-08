// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
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

type rtImpExp struct {
	rt  *netproto.RouteDistinguisher
	imp bool
	exp bool
}

func createSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	msc msTypes.EvpnSvcClient, nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {

	subnetReq, err := convertNetworkToSubnet(infraAPI, nw, uplinkIDs)
	if err != nil {
		return errors.Wrapf(types.ErrBadRequest, "Subnet %s | failed to get Vrf %s | Err: %v", nw.GetKey(), nw.Spec.VrfName, err)
	}

	log.Infof("SubnetReq: %+v", subnetReq)

	var success bool
	ctx := context.TODO()
	uid, err := uuid.FromString(nw.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}

	resp, err := client.SubnetCreate(context.Background(), subnetReq)
	if err != nil {
		log.Errorf("Subnet: %s Create failed | Err: %v", nw.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, "Subnet: %s Create failed | Err: %v", nw.GetKey(), err)
	}
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", nw.GetKind(), nw.GetKey())); err != nil {
			return err
		}
	}
	log.Infof("Subnet: %s Create returned | Status: %s | Response: %+v", nw.GetKey(), resp.ApiStatus, resp.Response)

	defer func() {
		if !success {
			delReq := &halapi.SubnetDeleteRequest{
				Id: [][]byte{uid.Bytes()},
			}
			delresp, err := client.SubnetDelete(ctx, delReq)
			if err != nil {
				log.Errorf("Subnet: %s Cleanup failed | Err: %v", nw.GetKey(), err)
			}
			if delresp.ApiStatus[0] != halapi.ApiStatus_API_STATUS_OK {
				log.Errorf("Subnet: %s Cleanup failed | Status: %v", nw.GetKey(), delresp.ApiStatus)
			}
		}
	}()

	eviReq := msTypes.EvpnEviRequest{
		Request: []*msTypes.EvpnEviSpec{
			{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				AutoRD:   msTypes.EvpnCfg_EVPN_CFG_AUTO,
				AutoRT:   msTypes.EvpnCfg_EVPN_CFG_MANUAL,
				RTType:   msTypes.EvpnRtType_EVPN_RT_NONE,
			},
		},
	}
	log.Infof("Evpn EVI request [%+v]", eviReq)
	evresp, err := msc.EvpnEviCreate(ctx, &eviReq)
	if err != nil {
		log.Errorf("failed to create EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}

	log.Infof("got EVPN EVI create response [%v]", evresp.ApiStatus)
	defer func() {
		if !success {
			key := &msTypes.EvpnEviKey{
				SubnetId: uid.Bytes(),
			}
			evidreq := msTypes.EvpnEviDeleteRequest{
				Request: []*msTypes.EvpnEviKeyHandle{
					{
						IdOrKey: &msTypes.EvpnEviKeyHandle_Key{key},
					},
				},
			}
			delresp, err := msc.EvpnEviDelete(ctx, &evidreq)
			if err != nil {
				log.Errorf("EVI: %s Cleanup failed | Err: %v", nw.GetKey(), err)
			}
			if delresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Errorf("EVI: %s Cleanup failed | Status: %v", nw.GetKey(), delresp.ApiStatus)
			}
		}
	}()
	// Create the RT import and export
	if nw.Spec.RouteImportExport != nil {
		var rts []*rtImpExp
		isEqual := func(a, b *netproto.RouteDistinguisher) bool {
			return a.Type == b.Type && a.AssignedValue == b.AssignedValue && a.AdminValue == b.AdminValue
		}
		eviRtReq := &msTypes.EvpnEviRtRequest{}
	outerLoop1:
		for _, rt := range nw.Spec.RouteImportExport.ExportRTs {
			for _, r := range rts {
				if isEqual(r.rt, rt) {
					r.exp = true
					continue outerLoop1
				}
			}
			rts = append(rts, &rtImpExp{rt: rt, exp: true})
		}

	outerLoop2:
		for _, rt := range nw.Spec.RouteImportExport.ImportRTs {
			for _, r := range rts {
				if isEqual(r.rt, rt) {
					r.imp = true
					continue outerLoop2
				}
			}
			rts = append(rts, &rtImpExp{rt: rt, imp: true})
		}

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
			evirt := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RTToBytes(rt.rt),
				RTType:   rtype,
			}
			log.Infof("add Evi RT [%+v]", evirt)
			eviRtReq.Request = append(eviRtReq.Request, &evirt)
		}
		evrtresp, err := msc.EvpnEviRtCreate(ctx, eviRtReq)
		if err != nil {
			log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
		}
		if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Errorf("failed to create EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
			return err
		}
		log.Infof("subnet EVI RT create [%v/%v] got response [%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		defer func() {
			if !success {
				key := &msTypes.EvpnEviRtKey{
					SubnetId: uid.Bytes(),
				}
				evidRtReq := &msTypes.EvpnEviRtDeleteRequest{
					Request: []*msTypes.EvpnEviRtKeyHandle{
						{
							IdOrKey: &msTypes.EvpnEviRtKeyHandle_Key{key},
						},
					},
				}
				delresp, err := msc.EvpnEviRtDelete(ctx, evidRtReq)
				if err != nil {
					log.Errorf("EVI RT: %s Cleanup failed | Err: %v", nw.GetKey(), err)
				}
				if delresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
					log.Errorf("EVI RT: %s Cleanup failed | Status: %v", nw.GetKey(), delresp.ApiStatus)
				}
			}
		}()
	}

	dat, _ := nw.Marshal()

	if err := infraAPI.Store(nw.Kind, nw.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Subnet: %s | Err: %v", nw.GetKey(), err)
	}
	success = true
	return nil
}

func updateSubnetHandler(infraAPI types.InfraAPI, client halapi.SubnetSvcClient,
	msc msTypes.EvpnSvcClient, nw netproto.Network, vpcID uint64, uplinkIDs []uint64) error {
	subnetReq, err := convertNetworkToSubnet(infraAPI, nw, uplinkIDs)
	if err != nil {
		return errors.Wrapf(types.ErrBadRequest, "Subnet: %v Convertion failed | Err: %v", nw.GetKey(), err)
	}
	resp, err := client.SubnetUpdate(context.Background(), subnetReq)
	if err != nil {
		log.Errorf("Subnet: %s Update failed | Err: %v", nw.GetKey(), err)
		return errors.Wrapf(types.ErrDatapathHandling, "Subnet: %s Update failed | Err: %v", nw.GetKey(), err)
	}
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Subnet: %s Update Failed | %s", nw.GetKind(), nw.GetKey())); err != nil {
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
			},
		},
	}

	evresp, err := msc.EvpnEviUpdate(ctx, &eviReq)
	if err != nil {
		log.Errorf("failed to update EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	log.Infof("got EVPN EVI update response [%v]", evresp.ApiStatus)

	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to update EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}
	curRts := curNw.Spec.RouteImportExport
	newRts := nw.Spec.RouteImportExport
	if curRts == nil {
		curRts = &netproto.RDSpec{}
	}
	if newRts == nil {
		newRts = &netproto.RDSpec{}
	}
	var rtDelReq, rtAddReq msTypes.EvpnEviRtRequest
	for _, r := range curRts.ExportRTs {
		found := false
		for _, r1 := range newRts.ExportRTs {
			if r == r1 {
				found = true
			}
		}
		if !found {
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RTToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
			log.Infof("add EVI RT export [%v]", ert)
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
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RTToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_EXPORT,
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
			log.Infof("del EVI RT export [%v]", ert)
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
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RTToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
			}
			rtAddReq.Request = append(rtAddReq.Request, &ert)
			log.Infof("add EVI RT import [%v]", ert)
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
			ert := msTypes.EvpnEviRtSpec{
				Id:       uid.Bytes(),
				SubnetId: uid.Bytes(),
				RT:       utils.RTToBytes(r),
				RTType:   msTypes.EvpnRtType_EVPN_RT_IMPORT,
			}
			rtDelReq.Request = append(rtDelReq.Request, &ert)
			log.Infof("del EVI RT import [%v]", ert)
		}
	}

	evrtresp, err := msc.EvpnEviRtCreate(ctx, &rtAddReq)
	if err != nil {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet update [%v/%v] RT create  got response [%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus)

	evrtresp, err = msc.EvpnEviRtCreate(ctx, &rtDelReq)
	if err != nil {
		log.Errorf("failed to create EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
	}
	if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
		return err
	}
	log.Infof("subnet update [%v/%v] delete RT got response [%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus)

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

	if curNw.Spec.RouteImportExport != nil {
		eviRtReq := &msTypes.EvpnEviRtDeleteRequest{}
		for _, rt := range curNw.Spec.RouteImportExport.ExportRTs {
			key := &msTypes.EvpnEviRtKey{
				RT: utils.RTToBytes(rt),
			}
			evirt := msTypes.EvpnEviRtKeyHandle{
				IdOrKey: &msTypes.EvpnEviRtKeyHandle_Key{key},
			}
			eviRtReq.Request = append(eviRtReq.Request, &evirt)
			log.Infof("del EVI RT export [%v]", evirt)
		}
		for _, rt := range curNw.Spec.RouteImportExport.ImportRTs {
			key := &msTypes.EvpnEviRtKey{
				RT: utils.RTToBytes(rt),
			}
			evirt := msTypes.EvpnEviRtKeyHandle{
				IdOrKey: &msTypes.EvpnEviRtKeyHandle_Key{key},
			}
			eviRtReq.Request = append(eviRtReq.Request, &evirt)
			log.Infof("del EVI RT import [%v]", evirt)
		}
		evrtresp, err := msc.EvpnEviRtDelete(ctx, eviRtReq)
		if err != nil {
			log.Errorf("failed to delete EVI RTs for subnet [%v/%v](%s)", nw.Tenant, nw.Name, err)
		}
		if evrtresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Errorf("failed to delete EVI RTs for subnet [%v/%v] (%v)", nw.Tenant, nw.Name, evrtresp.ApiStatus)
			return err
		}
		log.Infof("subnet delete RT [%v/%v] got response [%v] for ", nw.Tenant, nw.Name, evrtresp.ApiStatus)
	}
	key := &msTypes.EvpnEviKey{
		SubnetId: uid.Bytes(),
	}
	evireq := msTypes.EvpnEviDeleteRequest{
		Request: []*msTypes.EvpnEviKeyHandle{
			{
				IdOrKey: &msTypes.EvpnEviKeyHandle_Key{key},
			},
		},
	}
	evresp, err := msc.EvpnEviDelete(ctx, &evireq)
	if err != nil {
		log.Errorf("failed to delete EVI for subnet [%v/%v]", nw.Tenant, nw.Name)
		return err
	}
	log.Infof("got EVPN EVI delete response [%v]", evresp.ApiStatus)

	if evresp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete EVI for subnet [%v/%v](%v)", nw.Tenant, nw.Name, evresp.ApiStatus)
		return err
	}

	subnetDelReq := &halapi.SubnetDeleteRequest{
		Id: [][]byte{uid.Bytes()},
	}
	resp, err := client.SubnetDelete(context.Background(), subnetDelReq)
	if err != nil {
		log.Errorf("Subnet: %s failed to delete| Err: %v", nw.GetKey(), err)
	}
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

func convertNetworkToSubnet(infraAPI types.InfraAPI, nw netproto.Network, uplinkIDs []uint64) (*halapi.SubnetRequest, error) {
	var v6Prefix *halapi.IPv6Prefix
	var v4Prefix *halapi.IPv4Prefix
	var v4VrIP uint32
	var v6VrIP []byte
	ipamName := ""

	v6Prefix = nil
	v4Prefix = nil
	v4VrIP = 0
	v6VrIP = nil
	vrf, err := validator.ValidateVrf(infraAPI, nw.Tenant, nw.Namespace, nw.Spec.VrfName)
	if err != nil {
		log.Errorf("Get VRF failed for %s | %s", nw.GetKind(), nw.GetKey())
		return nil, err
	}
	log.Infof("Subnet %+v: Vrf %+v: ", nw, vrf)
	vrfuid, err := uuid.FromString(vrf.UUID)
	vrMac := nw.Spec.RouterMAC
	if vrMac == "" {
		vrMac = vrf.Spec.RouterMAC
	}
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

	uid, err := uuid.FromString(nw.UUID)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Network: Parsing UUID %s | %s Err: %v", nw.GetKey(), nw.UUID, err))
		return nil, err
	}

	var ipamuuid []byte
	if nw.Spec.IPAMPolicy != "" {
		ipamName = nw.Spec.IPAMPolicy
	} else {
		// pick the ipam policy from the vpc
		ipamName = vrf.Spec.IPAMPolicy
	}

	if ipamName != "" {
		policy, err := validator.ValidateIPAMPolicy(infraAPI, nw.Tenant, nw.Namespace, ipamName)
		if err != nil {
			log.Errorf("Get IPAMPolicy failed for %s | %s", nw.GetKind(), nw.GetKey())
			return nil, err
		}
		ipu, _ := uuid.FromString(policy.UUID)
		if err != nil {
			log.Errorf("Parse IPAMPolicy UUID failed for %s | %s", nw.GetKind(), nw.GetKey())
			return nil, err
		}
		ipamuuid = ipu.Bytes()
	}

	return &halapi.SubnetRequest{
		BatchCtxt: nil,
		Request: []*halapi.SubnetSpec{
			{
				Id:                  uid.Bytes(),
				VPCId:               vrfuid.Bytes(),
				IPv4VirtualRouterIP: v4VrIP,
				IPv6VirtualRouterIP: v6VrIP,
				VirtualRouterMac:    utils.MacStrtoUint64(vrMac),
				FabricEncap: &halapi.Encap{
					Type: halapi.EncapType_ENCAP_TYPE_VXLAN,
					Value: &halapi.EncapVal{
						Val: &halapi.EncapVal_Vnid{
							Vnid: nw.Spec.VxLANVNI,
						},
					},
				},
				V4Prefix:     v4Prefix,
				V6Prefix:     v6Prefix,
				DHCPPolicyId: ipamuuid,
				// IngV4SecurityPolicyId: utils.ConvertID32(nw.Spec.IngV4SecurityPolicyID...),
				// EgV4SecurityPolicyId:  utils.ConvertID32(nw.Spec.EgV4SecurityPolicyID...),
				// IngV6SecurityPolicyId: utils.ConvertID32(nw.Spec.IngV6SecurityPolicyID...),
				// EgV6SecurityPolicyId:  utils.ConvertID32(nw.Spec.EgV6SecurityPolicyID...),
			},
		},
	}, nil

}
