package pkg

import (
	"encoding/json"
	"io/ioutil"
	"log"
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

	NumFlows = 10

	UplinkIntefaceName = "uplink"
)

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

func runTrafficBetweenEps(epPair epPairInfo) error {

	srcNs, _ := setUpNs(epPair.srcEp, epPair.srcNw, epPair.srcLink)
	dstNs, _ := setUpNs(epPair.dstEp, epPair.dstNw, epPair.dstLink)
	defer srcNs.Delete()
	defer dstNs.Delete()

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

func generateTraffic(uplinkMap map[string]string, agentCfg *AgentConfig, trafficType int) error {

	_EpsReachable := func(ep *netproto.Endpoint, otherEp *netproto.Endpoint) bool {
		if ep != otherEp && ep.GetNamespace() == otherEp.GetNamespace() &&
			ep.Spec.GetNetworkName() == otherEp.Spec.GetNetworkName() &&
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

	for _, srcEp := range agentCfg.Endpoints {
		for _, dstEp := range agentCfg.Endpoints {
			if _EpsReachable(&srcEp, &dstEp) && _EpMatchingTrafficType(&srcEp, &dstEp) {
				epPair := epPairInfo{srcEp: &srcEp, dstEp: &dstEp,
					srcLink: uplinkMap[srcEp.Spec.Interface],
					dstLink: uplinkMap[dstEp.Spec.Interface],
					srcNw:   getNetworkFromConfig(srcEp.Spec.GetNetworkName(), agentCfg.Networks),
					dstNw:   getNetworkFromConfig(dstEp.Spec.GetNetworkName(), agentCfg.Networks),
				}
				runTrafficBetweenEps(epPair)
			}
		}

	}

	return nil
}

//RunTraffic from uplink
func RunTraffic(uplinkMapFile string, agentCfg *AgentConfig, trafficType int) error {

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

	return generateTraffic(uplinkMap, agentCfg, trafficType)
}
