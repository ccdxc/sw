// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"github.com/pensando/sw/nic/agent/netagent/datapath/fswitch"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// NaplesDatapath has the fake datapath for testing purposes
type NaplesDatapath struct {
	ag         state.DatapathIntf // agent
	fSwitch    *fswitch.Fswitch   // fswitch instance
	hostIntf   string             // host facing interface
	uplinkIntf string             // uplink interface
}

// NewNaplesDatapath returns a new fake datapath
func NewNaplesDatapath(hostIf, uplinkIf string) (*NaplesDatapath, error) {
	// create new naples datapath
	ndp := NaplesDatapath{
		hostIntf:   hostIf,
		uplinkIntf: uplinkIf,
	}

	// create new fswitch
	fs, err := fswitch.NewFswitch(&ndp, uplinkIf)
	if err != nil {
		log.Errorf("Error creating fswitch. Err: %v", err)
		return nil, err
	}
	ndp.fSwitch = fs

	// add the host port
	err = fs.AddPort(hostIf)
	if err != nil {
		log.Errorf("Error adding uplink port to fswitch. Err: %v", err)
		return nil, err
	}

	return &ndp, nil
}

// SetAgent sets the agent for this datapath
func (ndp *NaplesDatapath) SetAgent(ag state.DatapathIntf) error {
	// store the agent
	ndp.ag = ag
	return nil
}

// EndpointLearnNotif learn the endpoint from datapath
func (ndp *NaplesDatapath) EndpointLearnNotif(ep *netproto.Endpoint) error {
	_, _, err := ndp.ag.EndpointCreateReq(ep)
	return err
}

// CreateLocalEndpoint creates an endpoint
func (ndp *NaplesDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*state.IntfInfo, error) {

	// add the local endpoint
	err := ndp.fSwitch.AddLocalEndpoint(ndp.hostIntf, ep)
	if err != nil {
		log.Errorf("Error adding endpoint to fswitch. Err: %v", err)
		return nil, err
	}

	return nil, nil
}

// CreateRemoteEndpoint creates remote endpoint
func (ndp *NaplesDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	return ndp.fSwitch.AddRemoteEndpoint(ep, nw)
}

// UpdateLocalEndpoint updates an existing endpoint
func (ndp *NaplesDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// FIXME:
	return nil
}

// UpdateRemoteEndpoint updates an existing endpoint
func (ndp *NaplesDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// FIXME:
	return nil
}

// DeleteLocalEndpoint deletes an endpoint
func (ndp *NaplesDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	// delete endpoint from fswitch
	err := ndp.fSwitch.DelLocalEndpoint(ndp.hostIntf, ep)
	if err != nil {
		log.Errorf("Error deleting endpoint from fswitch. Err: %v", err)
	}

	return err
}

// DeleteRemoteEndpoint deletes remote endpoint
func (ndp *NaplesDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	return ndp.fSwitch.DelRemoteEndpoint(ep)
}

// CreateNetwork creates a network in datapath
func (ndp *NaplesDatapath) CreateNetwork(nw *netproto.Network, tn *netproto.Tenant) error {
	return ndp.fSwitch.CreateNetwork(nw)
}

// UpdateNetwork updates a network in datapath
func (ndp *NaplesDatapath) UpdateNetwork(nw *netproto.Network) error {
	return nil
}

// DeleteNetwork deletes a network from datapath
func (ndp *NaplesDatapath) DeleteNetwork(nw *netproto.Network) error {
	return ndp.fSwitch.DeleteNetwork(nw)
}

// CreateSecurityGroup creates a security group
func (ndp *NaplesDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// UpdateSecurityGroup updates a security group
func (ndp *NaplesDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// DeleteSecurityGroup deletes a security group
func (ndp *NaplesDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// CreateTenant creates a tenant. Stubbed out to satisfy the interface
func (ndp *NaplesDatapath) CreateTenant(tn *netproto.Tenant) error {
	return nil
}

// DeleteTenant deletes a tenant. Stubbed out to satisfy the interface
func (ndp *NaplesDatapath) DeleteTenant(tn *netproto.Tenant) error {
	return nil
}

// ListTenant lists all tenant. Stubbed out to satisfy the interface
func (ndp *NaplesDatapath) ListTenant() []*netproto.Tenant {
	return nil
}

// UpdateTenant updates a tenant. Stubbed out to satisfy the interface
func (ndp *NaplesDatapath) UpdateTenant(tn *netproto.Tenant) error {
	return nil
}
