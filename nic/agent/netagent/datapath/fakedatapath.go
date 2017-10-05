// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"fmt"

	"github.com/pensando/sw/nic/agent/netagent/datapath/fswitch"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// FakeDatapath has the fake datapath for testing purposes
type FakeDatapath struct {
	fSwitch *fswitch.Fswitch // fswitch instance
}

// NewFakeDatapath returns a new fake datapath
func NewFakeDatapath(uplinkIntf string) (*FakeDatapath, error) {
	// create new fake datapath
	fdp := FakeDatapath{}

	// create new fswitch
	fs, err := fswitch.NewFswitch(&fdp, uplinkIntf)
	if err != nil {
		log.Errorf("Error creating fswitch. Err: %v", err)
		return nil, err
	}
	fdp.fSwitch = fs

	return &fdp, nil
}

// SetAgent sets the agent for this datapath
func (fdp *FakeDatapath) SetAgent(ag state.DatapathIntf) error {
	return nil
}

// EndpointLearnNotif learn the endpoints from datapath
func (fdp *FakeDatapath) EndpointLearnNotif(ep *netproto.Endpoint) error {
	return nil
}

// CreateLocalEndpoint creates an endpoint
func (fdp *FakeDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*state.IntfInfo, error) {
	// container and switch interface names
	cintf := fmt.Sprintf("cvport-%s", ep.Spec.EndpointUUID[:8])
	sintf := fmt.Sprintf("svport-%s", ep.Spec.EndpointUUID[:8])

	// if the veth interfaces already exists, delete it
	netutils.DeleteVethPair(sintf, cintf)

	// create Veth pairs
	err := netutils.CreateVethPair(cintf, sintf)
	if err != nil {
		log.Errorf("Error creating veth pair: %s/%s. Err: %v", cintf, sintf, err)
		return nil, err
	}

	// configure switch side of the interface as up
	err = netutils.SetIntfUp(sintf)
	if err != nil {
		return nil, fmt.Errorf("failed to set switch link up: %v", err)
	}

	// add the switch port to fswitch
	err = fdp.fSwitch.AddPort(sintf)
	if err != nil {
		return nil, err
	}

	// add the local endpoint
	err = fdp.fSwitch.AddLocalEndpoint(sintf, ep)
	if err != nil {
		log.Errorf("Error adding endpoint to fswitch. Err: %v", err)
		return nil, err
	}

	// create an interface info
	intfInfo := state.IntfInfo{
		ContainerIntfName: cintf,
		SwitchIntfName:    sintf,
	}

	return &intfInfo, nil
}

// CreateRemoteEndpoint creates remote endpoint
func (fdp *FakeDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	return fdp.fSwitch.AddRemoteEndpoint(ep, nw)
}

// UpdateLocalEndpoint updates an existing endpoint
func (fdp *FakeDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// FIXME:
	return nil
}

// UpdateRemoteEndpoint updates an existing endpoint
func (fdp *FakeDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// FIXME:
	return nil
}

// DeleteLocalEndpoint deletes an endpoint
func (fdp *FakeDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	// container and switch interface names
	cintf := fmt.Sprintf("cvport-%s", ep.Spec.EndpointUUID[:8])
	sintf := fmt.Sprintf("svport-%s", ep.Spec.EndpointUUID[:8])

	// remove the port from fswitch
	err := fdp.fSwitch.DelPort(sintf)
	if err != nil {
		log.Errorf("Error deleting port %s from fswitch. Err: %v", sintf, err)
	}

	// delete endpoint from fswitch
	err = fdp.fSwitch.DelLocalEndpoint(sintf, ep)
	if err != nil {
		log.Errorf("Error deleting endpoint from fswitch. Err: %v", err)
	}

	// delete veth pair
	return netutils.DeleteVethPair(sintf, cintf)
}

// DeleteRemoteEndpoint deletes remote endpoint
func (fdp *FakeDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	return fdp.fSwitch.DelRemoteEndpoint(ep)
}

// CreateNetwork creates a network in datapath
func (fdp *FakeDatapath) CreateNetwork(nw *netproto.Network) error {
	return fdp.fSwitch.CreateNetwork(nw)
}

// UpdateNetwork updates a network in datapath
func (fdp *FakeDatapath) UpdateNetwork(nw *netproto.Network) error {
	return nil
}

// DeleteNetwork deletes a network from datapath
func (fdp *FakeDatapath) DeleteNetwork(nw *netproto.Network) error {
	return fdp.fSwitch.DeleteNetwork(nw)
}

// CreateSecurityGroup creates a security group
func (fdp *FakeDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// UpdateSecurityGroup updates a security group
func (fdp *FakeDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// DeleteSecurityGroup deletes a security group
func (fdp *FakeDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}
