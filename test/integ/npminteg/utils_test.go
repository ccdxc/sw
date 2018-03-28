// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	"github.com/prometheus/common/log"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Dpagent is an agent instance
type Dpagent struct {
	datapath *datapath.Datapath
	nagent   *netagent.Agent
}

// objKey returns endpoint key
func objKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// pollTimeout returns the poll timeout value based on number of agents
func (it *integTestSuite) pollTimeout() string {
	return fmt.Sprintf("%dms", 10000+(300*it.numAgents))
}

// CreateAgent creates an instance of agent
func CreateAgent(kind datapath.Kind, nodeUUID, srvURL string, resolver resolver.Interface) (*Dpagent, error) {
	// mock datapath
	dp, err := datapath.NewHalDatapath(kind)
	if err != nil {
		log.Errorf("Error creating hal datapath. Err: %v", err)
		return nil, err
	}

	// set tenant create expectations for mock clients
	if kind.String() == "mock" {
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	// create new network agent
	nagent, err := netagent.NewAgent(dp, "", nodeUUID, srvURL, "", resolver)
	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// create an agent instance
	ag := Dpagent{
		datapath: dp,
		nagent:   nagent,
	}

	return &ag, nil
}

func (ag *Dpagent) createEndpointReq(tenant, net, epname, host string) (*netproto.Endpoint, error) {
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   epname,
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: epname,
			WorkloadName: epname,
			WorkloadUUID: epname,
			NetworkName:  net,
		},
		Status: netproto.EndpointStatus{
			HomingHostAddr: host,
			HomingHostName: host,
		},
	}

	// make a create request
	ep, _, err := ag.nagent.NetworkAgent.EndpointCreateReq(&epinfo)

	return ep, err
}

func (ag *Dpagent) deleteEndpointReq(tenant, net, epname, host string) (*netproto.Endpoint, error) {
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   epname,
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: epname,
			WorkloadName: epname,
			WorkloadUUID: epname,
			NetworkName:  net,
		},
		Status: netproto.EndpointStatus{
			HomingHostAddr: host,
			HomingHostName: host,
		},
	}

	// make a create request
	err := ag.nagent.NetworkAgent.EndpointDeleteReq(&epinfo)
	return &epinfo, err
}
