// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import "github.com/pensando/sw/ctrler/npm/rpcserver/netproto"

// IntfInfo has the interface names to be plumbed into container
type IntfInfo struct {
	ContainerIntfName string //  Name of container side of the interface
	SwitchIntfName    string // Name of switch side of the interface
}

// CtrlerAPI is the API provided by controller modules to netagent
type CtrlerAPI interface {
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) // sends an endpoint create request
	EndpointAgeoutNotif(epinfo *netproto.Endpoint) error                     // sends an endpoint ageout notification
	EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) // sends an endpoint delete request
}

// NetworkInterface is part of CtrlerIntf
type NetworkInterface interface {
	CreateNetwork(nt *netproto.Network) error // create a network
	UpdateNetwork(nt *netproto.Network) error // update a network
	DeleteNetwork(nt *netproto.Network) error // delete a network
}

// EndpointInterface is part of CtrlerIntf
type EndpointInterface interface {
	CreateEndpoint(ep *netproto.Endpoint) (*IntfInfo, error) // create an endpoint
	UpdateEndpoint(ep *netproto.Endpoint) error              // update an endpoint
	DeleteEndpoint(ep *netproto.Endpoint) error              // delete an endpoint
}

// SecurityGroupInterface is part of CtrlerIntf
type SecurityGroupInterface interface {
	CreateSecurityGroup(nt *netproto.SecurityGroup) error // create a sg
	UpdateSecurityGroup(nt *netproto.SecurityGroup) error // update a sg
	DeleteSecurityGroup(nt *netproto.SecurityGroup) error // delete a sg
}

// CtrlerIntf is the API provided by netagent for the controllers
type CtrlerIntf interface {
	RegisterCtrlerIf(ctrlerif CtrlerAPI) error
	NetworkInterface
	EndpointInterface
	SecurityGroupInterface
}

// PluginIntf is the API provided by the netagent to plugins
type PluginIntf interface {
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) // Creates an endpoint
	EndpointDeleteReq(epinfo *netproto.Endpoint) error                                  // deletes an endpoint
}

// EndpointAPI is part of NetDatapathAPI
type EndpointAPI interface {
	CreateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) (*IntfInfo, error) // Creates a local endpoint in datapath
	UpdateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error              // Updates a local endpoint in datapath
	DeleteLocalEndpoint(ep *netproto.Endpoint) error                                                                   // deletes a local endpoint in datapath
	CreateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error             // Creates a remote endpoint in datapath
	UpdateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error             // Updates a remote endpoint in datapath
	DeleteRemoteEndpoint(ep *netproto.Endpoint) error                                                                  // deletes a remote endpoint in datapath
}

// NetworkAPI is part of NetDatapathAPI
type NetworkAPI interface {
	CreateNetwork(nw *netproto.Network) error // creates a network
	UpdateNetwork(nw *netproto.Network) error // updates a network in datapath
	DeleteNetwork(nw *netproto.Network) error // deletes a network from datapath
}

// SecurityGroupAPI is part of NetDatapathAPI
type SecurityGroupAPI interface {
	CreateSecurityGroup(sg *netproto.SecurityGroup) error                                                             // creates a security group
	UpdateSecurityGroup(sg *netproto.SecurityGroup) error                                                             // updates a security group
	DeleteSecurityGroup(sg *netproto.SecurityGroup) error                                                             // deletes a security group
	AddSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error    // adds a security rule
	DeleteSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error // deletes a security rule
}

// NetDatapathAPI is the API provided by datapath modules
type NetDatapathAPI interface {
	EndpointAPI
	NetworkAPI
	SecurityGroupAPI
}
