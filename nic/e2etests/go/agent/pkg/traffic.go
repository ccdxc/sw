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

	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
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
	ip[3]--
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

var hpingRun = func(ep *Infra.Endpoint, cmd []string) error {
	if _, err := ep.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Traffic generation failed")
	}
	return nil
}

func newEpFromAgentConfig(ep *netproto.Endpoint, nw *netproto.Network, intf string) *Infra.Endpoint {
	infraEp := &Infra.Endpoint{
		Name: ep.GetName(),

		Remote: ep.Spec.InterfaceType == "uplink",
	}
	infraEp.Interface.Name = intf
	infraEp.Interface.MacAddress = ep.Spec.GetMacAddress()
	infraEp.Interface.IPAddress = strings.Split(ep.Spec.GetIPv4Address(), "/")[0]
	infraEp.Interface.PrefixLen, _ = strconv.Atoi(strings.Split(nw.Spec.GetIPv4Subnet(), "/")[1])
	if infraEp.Remote {
		infraEp.Interface.EncapVlan = int(nw.Spec.GetVlanID())
	} else {
		infraEp.Interface.EncapVlan = int(ep.Spec.GetUsegVlan())
	}
	infraEp.Init(false)

	return infraEp
}

func setUpRoute(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	if err := srcEp.AppEngine.RouteAdd(dstEp.GetIP(), defaultRoute(srcEp.GetNetwork()),
		dstEp.Interface.MacAddress); err != nil {
		return err
	}
	return dstEp.AppEngine.RouteAdd(srcEp.GetIP(), defaultRoute(dstEp.GetNetwork()),
		srcEp.Interface.MacAddress)
}

func deleteRoute(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	if err := srcEp.AppEngine.RouteDelete(dstEp.GetIP(), defaultRoute(srcEp.GetNetwork())); err != nil {
		return err
	}
	return dstEp.AppEngine.RouteDelete(srcEp.GetIP(), defaultRoute(dstEp.GetNetwork()))
}

func runTestBetweenEps(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	/* If differet Network, setup routes */
	if srcEp.GetNetwork() != dstEp.GetNetwork() {
		setUpRoute(srcEp, dstEp)
		defer deleteRoute(srcEp, dstEp)
	}

	cmd := []string{"hping3", dstEp.GetIP(),
		"-S", "-p", "9999", "-i", "u5000000", "-c", strconv.Itoa(NumFlows)}

	if err := hpingRun(srcEp, cmd); err != nil {
		return err
	}
	cmd[1] = srcEp.GetIP()
	return hpingRun(dstEp, cmd)
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
				srcEphandle := newEpFromAgentConfig(&srcEp,
					getNetworkFromConfig(srcEp.Spec.GetNetworkName(), agentCfg.Networks),
					uplinkMap[srcEp.Spec.Interface])
				dstEphandle := newEpFromAgentConfig(&dstEp,
					getNetworkFromConfig(dstEp.Spec.GetNetworkName(), agentCfg.Networks),
					uplinkMap[dstEp.Spec.Interface])
				if err := runTestBetweenEps(srcEphandle, dstEphandle); err != nil {
					return err
				}
				srcEphandle.Delete()
				dstEphandle.Delete()
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
