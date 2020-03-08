package services

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"sync"
	"time"

	"github.com/satori/go.uuid"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	// "github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/rpckit"

	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type configCache struct {
	config *network.RoutingConfig
}

var cache configCache

// API Oper for pegasus updates
const (
	None   = 0
	Create = 1
	Update = 2
	Delete = 3
)

type pegasusPeerUpdate struct {
	Oper   int
	peer   pdstypes.BGPPeerSpec
	addAfs []string
	delAfs []string
}

type pegasusCfg struct {
	globalOper int
	globalCfg  pdstypes.BGPSpec
	peers      []*pegasusPeerUpdate
}

func (c *configCache) setConfig(in network.RoutingConfig) {
	cache.config = &in
}

func (c *configCache) getPegasusConfig(in *network.RoutingConfig) (*pegasusCfg, error) {
	ret := &pegasusCfg{}
	if c.config == nil {
		ret.globalOper = Create
	} else {
		ret.globalOper = Update
		if c.config.Spec.BGPConfig.RouterId == in.Spec.BGPConfig.RouterId && c.config.Spec.BGPConfig.ASNumber == in.Spec.BGPConfig.ASNumber {
			ret.globalOper = None
		}
	}

	uid, err := uuid.FromString(in.UUID)
	if err != nil {
		return nil, fmt.Errorf("failed to parse UUID [%s]", in.UUID)
	}
	ret.globalCfg.Id = uid.Bytes()
	ret.globalCfg.LocalASN = in.Spec.BGPConfig.ASNumber
	ret.globalCfg.RouterId = ip2uint32(in.Spec.BGPConfig.RouterId)

	unknLocal := &pdstypes.IPAddress{
		Af: pdstypes.IPAF_IP_AF_INET,
	}

	// limited number of peers for now, okay to O(n^2), change when RR clients are in the picture
	if c.config != nil {
	outer:
		for _, b := range c.config.Spec.BGPConfig.Neighbors {
			found := false
			uid, err := uuid.FromString(c.config.UUID)
			if err != nil {
				log.Errorf("failed to parse UUID [%v]", err)
				continue outer
			}
			for _, b1 := range in.Spec.BGPConfig.Neighbors {
				if b.IPAddress == b1.IPAddress {
					found = true
					if !reflect.DeepEqual(b, b1) {
						peer := pegasusPeerUpdate{Oper: Update, peer: pdstypes.BGPPeerSpec{
							Id:           uid.Bytes(),
							State:        pdstypes.AdminState_ADMIN_STATE_ENABLE,
							PeerAddr:     ip2PDSType(b1.IPAddress),
							LocalAddr:    unknLocal,
							RemoteASN:    b1.RemoteAS,
							SendComm:     true,
							SendExtComm:  true,
							ConnectRetry: 5,
							HoldTime:     in.Spec.BGPConfig.Holdtime,
							KeepAlive:    in.Spec.BGPConfig.KeepaliveInterval,
							Password:     []byte(b1.Password),
						}}
						// Add Afs
						for _, af1 := range b.EnableAddressFamilies {
							found := false
							for _, af2 := range b1.EnableAddressFamilies {
								if af1 == af2 {
									found = true
									break
								}
							}
							if !found {
								peer.addAfs = append(peer.delAfs, af1)
							}
						}

						// Add Afs
						for _, af1 := range b1.EnableAddressFamilies {
							found := false
							for _, af2 := range b.EnableAddressFamilies {
								if af1 == af2 {
									found = true
									break
								}
							}
							if !found {
								peer.addAfs = append(peer.addAfs, af1)
							}
						}

						ret.peers = append(ret.peers, &peer)
					}
				}
			}
			if !found {
				ret.peers = append(ret.peers, &pegasusPeerUpdate{Oper: Delete, peer: pdstypes.BGPPeerSpec{
					Id:           uid.Bytes(),
					State:        pdstypes.AdminState_ADMIN_STATE_ENABLE,
					PeerAddr:     ip2PDSType(b.IPAddress),
					LocalAddr:    unknLocal,
					RemoteASN:    b.RemoteAS,
					SendComm:     true,
					SendExtComm:  true,
					ConnectRetry: 5,
					HoldTime:     in.Spec.BGPConfig.Holdtime,
					KeepAlive:    in.Spec.BGPConfig.KeepaliveInterval,
					Password:     []byte(b.Password),
				}, delAfs: b.EnableAddressFamilies})
			}
		}
	}

	// Look for new neighbors
	for _, b := range in.Spec.BGPConfig.Neighbors {
		found := false
		if c.config != nil {
			for _, b1 := range c.config.Spec.BGPConfig.Neighbors {
				if b.IPAddress == b1.IPAddress {
					found = true
					break
				}
			}
		}

		if !found {
			uid, err := uuid.FromString(in.UUID)
			if err != nil {
				log.Errorf("failed to parse UUID [%v]", err)
				continue
			}
			ret.peers = append(ret.peers, &pegasusPeerUpdate{Oper: Create, peer: pdstypes.BGPPeerSpec{
				Id:           uid.Bytes(),
				State:        pdstypes.AdminState_ADMIN_STATE_ENABLE,
				PeerAddr:     ip2PDSType(b.IPAddress),
				LocalAddr:    unknLocal,
				RemoteASN:    b.RemoteAS,
				SendComm:     true,
				SendExtComm:  true,
				ConnectRetry: 5,
				HoldTime:     in.Spec.BGPConfig.Holdtime,
				KeepAlive:    in.Spec.BGPConfig.KeepaliveInterval,
				Password:     []byte(b.Password),
			}, addAfs: b.EnableAddressFamilies})
		}
	}
	return ret, nil
}

func (c *configCache) getTimers() (keepalive, holdtime uint32) {
	if c.config != nil && c.config.Spec.BGPConfig != nil {
		return c.config.Spec.BGPConfig.KeepaliveInterval, c.config.Spec.BGPConfig.KeepaliveInterval
	}
	return 60, 180
}

func ip2uint32(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if len(ip) == 0 {
		return 0
	}
	return (((uint32(ip[3])*256)+uint32(ip[2]))*256+uint32(ip[1]))*256 + uint32(ip[0])
}

func ip2PDSType(ipstr string) *pdstypes.IPAddress {
	return &pdstypes.IPAddress{
		Af:     pdstypes.IPAF_IP_AF_INET,
		V4OrV6: &pdstypes.IPAddress_V4Addr{V4Addr: ip2uint32(ipstr)},
	}
}

func (m *ServiceHandlers) connectToPegasus() {
	log.Infof("connecting to Pegasus")
	for {
		conn, err := grpc.Dial(m.pegasusURL, grpc.WithInsecure())
		if err != nil {
			log.Errorf("failed to create client (%s)", err)
			time.Sleep(time.Second)
			continue
		}
		m.pegasusClient = pdstypes.NewBGPSvcClient(conn)
		break
	}
}

func (m *ServiceHandlers) setupLBIf() {
	lbuid := uuid.NewV4().Bytes()
	lbreq := pdstypes.InterfaceRequest{
		Request: []*pdstypes.InterfaceSpec{
			{
				Id:          lbuid,
				Type:        pdstypes.IfType_IF_TYPE_LOOPBACK,
				AdminStatus: pdstypes.IfStatus_IF_STATUS_UP,
				Ifinfo: &pdstypes.InterfaceSpec_LoopbackIfSpec{
					LoopbackIfSpec: &pdstypes.LoopbackIfSpec{},
				},
			},
		},
	}
	for {
		conn, err := grpc.Dial(m.pegasusURL, grpc.WithInsecure())
		if err != nil {
			log.Errorf("failed to connect to if client 9%s)", err)
			time.Sleep(time.Second)
			continue
		}
		m.ifClient = pdstypes.NewIfSvcClient(conn)
		m.routeSvc = pdstypes.NewCPRouteSvcClient(conn)
		break
	}
	retries := 0
	for retries < 5 {
		resp, err := m.ifClient.InterfaceCreate(context.TODO(), &lbreq)
		if err != nil {
			log.Errorf("failed to create Loopback (%s)", err)
			retries++
			time.Sleep(time.Second)
			continue
		}
		if resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("failed to create Loopback Status: (%s)", resp.ApiStatus)
			retries++
			time.Sleep(time.Second)
			continue
		}
		log.Infof("Created loopback interface got response [%s][%+v]", resp.ApiStatus, resp.Response)

		route := pdstypes.CPStaticRouteRequest{
			Request: []*pdstypes.CPStaticRouteSpec{
				{
					PrefixLen: 0,
					State:     pdstypes.AdminState_ADMIN_STATE_ENABLE,
					Override:  false,
					DestAddr: &pdstypes.IPAddress{
						Af: pdstypes.IPAF_IP_AF_INET,
						V4OrV6: &pdstypes.IPAddress_V4Addr{
							V4Addr: ip2uint32("0.0.0.0"),
						},
					},
					NextHopAddr: &pdstypes.IPAddress{
						Af: pdstypes.IPAF_IP_AF_INET,
						V4OrV6: &pdstypes.IPAddress_V4Addr{
							V4Addr: ip2uint32("0.0.0.0"),
						},
					},
					InterfaceId: lbuid,
				},
			},
		}
		log.Infof("Setting Static Route [%+v]", route)
		rresp, err := m.routeSvc.CPStaticRouteCreate(context.TODO(), &route)
		if err != nil {
			log.Errorf("failed to create static default route (%s)", err)
			retries++
			time.Sleep(time.Second)
			continue
		}
		if rresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("failed to create static default route Status: (%s)", rresp.ApiStatus)
			retries++
			time.Sleep(time.Second)
			continue
		}
		log.Infof("Created static default route got response [%s]", resp.ApiStatus)
		break
	}
}

// HandleRoutingConfigEvent handles SmartNIC updates
func (m *ServiceHandlers) HandleRoutingConfigEvent(et kvstore.WatchEventType, evtRtConfig *network.RoutingConfig) {
	log.Infof("HandleRoutingConfigEvent called: Intf: %+v event type: %v", *evtRtConfig, et)

	uid, err := uuid.FromString(evtRtConfig.UUID)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return
	}
	req := pdstypes.BGPRequest{
		Request: &pdstypes.BGPSpec{
			LocalASN: evtRtConfig.Spec.BGPConfig.ASNumber,
			Id:       uid.Bytes(),
			RouterId: ip2uint32(evtRtConfig.Spec.BGPConfig.RouterId),
		},
	}
	ctx := context.Background()
	resp, err := m.pegasusClient.BGPCreate(ctx, &req)
	log.Infof("BGP Global Spec Create received resp (%v)[%+v]", err, resp)

	peerReq := pdstypes.BGPPeerRequest{}
	for _, n := range evtRtConfig.Spec.BGPConfig.Neighbors {
		peer := pdstypes.BGPPeerSpec{
			Id:          uid.Bytes(),
			State:       pdstypes.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:    ip2PDSType(n.IPAddress),
			RemoteASN:   n.RemoteAS,
			SendComm:    true,
			SendExtComm: true,
			HoldTime:    evtRtConfig.Spec.BGPConfig.Holdtime,
			KeepAlive:   evtRtConfig.Spec.BGPConfig.KeepaliveInterval,
		}
		peerReq.Request = append(peerReq.Request, &peer)
	}
	return
}

var once sync.Once
var pReq pdstypes.BGPPeerRequest

// HandleNodeConfigEvent handles Node updates
func (m *ServiceHandlers) HandleNodeConfigEvent(et kvstore.WatchEventType, evtNodeConfig *cmd.Node) {
	log.Infof("HandleNodeConfigEvent called: Intf: %+v event type: %v", *evtNodeConfig, et)

	if evtNodeConfig.Spec.RoutingConfig != "" {
		var err error
		ctx := context.TODO()
		for m.apiclient == nil {
			if env.ResolverClient != nil {
				m.apiclient, err = apiclient.NewGrpcAPIClient(globals.Perseus, globals.APIServer, env.Logger, rpckit.WithBalancer(balancer.New(env.ResolverClient)))
			} else {
				m.apiclient, err = apiclient.NewGrpcAPIClient(globals.Perseus, globals.APIServer, env.Logger, rpckit.WithRemoteServerName(globals.APIServer))
			}
			if err != nil {
				log.Infof("failed to create API Client (%s)", err)
				time.Sleep(time.Second)
				continue
			}
			break
		}
		rtConfig, err := m.apiclient.NetworkV1().RoutingConfig().Get(ctx, &api.ObjectMeta{Name: evtNodeConfig.Spec.RoutingConfig})
		if err != nil {
			log.Errorf("failed to get routing config [%v](%s)", evtNodeConfig.Spec.RoutingConfig, err)
		}

		updCfg, err := cache.getPegasusConfig(rtConfig)
		if err != nil {
			log.Errorf("failed to get pegasus config 9%s)", err)
			return
		}
		req := pdstypes.BGPRequest{
			Request: &updCfg.globalCfg,
		}
		switch updCfg.globalOper {
		case Create:
			resp, err := m.pegasusClient.BGPCreate(ctx, &req)
			if err != nil || resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("BGP Global Spec Create received resp (%v)[%+v]", err, resp)
				return
			}
			log.Infof("BGP Global Spec Create received resp (%v)[%v, %v]", err, resp.ApiStatus, resp.ApiStatus)
			CfgAsn = updCfg.globalCfg.LocalASN
			m.configurePeers()
			log.Infof("BGP setting Local ASN to [%v]", CfgAsn)
		case Update:
			resp, err := m.pegasusClient.BGPUpdate(ctx, &req)
			if err != nil || resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("BGP Global Spec Update received resp (%v)[%+v]", err, resp)
				return
			}
			CfgAsn = updCfg.globalCfg.LocalASN
			m.handleBGPConfigChange()
		case Delete:
			uid, err := uuid.FromString(rtConfig.UUID)
			if err != nil {
				log.Errorf("failed to parse UUID [%s]", rtConfig.UUID)
				return
			}
			dreq := pdstypes.BGPDeleteRequest{}
			dreq.Request = &pdstypes.BGPKeyHandle{
				Id: uid.Bytes(),
			}
			log.Infof("delete bgp [%+v]", dreq)
			resp, err := m.pegasusClient.BGPDelete(ctx, &dreq)
			if err != nil || resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("BGP Global Spec Delete received resp (%v)[%+v]", err, resp)
				return
			}
			CfgAsn = 0
		}

		addPeerReq := pdstypes.BGPPeerRequest{}
		delPeerReq := pdstypes.BGPPeerDeleteRequest{}
		updPeerReq := pdstypes.BGPPeerRequest{}

		addPeerAfReq := pdstypes.BGPPeerAfRequest{}
		delPeerAfReq := pdstypes.BGPPeerAfDeleteRequest{}
		for _, p := range updCfg.peers {
			switch p.Oper {
			case Create:
				addPeerReq.Request = append(addPeerReq.Request, &p.peer)
				log.Infof("Add Peer [%+v]", p.peer)
			case Update:
				updPeerReq.Request = append(updPeerReq.Request, &p.peer)
				log.Infof("Update Peer [%+v]", p.peer)
			case Delete:
				peer := pdstypes.BGPPeerKeyHandle{
					IdOrKey: &pdstypes.BGPPeerKeyHandle_Key{Key: &pdstypes.BGPPeerKey{PeerAddr: p.peer.PeerAddr}},
				}
				delPeerReq.Request = append(delPeerReq.Request, &peer)
				log.Infof("Del Peer [%+v]", p.peer)
			}
			for _, af := range p.addAfs {
				peerAf := pdstypes.BGPPeerAfSpec{
					Id:          p.peer.Id,
					PeerAddr:    p.peer.PeerAddr,
					LocalAddr:   p.peer.LocalAddr,
					NexthopSelf: false,
					DefaultOrig: false,
				}
				switch af {
				case network.BGPAddressFamily_EVPN.String():
					peerAf.Afi = pdstypes.BGPAfi_BGP_AFI_L2VPN
					peerAf.Safi = pdstypes.BGPSafi_BGP_SAFI_EVPN
				case network.BGPAddressFamily_IPv4Unicast.String():
					peerAf.Afi = pdstypes.BGPAfi_BGP_AFI_IPV4
					peerAf.Safi = pdstypes.BGPSafi_BGP_SAFI_UNICAST
				}
				log.Infof("ADD Peer AF [%+v]", peerAf)
				addPeerAfReq.Request = append(addPeerAfReq.Request, &peerAf)
			}
			for _, af := range p.delAfs {
				key := &pdstypes.BGPPeerAfKey{
					PeerAddr:  p.peer.PeerAddr,
					LocalAddr: p.peer.LocalAddr,
				}
				switch af {
				case network.BGPAddressFamily_EVPN.String():
					key.Afi = pdstypes.BGPAfi_BGP_AFI_L2VPN
					key.Safi = pdstypes.BGPSafi_BGP_SAFI_EVPN
				case network.BGPAddressFamily_IPv4Unicast.String():
					key.Afi = pdstypes.BGPAfi_BGP_AFI_IPV4
					key.Safi = pdstypes.BGPSafi_BGP_SAFI_UNICAST
				}
				peerAf := pdstypes.BGPPeerAfKeyHandle{
					IdOrKey: &pdstypes.BGPPeerAfKeyHandle_Key{Key: &pdstypes.BGPPeerAfKey{PeerAddr: key.PeerAddr, LocalAddr: key.LocalAddr, Afi: key.Afi, Safi: key.Safi}},
				}
				log.Infof("Del Peer AF [%+v]", peerAf)
				delPeerAfReq.Request = append(delPeerAfReq.Request, &peerAf)
			}
		}

		if len(addPeerReq.Request) > 0 {
			presp, err := m.pegasusClient.BGPPeerCreate(ctx, &addPeerReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer create Request returned (%v)[%+v]", err, presp)
				return
			}
			log.Infof("Peer create request returned[%v/%v]", err, presp.ApiStatus)
		}
		if len(updPeerReq.Request) > 0 {
			presp, err := m.pegasusClient.BGPPeerUpdate(ctx, &updPeerReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer update Request returned (%v)[%+v]", err, presp)
				return
			}
			log.Infof("Peer update request returned[%v/%v]", err, presp.ApiStatus)
		}
		if len(delPeerReq.Request) > 0 {
			presp, err := m.pegasusClient.BGPPeerDelete(ctx, &delPeerReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer delete Request returned (%v)[%+v]", err, presp)
				return
			}
			log.Infof("Peer delete request returned[%v/%v]", err, presp.ApiStatus)
		}

		if len(addPeerAfReq.Request) > 0 {
			afresp, err := m.pegasusClient.BGPPeerAfCreate(ctx, &addPeerAfReq)
			if err != nil || afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer create Request returned (%v)[%+v]", err, afresp)
				return
			}
			log.Infof("Peer AF create request returned[%v/%v]", err, afresp.ApiStatus)
		}
		if len(delPeerAfReq.Request) > 0 {
			afresp, err := m.pegasusClient.BGPPeerAfDelete(ctx, &delPeerAfReq)
			if err != nil || afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer delete Request returned (%v)[%+v]", err, afresp)
				return
			}
			log.Infof("Peer AF delete request returned[%v/%v]", err, afresp.ApiStatus)
		}

		m.updated = true
		cache.setConfig(*rtConfig)
		once.Do(m.pollStatus)
	}

}

var pollBGPStatus bool

// HandleDebugAction handles debug commands
func (m *ServiceHandlers) HandleDebugAction(action string, params map[string]string) (interface{}, error) {
	switch action {
	case "list-neighbors":
		return m.pegasusClient.BGPPeerGet(context.TODO(), &pdstypes.BGPPeerGetRequest{})
	case "poll-bgp-status":
		pollBGPStatus = true
		return "set to periodically poll BGP status", nil
	default:
		return fmt.Sprintf("unknown action [%v]", action), nil
	}

}

func (m *ServiceHandlers) pollStatus() {
	go func() {
		for {
			time.Sleep(30 * time.Second)
			if !pollBGPStatus {
				return
			}
			var req pdstypes.BGPPeerGetRequest
			for _, v := range pReq.Request {
				peer := pdstypes.BGPPeerKeyHandle{
					IdOrKey: &pdstypes.BGPPeerKeyHandle_Key{Key: &pdstypes.BGPPeerKey{PeerAddr: v.PeerAddr, LocalAddr: v.LocalAddr}},
				}
				req.Request = append(req.Request, &peer)
			}
			resp, err := m.pegasusClient.BGPPeerGet(context.TODO(), &req)
			if err != nil {
				log.Errorf("failed to get BGP Peer Get All (%ss)", err)
			} else {
				log.Infof("Got BGP Peer [%v][%v]", resp.ApiStatus, len(resp.Response))
				for _, v := range resp.Response {
					log.Infof("Peer: [%+v]", *v)
				}
			}
		}
	}()
}
