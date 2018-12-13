package TestsApi

import (
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
)

const (
	TrafficUplinkToUplink = 1
	TrafficUplinkToHost   = 2
	TrafficHostToHost     = 3

	NumFlows       = 1
	MaxTrafficPair = 1

	UplinkIntefaceName = "uplink"
)

//EpPair represent 2 endpoints
type EpPair struct {
	Src, Dst netproto.Endpoint
}

//TestInterface Interface
type TestInterface interface {
	//Setup(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI)
	Run(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error
	FilterEpPairs(epPair []EpPair, agentCfg *Pkg.AgentConfig) []EpPair
	//Teardown(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI)
}
