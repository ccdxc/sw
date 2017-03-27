// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import "net"

// EndpointInfo is the information about the endpoint
type EndpointInfo struct {
	EndpointUUID    string   // unique identifier for the endpoint
	ContainerUUID   string   // unique identifier for the container
	ContainerName   string   // Name of the pod/container if known
	NetworkName     string   // Name of the network this container belongs to
	TenantName      string   // Name of the tenant this container belongs to
	ContainerLabels []string // Labels associated with the container
}

// EndpointState has all the information about the endpoint
type EndpointState struct {
	EndpointInfo                    // Endpoint information
	IPv4Address    net.IP           // IPv4 address
	IPv4Netmask    net.IPMask       // IPv4 netmask
	IPv4Gateway    net.IP           // IPv4 Gateway address
	IPv6Address    net.IP           // IPv6 address
	IPv6Netmask    net.IPMask       // IPv6 netmask
	IPv6Gateway    net.IP           // IPv6 Gateway address
	MacAddress     net.HardwareAddr // Mac address
	HomingHostAddr net.IP           // IP address of the host
	HomingHostName string           // host name
}

// IntfInfo has the interface names to be plumbed into container
type IntfInfo struct {
	ContainerIntfName string //  Name of container side of the interface
	SwitchIntfName    string // Name of switch side of the interface
}

type NetAgentAPI interface {
	CreateEndpoint(epinfo *EndpointInfo) (*EndpointState, *IntfInfo, error) // Creates an endpoint
	DeleteEndpoint(epinfo *EndpointInfo) error                              // deletes an endpoint
}

type NetDatapathAPI interface {
	CreateEndpoint(ep *EndpointState) (*IntfInfo, error) // Creates an endpoint in datapath
	DeleteEndpoint(ep *EndpointState) error              // deletes an endpoint in datapath
}
