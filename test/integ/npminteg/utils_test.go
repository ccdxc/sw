// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	"github.com/pensando/sw/agent"
	"github.com/pensando/sw/agent/netagent/datapath"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/prometheus/common/log"
)

// objKey returns endpoint key
func objKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// Dpagent is an agent instance
type Dpagent struct {
	datapath *datapath.MockHalDatapath
	nagent   *agent.Agent
}

// CreateAgent creates an instance of agent
func CreateAgent(nodeUUID, srvURL string) (*Dpagent, error) {
	// mock datapath
	dp, err := datapath.NewMockHalDatapath()
	if err != nil {
		log.Errorf("Error creating hal datapath. Err: %v", err)
		return nil, err
	}

	// create new network agent
	nagent, err := agent.NewAgent(dp, nodeUUID, srvURL)
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
	ep, _, err := ag.nagent.Netagent.EndpointCreateReq(&epinfo)

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
	err := ag.nagent.Netagent.EndpointDeleteReq(&epinfo)
	return &epinfo, err
}
