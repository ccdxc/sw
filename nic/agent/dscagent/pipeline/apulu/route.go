// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"encoding/json"
	"fmt"
	"net"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	msTypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/nic/metaswitch/clientutils"
	"github.com/pensando/sw/venice/utils/log"
)

const oobIntfName = "oob_mnic0"

// currentRoutingConfig is cached routing config that has been configured on metaswitch
var currentRoutingConfig *netproto.RoutingConfig
var curLbIP string

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

func dumpNice(in interface{}) string {
	out, _ := json.MarshalIndent(in, "", "  ")
	return string(out)
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

// expandRoutingConfig evaluates the routing config taking into account auto-config and returns an expanded confguration
func expandRoutingConfig(infraAPI types.InfraAPI, lbIp string, rtCfg *netproto.RoutingConfig) *netproto.RoutingConfig {
	ret := &netproto.RoutingConfig{
		ObjectMeta: rtCfg.ObjectMeta,
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{},
		},
	}

	if rtCfg.Spec.BGPConfig == nil {
		return ret
	}
	var autoConfig bool

	dsccfg := infraAPI.GetConfig()

	ret.Spec.BGPConfig.RouterId = lbIp
	ret.Spec.BGPConfig.Holdtime, ret.Spec.BGPConfig.KeepaliveInterval = rtCfg.Spec.BGPConfig.Holdtime, rtCfg.Spec.BGPConfig.KeepaliveInterval
	ret.Spec.BGPConfig.ASNumber = rtCfg.Spec.BGPConfig.ASNumber
	var expCfgs []string
	for _, n := range rtCfg.Spec.BGPConfig.Neighbors {
		rip := net.ParseIP(n.IPAddress)

		// if set to 0.0.0.0 auto configure the neighborts learnt via DHCP
		if rip.IsUnspecified() {
			autoConfig = true
			if len(n.EnableAddressFamilies) != 1 {
				log.Errorf("number of address families not 1 in auto-config")
				continue
			}
			switch n.EnableAddressFamilies[0] {
			case "evpn", "l2vpn-evpn":
				for _, c := range dsccfg.Controllers {
					h, _, err := net.SplitHostPort(c)
					if err != nil {
						log.Errorf("hostport returned error for [%v](%v)", c, err)
					}
					a, err := net.LookupHost(h)
					if err != nil {
						log.Errorf("LookupHost returned error for [%v](%v)", a, err)
						continue
					}
					peer := new(netproto.BGPNeighbor)
					*peer = *n
					peer.IPAddress = a[0]
					peer.RemoteAS = rtCfg.Spec.BGPConfig.ASNumber
					ret.Spec.BGPConfig.Neighbors = append(ret.Spec.BGPConfig.Neighbors, peer)
					log.Infof("Expand - peer [%+v]", peer)
					expCfgs = append(expCfgs, fmt.Sprintf("%s-%v", peer.IPAddress, peer.EnableAddressFamilies))
				}
			case "ipv4-unicast":
				for _, i := range dsccfg.DSCInterfaceIPs {
					peer := new(netproto.BGPNeighbor)
					*peer = *n
					peer.IPAddress = i.GatewayIP
					peer.EnableAddressFamilies = make([]string, len(n.EnableAddressFamilies))
					copy(peer.EnableAddressFamilies, n.EnableAddressFamilies)
					ret.Spec.BGPConfig.Neighbors = append(ret.Spec.BGPConfig.Neighbors, peer)
					log.Infof("Expand - peer [%+v]", peer)
					expCfgs = append(expCfgs, fmt.Sprintf("%s-%v", peer.IPAddress, peer.EnableAddressFamilies))
				}

			}
			continue
		}
		// Does not need  deep copy as of now
		peer := new(netproto.BGPNeighbor)
		*peer = *n
		peer.EnableAddressFamilies = make([]string, len(n.EnableAddressFamilies))
		copy(peer.EnableAddressFamilies, n.EnableAddressFamilies)
		ret.Spec.BGPConfig.Neighbors = append(ret.Spec.BGPConfig.Neighbors, peer)
		log.Infof("Expand - peer [%+v]", peer)
	}

	if autoConfig {

	}
	log.Infof("expandedConfig: [%d] peers - %v", len(expCfgs), expCfgs)
	return ret
}

func getRouterID(infraAPI types.InfraAPI) string {
	rid := ""
	dsccfg := infraAPI.GetConfig()
	if dsccfg.LoopbackIP != "" {
		rid = dsccfg.LoopbackIP
	} else {
		if len(dsccfg.DSCInterfaceIPs) > 0 {
			rip := dsccfg.DSCInterfaceIPs[0].IPAddress
			ip, _, err := net.ParseCIDR(rip)
			if err == nil {
				rid = ip.String()
			} else {
				log.Errorf("could not parse ip [%v](%s)", rip, err)
			}
		} else {
			intf, err := net.InterfaceByName(oobIntfName)
			if err != nil {
				return ""
			}
			addrs, err := intf.Addrs()
			if err != nil {
				return ""
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
	return rid
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

	dsccfg := infraAPI.GetConfig()
	ctx := context.TODO()
	log.Infof("RID calculation [%s][%s] Interfaces [%v]", rid, dsccfg.LoopbackIP, dsccfg.DSCInterfaceIPs)

	rid = getRouterID(infraAPI)
	if rid == "" {
		log.Errorf("BGP Create could not get IP Address for router ID ")
		return errors.Wrap(types.ErrInvalidIP, "could not determine Router ID")
	}
	log.Infof("Choosing RID [%v]", rid)
	newCfg := expandRoutingConfig(infraAPI, rid, &rtCfg)
	newCfg.Spec.BGPConfig.RouterId = rid
	cfg, err := clientutils.GetBGPConfiguration(nil, newCfg, "0.0.0.0", rid)
	if err != nil {
		log.Errorf("BGPConfig: GetConfig failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] GetConfig failed | Err: %s", rtCfg.GetKey(), err)
	}
	log.Infof("BGP Config is [%v]", dumpNice(cfg))
	err = clientutils.UpdateBGPConfiguration(ctx, client, cfg)
	if err != nil {
		log.Errorf("BGPConfig: Create BGP failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] Create BGP failed | Err: %s", rtCfg.GetKey(), err)
	}
	currentRoutingConfig = &rtCfg
	curLbIP = rid

	dat, _ := rtCfg.Marshal()

	if err := infraAPI.Store(rtCfg.Kind, rtCfg.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "RoutingConfig: %s | Err: %v", rtCfg.GetKey(), err)
	}
	return nil
}

func updateRoutingConfigHandler(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, rtCfg netproto.RoutingConfig) error {
	if currentRoutingConfig == nil {
		log.Infof("ignoring Routing Config Update [%v]", rtCfg.Name)
		return nil
	}
	// NAPLES can have only one RoutingConfig.
	if rtCfg.Name != currentRoutingConfig.Name {
		log.Infof("ignoring Routing Config [%v]", rtCfg.Name)
		return nil
	}
	rip := net.ParseIP(rtCfg.Spec.BGPConfig.RouterId)

	// For now till NPM does proper filtering of RoutingConfig only accept Routing Config with AutoConfig
	if !rip.IsUnspecified() {
		log.Infof("ignoring Routing Config with set Router ID [%v]", rtCfg.Name)
		return nil
	}

	rid := getRouterID(infraAPI)
	if rid == "" {
		log.Errorf("BGP Create could not get IP Address for router ID ")
		return errors.Wrap(types.ErrInvalidIP, "could not determine Router ID")
	}
	oldCfg := expandRoutingConfig(infraAPI, rid, currentRoutingConfig)
	newCfg := expandRoutingConfig(infraAPI, rid, &rtCfg)

	cfg, err := clientutils.GetBGPConfiguration(oldCfg, newCfg, rid, rid)
	if err != nil {
		log.Errorf("BGPConfig: GetConfig failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] GetConfig failed | Err: %s", rtCfg.GetKey(), err)
	}
	err = clientutils.UpdateBGPConfiguration(context.TODO(), client, cfg)
	if err != nil {
		log.Errorf("BGPConfig: Update BGP failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] Update BGP failed | Err: %s", rtCfg.GetKey(), err)
	}
	currentRoutingConfig = &rtCfg
	curLbIP = rid

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

	rip := net.ParseIP(rtCfg.Spec.BGPConfig.RouterId)

	// For now till NPM does proper filtering of RoutingConfig only accept Routing Config with AutoConfig
	if !rip.IsUnspecified() {
		log.Infof("ignoring Routing Config with set Router ID [%v]", rtCfg.Name)
		return nil
	}

	oldCfg := expandRoutingConfig(infraAPI, curLbIP, currentRoutingConfig)
	cfg, err := clientutils.GetBGPConfiguration(oldCfg, nil, curLbIP, curLbIP)
	if err != nil {
		log.Errorf("BGPConfig: GetConfig failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] GetConfig failed | Err: %s", rtCfg.GetKey(), err)
	}
	err = clientutils.UpdateBGPConfiguration(context.TODO(), client, cfg)
	if err != nil {
		log.Errorf("BGPConfig: Update BGP failed | Err: %s", err)
		return errors.Wrapf(types.ErrBadRequest, "BGPConfig: [%s] Update BGP failed | Err: %s", rtCfg.GetKey(), err)
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
func UpdateBGPLoopbackConfig(infraAPI types.InfraAPI, client msTypes.BGPSvcClient, oldLb, newLb string) {
	// Loopback IP changed. Delete and create the loopback config
	if currentRoutingConfig == nil {
		log.Infof("New Loobback IP is %v", newLb)
		curLbIP = newLb
		return
	}
	rtcfg := currentRoutingConfig
	deleteRoutingConfigHandler(infraAPI, client, *currentRoutingConfig)
	log.Infof("New Loobback IP is %v", newLb)
	createRoutingConfigHandler(infraAPI, client, *rtcfg)
}
