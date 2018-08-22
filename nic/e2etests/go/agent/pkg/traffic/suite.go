package traffic

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"reflect"

	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Tests "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests"
	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"

	"github.com/pkg/errors"
)

const (
	TrafficUplinkToUplink = 1
	TrafficUplinkToHost   = 2
	TrafficHostToHost     = 3

	NumFlows       = 1
	MaxTrafficPair = 1

	UplinkIntefaceName = "uplink"
)

var trafficTypeMap map[string]int

type suite struct {
	Name        string `yaml:"name"`
	EpPairs     int    `yaml:"ep-pairs"`
	Enabled     bool   `yaml:"enabled"`
	TrafficType string `yaml:"traffic-type"`
	Modules     []struct {
		Module struct {
			Name        string `yaml:"name"`
			Program     string `yaml:"program"`
			EpPairs     int    `yaml:"ep-pairs"`
			Enabled     bool   `yaml:"enabled"`
			TrafficType string `yaml:"traffic-type"`
		} `yaml:"module"`
	} `yaml:"modules"`
}

func EpsReachable(ep *netproto.Endpoint, otherEp *netproto.Endpoint) bool {
	if ep != otherEp && ep.GetNamespace() == otherEp.GetNamespace() &&
		ep.Spec.GetNetworkName() != otherEp.Spec.GetNetworkName() &&
		ep.Spec.Interface != otherEp.Spec.Interface {
		return true
	}
	return false
}

func EpMatchingTrafficType(ep *netproto.Endpoint, otherEp *netproto.Endpoint, trafficType int) bool {
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

func getEps(agentCfg *Pkg.AgentConfig, trafficType int, maxTrafficPair int) []TestApi.EpPair {
	eps := []TestApi.EpPair{}
	trafficPair := 0
	for _, srcEp := range agentCfg.Endpoints {
		for _, dstEp := range agentCfg.Endpoints {
			if EpsReachable(&srcEp, &dstEp) && EpMatchingTrafficType(&srcEp, &dstEp, trafficType) {
				eps = append(eps, TestApi.EpPair{Src: &srcEp, Dst: &dstEp})
				trafficPair++
				if maxTrafficPair != 0 && trafficPair == maxTrafficPair {
					goto out
				}
			}
		}
	}
out:
	return eps
}

func (tsuite *suite) Run(uplinkMapFile string, agentCfg *Pkg.AgentConfig) error {

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

	for _, module := range tsuite.Modules {
		if !module.Module.Enabled {
			continue
		}
		if module.Module.TrafficType == "" || len(module.Module.TrafficType) == 0 {
			module.Module.TrafficType = tsuite.TrafficType
		}

		if module.Module.EpPairs == 0 {
			module.Module.EpPairs = tsuite.EpPairs
		}

		tsuite.runModule(module.Module.Program, uplinkMap, agentCfg,
			trafficTypeMap[module.Module.TrafficType], module.Module.EpPairs)

	}

	return nil
}

func (*suite) runModule(module string, uplinkMap map[string]string, agentCfg *Pkg.AgentConfig, trafficType int,
	maxTrafficPair int) error {

	for _, test := range Tests.GetTestManager().GetInstances(module) {
		epPairs := getEps(agentCfg, 1, 1)
		epPairs = test.FilterEpPairs(epPairs, agentCfg)
		for _, epPair := range epPairs {

			testname := reflect.TypeOf(test).String()
			fmt.Println("Running Test :" + testname)
			fmt.Println("SRC EP :", epPair.Src.Spec.GetIPv4Address())
			fmt.Println("DST EP :", epPair.Dst.Spec.GetIPv4Address())
			srcEphandle := newEpFromAgentConfig(epPair.Src,
				getNetworkFromConfig(epPair.Src.Spec.GetNetworkName(), agentCfg.Networks),
				uplinkMap[epPair.Src.Spec.Interface])
			dstEphandle := newEpFromAgentConfig(epPair.Dst,
				getNetworkFromConfig(epPair.Dst.Spec.GetNetworkName(), agentCfg.Networks),
				uplinkMap[epPair.Dst.Spec.Interface])
			/* If differet Network, setup routes */
			if srcEphandle.GetNetwork() != dstEphandle.GetNetwork() {
				setUpRoute(srcEphandle, dstEphandle)
			}
			if err := test.Run(srcEphandle, dstEphandle); err != nil {
				/* On purpose not cleaning to delete EPs for debugging */
				fmt.Println("Test Failed :", testname)
				return err
			}
			fmt.Println("Test Passed :", testname)
			deleteRoute(srcEphandle, dstEphandle)
			srcEphandle.Delete()
			dstEphandle.Delete()
		}

	}

	return nil
}

func init() {
	trafficTypeMap = make(map[string]int)
	trafficTypeMap["Uplink-Uplink"] = TrafficUplinkToUplink
	trafficTypeMap["Uplink-Host"] = TrafficUplinkToHost
	trafficTypeMap["Host-Host"] = TrafficHostToHost
}
