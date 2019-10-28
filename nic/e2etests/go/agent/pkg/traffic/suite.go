package traffic

import (
	"fmt"
	"log"
	"reflect"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/protos/netproto"
	pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Tests "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests"
	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
)

const (
	TrafficUplinkToUplink = 1
	TrafficUplinkToHost   = 2
	TrafficHostToHost     = 3

	NumFlows       = 1
	MaxTrafficPair = 1

	UplinkIntefaceName = "uplink"
	LifIntefaceName    = "lif"
)

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
	if ep.GetName() != otherEp.GetName() && ep.GetNamespace() == otherEp.GetNamespace() && ep.Spec.GetNetworkName() != otherEp.Spec.GetNetworkName() && (ep.Spec.Interface != otherEp.Spec.Interface) {
		return true
	}
	return false
}

func EpPairMatchingTrafficType(ep *netproto.Endpoint, otherEp *netproto.Endpoint, trafficType int) bool {
	switch trafficType {
	case TrafficUplinkToUplink:
		if ep.Spec.InterfaceType == UplinkIntefaceName && otherEp.Spec.InterfaceType == UplinkIntefaceName {
			return true
		}
	case TrafficHostToHost:
		if ep.Spec.InterfaceType == LifIntefaceName && otherEp.Spec.InterfaceType == LifIntefaceName {
			return true
		}
	case TrafficUplinkToHost:
		if (ep.Spec.InterfaceType == LifIntefaceName && otherEp.Spec.InterfaceType == UplinkIntefaceName) ||
			(ep.Spec.InterfaceType == UplinkIntefaceName && otherEp.Spec.InterfaceType == LifIntefaceName) {
			return true
		}
	default:
		log.Fatalln("Invalid traffic type!")

	}
	return false
}

func EpMatchingTrafficType(ep *netproto.Endpoint, trafficType int) bool {
	switch trafficType {
	case TrafficUplinkToUplink:
		if ep.Spec.InterfaceType == UplinkIntefaceName {
			return true
		}
	case TrafficHostToHost:
		if ep.Spec.InterfaceType == LifIntefaceName {
			return true
		}
	default:
		log.Fatalln("Invalid traffic type!")

	}
	return false
}

func getEpPairs(agentCfg *pkg.AgentConfig, trafficType int, maxTrafficPair int) []TestApi.EpPair {
	eps := []TestApi.EpPair{}
	trafficPair := 0
	for _, srcEp := range agentCfg.Endpoints {
		for _, dstEp := range agentCfg.Endpoints {
			if EpsReachable(&srcEp, &dstEp) && EpPairMatchingTrafficType(&srcEp, &dstEp, trafficType) {
				eps = append(eps, TestApi.EpPair{Src: srcEp, Dst: dstEp})
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

func getEps(agentCfg *pkg.AgentConfig, trafficType int, maxEps int) []netproto.Endpoint {
	eps := []netproto.Endpoint{}
	epCnt := 0
	for _, ep := range agentCfg.Endpoints {
		if EpMatchingTrafficType(&ep, trafficType) {
			eps = append(eps, ep)
			epCnt++
			if maxEps != 0 && epCnt == maxEps {
				goto out
			}
		}
	}
out:
	return eps
}

func (tsuite *suite) Run(trafficHelper TrafficHelper, agentCfg *pkg.AgentConfig) error {

	for _, module := range tsuite.Modules {
		if !module.Module.Enabled {
			continue
		}
		if module.Module.EpPairs == 0 {
			module.Module.EpPairs = tsuite.EpPairs
		}

		if err := tsuite.runModule(module.Module.Program, trafficHelper, agentCfg, module.Module.EpPairs); err != nil {
			return err
		}

	}

	return nil
}

func (*suite) runModule(module string, trafficHelper TrafficHelper, agentCfg *pkg.AgentConfig,
	maxTrafficPair int) error {

	for _, test := range Tests.GetTestManager().GetInstances(module) {
		epPairs := getEpPairs(agentCfg, trafficHelper.getTrafficType(), maxTrafficPair)
		epPairs = test.FilterEpPairs(epPairs, agentCfg)
		for _, epPair := range epPairs {

			testname := reflect.TypeOf(test).String()
			fmt.Println("Running Test :" + testname)
			srcIntf, err := trafficHelper.getTrafficInterface(epPair.Src.Spec.Interface)
			if err != nil {
				return errors.Wrapf(err, "Error getting traffic interface %s",
					epPair.Src.Spec.Interface)
			}
			dstIntf, err := trafficHelper.getTrafficInterface(epPair.Dst.Spec.Interface)
			if err != nil {
				return errors.Wrapf(err, "Error getting traffic interface %s",
					epPair.Dst.Spec.Interface)
			}
			fmt.Println("SRC EP :", epPair.Src.Spec.GetIPv4Addresses()[0], epPair.Src.Spec.Interface, srcIntf)
			fmt.Println("DST EP :", epPair.Dst.Spec.GetIPv4Addresses()[0], epPair.Dst.Spec.Interface, dstIntf)
			srcEphandle := newEpFromAgentConfig(&epPair.Src,
				getNetworkFromConfig(epPair.Src.Spec.GetNetworkName(), agentCfg.Networks),
				srcIntf)
			dstEphandle := newEpFromAgentConfig(&epPair.Dst,
				getNetworkFromConfig(epPair.Dst.Spec.GetNetworkName(), agentCfg.Networks),
				dstIntf)
			/* If differet Network, setup routes */
			if epPair.Src.Spec.GetNetworkName() != epPair.Dst.Spec.GetNetworkName() {
				setUpRoute(srcEphandle, dstEphandle)
			} else {
				sendGarp(srcEphandle, dstEphandle)
			}
			if err := test.Run(srcEphandle, dstEphandle); err != nil {
				/* On purpose not cleaning to delete EPs for debugging */
				fmt.Println("Test Failed :", testname)
				return err
			}
			fmt.Println("Test Passed :", testname)
			deleteRoute(srcEphandle, dstEphandle)
			deleteEp(srcEphandle)
			deleteEp(dstEphandle)
		}

	}

	return nil
}
