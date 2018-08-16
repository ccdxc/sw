package pkg

import (
	"encoding/binary"
	"encoding/json"
	"io/ioutil"
	"log"
	"net"
	"os"
	"strconv"
	"strings"

	"github.com/pkg/errors"

	App "github.com/pensando/sw/nic/e2etests/go/infra"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
)

const (
	TrafficUplinkToUplink = 1
	TrafficUplinkToHost   = 2
	TrafficHostToHost     = 3

	NumFlows       = 1
	MaxTrafficPair = 1

	UplinkIntefaceName = "uplink"
)

func lastIPv4Addr(n *net.IPNet) net.IP {
	ip := make(net.IP, len(n.IP.To4()))
	binary.BigEndian.PutUint32(ip, binary.BigEndian.Uint32(n.IP.To4())|^binary.BigEndian.Uint32(net.IP(n.Mask).To4()))
	ip[3] -= 1
	return ip
}

func defaultRoute(nw string) string {
	_, sip, _ := net.ParseCIDR(nw)
	return lastIPv4Addr(sip).String()
}

func getNetworkFromConfig(nwName string, networks []netproto.Network) *netproto.Network {
	for _, network := range networks {
		if network.GetName() == nwName {
			return &network
		}
	}
	return nil
}

var hpingRun = func(ns *App.NS, cmd []string) error {
	if _, err := ns.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Traffic generation failed")
	}
	return nil
}

func setUpNs(ep *netproto.Endpoint, nw *netproto.Network, intf string) (*App.NS, error) {
	ns := App.NewNS(ep.GetName())
	ns.Init(false)
	ns.AttachInterface(intf)
	ns.AddVlan(intf, int(nw.Spec.GetVlanID()))
	ns.SetMacAddress(intf, ep.Spec.GetMacAddress(), int(nw.Spec.GetVlanID()))
	ipAddr := strings.Split(ep.Spec.GetIPv4Address(), "/")[0]
	prefixLen := strings.Split(nw.Spec.GetIPv4Subnet(), "/")[1]
	intPrefix, _ := strconv.Atoi(prefixLen)
	ns.SetIPAddress(intf, ipAddr, intPrefix, int(nw.Spec.GetVlanID()))
	return ns, nil
}

func setUpRoute(srcNs *App.NS, dstNs *App.NS,
	epPair epPairInfo) error {

	routeSetup := func(ns *App.NS, dstIp string, nexHop string, nextHopMac string) {
		cmd := []string{"route", "add", dstIp, "gw", nexHop}
		ns.RunCommand(cmd, 0, false)
		cmd = []string{"arp", "-s", nexHop, nextHopMac}
		ns.RunCommand(cmd, 0, false)

	}

	routeSetup(srcNs, epPair.dstEp.Spec.IPv4Address, defaultRoute(epPair.srcNw.Spec.GetIPv4Subnet()),
		epPair.dstEp.Spec.MacAddress)
	routeSetup(dstNs, epPair.srcEp.Spec.IPv4Address, defaultRoute(epPair.dstNw.Spec.GetIPv4Subnet()),
		epPair.srcEp.Spec.MacAddress)
	return nil

}

func deleteRoute(srcNs *App.NS,
	srcEp *netproto.Endpoint, dstNs *App.NS, dstEp *netproto.Endpoint) error {

	routeDelete := func(ns *App.NS, dstIp string, nexHop string) {
		cmd := []string{"route", "delete", dstIp}
		srcNs.RunCommand(cmd, 0, false)
		cmd = []string{"arp", "-d", nexHop}
		srcNs.RunCommand(cmd, 0, false)

	}

	routeDelete(srcNs, dstEp.Spec.IPv4Address, defaultRoute(srcEp.Spec.IPv4Address))
	routeDelete(dstNs, srcEp.Spec.IPv4Address, defaultRoute(dstEp.Spec.IPv4Address))
	return nil

}

func runTrafficBetweenEps(epPair epPairInfo) error {

	srcNs, _ := setUpNs(epPair.srcEp, epPair.srcNw, epPair.srcLink)
	dstNs, _ := setUpNs(epPair.dstEp, epPair.dstNw, epPair.dstLink)
	defer srcNs.Delete()
	defer dstNs.Delete()

	if epPair.srcNw != epPair.dstNw {
		setUpRoute(srcNs, dstNs, epPair)
		defer deleteRoute(srcNs, epPair.srcEp, dstNs, epPair.dstEp)
	}

	ipaddr := strings.Split(epPair.dstEp.Spec.GetIPv4Address(), "/")[0]
	cmd := []string{"hping3", ipaddr,
		"-S", "-p", "9999", "-i", "u500000", "-c", strconv.Itoa(NumFlows)}

	if err := hpingRun(srcNs, cmd); err != nil {
		return err
	}
	cmd[1] = strings.Split(epPair.srcEp.Spec.GetIPv4Address(), "/")[0]
	return hpingRun(dstNs, cmd)
}

type epPairInfo struct {
	srcEp   *netproto.Endpoint
	dstEp   *netproto.Endpoint
	srcLink string
	dstLink string
	srcNw   *netproto.Network
	dstNw   *netproto.Network
}

func generateTraffic(uplinkMap map[string]string, agentCfg *AgentConfig, trafficType int,
	maxTrafficPair int) error {

	_EpsReachable := func(ep *netproto.Endpoint, otherEp *netproto.Endpoint) bool {
		if ep != otherEp && ep.GetNamespace() == otherEp.GetNamespace() &&
			ep.Spec.GetNetworkName() != otherEp.Spec.GetNetworkName() &&
			ep.Spec.Interface != otherEp.Spec.Interface {
			return true
		}
		return false
	}

	_EpMatchingTrafficType := func(ep *netproto.Endpoint, otherEp *netproto.Endpoint) bool {
		switch trafficType {
		case TrafficUplinkToUplink:
			if ep.Spec.InterfaceType == UplinkIntefaceName && otherEp.Spec.InterfaceType == UplinkIntefaceName {
				return true
			}
		default:
			log.Fatalln("Invalid traffic type!")

		}
		return false
	}

	trafficPair := 0
	for _, srcEp := range agentCfg.Endpoints {
		for _, dstEp := range agentCfg.Endpoints {
			if _EpsReachable(&srcEp, &dstEp) && _EpMatchingTrafficType(&srcEp, &dstEp) {
				epPair := epPairInfo{srcEp: &srcEp, dstEp: &dstEp,
					srcLink: uplinkMap[srcEp.Spec.Interface],
					dstLink: uplinkMap[dstEp.Spec.Interface],
					srcNw:   getNetworkFromConfig(srcEp.Spec.GetNetworkName(), agentCfg.Networks),
					dstNw:   getNetworkFromConfig(dstEp.Spec.GetNetworkName(), agentCfg.Networks),
				}
				if err := runTrafficBetweenEps(epPair); err != nil {
					return err
				}
				trafficPair++
				if maxTrafficPair != 0 && trafficPair == maxTrafficPair {
					return nil
				}
			}
		}

	}

	return nil
}

//RunTraffic from uplink
func RunTraffic(uplinkMapFile string, agentCfg *AgentConfig, trafficType int, maxTrafficPair int) error {

	jsonFile, err := os.Open(uplinkMapFile)
	if err != nil {
		return errors.Wrap(err, "Err opening uplink map file")
	}

	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)
	uplinkMap := map[string]string{}
	err = json.Unmarshal(byteValue, &uplinkMap)
	if err != nil {
		return errors.Wrap(err, "Err parsing uplink map file")
	}

	return generateTraffic(uplinkMap, agentCfg, trafficType, maxTrafficPair)
}
