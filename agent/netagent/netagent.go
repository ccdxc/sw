// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import "net"

// NetAgent is the network agent instance
type NetAgent struct {
	datapath NetDatapathAPI // network datapath
}

// NewNetAgent returns a new network agent
func NewNetAgent(dp NetDatapathAPI) (*NetAgent, error) {
	agent := NetAgent{
		datapath: dp,
	}

	return &agent, nil
}

// CreateEndpoint creates an endpoint
func (ag *NetAgent) CreateEndpoint(epinfo *EndpointInfo) (*EndpointState, *IntfInfo, error) {
	// FIXME: create fake endpoint state
	ep := EndpointState{
		EndpointInfo: *epinfo,
		IPv4Address:  net.ParseIP("10.1.1.1"),
		IPv4Netmask:  net.IPv4Mask(255, 255, 255, 0),
		IPv4Gateway:  net.ParseIP("10.1.1.254"),
	}

	// call the datapath
	intfInfo, err := ag.datapath.CreateEndpoint(&ep)
	return &ep, intfInfo, err
}

// DeleteEndpoint deletes an endpoint
func (ag *NetAgent) DeleteEndpoint(epinfo *EndpointInfo) error {
	// FIXME: create fake endpoint state
	ep := EndpointState{
		EndpointInfo: *epinfo,
	}

	// call the datapath
	return ag.datapath.DeleteEndpoint(&ep)
}
