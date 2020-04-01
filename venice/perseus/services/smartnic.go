package services

import (
	"context"
	"encoding/binary"
	"fmt"
	"net"
	"sync"
	"time"

	"github.com/pkg/errors"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/nic/metaswitch/clientutils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type configCache struct {
	sync.RWMutex
	config *network.RoutingConfig
}

var cache configCache

func (c *configCache) setConfig(in *network.RoutingConfig) {
	defer c.Unlock()
	c.Lock()
	if in != nil {
		cache.config = ref.DeepCopy(in).(*network.RoutingConfig)
	} else {
		cache.config = nil
	}
}

func (c *configCache) getTimers() (keepalive, holdtime uint32) {
	defer c.RUnlock()
	c.RLock()
	if c.config != nil && c.config.Spec.BGPConfig != nil {
		return c.config.Spec.BGPConfig.KeepaliveInterval, c.config.Spec.BGPConfig.Holdtime
	}
	return 60, 180
}

func (c *configCache) getUUID() []byte {
	defer c.RUnlock()
	c.RLock()
	if c.config != nil {
		uid, err := uuid.FromString(c.config.UUID)
		if err == nil {
			return uid.Bytes()
		}
	}
	return nil
}

func (c *configCache) needUpdate(in *network.RoutingConfig) bool {
	defer c.RUnlock()
	c.RLock()
	if c.config == nil || c.config.Name != in.Name {
		return false
	}
	return true
}

func ip2uint32(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if len(ip) == 0 {
		return 0
	}
	return binary.BigEndian.Uint32(ip)
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
	if !cache.needUpdate(evtRtConfig) {
		return
	}
	var err error
	if et == kvstore.Deleted {
		err = m.configureBGP(context.TODO(), nil)

	} else {
		err = m.configureBGP(context.TODO(), evtRtConfig)
	}
	if err != nil {
		log.Errorf("HandleRoutingConfigEvent: failed to apply config (%s)", err)
	}
	return
}

var once sync.Once
var pReq pdstypes.BGPPeerRequest

func (m *ServiceHandlers) handleAutoConfig(in *network.RoutingConfig) (*network.RoutingConfig, bool) {
	var forceUpdate bool
	if in == nil {
		return nil, forceUpdate
	}
	ret := ref.DeepCopy(in).(*network.RoutingConfig)
	if ret.Spec.BGPConfig == nil {
		return ret, forceUpdate
	}
	var peers []*network.BGPNeighbor
	for _, n := range ret.Spec.BGPConfig.Neighbors {
		if n.DSCAutoConfig {
			m.naplesTemplate = &network.BGPNeighbor{
				MultiHop: n.MultiHop,
				Password: n.Password,
				Shutdown: n.Shutdown,
			}
			continue
		}
		peers = append(peers, n)
	}
	ret.Spec.BGPConfig.Neighbors = peers
	return ret, forceUpdate
}

func (m *ServiceHandlers) configureBGP(ctx context.Context, in *network.RoutingConfig) error {
	oldCfg, _ := m.handleAutoConfig(cache.config)
	rtCfg, forceUpdate := m.handleAutoConfig(in)
	if oldCfg != nil && rtCfg != nil && rtCfg.ResourceVersion == oldCfg.ResourceVersion {
		log.Infof("Resouce version is same ignoring request [%v]", oldCfg.ResourceVersion)
		return nil
	}
	oldlb, newlb := "0.0.0.0", "0.0.0.0"
	if oldCfg != nil {
		oldlb = oldCfg.Spec.BGPConfig.RouterId
	}
	if rtCfg != nil {
		newlb = rtCfg.Spec.BGPConfig.RouterId
	}
	updCfg, err := clientutils.GetBGPConfiguration(oldCfg, rtCfg, oldlb, newlb)
	if err != nil {
		return errors.Wrap(err, "failed to construct pegasus config")
	}

	if updCfg.GlobalOper == clientutils.Delete {
		m.handleBGPConfigDelete()
		CfgAsn = 0
	}
	err = clientutils.UpdateBGPConfiguration(ctx, m.pegasusClient, updCfg)
	if err != nil {
		return errors.Wrap(err, "failed to apply config")
	}
	m.updated = true
	if in == nil {
		cache.setConfig(nil)
	} else {
		cache.setConfig(in)
	}
	switch updCfg.GlobalOper {
	case clientutils.Create:
		CfgAsn = updCfg.Global.Request.LocalASN
		m.configurePeers()
	case clientutils.Update:
		CfgAsn = updCfg.Global.Request.LocalASN
		forceUpdate = true
	case clientutils.Delete:
		// peers are already deleted
		forceUpdate = false
	}
	if forceUpdate {
		m.handleBGPConfigChange()
	}
	return nil
}

// HandleNodeConfigEvent handles Node updates
func (m *ServiceHandlers) HandleNodeConfigEvent(et kvstore.WatchEventType, evtNodeConfig *cmd.Node) {
	log.Infof("HandleNodeConfigEvent called: event type: %v: %+v ", et, *evtNodeConfig)

	ctx := context.TODO()
	if evtNodeConfig.Spec.RoutingConfig != "" {
		var err error

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
			// next node update event will have to fix this.
			return
		}
		err = m.configureBGP(ctx, rtConfig)
		if err != nil {
			log.Errorf("HandleNodeConfigEvent| updating BGP Config failed (%s)", err)
		}
	} else {
		if cache.config != nil {
			log.Infof("deleteing BGP config from node")
			rtcfg, _ := m.handleAutoConfig(cache.config)
			updCfg, err := clientutils.GetBGPConfiguration(rtcfg, nil, "0.0.0.0", "0.0.0.0")
			if err != nil {
				log.Errorf("failed to get pegasus config (%s)", err)
				return
			}
			m.handleBGPConfigDelete()
			err = clientutils.UpdateBGPConfiguration(ctx, m.pegasusClient, updCfg)
			if err != nil {
				log.Errorf("failed to update BGP config (%s)", err)
				return
			}
		}
		cache.config = nil
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
