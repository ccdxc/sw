// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package types

import (
	"sync"

	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/emstore"
)

const (
	// UplinkOffset will ensure that the agent will allocate uplink IDs starting from 128 and this will not clash with any of
	// the internal hal interfaces
	UplinkOffset = 127

	// SecurityProfileOffset will ensure that agent will allocate security profiles stating from 10
	SecurityProfileOffset = 10

	// EnicOffset will ensure that agent will allocate enic IDs starting from UplinkOffset + 10000
	EnicOffset = UplinkOffset + 10000

	// TunnelOffset will ensure that agent will allocate tunnel IDs starting from UplinkOffset + 20000
	TunnelOffset = UplinkOffset + 20000
)

// ID types
const (
	NetworkID         = "networkID"
	SecurityGroupID   = "sgID"
	VrfID             = "vrfID"
	InterfaceID       = "interfaceID"
	NatPoolID         = "natPoolID"
	NatPolicyID       = "natPolicyID"
	NatRuleID         = "natRuleID"
	RouteID           = "routeID"
	NatBindingID      = "natBindingID"
	IPSecPolicyID     = "ipSecPolicyID"
	IPSecSAEncryptID  = "ipSecSAEncryptID"
	IPSecSADecryptID  = "ipSecSADencryptID"
	IPSecRuleID       = "ipSecRuleID"
	SGPolicyID        = "securityGroupPolicyID"
	TCPProxyPolicyID  = "tcpProxyPolicyID"
	SecurityProfileID = "securityProfileID"
	AppID             = "appID"
)

// IntfInfo has the interface names to be plumbed into container
type IntfInfo struct {
	ContainerIntfName string //  Name of container side of the interface
	SwitchIntfName    string // Name of switch side of the interface
}

// ErrCannotDelete is returned when a delete on an object having dependent objects is attempted.
type ErrCannotDelete struct {
	//Message string `json:"message,omitempty"`
	References []string `json:"references,omitempty"`
}

func (e *ErrCannotDelete) Error() (err string) {
	if len(e.References) > 0 {
		//e.Message = fmt.Sprintf("cannot delete an object that has pending references.")
		//b, _ := json.Marshal(e)
		err = fmt.Sprintf("cannot delete an object that has pending references.")
		return
	}
	return
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

// NaplesInfo is a read-only object containing information about Naples
type NaplesInfo struct {
	UUID          string   `json:"naples-uuid,omitempty"`
	ControllerIPs []string `json:"controller-ips,omitempty"`
	Mode          string   `json:"naples-mode,omitempty"`
}

// DepSolver is a netagent state object dependency solver
type DepSolver interface {
	ObjectKey(o api.ObjectMeta, t api.TypeMeta) string        // ObjectKey generates an the lookup key for an object. tenant|name for tenants, namespace|tenant|name for namespaces and kind|tenant|namespace|name for all other objects
	Add(parent interface{}, children ...interface{}) error    // Adds the parent to children dependencies.
	Remove(parent interface{}, children ...interface{}) error // Removes the parent to children dependencies
	Solve(o interface{}) error                                // Checks if an object o has an pending dependencies. Will return ErrCannotDelete if there are pending dependencies
}

// NetAgent is the network agent instance
type NetAgent struct {
	sync.Mutex                                             // global lock for the agent
	Store             emstore.Emstore                      // embedded db
	NodeUUID          string                               // Node's UUID
	Datapath          NetDatapathAPI                       // network datapath
	Ctrlerif          CtrlerAPI                            // controller object
	Solver            DepSolver                            // Object dependency resolver
	NetworkDB         map[string]*netproto.Network         // Network object db ToDo Add updating in memory state from persisted DB in case of agent restarts
	EndpointDB        map[string]*netproto.Endpoint        // Endpoint object db
	SecgroupDB        map[string]*netproto.SecurityGroup   // security group object db
	TenantDB          map[string]*netproto.Tenant          // tenant object db
	NamespaceDB       map[string]*netproto.Namespace       // tenant object db
	EnicDB            map[string]*netproto.Interface       // ENIC interface object db
	NatPoolDB         map[string]*netproto.NatPool         // Nat Pool object DB
	NatPolicyDB       map[string]*netproto.NatPolicy       // Nat Policy Object DB
	NatBindingDB      map[string]*netproto.NatBinding      // Nat Binding Object DB
	RouteDB           map[string]*netproto.Route           // Route Object DB
	IPSecPolicyDB     map[string]*netproto.IPSecPolicy     // IPSecPolicy Object DB
	IPSecSAEncryptDB  map[string]*netproto.IPSecSAEncrypt  // IPSecSAEncrypt Object DB
	IPSecSADecryptDB  map[string]*netproto.IPSecSADecrypt  // IPSecSADecrypt Object DB
	SGPolicyDB        map[string]*netproto.SGPolicy        // Security group policy DB
	TunnelDB          map[string]*netproto.Tunnel          // Tunnel object DB
	TCPProxyPolicyDB  map[string]*netproto.TCPProxyPolicy  // TCP Proxy Policy DB
	IPSecPolicyLUT    map[string]*IPSecRuleRef             // IPSec Policy to rule look up table. Key: <IPSec SA Type>|<IPSec SA Name> This is used as an in memory binding between an IPSec encrypt/decrypt rule to its allocalted IDs. T
	NatPoolLUT        map[string]*NatPoolRef               // nat pool look up table. This is used as an in memory binding between a natpool and its corresponding allocated IDs.
	HwIfDB            map[string]*netproto.Interface       // Has all the Uplinks and Lifs
	PortDB            map[string]*netproto.Port            // HW Port DB
	AppDB             map[string]*netproto.App             // App DB
	SecurityProfileDB map[string]*netproto.SecurityProfile //Security Profile DB
	RuleIDAppLUT      sync.Map                             // SGPolicy Rule ID to App Objects look up table.
	ControllerIPs     []string                             // Controller IPs that NetAgent is using
	Mode              string                               //Netagent Mode
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
	CreateNetwork(nt *netproto.Network) error                                   // create a network
	UpdateNetwork(nt *netproto.Network) error                                   // update a network
	DeleteNetwork(tn, ns, name string) error                                    // delete a network
	ListNetwork() []*netproto.Network                                           // lists all networks
	FindNetwork(meta api.ObjectMeta) (*netproto.Network, error)                 // finds a network
	CreateEndpoint(ep *netproto.Endpoint) (*IntfInfo, error)                    // create an endpoint
	UpdateEndpoint(ep *netproto.Endpoint) error                                 // update an endpoint
	DeleteEndpoint(tn, ns, name string) error                                   // delete an endpoint
	ListEndpoint() []*netproto.Endpoint                                         // list all endpoints
	CreateSecurityGroup(nt *netproto.SecurityGroup) error                       // create a sg
	UpdateSecurityGroup(nt *netproto.SecurityGroup) error                       // update a sg
	DeleteSecurityGroup(tn, ns, name string) error                              // delete a sg
	ListSecurityGroup() []*netproto.SecurityGroup                               // list all sgs
	CreateTenant(tn *netproto.Tenant) error                                     // create a tenant
	DeleteTenant(name string) error                                             // delete a tenant
	ListTenant() []*netproto.Tenant                                             // lists all tenants
	UpdateTenant(tn *netproto.Tenant) error                                     // updates a tenant
	CreateNamespace(ns *netproto.Namespace) error                               // create a namespace
	DeleteNamespace(tn, name string) error                                      // delete a namespace
	ListNamespace() []*netproto.Namespace                                       // lists all namespaces
	UpdateNamespace(ns *netproto.Namespace) error                               // updates a namespace
	CreateInterface(intf *netproto.Interface) error                             // creates an interface
	UpdateInterface(intf *netproto.Interface) error                             // updates an interface
	DeleteInterface(tn, ns, name string) error                                  // deletes an interface
	ListInterface() []*netproto.Interface                                       // lists all interfaces
	CreateNatPool(np *netproto.NatPool) error                                   // creates nat pool
	FindNatPool(meta api.ObjectMeta) (*netproto.NatPool, error)                 // finds a nat pool
	ListNatPool() []*netproto.NatPool                                           // lists nat pools
	UpdateNatPool(np *netproto.NatPool) error                                   // updates a nat pool
	DeleteNatPool(tn, ns, name string) error                                    // deletes a nat pool
	CreateNatPolicy(np *netproto.NatPolicy) error                               // creates nat policy
	FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error)             // finds a nat policy
	ListNatPolicy() []*netproto.NatPolicy                                       // lists nat policy
	UpdateNatPolicy(np *netproto.NatPolicy) error                               // updates a nat policy
	DeleteNatPolicy(tn, ns, name string) error                                  // deletes a nat policy
	CreateNatBinding(np *netproto.NatBinding) error                             // creates nat binding
	FindNatBinding(meta api.ObjectMeta) (*netproto.NatBinding, error)           // finds a nat binding
	ListNatBinding() []*netproto.NatBinding                                     // lists nat binding
	UpdateNatBinding(np *netproto.NatBinding) error                             // updates a nat binding
	DeleteNatBinding(tn, ns, name string) error                                 // deletes a nat binding
	CreateRoute(rt *netproto.Route) error                                       // creates a route
	FindRoute(meta api.ObjectMeta) (*netproto.Route, error)                     // finds a route
	ListRoute() []*netproto.Route                                               // lists routes
	UpdateRoute(rt *netproto.Route) error                                       // updates a route
	DeleteRoute(tn, ns, name string) error                                      // deletes a route
	CreateIPSecPolicy(rt *netproto.IPSecPolicy) error                           // creates a route
	FindIPSecPolicy(meta api.ObjectMeta) (*netproto.IPSecPolicy, error)         // finds a route
	ListIPSecPolicy() []*netproto.IPSecPolicy                                   // lists routes
	UpdateIPSecPolicy(rt *netproto.IPSecPolicy) error                           // updates a route
	DeleteIPSecPolicy(tn, ns, name string) error                                // deletes a route
	CreateIPSecSAEncrypt(rt *netproto.IPSecSAEncrypt) error                     // creates an IPSec SA Encrypt Rule
	FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error)   // finds an IPSec SA Encrypt Rule
	ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt                             // lists IPSec SA Encrypt Rules
	UpdateIPSecSAEncrypt(rt *netproto.IPSecSAEncrypt) error                     // updates an IPSec SA Encrypt Rule
	DeleteIPSecSAEncrypt(tn, ns, name string) error                             // deletes an IPSec SA Encrypt Rule
	CreateIPSecSADecrypt(rt *netproto.IPSecSADecrypt) error                     // creates an IPSec SA Decrypt Rule
	FindIPSecSADecrypt(meta api.ObjectMeta) (*netproto.IPSecSADecrypt, error)   // finds an IPSec SA Decrypt Rule
	ListIPSecSADecrypt() []*netproto.IPSecSADecrypt                             // lists IPSec SA Decrypt Rules
	UpdateIPSecSADecrypt(rt *netproto.IPSecSADecrypt) error                     // updates an IPSec SA Decrypt Rule
	DeleteIPSecSADecrypt(tn, ns, name string) error                             // deletes an IPSec SA Decrypt Rule
	CreateSGPolicy(rt *netproto.SGPolicy) error                                 // creates an Security Group Policy
	FindSGPolicy(meta api.ObjectMeta) (*netproto.SGPolicy, error)               // finds an Security Group Policy
	ListSGPolicy() []*netproto.SGPolicy                                         // lists Security Group Policy
	UpdateSGPolicy(rt *netproto.SGPolicy) error                                 // updates an Security Group Policy
	DeleteSGPolicy(tn, ns, name string) error                                   // deletes an Security Group Policy
	CreateTunnel(tun *netproto.Tunnel) error                                    // creates an Tunnel
	FindTunnel(meta api.ObjectMeta) (*netproto.Tunnel, error)                   // finds an Tunnel
	ListTunnel() []*netproto.Tunnel                                             // lists Tunnel
	UpdateTunnel(tun *netproto.Tunnel) error                                    // updates an Tunnel
	DeleteTunnel(tn, ns, name string) error                                     // deletes an Tunnel
	CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy) error                    // creates an TCP Proxy Policy
	FindTCPProxyPolicy(meta api.ObjectMeta) (*netproto.TCPProxyPolicy, error)   // finds an TCP Proxy Policy
	ListTCPProxyPolicy() []*netproto.TCPProxyPolicy                             // lists TCP Proxy Policy
	UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy) error                    // updates an TCP Proxy Policy
	DeleteTCPProxyPolicy(tcp, ns, name string) error                            // deletes an TCP Proxy Policy
	CreatePort(tcp *netproto.Port) error                                        // creates an Port
	FindPort(meta api.ObjectMeta) (*netproto.Port, error)                       // finds an Port
	ListPort() []*netproto.Port                                                 // lists Port
	UpdatePort(port *netproto.Port) error                                       // updates an Port
	DeletePort(port, ns, name string) error                                     // deletes an Port
	CreateSecurityProfile(profile *netproto.SecurityProfile) error              // creates an SecurityProfile
	FindSecurityProfile(meta api.ObjectMeta) (*netproto.SecurityProfile, error) // finds an SecurityProfile
	ListSecurityProfile() []*netproto.SecurityProfile                           // lists SecurityProfile
	UpdateSecurityProfile(profile *netproto.SecurityProfile) error              // updates an SecurityProfile
	DeleteSecurityProfile(profile, ns, name string) error                       // deletes an SecurityProfile
	CreateApp(app *netproto.App) error                                          // creates an App
	FindApp(meta api.ObjectMeta) (*netproto.App, error)                         // finds an App
	ListApp() []*netproto.App                                                   // lists App
	UpdateApp(app *netproto.App) error                                          // updates an App
	DeleteApp(app, ns, name string) error                                       // deletes an App
	GetHwInterfaces() error                                                     // Gets all the uplinks created on the hal by nic mgr
	GetNaplesInfo() (*NaplesInfo, error)                                        // Returns Naples information
}

// PluginIntf is the API provided by the netagent to plugins
type PluginIntf interface {
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) // Creates an endpoint
	EndpointDeleteReq(epinfo *netproto.Endpoint) error                                  // deletes an endpoint
}

// NetDatapathAPI is the API provided by datapath modules
type NetDatapathAPI interface {
	SetAgent(ag DatapathIntf) error
	CreateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, lifID, enicID uint64, ns *netproto.Namespace) (*IntfInfo, error) // creates a local endpoint in datapath
	UpdateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error                                                            // updates a local endpoint in datapath
	DeleteLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, enicID uint64) error                                                                            // deletes a local endpoint in datapath
	CreateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, uplinkID uint64, ns *netproto.Namespace) error                  // creates a remote endpoint in datapath
	UpdateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error                                                           // updates a remote endpoint in datapath
	DeleteRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error                                                                                          // deletes a remote endpoint in datapath
	CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error                                                                 // creates a network
	UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error                                                                                                // updates a network in datapath
	DeleteNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error                                                                 // deletes a network from datapath
	CreateSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                            // creates a security group
	UpdateSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                            // updates a security group
	DeleteSecurityGroup(sg *netproto.SecurityGroup) error                                                                                                            // deletes a security group
	CreateVrf(vrfID uint64, vrfType string) error                                                                                                                    // creates a vrf
	DeleteVrf(vrfID uint64) error                                                                                                                                    // deletes a vrf
	UpdateVrf(vrfID uint64) error                                                                                                                                    // updates a vrf
	CreateInterface(intf *netproto.Interface, lif *netproto.Interface, port *netproto.Port, ns *netproto.Namespace) error                                            // creates an interface
	UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error                                                                                          // updates an interface
	DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error                                                                                          // deletes an interface
	ListInterfaces() ([]*netproto.Interface, []*netproto.Port, error)                                                                                                // Lists all the lifs and uplinks from the datapath state
	CreateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                                // creates a nat pool in the datapath
	UpdateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                                // updates a nat pool in the datapath
	DeleteNatPool(np *netproto.NatPool, ns *netproto.Namespace) error                                                                                                // deletes a nat pool in the datapath
	CreateNatPolicy(np *netproto.NatPolicy, npLUT map[string]*NatPoolRef, ns *netproto.Namespace) error                                                              // creates a nat policy in the datapath
	UpdateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*NatPoolRef, ns *netproto.Namespace) error                                                         // updates a nat policy in the datapath
	DeleteNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error                                                                                            // deletes a nat policy in the datapath
	CreateRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                                    // creates a route
	UpdateRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                                    // updates a route
	DeleteRoute(rt *netproto.Route, ns *netproto.Namespace) error                                                                                                    // deletes a route
	CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, ns *netproto.Namespace) (*netproto.NatBinding, error)                       // creates a nat policy in the datapath
	UpdateNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error                                                                                          // updates a nat policy in the datapath
	DeleteNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error                                                                                          // deletes a nat policy in the datapath
	CreateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*IPSecRuleRef) error                                                     // creates a IPSec policy in the datapath
	UpdateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error                                                                                        // updates a IPSec policy in the datapath
	DeleteIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error                                                                                        // deletes a IPSec policy in the datapath
	CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns, tep *netproto.Namespace) error                                                                             // creates a IPSecSA encrypt rule in the datapath
	UpdateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error                                                                                  // updates a IPSecSA encrypt rule in the datapath
	DeleteIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error                                                                                  // deletes a IPSecSA encrypt rule in the datapath
	CreateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns, tep *netproto.Namespace) error                                                                             // creates a IPSecSA decrypt rule in the datapath
	UpdateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error                                                                                  // updates a IPSecSA decrypt rule in the datapath
	DeleteIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error                                                                                  // deletes a IPSecSA decrypt rule in the datapath
	CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []*netproto.SecurityGroup, ruleIDAppLUT *sync.Map) error                                                // creates a security group policy in the datapath
	UpdateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error                                                                               // updates a security group policy in the datapath
	DeleteSGPolicy(sgp *netproto.SGPolicy, vrfID uint64) error                                                                                                       // deletes a security group policy in the datapath
	CreateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                                 // creates a tunnel in the datapath
	UpdateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                                 // updates a tunnel in the datapath
	DeleteTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error                                                                                                 // deletes a tunnel in the datapath
	CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error                                                                                 // creates a tcp proxy policy in the datapath
	UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error                                                                                 // updates a tcp proxy policy in the datapath
	DeleteTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error                                                                                 // deletes a tcp proxy policy in the datapath
	CreatePort(port *netproto.Port) (*netproto.Port, error)                                                                                                          // Creates a port in the datapath
	UpdatePort(port *netproto.Port) (*netproto.Port, error)                                                                                                          // Updates a port in the datapath
	DeletePort(port *netproto.Port) error                                                                                                                            // Deletes a port in the datapath
	CreateSecurityProfile(profile *netproto.SecurityProfile, vrf *netproto.Namespace) error                                                                          // Creates a security profile in the datapath
	UpdateSecurityProfile(profile *netproto.SecurityProfile) error                                                                                                   // Updates a  security profile in the datapath
	DeleteSecurityProfile(profile *netproto.SecurityProfile, vrf *netproto.Namespace) error                                                                          // Deletes a security profile in the datapath
	GetUUID() (string, error)                                                                                                                                        // GetUUID gets the FRU information for the NAPLES from HAL.
}

// DatapathIntf is the API provided by the netagent to datapaths
type DatapathIntf interface {
	GetAgentID() string
	EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) // Creates an endpoint
	EndpointDeleteReq(epinfo *netproto.Endpoint) error                                  // deletes an endpoint
}
