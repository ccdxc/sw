// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"net"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/venice/utils/log"
)

// currentRoutingConfig is cached routing config that has been configured on metaswitch
var currentRoutingConfig *netproto.RoutingConfig

func ip2uint32(ipstr string) uint32 {
	log.Info("parsing IP address ", ipstr)
	ip := net.ParseIP(ipstr).To4()
	if ip != nil {
		return (((uint32(ip[3])*256)+uint32(ip[2]))*256+uint32(ip[1]))*256 + uint32(ip[0])
	}
	log.Errorf("unable to parse IP address %s", ipstr)
	return 0
	// return (((uint32(ip[0])*256)+uint32(ip[1]))*256+uint32(ip[2]))*256 + uint32(ip[3])
}

func ip2PDSType(ipstr string) *pdstypes.IPAddress {
	return &pdstypes.IPAddress{
		Af:     pdstypes.IPAF_IP_AF_INET,
		V4OrV6: &pdstypes.IPAddress_V4Addr{V4Addr: ip2uint32(ipstr)},
	}
}

// HandleRouteConfig handles crud operations on vrf TODO use VPCClient here
func HandleRouteConfig(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, oper types.Operation, rtCfg netproto.RoutingConfig) error {
	switch oper {
	case types.Create:
		return createRoutingConfigHandler(infraAPI, client, rtCfg)
	case types.Update:
		return updateRoutingConfigHandler(infraAPI, client, rtCfg)
	case types.Delete:
		return deleteRoutingConfigHandler(infraAPI, client, rtCfg)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	// we can have only one routing config on the NAPLES card.
	if currentRoutingConfig != nil {
		log.Errorf("RoutingConfig: %s | Err: Create with existing routing config[%s]", rtCfg.GetKey(), currentRoutingConfig.GetKey())
		return errors.Wrapf(types.ErrBadRequest, "RoutingConfig: %s | Err: Create with existing routing config[%s]", rtCfg.GetKey(), currentRoutingConfig.GetKey())
	}
	uid, err := uuid.FromString(rtCfg.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	ctx := context.TODO()
	req := msTypes.BGPRequest{
		Request: &msTypes.BGPSpec{
			Id:       uid.Bytes(),
			LocalASN: rtCfg.Spec.BGPConfig.ASNumber,
			RouterId: ip2uint32(rtCfg.Spec.BGPConfig.RouterId),
		},
	}
	resp, err := client.BGPCreate(ctx, &req)
	if err != nil {
		log.Infof("BGP Create received resp (%v)[%+v]", err, resp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Global Config (%s)", rtCfg.GetKey(), err)
	}
	if resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("BGP Create received resp (%v)[%v, %v]", err, resp.ApiStatus, resp.Response)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Global config status(%s)", rtCfg.GetKey(), resp.ApiStatus)
	}
	log.Infof("BGP create responded [%v]", resp.Response)
	// Get the controller IPs
	dscConfig := infraAPI.GetConfig()
	log.Infof("Adding peers at [%v]", dscConfig.Controllers)

	unknLocal := &pdstypes.IPAddress{
		Af: pdstypes.IPAF_IP_AF_INET,
	}
	// Configure the Underlay Peers
	peerReq := msTypes.BGPPeerRequest{}
	peerAFReq := msTypes.BGPPeerAfRequest{}
	for _, n := range rtCfg.Spec.BGPConfig.Neighbors {
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        msTypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:     ip2PDSType(n.IPAddress),
			LocalAddr:    unknLocal,
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
		}
		log.Infof("Add create peer [%+v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		peerAf := msTypes.BGPPeerAfSpec{
			Id:          uid.Bytes(),
			PeerAddr:    ip2PDSType(n.IPAddress),
			LocalAddr:   unknLocal,
			Afi:         msTypes.BGPAfi_BGP_AFI_IPV4,
			Safi:        msTypes.BGPSafi_BGP_SAFI_UNICAST,
			Disable:     false,
			NexthopSelf: false,
			DefaultOrig: false,
		}
		log.Infof("Add create peer AF [%+v]", peerAf)
		peerAFReq.Request = append(peerAFReq.Request, &peerAf)
	}

	// Add the Venice RRs
	for _, n := range dscConfig.Controllers {
		h, _, err := net.SplitHostPort(n)
		if err != nil {
			log.Errorf("hostport returned error for [%v](%v)", n, err)
		}
		a, err := net.LookupHost(h)
		if err != nil {
			log.Errorf("LookupHost returned error for [%v](%v)", a, err)
			continue
		}
		log.Info("lookuphost returned ", a)
		peer := msTypes.BGPPeerSpec{
			Id:       uid.Bytes(),
			State:    msTypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr: ip2PDSType(a[0]),
			// XXX-TBD change to appropriate address
			LocalAddr:    unknLocal,
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
		}
		log.Infof("Add create peer [%+v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		peerAf := msTypes.BGPPeerAfSpec{
			Id:          uid.Bytes(),
			PeerAddr:    ip2PDSType(a[0]),
			LocalAddr:   unknLocal,
			Afi:         msTypes.BGPAfi_BGP_AFI_L2VPN,
			Safi:        msTypes.BGPSafi_BGP_SAFI_EVPN,
			Disable:     false,
			NexthopSelf: false,
			DefaultOrig: false,
		}
		log.Infof("Add create peer AF [%+v]", peerAf)
		peerAFReq.Request = append(peerAFReq.Request, &peerAf)
	}
	presp, err := client.BGPPeerCreate(ctx, &peerReq)
	if err != nil {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer Config Status(%v)", rtCfg.GetKey(), presp.ApiStatus)
	}

	afresp, err := client.BGPPeerAfCreate(ctx, &peerAFReq)
	if err != nil {
		log.Infof("PeerAF create Request returned (%v)[%v]", err, afresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config (%s)", rtCfg.GetKey(), err)
	}
	if afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("PeerAF create Request returned (%v)[%v]", err, afresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config Status(%v)", rtCfg.GetKey(), afresp.ApiStatus)
	}
	currentRoutingConfig = &rtCfg
	return nil
}

func updateRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	// check the current config and add/delete as needed
	// Global spec cannot be updated
	// Check configured peers
	var newPeers []*netproto.BGPNeighbor
	var delPeers []*netproto.BGPNeighbor

	// Peer counts are only a handful a seq walk is fine.
	for _, o := range currentRoutingConfig.Spec.BGPConfig.Neighbors {
		found := false
		for _, o1 := range rtCfg.Spec.BGPConfig.Neighbors {
			if o1.IPAddress == o.IPAddress {
				found = true
			}
		}
		if !found {
			delPeers = append(delPeers, o)
		}
	}

	// Peer counts are only a handful a seq walk is fine.
	for _, o := range rtCfg.Spec.BGPConfig.Neighbors {
		found := false
		for _, o1 := range currentRoutingConfig.Spec.BGPConfig.Neighbors {
			if o1.IPAddress == o.IPAddress {
				found = true
			}
		}
		if !found {
			newPeers = append(newPeers, o)
		}
	}
	uid, err := uuid.FromString(currentRoutingConfig.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	peerReq := msTypes.BGPPeerRequest{}
	peerAFReq := msTypes.BGPPeerAfRequest{}
	for _, o := range delPeers {
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        msTypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:     ip2PDSType(o.IPAddress),
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		peerAf := msTypes.BGPPeerAfSpec{
			Id:       uid.Bytes(),
			PeerAddr: ip2PDSType(o.IPAddress),
			Afi:      msTypes.BGPAfi_BGP_AFI_L2VPN,
			Safi:     msTypes.BGPSafi_BGP_SAFI_EVPN,
		}
		log.Infof("Add Delete peer AF [%+v]", peerAf)
		peerAFReq.Request = append(peerAFReq.Request, &peerAf)
	}
	ctx := context.TODO()

	presp, err := client.BGPPeerDelete(ctx, &peerReq)
	if err != nil {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config Status(%v)", rtCfg.GetKey(), presp.ApiStatus)
	}

	afresp, err := client.BGPPeerAfDelete(ctx, &peerAFReq)
	if err != nil {
		log.Infof("PeerAF delete request returned (%v)[%v]", err, afresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config (%s)", rtCfg.GetKey(), err)
	}
	if afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("PeerAF delete request returned (%v)[%v]", err, afresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer AF Config Status(%v)", rtCfg.GetKey(), afresp.ApiStatus)
	}

	peerReq = msTypes.BGPPeerRequest{}
	peerAFReq = msTypes.BGPPeerAfRequest{}

	// use the UUID from the new config
	uid, err = uuid.FromString(rtCfg.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	for _, o := range newPeers {
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        msTypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:     ip2PDSType(o.IPAddress),
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		peerAf := msTypes.BGPPeerAfSpec{
			Id:       uid.Bytes(),
			PeerAddr: ip2PDSType(o.IPAddress),
			Afi:      msTypes.BGPAfi_BGP_AFI_L2VPN,
			Safi:     msTypes.BGPSafi_BGP_SAFI_EVPN,
		}
		log.Infof("Add Delete peer AF [%+v]", peerAf)
		peerAFReq.Request = append(peerAFReq.Request, &peerAf)
	}
	presp, err = client.BGPPeerCreate(ctx, &peerReq)
	if err != nil {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer Config Status(%v)", rtCfg.GetKey(), presp.ApiStatus)
	}

	afresp, err = client.BGPPeerAfCreate(ctx, &peerAFReq)
	if err != nil {
		log.Infof("PeerAF create Request returned (%v)[%v]", err, afresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config (%s)", rtCfg.GetKey(), err)
	}
	if afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("PeerAF create Request returned (%v)[%v]", err, afresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config Status(%v)", rtCfg.GetKey(), afresp.ApiStatus)
	}
	currentRoutingConfig = &rtCfg
	return nil
}

func deleteRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	// Delte configured peers
	uid, err := uuid.FromString(currentRoutingConfig.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	peerReq := msTypes.BGPPeerRequest{}
	peerAFReq := msTypes.BGPPeerAfRequest{}
	for _, o := range currentRoutingConfig.Spec.BGPConfig.Neighbors {
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        msTypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:     ip2PDSType(o.IPAddress),
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		peerAf := msTypes.BGPPeerAfSpec{
			Id:       uid.Bytes(),
			PeerAddr: ip2PDSType(o.IPAddress),
			Afi:      msTypes.BGPAfi_BGP_AFI_L2VPN,
			Safi:     msTypes.BGPSafi_BGP_SAFI_EVPN,
		}
		log.Infof("Add Delete peer AF [%+v]", peerAf)
		peerAFReq.Request = append(peerAFReq.Request, &peerAf)
	}
	ctx := context.TODO()

	presp, err := client.BGPPeerDelete(ctx, &peerReq)
	if err != nil {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer create Request returned (%v)[%v]", err, presp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config Status(%v)", rtCfg.GetKey(), presp.ApiStatus)
	}
	currentRoutingConfig = nil
	return nil
}
