// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"fmt"
	"io"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"

	ptypes "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"

	context "golang.org/x/net/context"
)

type fakeAgent struct {
	sync.Mutex
	name       string
	netAdded   map[string]*netproto.Network
	netUpdated map[string]*netproto.Network
	netDeleted map[string]bool

	epAdded   map[string]*netproto.Endpoint
	epUpdated map[string]*netproto.Endpoint
	epDeleted map[string]bool

	sgAdded   map[string]*netproto.SecurityGroup
	sgUpdated map[string]*netproto.SecurityGroup
	sgDeleted map[string]bool

	sgpAdded   map[string]*netproto.NetworkSecurityPolicy
	sgpUpdated map[string]*netproto.NetworkSecurityPolicy
	sgpDeleted map[string]bool

	secpAdded   map[string]*netproto.SecurityProfile
	secpUpdated map[string]*netproto.SecurityProfile
	secpDeleted map[string]bool

	appAdded   map[string]*netproto.App
	appUpdated map[string]*netproto.App
	appDeleted map[string]bool
}

// LifUpdateHandler stubbed out to satisfy interface
func (ag *fakeAgent) LifUpdateHandler(lif *halproto.LifGetResponse) error {
	return nil
}

func createFakeAgent(name string) *fakeAgent {
	return &fakeAgent{
		name:        name,
		netAdded:    make(map[string]*netproto.Network),
		netUpdated:  make(map[string]*netproto.Network),
		netDeleted:  make(map[string]bool),
		epAdded:     make(map[string]*netproto.Endpoint),
		epUpdated:   make(map[string]*netproto.Endpoint),
		epDeleted:   make(map[string]bool),
		sgAdded:     make(map[string]*netproto.SecurityGroup),
		sgUpdated:   make(map[string]*netproto.SecurityGroup),
		sgDeleted:   make(map[string]bool),
		sgpAdded:    make(map[string]*netproto.NetworkSecurityPolicy),
		sgpUpdated:  make(map[string]*netproto.NetworkSecurityPolicy),
		sgpDeleted:  make(map[string]bool),
		secpAdded:   make(map[string]*netproto.SecurityProfile),
		secpUpdated: make(map[string]*netproto.SecurityProfile),
		secpDeleted: make(map[string]bool),
		appAdded:    make(map[string]*netproto.App),
		appUpdated:  make(map[string]*netproto.App),
		appDeleted:  make(map[string]bool),
	}
}
func (ag *fakeAgent) RegisterCtrlerIf(ctrlerif types.CtrlerAPI) error {
	return nil
}

func (ag *fakeAgent) GetAgentID() string {
	return "fakeAgent_" + ag.name
}

func (ag *fakeAgent) GetNetagentUptime() (string, error) {
	return "", nil
}

func (ag *fakeAgent) CreateNetwork(nt *netproto.Network) error {
	ag.Lock()
	defer ag.Unlock()
	ag.netAdded[objectKey(nt.ObjectMeta)] = nt
	return nil
}

func (ag *fakeAgent) UpdateNetwork(nt *netproto.Network) error {
	ag.Lock()
	defer ag.Unlock()
	ag.netUpdated[objectKey(nt.ObjectMeta)] = nt
	return nil
}

func (ag *fakeAgent) DeleteNetwork(tn, namespace, name string) error {
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: namespace,
		Name:      name,
	}
	ag.Lock()
	defer ag.Unlock()
	ag.netDeleted[objectKey(meta)] = true
	return nil
}

func (ag *fakeAgent) ListNetwork() []*netproto.Network {
	var netlist []*netproto.Network

	// walk all networks
	for _, nw := range ag.netAdded {
		netlist = append(netlist, nw)
	}

	return netlist
}

func (ag *fakeAgent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
	nw, ok := ag.netAdded[objectKey(meta)]
	if ok {
		return nw, nil
	}

	return nil, fmt.Errorf("Network not found")
}

func (ag *fakeAgent) CreateEndpoint(ep *netproto.Endpoint) error {
	ag.Lock()
	defer ag.Unlock()
	ag.epAdded[objectKey(ep.ObjectMeta)] = ep
	return nil
}

func (ag *fakeAgent) UpdateEndpoint(ep *netproto.Endpoint) error {
	ag.Lock()
	defer ag.Unlock()
	ag.epUpdated[objectKey(ep.ObjectMeta)] = ep
	return nil
}

func (ag *fakeAgent) FindEndpoint(meta api.ObjectMeta) (*netproto.Endpoint, error) {
	ep, ok := ag.epAdded[objectKey(meta)]
	if ok {
		return ep, nil
	}

	return nil, fmt.Errorf("Endpoint not found")
}

func (ag *fakeAgent) DeleteEndpoint(tn, namespace, name string) error {
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: namespace,
		Name:      name,
	}
	ag.Lock()
	defer ag.Unlock()
	ag.epDeleted[objectKey(meta)] = true
	return nil
}

func (ag *fakeAgent) ListEndpoint() []*netproto.Endpoint {
	var eplist []*netproto.Endpoint

	// walk all endpoints
	for _, ep := range ag.epAdded {
		eplist = append(eplist, ep)
	}

	return eplist
}

func (ag *fakeAgent) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	ag.Lock()
	defer ag.Unlock()
	ag.sgAdded[objectKey(sg.ObjectMeta)] = sg
	return nil
}

func (ag *fakeAgent) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	ag.Lock()
	defer ag.Unlock()
	ag.sgUpdated[objectKey(sg.ObjectMeta)] = sg
	return nil
}

func (ag *fakeAgent) DeleteSecurityGroup(tn, namespace, name string) error {
	ag.Lock()
	defer ag.Unlock()
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: namespace,
		Name:      name,
	}
	ag.sgDeleted[objectKey(meta)] = true
	return nil
}

func (ag *fakeAgent) FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) {
	sg, ok := ag.sgAdded[objectKey(meta)]
	if ok {
		return sg, nil
	}

	return nil, fmt.Errorf("SecurityGroup not found")
}

func (ag *fakeAgent) ListSecurityGroup() []*netproto.SecurityGroup {
	var sglist []*netproto.SecurityGroup

	// walk all sgs
	for _, sg := range ag.sgAdded {
		sglist = append(sglist, sg)
	}

	return sglist
}

// CreateTenant creates a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) CreateTenant(tn *netproto.Tenant) error {
	return nil
}

// DeleteTenant deletes a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) DeleteTenant(tn, ns, name string) error {
	return nil
}

func (ag *fakeAgent) FindTenant(meta api.ObjectMeta) (*netproto.Tenant, error) {
	return nil, nil
}

// ListTenant lists tenants. Stubbed out to satisfy the interface
func (ag *fakeAgent) ListTenant() []*netproto.Tenant {
	return nil
}

// UpdateTenant updates a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) UpdateTenant(tn *netproto.Tenant) error {
	return nil
}

// CreateNamespace creates a namespace. Stubbed out to satisfy the interface
func (ag *fakeAgent) CreateNamespace(ns *netproto.Namespace) error {
	return nil
}

// DeleteNamespace deletes a namespace. Stubbed out to satisfy the interface
func (ag *fakeAgent) DeleteNamespace(tn, ns, name string) error {
	return nil
}

// ListNamespace lists namespaces. Stubbed out to satisfy the interface
func (ag *fakeAgent) ListNamespace() []*netproto.Namespace {
	return nil
}

func (ag *fakeAgent) FindNamespace(meta api.ObjectMeta) (*netproto.Namespace, error) {
	return nil, nil
}

// UpdateNamespace updates a namespace. Stubbed out to satisfy the interface
func (ag *fakeAgent) UpdateNamespace(ns *netproto.Namespace) error {
	return nil
}

// CreateInterface creates an interface. Stubbed out to satisfy the ctrlerIf interface
func (ag *fakeAgent) CreateInterface(intf *netproto.Interface) error {
	return nil
}

// DeleteInterface deletes an interface. Stubbed out to satisfy the ctrlerIf interface
func (ag *fakeAgent) DeleteInterface(tn, namespace, name string) error {
	return nil
}

// ListInterface lists interfaces. Stubbed out to satisfy the ctrlerIf interface
func (ag *fakeAgent) ListInterface() []*netproto.Interface {
	return nil
}

// GetInterfaceByID gets interface based on hw id. Stubbed out to satisfy interface
func (ag *fakeAgent) GetInterfaceByID(intfID uint64) (*netproto.Interface, error) {
	return nil, nil
}

// UpdateInterface updates an interface. Stubbed out to satisfy the ctrlerIf interface
func (ag *fakeAgent) UpdateInterface(intf *netproto.Interface) error {
	return nil
}

// GetHwInterfaces implements fetching pre created lifs and uplinks from the datapath
func (ag *fakeAgent) GetHwInterfaces() error {
	return nil
}

// CreateNatPool creates a NAT Pool. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateNatPool(np *netproto.NatPool) error {

	return nil
}

// FindNatPool finds a NAT Pool. Stubbed out to satisfy interface
func (ag *fakeAgent) FindNatPool(meta api.ObjectMeta) (*netproto.NatPool, error) {

	return nil, nil
}

// ListNatPool lists a NAT Pool. Stubbed out to satisfy interface
func (ag *fakeAgent) ListNatPool() []*netproto.NatPool {

	return nil
}

// UpdateNatPool updates a NAT Pool. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateNatPool(np *netproto.NatPool) error {

	return nil
}

// DeleteNatPool deletes a NAT Pool. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteNatPool(tn, namespace, name string) error {

	return nil
}

// CreateNatPolicy creates a NAT Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateNatPolicy(np *netproto.NatPolicy) error {

	return nil
}

// FindNatPolicy finds a NAT Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error) {

	return nil, nil
}

// ListNatPolicy lists a NAT Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) ListNatPolicy() []*netproto.NatPolicy {

	return nil
}

// UpdateNatPolicy updates a NAT Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateNatPolicy(np *netproto.NatPolicy) error {

	return nil
}

// DeleteNatPolicy deletes a NAT Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteNatPolicy(tn, namespace, name string) error {

	return nil
}

// CreateRoute creates a Route. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateRoute(rt *netproto.Route) error {
	return nil
}

// CreateNatBinding creates a NAT Binding. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateNatBinding(np *netproto.NatBinding) error {

	return nil
}

// FindRoute finds a Route. Stubbed out to satisfy interface
func (ag *fakeAgent) FindRoute(meta api.ObjectMeta) (*netproto.Route, error) {
	return nil, nil
}

// FindNatBinding finds a NAT Binding. Stubbed out to satisfy interface
func (ag *fakeAgent) FindNatBinding(meta api.ObjectMeta) (*netproto.NatBinding, error) {

	return nil, nil
}

// ListRoute lists a Route. Stubbed out to satisfy interface
func (ag *fakeAgent) ListRoute() []*netproto.Route {
	return nil
}

// ListNatBinding lists a NAT Binding. Stubbed out to satisfy interface
func (ag *fakeAgent) ListNatBinding() []*netproto.NatBinding {

	return nil
}

// UpdateRoute updates a Route. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateRoute(rt *netproto.Route) error {
	return nil
}

// UpdateNatBinding updates a NAT Binding. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateNatBinding(np *netproto.NatBinding) error {

	return nil
}

// DeleteRoute deletes a Route. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteRoute(tn, namespace, name string) error {

	return nil
}

// DeleteNatBinding deletes a NAT Binding. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteNatBinding(tn, namespace, name string) error {

	return nil
}

// CreateIPSecPolicy creates a IPSec Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateIPSecPolicy(np *netproto.IPSecPolicy) error {
	return nil
}

// FindIPSecPolicy finds a IPSec Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) FindIPSecPolicy(meta api.ObjectMeta) (*netproto.IPSecPolicy, error) {
	return nil, nil
}

// ListIPSecPolicy lists a IPSec Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) ListIPSecPolicy() []*netproto.IPSecPolicy {
	return nil
}

// UpdateIPSecPolicy updates a IPSec Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateIPSecPolicy(np *netproto.IPSecPolicy) error {
	return nil
}

// DeleteIPSecPolicy deletes a IPSec Policy. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteIPSecPolicy(tn, namespace, name string) error {
	return nil
}

// CreateIPSecSAEncrypt creates a IPSec SA Encrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt) error {
	return nil
}

// FindIPSecSAEncrypt finds a IPSec SA Encrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error) {
	return nil, nil
}

// ListIPSecSAEncrypt lists a IPSec SA Encrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt {
	return nil
}

// UpdateIPSecSAEncrypt updates a IPSec SA Encrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes a IPSec SA Encrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteIPSecSAEncrypt(tn, namespace, name string) error {
	return nil
}

// CreateIPSecSADecrypt creates a IPSec SA Decrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateIPSecSADecrypt(np *netproto.IPSecSADecrypt) error {
	return nil
}

// FindIPSecSADecrypt finds a IPSec SA Decrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) FindIPSecSADecrypt(meta api.ObjectMeta) (*netproto.IPSecSADecrypt, error) {
	return nil, nil
}

// ListIPSecSADecrypt lists a IPSec SA Decrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) ListIPSecSADecrypt() []*netproto.IPSecSADecrypt {
	return nil
}

// UpdateIPSecSADecrypt updates a IPSec SA Decrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateIPSecSADecrypt(np *netproto.IPSecSADecrypt) error {
	return nil
}

// DeleteIPSecSADecrypt deletes a IPSec SA Decrypt. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteIPSecSADecrypt(tn, namespace, name string) error {
	return nil
}

// CreateNetworkSecurityPolicy creates a security group policy
func (ag *fakeAgent) CreateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	ag.Lock()
	defer ag.Unlock()
	ag.sgpAdded[objectKey(sgp.ObjectMeta)] = sgp
	return nil
}

// FindNetworkSecurityPolicy finds a security group policy
func (ag *fakeAgent) FindNetworkSecurityPolicy(meta api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error) {
	sgp, ok := ag.sgpAdded[objectKey(meta)]
	if ok {
		return sgp, nil
	}

	return nil, fmt.Errorf("NetworkSecurityPolicy not found")
}

// ListNetworkSecurityPolicy lists a security group policy
func (ag *fakeAgent) ListNetworkSecurityPolicy() []*netproto.NetworkSecurityPolicy {
	var sgplist []*netproto.NetworkSecurityPolicy

	// walk all sgs
	for _, sgp := range ag.sgpAdded {
		sgplist = append(sgplist, sgp)
	}

	return sgplist
}

// UpdateNetworkSecurityPolicy updates a security group policy
func (ag *fakeAgent) UpdateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	ag.Lock()
	defer ag.Unlock()
	ag.sgpUpdated[objectKey(sgp.ObjectMeta)] = sgp
	return nil
}

// DeleteNetworkSecurityPolicy deletes a security group policy
func (ag *fakeAgent) DeleteNetworkSecurityPolicy(tn, namespace, name string) error {
	ag.Lock()
	defer ag.Unlock()
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: namespace,
		Name:      name,
	}
	ag.sgpDeleted[objectKey(meta)] = true
	return nil
}

// CreateTunnel creates a tunnel. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateTunnel(np *netproto.Tunnel) error {
	return nil
}

// FindTunnel finds a tunnel. Stubbed out to satisfy interface
func (ag *fakeAgent) FindTunnel(meta api.ObjectMeta) (*netproto.Tunnel, error) {
	return nil, nil
}

// ListTunnel lists a tunnel. Stubbed out to satisfy interface
func (ag *fakeAgent) ListTunnel() []*netproto.Tunnel {
	return nil
}

// UpdateTunnel updates a tunnel. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateTunnel(np *netproto.Tunnel) error {
	return nil
}

// DeleteTunnel deletes a tunnel. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteTunnel(tn, namespace, name string) error {
	return nil
}

// CreateTCPProxyPolicy creates a tcp proxy policy. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy) error {
	return nil
}

// FindTCPProxyPolicy finds a tcp proxy policy. Stubbed out to satisfy interface
func (ag *fakeAgent) FindTCPProxyPolicy(meta api.ObjectMeta) (*netproto.TCPProxyPolicy, error) {
	return nil, nil
}

// ListTCPProxyPolicy lists a tcp proxy policy. Stubbed out to satisfy interface
func (ag *fakeAgent) ListTCPProxyPolicy() []*netproto.TCPProxyPolicy {
	return nil
}

// UpdateTCPProxyPolicy updates a tcp proxy policy. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy) error {
	return nil
}

// DeleteTCPProxyPolicy deletes a tcp proxy policy. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteTCPProxyPolicy(tcp, namespace, name string) error {
	return nil
}

// CreatePort creates a port. Stubbed out to satisfy interface
func (ag *fakeAgent) CreatePort(np *netproto.Port) error {
	return nil
}

// FindPort finds a port. Stubbed out to satisfy interface
func (ag *fakeAgent) FindPort(meta api.ObjectMeta) (*netproto.Port, error) {
	return nil, nil
}

// ListPort lists a port. Stubbed out to satisfy interface
func (ag *fakeAgent) ListPort() []*netproto.Port {
	return nil
}

// UpdatePort updates a port. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdatePort(port *netproto.Port) error {
	return nil
}

// DeletePort deletes a port. Stubbed out to satisfy interface
func (ag *fakeAgent) DeletePort(tn, ns, name string) error {
	return nil
}

// CreateSecurityProfile creates a security profile. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateSecurityProfile(profile *netproto.SecurityProfile) error {
	ag.Lock()
	defer ag.Unlock()
	ag.secpAdded[objectKey(profile.ObjectMeta)] = profile
	return nil
}

// FindSecurityProfile finds a security profile. Stubbed out to satisfy interface
func (ag *fakeAgent) FindSecurityProfile(meta api.ObjectMeta) (*netproto.SecurityProfile, error) {
	secp, ok := ag.secpAdded[objectKey(meta)]
	if ok {
		return secp, nil
	}

	return nil, fmt.Errorf("SecurityProfile not found")
}

// ListSecurityProfile lists a security profile. Stubbed out to satisfy interface
func (ag *fakeAgent) ListSecurityProfile() []*netproto.SecurityProfile {
	return nil
}

// UpdateSecurityProfile updates a security profile. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateSecurityProfile(profile *netproto.SecurityProfile) error {
	ag.Lock()
	defer ag.Unlock()
	ag.secpUpdated[objectKey(profile.ObjectMeta)] = profile

	return nil
}

// DeleteSecurityProfile deletes a security profile. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteSecurityProfile(tn, ns, name string) error {
	ag.Lock()
	defer ag.Unlock()
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: ns,
		Name:      name,
	}
	ag.secpDeleted[objectKey(meta)] = true
	return nil
}

// CreateVrf creates a vrf. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateVrf(profile *netproto.Vrf) error {
	return nil
}

// FindVrf finds a vrf. Stubbed out to satisfy interface
func (ag *fakeAgent) FindVrf(meta api.ObjectMeta) (*netproto.Vrf, error) {
	return nil, nil
}

// ListVrf lists a vrf. Stubbed out to satisfy interface
func (ag *fakeAgent) ListVrf() []*netproto.Vrf {
	return nil
}

// UpdateVrf updates a vrf. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateVrf(profile *netproto.Vrf) error {

	return nil
}

// DeleteVrf deletes a vrf. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteVrf(tn, ns, name string) error {
	return nil
}

// CreateApp creates an app. Stubbed out to satisfy interface
func (ag *fakeAgent) CreateApp(app *netproto.App) error {
	ag.Lock()
	defer ag.Unlock()
	ag.appAdded[objectKey(app.ObjectMeta)] = app
	return nil
}

// FindApp finds an app. Stubbed out to satisfy interface
func (ag *fakeAgent) FindApp(meta api.ObjectMeta) (*netproto.App, error) {
	ap, ok := ag.appAdded[objectKey(meta)]
	if ok {
		return ap, nil
	}

	return nil, fmt.Errorf("SecurityProfile not found")
}

// ListApp lists an app. Stubbed out to satisfy interface
func (ag *fakeAgent) ListApp() []*netproto.App {
	return nil
}

// UpdateApp updates an app. Stubbed out to satisfy interface
func (ag *fakeAgent) UpdateApp(app *netproto.App) error {
	ag.Lock()
	defer ag.Unlock()
	ag.appUpdated[objectKey(app.ObjectMeta)] = app
	return nil
}

// GetNaplesInfo returns naples info.
func (ag *fakeAgent) GetNaplesInfo() (info *types.NaplesInfo, err error) {
	return
}

// DeleteApp deletes an app. Stubbed out to satisfy interface
func (ag *fakeAgent) DeleteApp(tn, ns, name string) error {
	ag.Lock()
	defer ag.Unlock()
	meta := api.ObjectMeta{
		Tenant:    tn,
		Namespace: ns,
		Name:      name,
	}
	ag.appDeleted[objectKey(meta)] = true
	return nil
}

// CreateLateralNetAgentObjects is a stubbed out to satisfy the interface
func (ag *fakeAgent) CreateLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	return nil
}

// DeleteLateralNetAgentObjects is a stubbed out to satisfy the interface
func (ag *fakeAgent) DeleteLateralNetAgentObjects(owner string, mgmtIP, destIP string, tunnelOp bool) error {
	return nil
}

// PurgeConfigs deletes all netagent configs. stubbed to satisfy the interface
func (ag *fakeAgent) PurgeConfigs() error {
	return nil
}

// GetWatchOptions gets watch options to be used when establishing a watch
func (ag *fakeAgent) GetWatchOptions(cts context.Context, kind string) api.ObjectMeta {
	return api.ObjectMeta{}
}

type fakeRPCServer struct {
	sync.Mutex
	grpcServer  *rpckit.RPCServer
	netdp       map[string]*netproto.Network
	epdb        map[string]*netproto.Endpoint
	sgdb        map[string]*netproto.SecurityGroup
	sgpdb       map[string]*netproto.NetworkSecurityPolicy
	sgpUpdatedb map[string]*netproto.NetworkSecurityPolicy
	secpdb      map[string]*netproto.SecurityProfile
	appdb       map[string]*netproto.App
}

func createRPCServer(t *testing.T) *fakeRPCServer {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer("netctrler", ":0")
	if err != nil {
		t.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// create fake rpc server
	srv := fakeRPCServer{
		grpcServer:  grpcServer,
		netdp:       make(map[string]*netproto.Network),
		epdb:        make(map[string]*netproto.Endpoint),
		sgdb:        make(map[string]*netproto.SecurityGroup),
		sgpdb:       make(map[string]*netproto.NetworkSecurityPolicy),
		sgpUpdatedb: make(map[string]*netproto.NetworkSecurityPolicy),
		secpdb:      make(map[string]*netproto.SecurityProfile),
		appdb:       make(map[string]*netproto.App),
	}

	// register self as rpc handler
	netproto.RegisterNetworkApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterEndpointApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterSecurityGroupApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterNetworkSecurityPolicyApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterSecurityProfileApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterAppApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterAggWatchApiServer(grpcServer.GrpcServer, &srv)
	grpcServer.Start()

	return &srv
}

func (srv *fakeRPCServer) ListObjects(ctx context.Context, kinds *netproto.AggKinds) (*netproto.AggObjectList, error) {
	//No need for implemebtation
	return &netproto.AggObjectList{}, nil
}

func (srv *fakeRPCServer) ObjectOperUpdate(stream netproto.AggWatchApi_ObjectOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) WatchObjects(kinds *netproto.AggKinds, stream netproto.AggWatchApi_WatchObjectsServer) error {
	// walk local db and send stream resp
	for _, app := range srv.appdb {
		// watch event
		watchEvt := netproto.AggObjectEvent{
			EventType: api.EventType_CreateEvent,
			AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
		}

		watchEvts := netproto.AggObjectEventList{}

		mobj, err := ptypes.MarshalAny(app)
		if err != nil {
			log.Errorf("Error  marshalling any object. Err: %v", err)
			return err
		}

		watchEvt.AggObj.Object = &api.Any{Any: *mobj}

		watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
		// send create event
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// walk local db and send stream resp
	for _, sgp := range srv.sgpdb {
		// watch event
		watchEvt := netproto.AggObjectEvent{
			EventType: api.EventType_CreateEvent,
			AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
		}

		watchEvts := netproto.AggObjectEventList{}

		mobj, err := ptypes.MarshalAny(sgp)
		if err != nil {
			log.Errorf("Error  marshalling any object. Err: %v", err)
			return err
		}

		watchEvt.AggObj.Object = &api.Any{Any: *mobj}

		watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)

		// send create event
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}
	return nil
}

func (srv *fakeRPCServer) GetNetwork(context.Context, *api.ObjectMeta) (*netproto.Network, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListNetworks(context.Context, *api.ObjectMeta) (*netproto.NetworkList, error) {
	return &netproto.NetworkList{}, nil
}

func (srv *fakeRPCServer) WatchNetworks(meta *api.ObjectMeta, stream netproto.NetworkApi_WatchNetworksServer) error {
	// walk local db and send stream resp
	for _, net := range srv.netdp {
		// watch event
		watchEvt := netproto.NetworkEvent{
			EventType: api.EventType_CreateEvent,
			Network:   *net,
		}

		watchEvts := netproto.NetworkEventList{}
		watchEvts.NetworkEvents = append(watchEvts.NetworkEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) NetworkOperUpdate(stream netproto.NetworkApi_NetworkOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) UpdateNetwork(ctx context.Context, obj *netproto.Network) (*netproto.Network, error) {
	return obj, nil
}

func (srv *fakeRPCServer) CreateEndpoint(ctx context.Context, ep *netproto.Endpoint) (*netproto.Endpoint, error) {
	return ep, nil
}

func (srv *fakeRPCServer) GetEndpoint(context.Context, *api.ObjectMeta) (*netproto.Endpoint, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListEndpoints(context.Context, *api.ObjectMeta) (*netproto.EndpointList, error) {
	return &netproto.EndpointList{}, nil
}

func (srv *fakeRPCServer) DeleteEndpoint(ctx context.Context, ep *netproto.Endpoint) (*netproto.Endpoint, error) {
	return ep, nil
}

func (srv *fakeRPCServer) WatchEndpoints(meta *api.ObjectMeta, stream netproto.EndpointApi_WatchEndpointsServer) error {
	// walk local db and send stream resp
	for _, ep := range srv.epdb {
		// watch event
		watchEvt := netproto.EndpointEvent{
			EventType: api.EventType_CreateEvent,
			Endpoint:  *ep,
		}

		watchEvts := netproto.EndpointEventList{}
		watchEvts.EndpointEvents = append(watchEvts.EndpointEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) EndpointOperUpdate(stream netproto.EndpointApi_EndpointOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) UpdateEndpoint(ctx context.Context, obj *netproto.Endpoint) (*netproto.Endpoint, error) {
	return obj, nil
}

func (srv *fakeRPCServer) GetSecurityGroup(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SecurityGroup, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListSecurityGroups(context.Context, *api.ObjectMeta) (*netproto.SecurityGroupList, error) {
	return &netproto.SecurityGroupList{}, nil
}

func (srv *fakeRPCServer) WatchSecurityGroups(sel *api.ObjectMeta, stream netproto.SecurityGroupApi_WatchSecurityGroupsServer) error {
	// walk local db and send stream resp
	for _, sg := range srv.sgdb {
		// watch event
		watchEvt := netproto.SecurityGroupEvent{
			EventType:     api.EventType_CreateEvent,
			SecurityGroup: *sg,
		}

		watchEvts := netproto.SecurityGroupEventList{}
		watchEvts.SecurityGroupEvents = append(watchEvts.SecurityGroupEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) SecurityGroupOperUpdate(stream netproto.SecurityGroupApi_SecurityGroupOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) UpdateSecurityGroup(ctx context.Context, obj *netproto.SecurityGroup) (*netproto.SecurityGroup, error) {
	return obj, nil
}

func (srv *fakeRPCServer) GetNetworkSecurityPolicy(ctx context.Context, ometa *api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListNetworkSecurityPolicys(context.Context, *api.ObjectMeta) (*netproto.NetworkSecurityPolicyList, error) {
	return &netproto.NetworkSecurityPolicyList{}, nil
}

func (srv *fakeRPCServer) UpdateNetworkSecurityPolicy(ctx context.Context, sgp *netproto.NetworkSecurityPolicy) (*netproto.NetworkSecurityPolicy, error) {
	srv.Lock()
	defer srv.Unlock()
	srv.sgpUpdatedb[objectKey(sgp.ObjectMeta)] = sgp
	return sgp, nil
}

func (srv *fakeRPCServer) WatchNetworkSecurityPolicys(sel *api.ObjectMeta, stream netproto.NetworkSecurityPolicyApi_WatchNetworkSecurityPolicysServer) error {
	// walk local db and send stream resp
	for _, sgp := range srv.sgpdb {
		// watch event
		watchEvt := netproto.NetworkSecurityPolicyEvent{
			EventType:             api.EventType_CreateEvent,
			NetworkSecurityPolicy: *sgp,
		}

		watchEvts := netproto.NetworkSecurityPolicyEventList{}
		watchEvts.NetworkSecurityPolicyEvents = append(watchEvts.NetworkSecurityPolicyEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) NetworkSecurityPolicyOperUpdate(stream netproto.NetworkSecurityPolicyApi_NetworkSecurityPolicyOperUpdateServer) error {
	for {
		oper, err := stream.Recv()
		if err == io.EOF {
			log.Errorf("NetworkSecurityPolicyOperUpdate stream ended. closing..")
			return stream.SendAndClose(&api.TypeMeta{})
		} else if err != nil {
			log.Errorf("Error receiving from NetworkSecurityPolicyOperUpdate stream. Err: %v", err)
			return err
		}

		srv.Lock()
		srv.sgpUpdatedb[objectKey(oper.NetworkSecurityPolicy.ObjectMeta)] = &oper.NetworkSecurityPolicy
		srv.Unlock()
	}
}

func (srv *fakeRPCServer) GetSecurityProfile(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SecurityProfile, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListSecurityProfiles(context.Context, *api.ObjectMeta) (*netproto.SecurityProfileList, error) {
	return &netproto.SecurityProfileList{}, nil
}

func (srv *fakeRPCServer) WatchSecurityProfiles(sel *api.ObjectMeta, stream netproto.SecurityProfileApi_WatchSecurityProfilesServer) error {
	// walk local db and send stream resp
	for _, sp := range srv.secpdb {
		// watch event
		watchEvt := netproto.SecurityProfileEvent{
			EventType:       api.EventType_CreateEvent,
			SecurityProfile: *sp,
		}

		watchEvts := netproto.SecurityProfileEventList{}
		watchEvts.SecurityProfileEvents = append(watchEvts.SecurityProfileEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) SecurityProfileOperUpdate(stream netproto.SecurityProfileApi_SecurityProfileOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) UpdateSecurityProfile(ctx context.Context, obj *netproto.SecurityProfile) (*netproto.SecurityProfile, error) {
	return obj, nil
}

func (srv *fakeRPCServer) GetApp(ctx context.Context, ometa *api.ObjectMeta) (*netproto.App, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListApps(context.Context, *api.ObjectMeta) (*netproto.AppList, error) {
	return &netproto.AppList{}, nil
}

func (srv *fakeRPCServer) WatchApps(sel *api.ObjectMeta, stream netproto.AppApi_WatchAppsServer) error {
	// walk local db and send stream resp
	for _, app := range srv.appdb {
		// watch event
		watchEvt := netproto.AppEvent{
			EventType: api.EventType_CreateEvent,
			App:       *app,
		}

		watchEvts := netproto.AppEventList{}
		watchEvts.AppEvents = append(watchEvts.AppEvents, &watchEvt)
		// send create event
		err := stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) AppOperUpdate(stream netproto.AppApi_AppOperUpdateServer) error {
	return nil
}

func (srv *fakeRPCServer) UpdateApp(ctx context.Context, obj *netproto.App) (*netproto.App, error) {
	return obj, nil
}

func TestNpmclient(t *testing.T) {

	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")

	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}

	// verify we can make an rpc call from client to server
	_, err = cl.EndpointCreateReq(&epinfo)
	AssertOk(t, err, "Error making endpoint create request")
	_, err = cl.EndpointDeleteReq(&epinfo)
	AssertOk(t, err, "Error making endpoint delete request")

	// stop the server
	srv.grpcServer.Stop()

	// verify rpc call returns error
	// verify we can make an rpc call from client to server
	_, err = cl.EndpointCreateReq(&epinfo)
	Assert(t, (err != nil), "endpoint create request succeeded while expecting it to fail")
	_, err = cl.EndpointDeleteReq(&epinfo)
	Assert(t, (err != nil), "endpoint delete request succeeded while expecting it to fail")

	// stop the client
	cl.Stop()
	time.Sleep(time.Second)
}

func TestNpmClientWatch(t *testing.T) {
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create a network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	srv.netdp["testNetwork"] = &nt

	// create an endpoint
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}
	srv.epdb["testEndpoint"] = &epinfo

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// create http REST server
	restSrv, err := restapi.NewRestServer(ag, nil, nil, ":0")
	AssertOk(t, err, "Error creating the rest server")

	// verify client got the network & ep
	AssertEventually(t, func() (bool, interface{}) {
		nw := ag.netAdded[objectKey(nt.ObjectMeta)]
		return (nw != nil && nw.Name == nt.Name), nil
	}, "Network add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		nw := ag.netUpdated[objectKey(nt.ObjectMeta)]
		return (nw != nil && nw.Name == nt.Name), nil
	}, "Network update not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.netDeleted[objectKey(nt.ObjectMeta)]
		return ok, nil
	}, "Network delete not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ep, ok := ag.epAdded[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Endpoint add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ep, ok := ag.epUpdated[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Endpoint update not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.epDeleted[objectKey(epinfo.ObjectMeta)]
		return ok, nil
	}, "Endpoint delete not found in agent")

	// verify the network REST api
	var netList []*netproto.Network
	restSrvURL := restSrv.GetListenURL()
	err = netutils.HTTPGet("http://"+restSrvURL+"/api/networks/", &netList)
	AssertOk(t, err, "Error getting networks from REST server")
	Assert(t, len(netList) == 1, "Incorrect number of networks")

	// verify endpoint REST api
	var epList []*netproto.Endpoint
	err = netutils.HTTPGet("http://"+restSrvURL+"/api/endpoints/", &epList)
	AssertOk(t, err, "Error getting endpoints from REST server")
	Assert(t, len(epList) == 1, "Incorrect number of endpoints")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
	restSrv.Stop()
}

func TestEndpointDiff(t *testing.T) {

	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an endpoint
	veniceEp := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testVeniceEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}
	srv.epdb["testVeniceEndpoint"] = &veniceEp

	// create a fake agent
	ag := createFakeAgent(t.Name())

	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testLocalEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}

	err := ag.CreateEndpoint(&epinfo)
	AssertOk(t, err, "Error creating endpoint")

	delEp := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "tobeDeletedEndpoint",
			Labels: map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}

	err = ag.CreateEndpoint(&delEp)
	AssertOk(t, err, "Error creating endpoint")
	Assert(t, len(ag.epAdded) == 2, "Invalid number of endpoints in agent")

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")

	AssertEventually(t, func() (bool, interface{}) {
		return (len(ag.epAdded) == 3), ag.epAdded
	}, "Endpoint count incorrect in agent")

	// verify Venice endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		ep, ok := ag.epAdded[objectKey(veniceEp.ObjectMeta)]
		return (ok && ep.Name == veniceEp.Name), nil
	}, "Venice Endpoint not found in agent")

	// verify local endpoint is not deleted by Venice
	AssertEventually(t, func() (bool, interface{}) {
		ep, ok := ag.epAdded[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Local Endpoint not found in agent")

	// verify tobe deleted endpoint is deleted by Venice
	AssertEventually(t, func() (bool, interface{}) {
		isdel, ok := ag.epDeleted[objectKey(delEp.ObjectMeta)]
		return (ok && isdel), ag.epDeleted
	}, "Local Endpoint not found in agent")

	// stop the server
	srv.grpcServer.Stop()

	// stop the client
	cl.Stop()
	time.Sleep(time.Second)
}
func TestSecurityGroupWatch(t *testing.T) {
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an sg
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
		},
	}
	srv.sgdb["testsg"] = &sg

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security group
	AssertEventually(t, func() (bool, interface{}) {
		sgs, ok := ag.sgAdded[objectKey(sg.ObjectMeta)]
		return (ok && sgs.Name == sg.Name), nil
	}, "Security group add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		sgs, ok := ag.sgUpdated[objectKey(sg.ObjectMeta)]
		return (ok && sgs.Name == sg.Name), nil
	}, "Security group update not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.sgDeleted[objectKey(sg.ObjectMeta)]
		return ok, nil
	}, "Security group delete not found in agent")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
}

func TestSecurityPolicyWatch(t *testing.T) {
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an sg
	sgp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsgp",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachGroup: make([]string, 0),
			Rules:       make([]netproto.PolicyRule, 0),
		},
	}
	srv.sgpdb["testsgp"] = &sgp

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security policy
	AssertEventually(t, func() (bool, interface{}) {
		sgs, ok := ag.sgpAdded[objectKey(sgp.ObjectMeta)]
		return (ok && sgs.Name == sgp.Name), nil
	}, "Security group policy add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		sgs, ok := ag.sgpUpdated[objectKey(sgp.ObjectMeta)]
		return (ok && sgs.Name == sgp.Name), nil
	}, "Security group policy update not found in server")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.sgpDeleted[objectKey(sgp.ObjectMeta)]
		return ok, nil
	}, "Security policy delete not found in agent")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
}

func TestSecurityProfileWatch(t *testing.T) {
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an sg
	sp := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsecp",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle: "10s",
			},
		},
	}
	srv.secpdb["testsecp"] = &sp

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security policy
	AssertEventually(t, func() (bool, interface{}) {
		sps, ok := ag.secpAdded[objectKey(sp.ObjectMeta)]
		return (ok && sps.Name == sp.Name), nil
	}, "Security profile add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		sps, ok := ag.secpUpdated[objectKey(sp.ObjectMeta)]
		return (ok && sps.Name == sp.Name), nil
	}, "Security profile update not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.secpDeleted[objectKey(sp.ObjectMeta)]
		return ok, nil
	}, "Security profile delete not found in agent")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
}

func TestAppWatch(t *testing.T) {
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an sg
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testapp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []string{"tcp/1000"},
		},
	}
	srv.appdb["testapp"] = &app

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security policy
	AssertEventually(t, func() (bool, interface{}) {
		apps, ok := ag.appAdded[objectKey(app.ObjectMeta)]
		return (ok && apps.Name == app.Name), nil
	}, "app add not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		apps, ok := ag.appUpdated[objectKey(app.ObjectMeta)]
		return (ok && apps.Name == app.Name), nil
	}, "app update not found in agent")
	AssertEventually(t, func() (bool, interface{}) {
		ok := ag.appDeleted[objectKey(app.ObjectMeta)]
		return ok, nil
	}, "app delete not found in agent")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
}
