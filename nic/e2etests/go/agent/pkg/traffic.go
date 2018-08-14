package pkg

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"strings"

	"github.com/pkg/errors"

	Cfg "github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
)

const (
	TrafficUplinkToUplink = 1
	TrafficUplinkToHost   = 2
	TrafficHostToHost     = 3

	NumFlows = 10

	UplinkIntefaceName = "uplink"
)

type nwEpCfg struct {
	UplinkIntfs []string
	Vlan        int
	Eps         []Cfg.Endpoint
}

func getNetworkFromConfig(nwName string, fullCfg *Cfg.E2eCfg) *Cfg.Network {
	for _, network := range fullCfg.NetworksInfo.Networks {
		if network.NetworkMeta.Name == nwName {
			return &network
		}
	}

	return nil
}

func readAgentConfigFile(file string) *Cfg.E2eCfg {

	return Cfg.GetAgentConfig(file)

}

func appendIfMissing(slice []string, i string) []string {
	for _, ele := range slice {
		if ele == i {
			return slice
		}
	}
	return append(slice, i)
}

func getNwEpCfg(cfg *Cfg.E2eCfg) *map[string]*nwEpCfg {

	nwEpMap := make(map[string]*nwEpCfg)
	for _, ep := range cfg.EndpointsInfo.Endpoints {
		nw := getNetworkFromConfig(ep.EndpointSpec.NetworkName, cfg)
		if nw == nil {
			log.Fatalln("Network not found in config!", ep.EndpointSpec.NetworkName)
		}
		epCfg, ok := nwEpMap[ep.EndpointSpec.NetworkName]
		if !ok {
			epCfg = &nwEpCfg{Vlan: nw.NetworkSpec.VlanID}
			nwEpMap[ep.EndpointSpec.NetworkName] = epCfg
		}
		epCfg.Eps = append(epCfg.Eps, ep)
		if ep.EndpointSpec.InterfaceType == UplinkIntefaceName {
			epCfg.UplinkIntfs = appendIfMissing(epCfg.UplinkIntfs, ep.EndpointSpec.Interface)
		}

	}
	return &nwEpMap
}

func configureUplinks(uplinkMap map[string]string, nwEpCfg *map[string]*nwEpCfg) error {

	configureUplink := func(intf string, vlan int) error {
		vlanIntf := intf + "_" + strconv.Itoa(vlan)
		addVlanCmd := []string{"ip", "link", "add", "link", intf, "name", vlanIntf,
			"type", "vlan", "id", strconv.Itoa(vlan)}
		Common.Run(addVlanCmd, 0, false)
		return nil
	}

	for _, nwEp := range *nwEpCfg {

		if len(nwEp.UplinkIntfs) > len(uplinkMap) {
			return errors.New("Number of Uplink Intfs more than actual uplinks")
		}

		for i := range nwEp.UplinkIntfs {
			configureUplink(uplinkMap[nwEp.UplinkIntfs[i]], nwEp.Vlan)
		}
	}

	return nil
}

var hpingRun = func(cmd []string) error {
	if _, err := Common.Run(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Traffic generation failed")
	}
	return nil
}

func createTrafficBetweenEps(srcUplink string, srcEp Cfg.Endpoint, dstUplink string,
	dstEp Cfg.Endpoint, numFlows int) error {

	ipaddr := strings.Split(dstEp.EndpointSpec.Ipv4Address, "/")[0]
	cmd := []string{"hping3", ipaddr, "-I", srcUplink,
		"-S", "-p", "9999", "-i", "u500000", "-c", strconv.Itoa(numFlows)}

	if err := hpingRun(cmd); err != nil {
		return err
	}
	cmd[1] = strings.Split(srcEp.EndpointSpec.Ipv4Address, "/")[0]
	cmd[3] = dstUplink
	return hpingRun(cmd)
}

func generateTraffic(uplinkMap map[string]string, nwEpMap *map[string]*nwEpCfg, trafficType int) error {

	_EpsReachable := func(ep Cfg.Endpoint, otherEp Cfg.Endpoint) bool {
		if ep != otherEp && ep.EndpointMeta.Namespace == otherEp.EndpointMeta.Namespace &&
			ep.EndpointSpec.NetworkName == otherEp.EndpointSpec.NetworkName &&
			ep.EndpointSpec.Interface != otherEp.EndpointSpec.Interface {
			return true
		}
		return false
	}

	_EpMatchingTrafficType := func(ep Cfg.Endpoint, otherEp Cfg.Endpoint) bool {
		switch trafficType {
		case TrafficUplinkToUplink:
			if ep.EndpointSpec.InterfaceType == UplinkIntefaceName && otherEp.EndpointSpec.InterfaceType == UplinkIntefaceName {
				return true
			}
		default:
			log.Fatalln("Invalid traffic type!")

		}
		return false
	}

	for _, nwEp := range *nwEpMap {
		for _, srcEp := range nwEp.Eps {
			for _, dstEp := range nwEp.Eps {
				if _EpsReachable(srcEp, dstEp) && _EpMatchingTrafficType(srcEp, dstEp) {
					err := createTrafficBetweenEps(uplinkMap[srcEp.EndpointSpec.Interface],
						srcEp, uplinkMap[dstEp.EndpointSpec.Interface], dstEp, NumFlows)
					if err != nil {
						return errors.Wrap(err, "Traffic generation failed")
					}
				}
			}
		}
	}

	return nil
}

//RunTraffic from uplink
func RunTraffic(uplinkMapFile string, file string, trafficType int) error {

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

	agentCfg := readAgentConfigFile(file)
	if agentCfg == nil {
		return errors.New("Error reading agent configuration file " + file)
	}
	nwEpCfg := getNwEpCfg(agentCfg)
	if nwEpCfg == nil {
		return errors.New("Error in building nw Ep cfg")
	}

	if err := configureUplinks(uplinkMap, nwEpCfg); err != nil {
		return errors.Wrap(err, "Configuring uplinks failed")
	}

	return generateTraffic(uplinkMap, nwEpCfg, trafficType)
}
