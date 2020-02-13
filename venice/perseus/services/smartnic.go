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
	pegasusClient "github.com/pensando/sw/nic/metaswitch/gen/agent"
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
	Oper int
	peer pegasusClient.BGPPeerSpec
}

type pegasusCfg struct {
	globalOper int
	globalCfg  pegasusClient.BGPSpec
	peers      []pegasusPeerUpdate
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
	}

	// Does not handle change in RouterID or ASN Number

	// limited number of peers for now, okay to O(n^2), change when RR clients are in the picture
	for _, b := range c.config.Spec.BGPConfig.Neighbors {
		for _, b1 := range in.Spec.BGPConfig.Neighbors {
			if b.IPAddress == b1.IPAddress {
				if !reflect.DeepEqual(b, b1) {
					ret.peers = append(ret.peers, pegasusPeerUpdate{Oper: Update, peer: pegasusClient.BGPPeerSpec{}})
				}
			}
		}
	}
	return ret, nil
}

func ip2uint32(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if len(ip) == 0 {
		return 0
	}
	return (((uint32(ip[3])*256)+uint32(ip[2]))*256+uint32(ip[1]))*256 + uint32(ip[0])
	// return (((uint32(ip[0])*256)+uint32(ip[1]))*256+uint32(ip[2]))*256 + uint32(ip[3])
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
		m.pegasusClient = pegasusClient.NewBGPSvcClient(conn)
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
		m.routeSvc = pegasusClient.NewCPRouteSvcClient(conn)
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

		route := pegasusClient.CPStaticRouteRequest{
			Request: []*pegasusClient.CPStaticRouteSpec{
				{
					PrefixLen: 0,
					State:     pegasusClient.AdminState_ADMIN_STATE_ENABLE,
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
		log.Infof("Created static default route got response [%s][%+v]", resp.ApiStatus, rresp.Response)
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
	cfgAsn = evtRtConfig.Spec.BGPConfig.ASNumber
	req := pegasusClient.BGPRequest{
		Request: &pegasusClient.BGPSpec{
			LocalASN: evtRtConfig.Spec.BGPConfig.ASNumber,
			Id:       uid.Bytes(),
			RouterId: ip2uint32(evtRtConfig.Spec.BGPConfig.RouterId),
		},
	}
	ctx := context.Background()
	resp, err := m.pegasusClient.BGPCreate(ctx, &req)
	log.Infof("BGP Global Spec Create received resp (%v)[%+v]", err, resp)

	peerReq := pegasusClient.BGPPeerRequest{}
	for _, n := range evtRtConfig.Spec.BGPConfig.Neighbors {
		peer := pegasusClient.BGPPeerSpec{
			Id:          uid.Bytes(),
			State:       pegasusClient.AdminState_ADMIN_STATE_ENABLE,
			PeerAddr:    ip2PDSType(n.IPAddress),
			RemoteASN:   n.RemoteAS,
			SendComm:    true,
			SendExtComm: true,
		}
		peerReq.Request = append(peerReq.Request, &peer)
	}
	return
}

var once sync.Once
var pReq pegasusClient.BGPPeerRequest

// HandleNodeConfigEvent handles Node updates
func (m *ServiceHandlers) HandleNodeConfigEvent(et kvstore.WatchEventType, evtNodeConfig *cmd.Node) {
	log.Infof("HandleNodeConfigEvent called: Intf: %+v event type: %v", *evtNodeConfig, et)

	if evtNodeConfig.Spec.RoutingConfig != "" {
		if m.updated {
			log.Infof("skipping updating pegasus, second time")
			return
		}
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
		uid, err := uuid.FromString(rtConfig.UUID)
		if err != nil {
			log.Errorf("failed to parse UUID (%v)", err)
			return
		}
		req := pegasusClient.BGPRequest{
			Request: &pegasusClient.BGPSpec{
				LocalASN: rtConfig.Spec.BGPConfig.ASNumber,
				Id:       uid.Bytes(),
				RouterId: ip2uint32(rtConfig.Spec.BGPConfig.RouterId),
			},
		}
		log.Infof("Updating pegasus with Config [%+v]", req.Request)
		resp, err := m.pegasusClient.BGPCreate(ctx, &req)
		if err != nil || resp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("BGP Global Spec Create received resp (%v)[%+v]", err, resp)
			return
		}
		log.Infof("BGP Global Spec Create received resp (%v)[%v, %v]", err, resp.ApiStatus, resp.ApiStatus)

		unknLocal := &pdstypes.IPAddress{
			Af: pdstypes.IPAF_IP_AF_INET,
		}
		peerReq := pegasusClient.BGPPeerRequest{}
		peerAfReq := pegasusClient.BGPPeerAfRequest{}
		for _, n := range rtConfig.Spec.BGPConfig.Neighbors {
			log.Infof("Got neighbor [%+v]", n)
			peer := pegasusClient.BGPPeerSpec{
				Id:           uid.Bytes(),
				State:        pegasusClient.AdminState_ADMIN_STATE_ENABLE,
				PeerAddr:     ip2PDSType(n.IPAddress),
				LocalAddr:    unknLocal,
				RemoteASN:    n.RemoteAS,
				SendComm:     true,
				SendExtComm:  true,
				ConnectRetry: 5,
				Password:     []byte(n.Password),
			}
			// Temp till perseus dynamic add for DSC is available
			if n.SourceFromLoopback {
				peer.RRClient = pegasusClient.BGPPeerRRClient_BGP_PEER_RR_CLIENT
			}
			log.Infof("Add create peer [%+v]", peer)
			peerReq.Request = append(peerReq.Request, &peer)
			for _, a := range n.EnableAddressFamilies {
				peerAf := pegasusClient.BGPPeerAfSpec{
					Id:          uid.Bytes(),
					PeerAddr:    ip2PDSType(n.IPAddress),
					LocalAddr:   unknLocal,
					Disable:     false,
					NexthopSelf: false,
					DefaultOrig: false,
				}
				switch a {
				case network.BGPAddressFamily_EVPN.String():
					peerAf.Afi = pegasusClient.BGPAfi_BGP_AFI_L2VPN
					peerAf.Safi = pegasusClient.BGPSafi_BGP_SAFI_EVPN
				case network.BGPAddressFamily_IPv4Unicast.String():
					peerAf.Afi = pegasusClient.BGPAfi_BGP_AFI_IPV4
					peerAf.Safi = pegasusClient.BGPSafi_BGP_SAFI_UNICAST
				}
				log.Infof("ADD Peer AF [%+v]", peerAf)
				peerAfReq.Request = append(peerAfReq.Request, &peerAf)
			}
		}
		presp, err := m.pegasusClient.BGPPeerCreate(ctx, &peerReq)
		if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("Peer create Request returned (%v)[%+v]", err, presp)
			return
		}

		log.Infof("Peer create Request returned (%v)[%v]", err, presp.ApiStatus)
		afresp, err := m.pegasusClient.BGPPeerAfCreate(ctx, &peerAfReq)
		if err != nil || afresp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("PeerAF create Request returned (%v)[%+v]", err, afresp)
		}
		log.Infof("PeerAF create Request returned (%v)[%v]", err, afresp.ApiStatus)

		m.updated = true
		pReq = peerReq
		once.Do(m.pollStatus)
	}

}

var pollBGPStatus bool

// HandleDebugAction handles debug commands
func (m *ServiceHandlers) HandleDebugAction(action string, params map[string]string) (interface{}, error) {
	switch action {
	case "list-neighbors":
		return m.pegasusClient.BGPPeerGet(context.TODO(), &pegasusClient.BGPPeerRequest{})
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
			req := pReq
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
