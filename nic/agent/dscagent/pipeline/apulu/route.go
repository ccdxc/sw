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
	msTypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

const oobIntfName = "oob_mnic0"

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
		log.Infof("RoutingConfig: %s | Err: Create with existing routing config[%s]", rtCfg.GetKey(), currentRoutingConfig.GetKey())
		return nil
	}
	log.Infof("RoutingConfig: %s Begin create", rtCfg.GetKey())
	rid := rtCfg.Spec.BGPConfig.RouterId
	rip := net.ParseIP(rtCfg.Spec.BGPConfig.RouterId)

	// For now till NPM does proper filtering of RoutingConfig only accept Routing Config with AutoConfig
	if !rip.IsUnspecified() {
		log.Infof("ignoring Routing Config with set Router ID [%v]", rtCfg.Name)
		return nil
	}
	uid, err := uuid.FromString(rtCfg.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	dsccfg := infraAPI.GetConfig()
	ctx := context.TODO()

	if dsccfg.LoopbackIP != "" {
		rid = dsccfg.LoopbackIP
	} else {
		if len(dsccfg.DSCInterfaceIPs) > 0 {
			rid = dsccfg.DSCInterfaceIPs[0].IPAddress
		} else {
			intf, err := net.InterfaceByName(oobIntfName)
			if err != nil {
				log.Errorf("BGP Create could not get IP Address for router ID (%s)", err)
				return errors.Wrap(types.ErrInvalidIP, "could not determine Router ID")
			}
			addrs, err := intf.Addrs()
			if err != nil {
				log.Errorf("BGP Create could not get IP Address for router ID (%s)", err)
				return errors.Wrap(types.ErrInvalidIP, "could not determine Router ID")
			}
		GotIP:
			for _, a := range addrs {
				switch aip := a.(type) {
				case *net.IPNet:
					rid = aip.IP.String()
					break GotIP
				case *net.IPAddr:
					rid = aip.IP.String()
					break GotIP
				}
			}
		}
	}
	if rid == "" {
		log.Errorf("BGP Create could not get IP Address for router ID (%s)", err)
		return errors.Wrap(types.ErrInvalidIP, "could not determine Router ID")
	}

	req := msTypes.BGPRequest{
		Request: &msTypes.BGPSpec{
			Id:       uid.Bytes(),
			LocalASN: rtCfg.Spec.BGPConfig.ASNumber,
			RouterId: ip2uint32(rid),
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

	var autoConfig bool
	var state msTypes.AdminState

	// Configure the Underlay Peers
	peerReq := msTypes.BGPPeerRequest{}
	peerAFReq := msTypes.BGPPeerAfRequest{}
	for _, n := range rtCfg.Spec.BGPConfig.Neighbors {
		rip := net.ParseIP(n.IPAddress)
		if n.Shutdown == true {
			state = pdstypes.AdminState_ADMIN_STATE_DISABLE
		} else {
			state = pdstypes.AdminState_ADMIN_STATE_ENABLE
		}

		// if set to 0.0.0.0 auto configure the neighborts learnt via DHCP
		if rip.IsUnspecified() {
			autoConfig = true
			for _, i := range dsccfg.DSCInterfaceIPs {
				peer := msTypes.BGPPeerSpec{
					Id:           uid.Bytes(),
					State:        state,
					PeerAddr:     ip2PDSType(i.GatewayIP),
					LocalAddr:    unknLocal,
					RemoteASN:    n.RemoteAS,
					Password:     []byte(n.Password),
					SendComm:     true,
					SendExtComm:  true,
					ConnectRetry: 5,
					KeepAlive:    rtCfg.Spec.BGPConfig.KeepaliveInterval,
					HoldTime:     rtCfg.Spec.BGPConfig.Holdtime,
				}
				log.Infof("Add create peer [%+v]", peer)
				peerReq.Request = append(peerReq.Request, &peer)

				peerAf := msTypes.BGPPeerAfSpec{
					Id:          uid.Bytes(),
					PeerAddr:    ip2PDSType(i.GatewayIP),
					LocalAddr:   unknLocal,
					Afi:         msTypes.BGPAfi_BGP_AFI_IPV4,
					Safi:        msTypes.BGPSafi_BGP_SAFI_UNICAST,
					NexthopSelf: false,
					DefaultOrig: false,
				}
				log.Infof("Add create peer AF [%+v]", peerAf)
				peerAFReq.Request = append(peerAFReq.Request, &peerAf)
			}
			continue
		}
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        state,
			PeerAddr:     ip2PDSType(n.IPAddress),
			LocalAddr:    unknLocal,
			RemoteASN:    n.RemoteAS,
			Password:     []byte(n.Password),
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
			KeepAlive:    rtCfg.Spec.BGPConfig.KeepaliveInterval,
			HoldTime:     rtCfg.Spec.BGPConfig.Holdtime,
		}
		log.Infof("Add create peer [%+v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		for _, af := range n.EnableAddressFamilies {
			peerAf := msTypes.BGPPeerAfSpec{
				Id:          uid.Bytes(),
				PeerAddr:    ip2PDSType(n.IPAddress),
				LocalAddr:   unknLocal,
				NexthopSelf: false,
				DefaultOrig: false,
			}
			switch af {
			case "evpn", "l2vpn-evpn":
				peerAf.Afi = msTypes.BGPAfi_BGP_AFI_L2VPN
				peerAf.Safi = msTypes.BGPSafi_BGP_SAFI_EVPN
			case "ipv4-unicast":
				peerAf.Afi = msTypes.BGPAfi_BGP_AFI_IPV4
				peerAf.Safi = msTypes.BGPSafi_BGP_SAFI_UNICAST
			}
			log.Infof("Add create peer AF [%+v]", peerAf)
			peerAFReq.Request = append(peerAFReq.Request, &peerAf)
		}
	}

	if autoConfig {
		// Add the Venice RRs
		dsccfg := infraAPI.GetConfig()
		localIP := unknLocal
		if dsccfg.LoopbackIP != "" {
			localIP = ip2PDSType(dsccfg.LoopbackIP)
		}
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
				State:    pdstypes.AdminState_ADMIN_STATE_ENABLE,
				PeerAddr: ip2PDSType(a[0]),
				// XXX-TBD change to appropriate address
				LocalAddr:    localIP,
				RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
				SendComm:     true,
				SendExtComm:  true,
				ConnectRetry: 5,
				KeepAlive:    rtCfg.Spec.BGPConfig.KeepaliveInterval,
				HoldTime:     rtCfg.Spec.BGPConfig.Holdtime,
			}
			log.Infof("Add create peer [%+v]", peer)
			peerReq.Request = append(peerReq.Request, &peer)

			peerAf := msTypes.BGPPeerAfSpec{
				Id:          uid.Bytes(),
				PeerAddr:    ip2PDSType(a[0]),
				LocalAddr:   localIP,
				Afi:         msTypes.BGPAfi_BGP_AFI_L2VPN,
				Safi:        msTypes.BGPSafi_BGP_SAFI_EVPN,
				NexthopSelf: false,
				DefaultOrig: false,
			}
			log.Infof("Add create peer AF [%+v]", peerAf)
			peerAFReq.Request = append(peerAFReq.Request, &peerAf)
		}
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

	dat, _ := rtCfg.Marshal()

	if err := infraAPI.Store(rtCfg.Kind, rtCfg.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err)
	}
	return nil
}

func updateRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	// check the current config and add/delete as needed
	// Global spec cannot be updated
	// Check configured peers
	var newPeers []*netproto.BGPNeighbor
	var delPeers []*netproto.BGPNeighbor

	if currentRoutingConfig == nil {
		log.Infof("ignoring Routing Config Update [%v]", rtCfg.Name)
		return nil
	}
	// NAPLES can have only one RoutingConfig.
	if rtCfg.Name != currentRoutingConfig.Name {
		log.Infof("ignoring Routing Config [%v]", rtCfg.Name)
		return nil
	}
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
	peerDelReq := msTypes.BGPPeerDeleteRequest{}
	peerAFDelReq := msTypes.BGPPeerAfDeleteRequest{}
	for _, o := range delPeers {
		key := &msTypes.BGPPeerKey{
			PeerAddr: ip2PDSType(o.IPAddress),
		}
		peer := msTypes.BGPPeerKeyHandle{
			IdOrKey: &msTypes.BGPPeerKeyHandle_Key{key},
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerDelReq.Request = append(peerDelReq.Request, &peer)

		for _, af := range o.EnableAddressFamilies {
			switch af {
			case "evpn", "l2vpn-evpn":
				key := &msTypes.BGPPeerAfKey{
					PeerAddr: ip2PDSType(o.IPAddress),
					Afi:      msTypes.BGPAfi_BGP_AFI_L2VPN,
					Safi:     msTypes.BGPSafi_BGP_SAFI_EVPN,
				}
				peerAf := msTypes.BGPPeerAfKeyHandle{
					IdOrKey: &msTypes.BGPPeerAfKeyHandle_Key{key},
				}
				log.Infof("Add Delete peer AF [%+v]", peerAf)
				peerAFDelReq.Request = append(peerAFDelReq.Request, &peerAf)
			case "ipv4-unicast":
				key := &msTypes.BGPPeerAfKey{
					PeerAddr: ip2PDSType(o.IPAddress),
					Afi:      msTypes.BGPAfi_BGP_AFI_IPV4,
					Safi:     msTypes.BGPSafi_BGP_SAFI_UNICAST,
				}
				peerAf := msTypes.BGPPeerAfKeyHandle{
					IdOrKey: &msTypes.BGPPeerAfKeyHandle_Key{key},
				}
				log.Infof("Add Delete peer AF [%+v]", peerAf)
				peerAFDelReq.Request = append(peerAFDelReq.Request, &peerAf)
			}
		}
	}
	ctx := context.TODO()

	pdresp, err := client.BGPPeerDelete(ctx, &peerDelReq)
	if err != nil {
		log.Infof("Peer delete Request returned (%v)[%v]", err, pdresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if pdresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer delete Request returned (%v)[%v]", err, pdresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config Status(%v)", rtCfg.GetKey(), pdresp.ApiStatus)
	}
	log.Infof("Peer delete returned [%v]", pdresp.ApiStatus)

	afdresp, err := client.BGPPeerAfDelete(ctx, &peerAFDelReq)
	if err != nil {
		log.Infof("PeerAF delete request returned (%v)[%v]", err, afdresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Configuring Peer AF Config (%s)", rtCfg.GetKey(), err)
	}
	if afdresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("PeerAF delete request returned (%v)[%v]", err, afdresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer AF Config Status(%v)", rtCfg.GetKey(), afdresp.ApiStatus)
	}
	log.Infof("Peer AF delete returned [%v]", afdresp.ApiStatus)

	peerReq := msTypes.BGPPeerRequest{}
	peerAFReq := msTypes.BGPPeerAfRequest{}
	var state msTypes.AdminState

	// use the UUID from the new config
	uid, err = uuid.FromString(rtCfg.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return err
	}
	unknLocal := &pdstypes.IPAddress{
		Af: pdstypes.IPAF_IP_AF_INET,
	}
	for _, o := range newPeers {
		if o.Shutdown == true {
			state = pdstypes.AdminState_ADMIN_STATE_DISABLE
		} else {
			state = pdstypes.AdminState_ADMIN_STATE_ENABLE
		}
		peer := msTypes.BGPPeerSpec{
			Id:           uid.Bytes(),
			State:        state,
			PeerAddr:     ip2PDSType(o.IPAddress),
			LocalAddr:    unknLocal,
			RemoteASN:    rtCfg.Spec.BGPConfig.ASNumber,
			Password:     []byte(o.Password),
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
			KeepAlive:    rtCfg.Spec.BGPConfig.KeepaliveInterval,
			HoldTime:     rtCfg.Spec.BGPConfig.Holdtime,
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)

		for _, af := range o.EnableAddressFamilies {
			switch af {
			case "evpn", "l2vpn-evpn":
				peerAf := msTypes.BGPPeerAfSpec{
					Id:          uid.Bytes(),
					PeerAddr:    ip2PDSType(o.IPAddress),
					LocalAddr:   unknLocal,
					Afi:         msTypes.BGPAfi_BGP_AFI_L2VPN,
					Safi:        msTypes.BGPSafi_BGP_SAFI_EVPN,
					NexthopSelf: false,
					DefaultOrig: false,
				}
				log.Infof("Add new peer AF [%+v]", peerAf)
				peerAFReq.Request = append(peerAFReq.Request, &peerAf)
			case "ipv4-unicast":
				peerAf := msTypes.BGPPeerAfSpec{
					Id:          uid.Bytes(),
					PeerAddr:    ip2PDSType(o.IPAddress),
					LocalAddr:   unknLocal,
					Afi:         msTypes.BGPAfi_BGP_AFI_IPV4,
					Safi:        msTypes.BGPSafi_BGP_SAFI_UNICAST,
					NexthopSelf: false,
					DefaultOrig: false,
				}
				log.Infof("Add new peer AF [%+v]", peerAf)
				peerAFReq.Request = append(peerAFReq.Request, &peerAf)
			}
		}
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
	dat, _ := rtCfg.Marshal()

	if err := infraAPI.Store(rtCfg.Kind, rtCfg.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err)
	}
	return nil
}

func deleteRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	// NAPLES can have only one RoutingConfig.
	if rtCfg.Name != currentRoutingConfig.Name {
		log.Infof("ignoring Routing Config [%v]", rtCfg.Name)
		return nil
	}

	// Delete configured peers
	peerDelReq := msTypes.BGPPeerDeleteRequest{}
	peerAFDelReq := msTypes.BGPPeerAfDeleteRequest{}
	for _, o := range currentRoutingConfig.Spec.BGPConfig.Neighbors {
		key := &msTypes.BGPPeerKey{
			PeerAddr: ip2PDSType(o.IPAddress),
		}
		peer := msTypes.BGPPeerKeyHandle{
			IdOrKey: &msTypes.BGPPeerKeyHandle_Key{key},
		}
		log.Infof("adding peer to be deleted [%v]", peer)
		peerDelReq.Request = append(peerDelReq.Request, &peer)

		for _, af := range o.EnableAddressFamilies {
			switch af {
			case "evpn", "l2vpn-evpn":
				key := &msTypes.BGPPeerAfKey{
					PeerAddr: ip2PDSType(o.IPAddress),
					Afi:      msTypes.BGPAfi_BGP_AFI_L2VPN,
					Safi:     msTypes.BGPSafi_BGP_SAFI_EVPN,
				}
				peerAf := msTypes.BGPPeerAfKeyHandle{
					IdOrKey: &msTypes.BGPPeerAfKeyHandle_Key{key},
				}
				log.Infof("Add Delete peer AF [%+v]", peerAf)
				peerAFDelReq.Request = append(peerAFDelReq.Request, &peerAf)

			case "ipv4-unicast":
				key := &msTypes.BGPPeerAfKey{
					PeerAddr: ip2PDSType(o.IPAddress),
					Afi:      msTypes.BGPAfi_BGP_AFI_IPV4,
					Safi:     msTypes.BGPSafi_BGP_SAFI_UNICAST,
				}
				peerAf := msTypes.BGPPeerAfKeyHandle{
					IdOrKey: &msTypes.BGPPeerAfKeyHandle_Key{key},
				}
				log.Infof("Add Delete peer AF [%+v]", peerAf)
				peerAFDelReq.Request = append(peerAFDelReq.Request, &peerAf)
			}
		}
	}
	ctx := context.TODO()

	pdresp, err := client.BGPPeerDelete(ctx, &peerDelReq)
	if err != nil {
		log.Infof("Peer Delete Request returned (%v)[%v]", err, pdresp)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config (%s)", rtCfg.GetKey(), err)
	}
	if pdresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
		log.Infof("Peer create Request returned (%v)[%v]", err, pdresp.ApiStatus)
		return errors.Wrapf(types.ErrControlPlaneHanlding, "RoutingConfig: %s | Err: Deleting Peer Config Status(%v)", rtCfg.GetKey(), pdresp.ApiStatus)
	}
	currentRoutingConfig = nil

	if err := infraAPI.Delete(rtCfg.Kind, rtCfg.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err)
	}
	return nil
}

// HandleRouteTable handles crud operations on vrf TODO use VPCClient here
func HandleRouteTable(infraAPI types.InfraAPI, rtSvc pdstypes.RouteSvcClient, oper types.Operation, rtCfg netproto.RouteTable) error {
	log.Infof("HandleRouteTable: oper %v Object: %v", oper, rtCfg)
	switch oper {
	case types.Create, types.Update:
		// PDSA does not need Netagent to create routing table currently. It setups the route table automatically. But still expects
		//  a UUID
		dat, _ := rtCfg.Marshal()
		if err := infraAPI.Store(rtCfg.Kind, rtCfg.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "RouteTable: Update %s | Err: %v", rtCfg.GetKey(), err))
			return errors.Wrapf(types.ErrBoltDBStoreUpdate, "RouteTable: %s | Err: %v", rtCfg.GetKey(), err)
		}
	case types.Delete:
		if err := infraAPI.Delete(rtCfg.Kind, rtCfg.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "RouteTable: Delete %s | Err: %v", rtCfg.GetKey(), err))
			return errors.Wrapf(types.ErrBoltDBStoreDelete, "RouteTable: %s | Err: %v", rtCfg.GetKey(), err)
		}
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
	return nil
}

// UpdateBGPLoopbackConfig updates the BGP config when loopback IP address changes
func UpdateBGPLoopbackConfig(infraAPI types.InfraAPI, client msTypes.BGPSvcClient) {
	// Loopback IP changed. Delete and create the loopback config
	if currentRoutingConfig == nil {
		return
	}
	rtcfg := currentRoutingConfig
	deleteRoutingConfigHandler(infraAPI, client, *currentRoutingConfig)
	createRoutingConfigHandler(infraAPI, client, *rtcfg)
}
