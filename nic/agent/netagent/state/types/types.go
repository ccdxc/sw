// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package types

import (
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/emstore"
)

// ID types
const (
	NetworkID        = "networkID"
	SecurityGroupID  = "sgID"
	VrfID            = "vrfID"
	InterfaceID      = "interfaceID"
	NatPoolID        = "natPoolID"
	NatPolicyID      = "natPolicyID"
	NatRuleID        = "natRuleID"
	RouteID          = "routeID"
	NatBindingID     = "natBindingID"
	IPSecPolicyID    = "ipSecPolicyID"
	IPSecSAEncryptID = "ipSecSAEncryptID"
	IPSecSADecryptID = "ipSecSADencryptID"
	IPSecRuleID      = "ipSecRuleID"
	SGPolicyID       = "securityGroupPolicyID"
)

// IntfInfo has the interface names to be plumbed into container
type IntfInfo struct {
	ContainerIntfName string //  Name of container side of the interface
	SwitchIntfName    string // Name of switch side of the interface
}

// NatPoolRef keeps the mapping between a natpool ID and its corresponding NamespaceID.
// We need this to build a look up table between the natpool name and its refs which can be in non local namespaces
type NatPoolRef struct {
	NamespaceID uint64
	PoolID      uint64
}

// IPSecRuleRef keeps the mapping between a ipsec encrypt/decrypt ID and its corresponding NamespaceID.
// We need this to build a look up table between the ipsec SA encrypt/decrypt rule and its refs which can be in non local namespaces
type IPSecRuleRef struct {
	NamespaceID uint64
	RuleID      uint64
}

// NetAgent is the network agent instance
type NetAgent struct {
	sync.Mutex                                           // global lock for the agent
	Store            emstore.Emstore                     // embedded db
	NodeUUID         string                              // Node's UUID
	Datapath         NetDatapathAPI                      // network datapath
	Ctrlerif         CtrlerAPI                           // controller object
	NetworkDB        map[string]*netproto.Network        // Network object db ToDo Add updating in memory state from persisted DB in case of agent restarts
	EndpointDB       map[string]*netproto.Endpoint       // Endpoint object db
	SecgroupDB       map[string]*netproto.SecurityGroup  // security group object db
	TenantDB         map[string]*netproto.Tenant         // tenant object db
	NamespaceDB      map[string]*netproto.Namespace      // tenant object db
	EnicDB           map[string]*netproto.Interface      // ENIC interface object db
	NatPoolDB        map[string]*netproto.NatPool        // Nat Pool object DB
	NatPolicyDB      map[string]*netproto.NatPolicy      // Nat Policy Object DB
	NatBindingDB     map[string]*netproto.NatBinding     // Nat Binding Object DB
	RouteDB          map[string]*netproto.Route          // Route Object DB
	IPSecPolicyDB    map[string]*netproto.IPSecPolicy    // IPSecPolicy Object DB
	IPSecSAEncryptDB map[string]*netproto.IPSecSAEncrypt // IPSecSAEncrypt Object DB
	IPSecSADecryptDB map[string]*netproto.IPSecSADecrypt // IPSecSADecrypt Object DB
	SGPolicyDB       map[string]*netproto.SGPolicy       // Security group policy DB
	TunnelDB         map[string]*netproto.Tunnel         // Tunnel object DB
	IPSecPolicyLUT   map[string]*IPSecRuleRef            // IPSec Policy to rule look up table. Key: <IPSec SA Type>|<IPSec SA Name> This is used as an in memory binding between an IPSec encrypt/decrypt rule to its allocalted IDs. T
	NatPoolLUT       map[string]*NatPoolRef              // nat pool look up table. This is used as an in memory binding between a natpool and its corresponding allocated IDs.
	HwIfDB           map[string]*netproto.Interface      // Has all the Uplinks and Lifs
}

// CtrlerAPI is the API provided by controller modules to netagent
type CtrlerAPI interface {
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) // sends an endpoint create request
	EndpointAgeoutNotif(epinfo *netproto.Endpoint) error                     // sends an endpoint ageout notification
	EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) // sends an endpoint delete request
}

// CtrlerIntf is the API provided by netagent for the controllers
type CtrlerIntf interface {
	RegisterCtrlerIf(ctrlerif CtrlerAPI) error
	GetAgentID() string
	CreateNetwork(nt *netproto.Network) error                                 // create a network
	UpdateNetwork(nt *netproto.Network) error                                 // update a network
	DeleteNetwork(nt *netproto.Network) error                                 // delete a network
	ListNetwork() []*netproto.Network                                         // lists all networks
	FindNetwork(meta api.ObjectMeta) (*netproto.Network, error)               // finds a network
	CreateEndpoint(ep *netproto.Endpoint) (*IntfInfo, error)                  // create an endpoint
	UpdateEndpoint(ep *netproto.Endpoint) error                               // update an endpoint
	DeleteEndpoint(ep *netproto.Endpoint) error                               // delete an endpoint
	ListEndpoint() []*netproto.Endpoint                                       // list all endpoints
	CreateSecurityGroup(nt *netproto.SecurityGroup) error                     // create a sg
	UpdateSecurityGroup(nt *netproto.SecurityGroup) error                     // update a sg
	DeleteSecurityGroup(nt *netproto.SecurityGroup) error                     // delete a sg
	ListSecurityGroup() []*netproto.SecurityGroup                             // list all sgs
	CreateTenant(tn *netproto.Tenant) error                                   // create a tenant
	DeleteTenant(tn *netproto.Tenant) error                                   // delete a tenant
	ListTenant() []*netproto.Tenant                                           // lists all tenants
	UpdateTenant(tn *netproto.Tenant) error                                   // updates a tenant
	CreateNamespace(ns *netproto.Namespace) error                             // create a namespace
	DeleteNamespace(ns *netproto.Namespace) error                             // delete a namespace
	ListNamespace() []*netproto.Namespace                                     // lists all namespaces
	UpdateNamespace(ns *netproto.Namespace) error                             // updates a namespace
	CreateInterface(intf *netproto.Interface) error                           // creates an interface
	UpdateInterface(intf *netproto.Interface) error                           // updates an interface
	DeleteInterface(intf *netproto.Interface) error                           // deletes an interface
	ListInterface() []*netproto.Interface                                     // lists all interfaces
	CreateNatPool(np *netproto.NatPool) error                                 // creates nat pool
	FindNatPool(meta api.ObjectMeta) (*netproto.NatPool, error)               // finds a nat pool
	ListNatPool() []*netproto.NatPool                                         // lists nat pools
	UpdateNatPool(np *netproto.NatPool) error                                 // updates a nat pool
	DeleteNatPool(np *netproto.NatPool) error                                 // deletes a nat pool
	CreateNatPolicy(np *netproto.NatPolicy) error                             // creates nat policy
	FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error)           // finds a nat policy
	ListNatPolicy() []*netproto.NatPolicy                                     // lists nat policy
	UpdateNatPolicy(np *netproto.NatPolicy) error                             // updates a nat policy
	DeleteNatPolicy(np *netproto.NatPolicy) error                             // deletes a nat policy
	CreateNatBinding(np *netproto.NatBinding) error                           // creates nat binding
	FindNatBinding(meta api.ObjectMeta) (*netproto.NatBinding, error)         // finds a nat binding
	ListNatBinding() []*netproto.NatBinding                                   // lists nat binding
	UpdateNatBinding(np *netproto.NatBinding) error                           // updates a nat binding
	DeleteNatBinding(np *netproto.NatBinding) error                           // deletes a nat binding
	CreateRoute(rt *netproto.Route) error                                     // creates a route
	FindRoute(meta api.ObjectMeta) (*netproto.Route, error)                   // finds a route
	ListRoute() []*netproto.Route                                             // lists routes
	UpdateRoute(rt *netproto.Route) error                                     // updates a route
	DeleteRoute(rt *netproto.Route) error                                     // deletes a route
	CreateIPSecPolicy(rt *netproto.IPSecPolicy) error                         // creates a route
	FindIPSecPolicy(meta api.ObjectMeta) (*netproto.IPSecPolicy, error)       // finds a route
	ListIPSecPolicy() []*netproto.IPSecPolicy                                 // lists routes
	UpdateIPSecPolicy(rt *netproto.IPSecPolicy) error                         // updates a route
	DeleteIPSecPolicy(rt *netproto.IPSecPolicy) error                         // deletes a route
	CreateIPSecSAEncrypt(rt *netproto.IPSecSAEncrypt) error                   // creates an IPSec SA Encrypt Rule
	FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error) // finds an IPSec SA Encrypt Rule
	ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt                           // lists IPSec SA Encrypt Rules
	UpdateIPSecSAEncrypt(rt *netproto.IPSecSAEncrypt) error                   // updates an IPSec SA Encrypt Rule
	DeleteIPSecSAEncrypt(rt *netproto.IPSecSAEncrypt) error                   // deletes an IPSec SA Encrypt Rule
	CreateIPSecSADecrypt(rt *netproto.IPSecSADecrypt) error                   // creates an IPSec SA Decrypt Rule
	FindIPSecSADecrypt(meta api.ObjectMeta) (*netproto.IPSecSADecrypt, error) // finds an IPSec SA Decrypt Rule
	ListIPSecSADecrypt() []*netproto.IPSecSADecrypt                           // lists IPSec SA Decrypt Rules
	UpdateIPSecSADecrypt(rt *netproto.IPSecSADecrypt) error                   // updates an IPSec SA Decrypt Rule
	DeleteIPSecSADecrypt(rt *netproto.IPSecSADecrypt) error                   // deletes an IPSec SA Decrypt Rule
	CreateSGPolicy(rt *netproto.SGPolicy) error                               // creates an Security Group Policy
	FindSGPolicy(meta api.ObjectMeta) (*netproto.SGPolicy, error)             // finds an Security Group Policy
	ListSGPolicy() []*netproto.SGPolicy                                       // lists Security Group Policy
	UpdateSGPolicy(rt *netproto.SGPolicy) error                               // updates an Security Group Policy
	DeleteSGPolicy(rt *netproto.SGPolicy) error                               // deletes an Security Group Policy
	CreateTunnel(tun *netproto.Tunnel) error                                  // creates an Tunnel
	FindTunnel(meta api.ObjectMeta) (*netproto.Tunnel, error)                 // finds an Tunnel
	ListTunnel() []*netproto.Tunnel                                           // lists Tunnel
	UpdateTunnel(tun *netproto.Tunnel) error                                  // updates an Tunnel
	DeleteTunnel(tun *netproto.Tunnel) error                                  // deletes an Tunnel
	GetHwInterfaces() error                                                   // Gets all the uplinks created on the hal by nic mgr
}

// PluginIntf is the API provided by the netagent to plugins
type PluginIntf interface {
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) // Creates an endpoint
	EndpointDeleteReq(epinfo *netproto.Endpoint) error                                  // deletes an endpoint
}

// NetDatapathAPI is the API provided by datapath modules
type NetDatapathAPI interface {
	SetAgent(ag DatapathIntf) error
	CreateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) (*IntfInfo, error)                                         // creates a local endpoint in datapath
	UpdateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error                                                      // updates a local endpoint in datapath
	DeleteLocalEndpoint(ep *netproto.Endpoint) error                                                                                                           // deletes a local endpoint in datapath
	CreateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, uplink *netproto.Interface, ns *netproto.Namespace) error // creates a remote endpoint in datapath
	UpdateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error                                                     // updates a remote endpoint in datapath
	DeleteRemoteEndpoint(ep *netproto.Endpoint) error                                                                                                          // deletes a remote endpoint in datapath
	CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error                                                           // creates a network
	UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error                                                                                          // updates a network in datapath
	DeleteNetwork(nw *netproto.Network, ns *netproto.Namespace) error                                                                                          // deletes a network from datapath
	CreateSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                      // creates a security group
	UpdateSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                      // updates a security group
	DeleteSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                      // deletes a security group
	CreateVrf(vrfID uint64, vrfType string) error                                                                                                              // creates a vrf
	DeleteVrf(vrfID uint64) error                                                                                                                              // deletes a vrf
	UpdateVrf(vrfID uint64) error                                                                                                                              // updates a vrf
	CreateInterface(intf *netproto.Interface, lif *netproto.Interface, ns *netproto.Namespace) error                                                           // creates an interface
	UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error                                                                                    // updates an interface
	DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error                                                                                    // deletes an interface
	ListInterfaces() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error)                                                                   // Lists all the lifs and uplinks from the datapath state
	CreateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                          // creates a nat pool in the datapath
	UpdateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                          // updates a nat pool in the datapath
	DeleteNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                          // deletes a nat pool in the datapath
	CreateNatPolicy(np *netproto.NatPolicy, npLUT map[string]*NatPoolRef, ns *netproto.Namespace) error                                                        // creates a nat policy in the datapath
	UpdateNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error                                                                                      // updates a nat policy in the datapath
	DeleteNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error                                                                                      // deletes a nat policy in the datapath
	CreateRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                              // creates a route
	UpdateRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                              // updates a route
	DeleteRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                              // deletes a route
	CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, ns *netproto.Namespace) (*netproto.NatBinding, error)                 // creates a nat policy in the datapath
	UpdateNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error                                                                                    // updates a nat policy in the datapath
	DeleteNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error                                                                                    // deletes a nat policy in the datapath
	CreateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*IPSecRuleRef) error                                               // creates a IPSec policy in the datapath
	UpdateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error                                                                                  // updates a IPSec policy in the datapath
	DeleteIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error                                                                                  // deletes a IPSec policy in the datapath
	CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error                                                                            // creates a IPSecSA encrypt rule in the datapath
	UpdateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error                                                                            // updates a IPSecSA encrypt rule in the datapath
	DeleteIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error                                                                            // deletes a IPSecSA encrypt rule in the datapath
	CreateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error                                                                            // creates a IPSecSA decrypt rule in the datapath
	UpdateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error                                                                            // updates a IPSecSA decrypt rule in the datapath
	DeleteIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error                                                                            // deletes a IPSecSA decrypt rule in the datapath
	CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []uint64) error                                                                                   // creates a security group policy in the datapath
	UpdateSGPolicy(sgp *netproto.SGPolicy, ns *netproto.Namespace) error                                                                                       // updates a security group policy in the datapath
	DeleteSGPolicy(sgp *netproto.SGPolicy, ns *netproto.Namespace) error                                                                                       // deletes a security group policy in the datapath
	CreateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                           // creates a tunnel in the datapath
	UpdateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                           // updates a tunnel in the datapath
	DeleteTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                           // deletes a tunnel in the datapath
}

// DatapathIntf is the API provided by the netagent to datapaths
type DatapathIntf interface {
	GetAgentID() string
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) // Creates an endpoint
	EndpointDeleteReq(epinfo *netproto.Endpoint) error                                  // deletes an endpoint
}
