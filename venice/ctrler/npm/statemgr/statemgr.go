// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/featureflags"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

var singletonStatemgr Statemgr
var once sync.Once
var featuremgrs map[string]FeatureStateMgr

// maxUpdateChannelSize is the size of the update pending channel
const maxUpdateChannelSize = 16384

// updatable is an interface all updatable objects have to implement
type updatable interface {
	Write() error
	GetKey() string
}

// dscUpdateIntf
type dscUpdateIntf interface {
	processDSCUpdate(dsc *cluster.DistributedServiceCard) error
	GetKey() string
}

// Topics are the Nimbus message bus topics
type Topics struct {
	AppTopic                   *nimbus.AppTopic
	EndpointTopic              *nimbus.EndpointTopic
	NetworkTopic               *nimbus.NetworkTopic
	SecurityProfileTopic       *nimbus.SecurityProfileTopic
	NetworkSecurityPolicyTopic *nimbus.NetworkSecurityPolicyTopic
	NetworkInterfaceTopic      *nimbus.InterfaceTopic
	CollectorTopic             *nimbus.CollectorTopic
	AggregateTopic             *nimbus.AggregateTopic
	IPAMPolicyTopic            *nimbus.IPAMPolicyTopic
	RoutingConfigTopic         *nimbus.RoutingConfigTopic
	ProfileTopic               *nimbus.ProfileTopic
}

// Statemgr is the object state manager
type Statemgr struct {
	sync.Mutex
	mbus                          *nimbus.MbusServer       // nimbus server
	periodicUpdaterQueue          chan updatable           // queue for periodically writing items back to apiserver
	dscUpdateNotifObjects         map[string]dscUpdateIntf // objects which are watching dsc update
	ctrler                        ctkit.Controller         // controller instance
	topics                        Topics                   // message bus topics
	networkLocks                  map[string]*sync.Mutex   // lock for performing network operation
	logger                        log.Logger
	ModuleReactor                 ctkit.ModuleHandler
	TenantReactor                 ctkit.TenantHandler
	SecurityGroupReactor          ctkit.SecurityGroupHandler
	AppReactor                    ctkit.AppHandler
	VirtualRouterReactor          ctkit.VirtualRouterHandler
	RouteTableReactor             ctkit.RouteTableHandler
	NetworkReactor                ctkit.NetworkHandler
	FirewallProfileReactor        ctkit.FirewallProfileHandler
	DistributedServiceCardReactor ctkit.DistributedServiceCardHandler
	HostReactor                   ctkit.HostHandler
	EndpointReactor               ctkit.EndpointHandler
	NetworkSecurityPolicyReactor  ctkit.NetworkSecurityPolicyHandler
	WorkloadReactor               ctkit.WorkloadHandler
	NetworkInterfaceReactor       ctkit.NetworkInterfaceHandler
	IPAMPolicyReactor             ctkit.IPAMPolicyHandler
	RoutingConfigReactor          ctkit.RoutingConfigHandler
	DSCProfileReactor             ctkit.DSCProfileHandler
	MirrorSessionReactor          ctkit.MirrorSessionHandler
	FlowExportPolicyReactor       ctkit.FlowExportPolicyHandler

	SecurityProfileStatusReactor       nimbus.SecurityProfileStatusReactor
	AppStatusReactor                   nimbus.AppStatusReactor
	NetworkStatusReactor               nimbus.NetworkStatusReactor
	NetworkInterfaceStatusReactor      nimbus.InterfaceStatusReactor
	EndpointStatusReactor              nimbus.EndpointStatusReactor
	NetworkSecurityPolicyStatusReactor nimbus.NetworkSecurityPolicyStatusReactor
	IPAMPolicyStatusReactor            nimbus.IPAMPolicyStatusReactor
	AggregateStatusReactor             nimbus.AggStatusReactor
	RoutingConfigStatusReactor         nimbus.RoutingConfigStatusReactor
	CollectorStatusReactor             nimbus.CollectorStatusReactor
	ProfileStatusReactor               nimbus.ProfileStatusReactor
}

// SetProfileStatusReactor sets the ProfileStatusReactor
func (sm *Statemgr) SetProfileStatusReactor(handler nimbus.ProfileStatusReactor) {
	sm.ProfileStatusReactor = handler
}

// SetSecurityProfileStatusReactor sets the SecurityProfileStatusReactor
func (sm *Statemgr) SetSecurityProfileStatusReactor(handler nimbus.SecurityProfileStatusReactor) {
	sm.SecurityProfileStatusReactor = handler
}

// SetNetworkInterfaceStatusReactor sets the InterfaceStatusReactor
func (sm *Statemgr) SetNetworkInterfaceStatusReactor(handler nimbus.InterfaceStatusReactor) {
	sm.NetworkInterfaceStatusReactor = handler
}

// SetCollectorStatusReactor sets the CollectorStatusReactor
func (sm *Statemgr) SetCollectorStatusReactor(handler nimbus.CollectorStatusReactor) {
	sm.CollectorStatusReactor = handler
}

// SetAppStatusReactor sets the AppStatusReactor
func (sm *Statemgr) SetAppStatusReactor(handler nimbus.AppStatusReactor) {
	sm.AppStatusReactor = handler
}

// SetNetworkStatusReactor sets the NetworkStatusReactor
func (sm *Statemgr) SetNetworkStatusReactor(handler nimbus.NetworkStatusReactor) {
	sm.NetworkStatusReactor = handler
}

// SetAggregateStatusReactor sets the AggregateStatusReactor
func (sm *Statemgr) SetAggregateStatusReactor(handler nimbus.AggStatusReactor) {
	sm.AggregateStatusReactor = handler
}

// SetIPAMPolicyStatusReactor sets the IPAMPolicyStatusReactor
func (sm *Statemgr) SetIPAMPolicyStatusReactor(handler nimbus.IPAMPolicyStatusReactor) {
	sm.IPAMPolicyStatusReactor = handler
}

// SetNetworkSecurityPolicyStatusReactor sets the NetworkSecurityPolicyStatusReactor
func (sm *Statemgr) SetNetworkSecurityPolicyStatusReactor(handler nimbus.NetworkSecurityPolicyStatusReactor) {
	sm.NetworkSecurityPolicyStatusReactor = handler
}

// SetEndpointStatusReactor sets the EndpointStatusReactor
func (sm *Statemgr) SetEndpointStatusReactor(handler nimbus.EndpointStatusReactor) {
	sm.EndpointStatusReactor = handler
}

// SetRoutingConfigStatusReactor sets the RoutingConfigStatusReactor
func (sm *Statemgr) SetRoutingConfigStatusReactor(handler nimbus.RoutingConfigStatusReactor) {
	sm.RoutingConfigStatusReactor = handler
}

// SetIPAMPolicyReactor sets the IPAMPolicy reactor
func (sm *Statemgr) SetIPAMPolicyReactor(handler ctkit.IPAMPolicyHandler) {
	sm.IPAMPolicyReactor = handler
}

// SetModuleReactor sets the Module reactor
func (sm *Statemgr) SetModuleReactor(handler ctkit.ModuleHandler) {
	sm.ModuleReactor = handler
}

// SetTenantReactor sets the Tenant reactor
func (sm *Statemgr) SetTenantReactor(handler ctkit.TenantHandler) {
	sm.TenantReactor = handler
}

// SetSecurityGroupReactor sets the SecurityGroup reactor
func (sm *Statemgr) SetSecurityGroupReactor(handler ctkit.SecurityGroupHandler) {
	sm.SecurityGroupReactor = handler
}

// SetAppReactor sets the App reactor
func (sm *Statemgr) SetAppReactor(handler ctkit.AppHandler) {
	sm.AppReactor = handler
}

// SetVirtualRouterReactor sets the VirtualRouter reactor
func (sm *Statemgr) SetVirtualRouterReactor(handler ctkit.VirtualRouterHandler) {
	sm.VirtualRouterReactor = handler
}

// SetRouteTableReactor sets the VirtualRouter reactor
func (sm *Statemgr) SetRouteTableReactor(handler ctkit.RouteTableHandler) {
	sm.RouteTableReactor = handler
}

// SetNetworkReactor sets the Network reactor
func (sm *Statemgr) SetNetworkReactor(handler ctkit.NetworkHandler) {
	sm.NetworkReactor = handler
}

// SetFirewallProfileReactor sets the FirewallProfile reactor
func (sm *Statemgr) SetFirewallProfileReactor(handler ctkit.FirewallProfileHandler) {
	sm.FirewallProfileReactor = handler
}

// SetDistributedServiceCardReactor sets the DistributedServiceCard reactor
func (sm *Statemgr) SetDistributedServiceCardReactor(handler ctkit.DistributedServiceCardHandler) {
	sm.DistributedServiceCardReactor = handler
}

// SetHostReactor sets the Host reactor
func (sm *Statemgr) SetHostReactor(handler ctkit.HostHandler) {
	sm.HostReactor = handler
}

// SetEndpointReactor sets the Endpoint reactor
func (sm *Statemgr) SetEndpointReactor(handler ctkit.EndpointHandler) {
	sm.EndpointReactor = handler
}

// SetNetworkSecurityPolicyReactor sets the NetworkSecurity reactor
func (sm *Statemgr) SetNetworkSecurityPolicyReactor(handler ctkit.NetworkSecurityPolicyHandler) {
	sm.NetworkSecurityPolicyReactor = handler
}

// SetWorkloadReactor sets the Workload reactor
func (sm *Statemgr) SetWorkloadReactor(handler ctkit.WorkloadHandler) {
	sm.WorkloadReactor = handler
}

// SetNetworkInterfaceReactor sets the NetworkInterface reactor
func (sm *Statemgr) SetNetworkInterfaceReactor(handler ctkit.NetworkInterfaceHandler) {
	sm.NetworkInterfaceReactor = handler
}

// SetMirrorSessionReactor sets the  MirrorSession reactor
func (sm *Statemgr) SetMirrorSessionReactor(handler ctkit.MirrorSessionHandler) {
	sm.MirrorSessionReactor = handler
}

// SetFlowExportPolicyReactor sets the  MirrorSession reactor
func (sm *Statemgr) SetFlowExportPolicyReactor(handler ctkit.FlowExportPolicyHandler) {
	sm.FlowExportPolicyReactor = handler
}

// SetRoutingConfigReactor sets the RoutingConfig reactor
func (sm *Statemgr) SetRoutingConfigReactor(handler ctkit.RoutingConfigHandler) {
	sm.RoutingConfigReactor = handler
}

// SetDSCProfileReactor sets the DSCProfile reactor
func (sm *Statemgr) SetDSCProfileReactor(handler ctkit.DSCProfileHandler) {
	sm.DSCProfileReactor = handler
}

// ErrIsObjectNotFound returns true if the error is object not found
func ErrIsObjectNotFound(err error) bool {
	return (err == memdb.ErrObjectNotFound) || strings.Contains(err.Error(), "not found")
}

// Option fills the optional params for Statemgr
type Option func(*Statemgr)

// WithDiagnosticsHandler adds a diagnostics query handler to controller kit
func WithDiagnosticsHandler(rpcMethod, query string, diagHdlr diagnostics.Handler) Option {
	return func(sm *Statemgr) {
		if sm.ctrler != nil {
			sm.ctrler.RegisterDiagnosticsHandler(rpcMethod, query, diagHdlr)
		}
	}
}

// FindObject looks up an object in local db
func (sm *Statemgr) FindObject(kind, tenant, ns, name string) (runtime.Object, error) {
	// form network key
	ometa := api.ObjectMeta{
		Tenant:    tenant,
		Namespace: ns,
		Name:      name,
	}

	// find it in db
	return sm.ctrler.FindObject(kind, &ometa)
}

// IsPending looks up an object in local db
func (sm *Statemgr) IsPending(kind, tenant, ns, name string) (bool, error) {
	// form network key
	ometa := api.ObjectMeta{
		Tenant:    tenant,
		Namespace: ns,
		Name:      name,
	}

	// find it in db
	return sm.ctrler.IsPending(kind, &ometa)
}

// ListObjects list all objects of a kind
func (sm *Statemgr) ListObjects(kind string) []runtime.Object {
	return sm.ctrler.ListObjects(kind)
}

// Stop stops the watchers
func (sm *Statemgr) Stop() error {
	log.Infof("Statemanager stop called")
	return sm.ctrler.Stop()
}

func initStatemgr() {
	singletonStatemgr = Statemgr{
		networkLocks:          make(map[string]*sync.Mutex),
		dscUpdateNotifObjects: make(map[string]dscUpdateIntf),
	}
	featuremgrs = make(map[string]FeatureStateMgr)
}

// MustGetStatemgr returns a singleton statemgr
func MustGetStatemgr() *Statemgr {
	once.Do(initStatemgr)
	return &singletonStatemgr
}

// Register is statemgr implemention of the interface
func (sm *Statemgr) Register(name string, svc FeatureStateMgr) {
	featuremgrs[name] = svc

}

func (sm *Statemgr) registerForDscUpdate(object dscUpdateIntf) {
	sm.Lock()
	defer sm.Unlock()
	sm.dscUpdateNotifObjects[object.GetKey()] = object
}

func (sm *Statemgr) unRegisterForDscUpdate(object dscUpdateIntf) {
	sm.Lock()
	defer sm.Unlock()
	delete(sm.dscUpdateNotifObjects, object.GetKey())
}

func (sm *Statemgr) sendDscUpdateNotification(dsc *cluster.DistributedServiceCard) {
	sm.Lock()
	defer sm.Unlock()
	for _, obj := range sm.dscUpdateNotifObjects {
		obj.processDSCUpdate(dsc)
	}
}

// AddObject adds object to memDb
func (sm *Statemgr) AddObject(obj memdb.Object) error {
	return sm.mbus.AddObject(obj)
}

// UpdateObject adds object to memDb
func (sm *Statemgr) UpdateObject(obj memdb.Object) error {
	return sm.mbus.UpdateObject(obj)
}

// DeleteObject adds object to memDb
func (sm *Statemgr) DeleteObject(obj memdb.Object) error {
	return sm.mbus.DeleteObject(obj)
}

func (sm *Statemgr) setDefaultReactors(reactor ctkit.CtrlDefReactor) {

	sm.SetModuleReactor(reactor)

	sm.SetTenantReactor(reactor)

	sm.SetSecurityGroupReactor(reactor)

	sm.SetAppReactor(reactor)

	sm.SetVirtualRouterReactor(reactor)

	sm.SetRouteTableReactor(reactor)

	sm.SetNetworkReactor(reactor)

	sm.SetFirewallProfileReactor(reactor)

	sm.SetDistributedServiceCardReactor(reactor)

	sm.SetHostReactor(reactor)

	sm.SetEndpointReactor(reactor)

	sm.SetNetworkSecurityPolicyReactor(reactor)

	sm.SetWorkloadReactor(reactor)

	sm.SetNetworkInterfaceReactor(reactor)

	sm.SetFlowExportPolicyReactor(reactor)

	sm.SetMirrorSessionReactor(reactor)

	sm.SetIPAMPolicyReactor(reactor)

	sm.SetRoutingConfigReactor(reactor)

	sm.SetDSCProfileReactor(reactor)

}

// Run calls the feature statemgr callbacks and eastablishes the Watches
func (sm *Statemgr) Run(rpcServer *rpckit.RPCServer, apisrvURL string, rslvr resolver.Interface, mserver *nimbus.MbusServer, logger log.Logger, options ...Option) error {

	sm.mbus = mserver

	sm.registerKindsWithMbus()

	sm.logger = logger

	// create controller instance
	// disable object resolution
	ctrler, defReactor, err := ctkit.NewController(globals.Npm, rpcServer, apisrvURL, rslvr, logger, false)
	if err != nil {
		logger.Fatalf("Error creating controller. Err: %v", err)
	}
	sm.ctrler = ctrler

	for _, o := range options {
		o(sm)
	}

	// newPeriodicUpdater creates a new go subroutines
	// Given that objects returned by `NewStatemgr` should live for the duration
	// of the process, we don't have to worry about leaked go subroutines
	sm.periodicUpdaterQueue = newPeriodicUpdater()

	// init the watch reactors
	sm.setDefaultReactors(defReactor)

	// Fetch feature flags if available before proceeding.
	featureflags.Initialize(globals.Npm, apisrvURL, rslvr)

	for name, svc := range featuremgrs {
		logger.Info("svc", name, "complete registration")
		svc.CompleteRegistration()
	}

	sm.EnableSelectivePushForKind("Profile")

	// start all object watches
	// there is a specific order we do these watches to meet dependency requirements
	// 1. apps before sgpolicies
	// 2. endpoints before workload
	// 3. smartnics before hosts
	// 4. security-groups before sgpolicies
	err = ctrler.Module().Watch(sm.ModuleReactor)
	if err != nil {
		logger.Fatalf("Error watching module object")
	}
	err = ctrler.Tenant().Watch(sm.TenantReactor)
	if err != nil {
		logger.Fatalf("Error watching sg policy")
	}

	err = ctrler.SecurityGroup().Watch(sm.SecurityGroupReactor)
	if err != nil {
		logger.Fatalf("Error watching security group")
	}
	err = ctrler.App().Watch(sm.AppReactor)
	if err != nil {
		logger.Fatalf("Error watching app")
	}

	err = ctrler.VirtualRouter().Watch(sm.VirtualRouterReactor)
	if err != nil {
		logger.Fatalf("Error watching virtual router")
	}
	err = ctrler.RouteTable().Watch(sm.RouteTableReactor)
	if err != nil {
		logger.Fatalf("Error watching virtual router")
	}
	err = ctrler.Network().Watch(sm.NetworkReactor)
	if err != nil {
		logger.Fatalf("Error watching network")
	}
	err = ctrler.FirewallProfile().Watch(sm.FirewallProfileReactor)
	if err != nil {
		logger.Fatalf("Error watching firewall profile")
	}

	logger.Info("start watch for DSCProfile")
	err = ctrler.DSCProfile().Watch(sm.DSCProfileReactor)
	if err != nil {
		logger.Fatalf("Error watching DSCProfile")
	}
	err = ctrler.DistributedServiceCard().Watch(sm.DistributedServiceCardReactor)
	if err != nil {
		logger.Fatalf("Error watching smartnic")
	}
	err = ctrler.Host().Watch(sm.HostReactor)
	if err != nil {
		logger.Fatalf("Error watching host")
	}
	err = ctrler.Endpoint().Watch(sm.EndpointReactor)
	if err != nil {
		logger.Fatalf("Error watching endpoint")
	}
	err = ctrler.NetworkSecurityPolicy().Watch(sm.NetworkSecurityPolicyReactor)
	if err != nil {
		logger.Fatalf("Error watching sg policy")
	}
	err = ctrler.Workload().Watch(sm.WorkloadReactor)
	if err != nil {
		logger.Fatalf("Error watching workloads")
	}
	err = ctrler.NetworkInterface().Watch(sm.NetworkInterfaceReactor)
	if err != nil {
		logger.Fatalf("Error watching network-interface")
	}

	err = ctrler.MirrorSession().Watch(sm.MirrorSessionReactor)
	if err != nil {
		logger.Fatalf("Error watching mirror")
	}

	err = ctrler.FlowExportPolicy().Watch(sm.FlowExportPolicyReactor)
	if err != nil {
		logger.Fatalf("Error watching flow export")
	}

	err = ctrler.IPAMPolicy().Watch(sm.IPAMPolicyReactor)
	if err != nil {
		logger.Fatalf("Error watching ipam-policy")
	}

	err = ctrler.RoutingConfig().Watch(sm.RoutingConfigReactor)
	if err != nil {
		logger.Fatalf("Error watching routing-config")
	}

	//Remove state endpoints after we start the watch
	//Start watch would synchronosly does diff of all workload and endpoints.
	//Stale endpoints would then be deleted.
	//statemgr.RemoveStaleEndpoints()

	// create all topics on the message bus
	sm.topics.EndpointTopic, err = nimbus.AddEndpointTopic(mserver, sm.EndpointStatusReactor)
	if err != nil {
		logger.Errorf("Error starting endpoint RPC server")
		return err
	}
	sm.topics.AppTopic, err = nimbus.AddAppTopic(mserver, sm.AppStatusReactor)
	if err != nil {
		logger.Errorf("Error starting App RPC server")
		return err
	}
	sm.topics.SecurityProfileTopic, err = nimbus.AddSecurityProfileTopic(mserver, sm.SecurityProfileStatusReactor)
	if err != nil {
		logger.Errorf("Error starting SecurityProfile RPC server")
		return err
	}
	sm.topics.NetworkSecurityPolicyTopic, err = nimbus.AddNetworkSecurityPolicyTopic(mserver, sm.NetworkSecurityPolicyStatusReactor)
	if err != nil {
		logger.Errorf("Error starting SG policy RPC server")
		return err
	}
	sm.topics.NetworkTopic, err = nimbus.AddNetworkTopic(mserver, sm.NetworkStatusReactor)
	if err != nil {
		logger.Errorf("Error starting network RPC server")
		return err
	}
	sm.topics.NetworkInterfaceTopic, err = nimbus.AddInterfaceTopic(mserver, sm.NetworkInterfaceStatusReactor)
	if err != nil {
		log.Errorf("Error starting network interface RPC server")
		return err
	}

	sm.topics.CollectorTopic, err = nimbus.AddCollectorTopic(mserver, sm.CollectorStatusReactor)
	if err != nil {
		log.Errorf("Error starting collector interface RPC server")
		return err
	}

	sm.topics.IPAMPolicyTopic, err = nimbus.AddIPAMPolicyTopic(mserver, sm.IPAMPolicyStatusReactor)
	if err != nil {
		log.Errorf("Error starting network interface RPC server: %v", err)
		return err
	}

	sm.topics.AggregateTopic, err = nimbus.AddAggregateTopic(mserver, sm.AggregateStatusReactor)
	if err != nil {
		log.Errorf("Error starting Aggregate RPC server")
		return err
	}
	return err
}

func (sm *Statemgr) registerKindsWithMbus() {
	sm.mbus.RegisterKind("Tenant")
	sm.mbus.RegisterKind("Vrf")
	sm.mbus.RegisterKind("Network")
	sm.mbus.RegisterKind("RouteTable")
	sm.mbus.RegisterKind("Endpoint")
	sm.mbus.RegisterKind("App")
	sm.mbus.RegisterKind("SecurityProfile")
	sm.mbus.RegisterKind("SecurityGroup")
	sm.mbus.RegisterKind("NetworkSecurityPolicy")
	sm.mbus.RegisterKind("NetworkInterface")
	sm.mbus.RegisterKind("Collector")
	sm.mbus.RegisterKind("Profile")
	sm.mbus.RegisterKind("MirrorSession")
	sm.mbus.RegisterKind("FlowExportPolicy")
}

//EnableSelectivePushForKind enable selective push for a kind
func (sm *Statemgr) EnableSelectivePushForKind(kind string) error {
	return sm.mbus.EnableSelectivePushForKind(kind)
}

//DisableSelectivePushForKind disable selective push for a kind
func (sm *Statemgr) DisableSelectivePushForKind(kind string) error {
	return sm.mbus.DisableSelectivePushForKind(kind)
}

//DSCAddedAsReceiver checks whether DSC is added as receiver
func (sm *Statemgr) DSCAddedAsReceiver(ID string) (bool, error) {
	if _, err := sm.mbus.FindReceiver(ID); err == nil {
		return true, nil
	}

	return false, fmt.Errorf("DSC %v not added as receiver", ID)
}

/*

//Sample Code to push object to receivers

func (sm *Statemgr) ReferenceCodeForSelectivePush() {

	//NOTE :

	//Push object should be enabled by kind that will be sent to agents

	//This should be done when npm start to specicy which kind of  objects should be subjected to selective push
	sm.EnableSelectivePushForKind("SecurityProfile")


	//Try to find the DSC by primary MAC
	_, err := sm.mbus.FindReceiver(ID)
	if err != nil {
		return false, fmt.Errorf("Receiver %v", ID)
	}

	//Add new object along with receivers.
	receivers := []memdb.Receiver{recvr}
	pushObj, err := sm.mbus.AddPushObject(app.MakeKey("security"), convertApp(aps), references(app), receivers)
	if err != nil {
		return false, fmt.Errorf("Error adding  %v", ID)
	}

	//Try to add a different receiver to existing object
	diffRecvr, err = sm.mbus.FindReceiver(ID_DIFFERNET)
	if err != nil {
		return false, fmt.Errorf("Receiver %v", ID)
	}


	receivers = []memdb.Receiver{diffRecvr}
	pushObj.AddObjReceivers(receivers)
	if err != nil {
		return false, fmt.Errorf("Receiver %v", ID)
	}

	//Remove Receivers (Delete will be sent)
	err = pushObj.RemoveObjReceivers(receivers)
	if err != nil {
		return false, fmt.Errorf("Receiver %v", ID)
	}

	//udpateObect as is
	pushObj.UpdateObjectWithReferences()

	pushObj.DeleteObjectWithReferences()

	if err != nil {
		return false, fmt.Errorf("Error adding  %v", ID)
	}
}


*/
// runPeriodicUpdater runs periodic and write objects back
func runPeriodicUpdater(queue chan updatable) {
	ticker := time.NewTicker(1 * time.Second)
	pending := make(map[string]updatable)
	shouldExit := false
	for {
		select {
		case obj, ok := <-queue:
			if ok == false {
				shouldExit = true
				continue
			}
			pending[obj.GetKey()] = obj
		case _ = <-ticker.C:
			failedUpdate := []updatable{}
			for _, obj := range pending {
				if err := obj.Write(); err != nil {
					failedUpdate = append(failedUpdate, obj)
				}
			}
			pending = make(map[string]updatable)
			for _, obj := range failedUpdate {
				pending[obj.GetKey()] = obj
			}
			if shouldExit == true {
				log.Warnf("Exiting periodic updater")
				return
			}
		}
	}
}

// NewPeriodicUpdater creates a new periodic updater
func newPeriodicUpdater() chan updatable {
	updateChan := make(chan updatable, maxUpdateChannelSize)
	go runPeriodicUpdater(updateChan)
	return updateChan
}

// PeriodicUpdaterPush enqueues an object to the periodic updater
func (sm *Statemgr) PeriodicUpdaterPush(obj updatable) {
	sm.periodicUpdaterQueue <- obj
}

// agentObjectMeta converts venice object meta to agent object meta
func agentObjectMeta(vmeta api.ObjectMeta) api.ObjectMeta {
	return api.ObjectMeta{
		Tenant:          vmeta.Tenant,
		Namespace:       vmeta.Namespace,
		Name:            vmeta.Name,
		GenerationID:    vmeta.GenerationID,
		ResourceVersion: vmeta.ResourceVersion,
		UUID:            vmeta.UUID,
	}
}

type apiServerObject interface {
	References(tenant string, path string, resp map[string]apiintf.ReferenceObj)
	GetObjectMeta() *api.ObjectMeta // returns the object meta
}

//wrapper to get references
func references(obj apiServerObject) map[string]apiintf.ReferenceObj {
	resp := make(map[string]apiintf.ReferenceObj)
	obj.References(obj.GetObjectMeta().Name, obj.GetObjectMeta().Namespace, resp)
	return resp
}

func fromVersionFilterFn(fromVer uint64) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		meta := obj.GetObjectMeta()
		ver, err := strconv.ParseUint(meta.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", meta.ResourceVersion, err)
		}
		return ver >= fromVer
	}
}

func nameFilterFn(name string) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Name == name
	}
}

func tenantFilterFn(tenant string) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Tenant == tenant
	}
}

func namespaceFilterFn(namespace string) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Namespace == namespace
	}
}

func labelSelectorFilterFn(selector *labels.Selector) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		meta := obj.GetObjectMeta()
		labels := labels.Set(meta.Labels)
		return selector.Matches(labels)
	}
}

func fieldSelectorFilterFn(selector *fields.Selector) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		return selector.MatchesObj(obj)
	}
}

func fieldChangeSelectorFilterFn(selectors []string) memdb.FilterFn {
	return func(obj, prev memdb.Object) bool {
		diffs, ok := ref.ObjDiff(obj, prev)
		if !ok {
			return false
		}
		for _, f := range selectors {
			if diffs.Lookup(f) {
				return true
			}
		}
		return false
	}
}

// GetWatchFilter returns a filter function to filter Watch Events
func (sm *Statemgr) GetWatchFilter(kind string, opts *api.ListWatchOptions) []memdb.FilterFn {
	var filters []memdb.FilterFn

	if opts.ResourceVersion != "" {
		ver, err := strconv.ParseUint(opts.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", opts.ResourceVersion, err)
		}
		filters = append(filters, fromVersionFilterFn(ver))
	}

	if opts.Name != "" {
		// FIX for VS-1305, with Naples running release A code and Venice running newer code, Naples will send the
		// node ID as the name for endpoints to filter.
		if kind == "netproto.Endpoint" {
			newOpts := &api.ListWatchOptions{}
			str := fmt.Sprintf("spec.node-uuid=%s", opts.Name)
			newOpts.FieldSelector = str
			return sm.GetWatchFilter(kind, newOpts)
		}
		filters = append(filters, nameFilterFn(opts.Name))
	}

	if opts.Tenant != "" {
		filters = append(filters, tenantFilterFn(opts.Tenant))
	}

	if opts.Namespace != "" {
		filters = append(filters, namespaceFilterFn(opts.Namespace))
	}

	if opts.LabelSelector != "" {
		selector, err := labels.Parse(opts.LabelSelector)
		if err != nil {
			log.Errorf("invalid label selector specification(%s)", err)
			return nil
		}
		filters = append(filters, labelSelectorFilterFn(selector))
	}

	if opts.FieldSelector != "" {
		var selector *fields.Selector
		var err error
		if kind != "" {
			selector, err = fields.ParseWithValidation(kind, opts.FieldSelector)
			if err != nil {
				log.Errorf("invalid field selector specification(%s)", err)
				return nil
			}
		} else {
			selector, err = fields.Parse(opts.FieldSelector)
			if err != nil {
				log.Errorf("invalid field selector specification(%s)", err)
				return nil
			}
		}
		filters = append(filters, fieldSelectorFilterFn(selector))
	}
	if len(opts.FieldChangeSelector) != 0 {
		filters = append(filters, fieldChangeSelectorFilterFn(opts.FieldChangeSelector))
	}
	return filters
}

//
type evStatus struct {
	Status map[string]bool
}

type nodeStatus struct {
	NodeID     string
	KindStatus map[string]*evStatus
}

type configPushStatus struct {
	KindObjects map[string]int
	NodesStatus []*nodeStatus
}

//DBWatch db watch
type DBWatch struct {
	Name              string
	status            string
	registeredCount   int
	unRegisteredCount int
}

//DBWatchers status of DB watchers
type DBWatchers struct {
	DBType   string
	Watchers []DBWatch
}

type dbWatchStatus struct {
	KindWatchers map[string]DBWatchers
}

type stat struct {
	MinMs, MaxMs, MeanMs float64
}

type nodeConfigHistogram struct {
	NodeID    string
	KindStats map[string]stat
	AggrStats stat
}

type configPushHistogram struct {
	KindHistogram       map[string]stat
	NodeConfigHistogram []*nodeConfigHistogram
}

//ResetConfigPushStats for debugging
func (sm *Statemgr) ResetConfigPushStats() {

	hdr.Reset("App")
	hdr.Reset("Endpoint")
	hdr.Reset("NetworkSecurityPolicy")

	objs := sm.ListObjects("DistributedServiceCard")
	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil {
			continue
		}

		hdr.Reset(snic.DistributedServiceCard.Name)

		resetKindStat := func(kind string) {
			kind = snic.DistributedServiceCard.Name + "_" + kind
			hdr.Reset(kind)
		}

		resetKindStat("App")
		resetKindStat("Endpoint")
		resetKindStat("NetworkSecurityPolicy")
	}
}

//GetConfigPushStats for debugging
func (sm *Statemgr) GetConfigPushStats() interface{} {
	pushStats := &configPushHistogram{}
	pushStats.KindHistogram = make(map[string]stat)
	sm.logger.Info("Querying histogram stats....")
	histStats := hdr.GetStats()
	sm.logger.Info("Querying histogram stats complete....")

	updateKindStat := func(kind string) {
		if hstat, ok := histStats[kind]; ok {
			pushStats.KindHistogram[kind] = stat{MaxMs: hstat.MaxMs,
				MeanMs: hstat.MeanMs, MinMs: hstat.MinMs}
		}
	}

	updateKindStat("App")
	updateKindStat("Endpoint")
	updateKindStat("NetworkSecurityPolicy")

	objs := sm.ListObjects("DistributedServiceCard")
	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil || !sm.isDscAdmitted(&snic.DistributedServiceCard.DistributedServiceCard) {
			continue
		}

		nodeStat, ok := histStats[snic.DistributedServiceCard.Name]
		if !ok {
			continue
		}
		nodeState := &nodeConfigHistogram{NodeID: snic.DistributedServiceCard.Name,
			AggrStats: stat{MaxMs: nodeStat.MaxMs,
				MeanMs: nodeStat.MeanMs, MinMs: nodeStat.MinMs}}
		nodeState.KindStats = make(map[string]stat)
		pushStats.NodeConfigHistogram = append(pushStats.NodeConfigHistogram, nodeState)

		updateKindStat := func(kind string) {
			kind = snic.DistributedServiceCard.Name + "_" + kind
			if hstat, ok := histStats[kind]; ok {
				nodeState.KindStats[kind] = stat{MaxMs: hstat.MaxMs,
					MeanMs: hstat.MeanMs, MinMs: hstat.MinMs}
			}
		}

		updateKindStat("App")
		updateKindStat("Endpoint")
		updateKindStat("NetworkSecurityPolicy")

	}
	return pushStats
}

//GetConfigPushStatus for debugging
func (sm *Statemgr) GetConfigPushStatus() interface{} {
	pushStatus := &configPushStatus{}
	pushStatus.KindObjects = make(map[string]int)
	objs := sm.ListObjects("DistributedServiceCard")
	apps, _ := sm.ListApps()
	pushStatus.KindObjects["App"] = len(apps)
	eps, _ := sm.ListEndpoints()
	pushStatus.KindObjects["Endpoint"] = len(eps)
	policies, _ := sm.ListSgpolicies()
	pushStatus.KindObjects["NetworkSecurityPolicy"] = len(policies)
	events := []api.EventType{api.EventType_CreateEvent, api.EventType_DeleteEvent, api.EventType_UpdateEvent}
	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil || !sm.isDscHealthy(&snic.DistributedServiceCard.DistributedServiceCard) {
			continue
		}

		nodeState := &nodeStatus{NodeID: snic.DistributedServiceCard.Name}
		nodeState.KindStatus = make(map[string]*evStatus)
		pushStatus.NodesStatus = append(pushStatus.NodesStatus, nodeState)

		nodeState.KindStatus["App"] = &evStatus{}
		nodeState.KindStatus["Endpoint"] = &evStatus{}
		nodeState.KindStatus["Network"] = &evStatus{}
		nodeState.KindStatus["NetworkSecurityPolicy"] = &evStatus{}
		nodeState.KindStatus["App"].Status = make(map[string]bool)
		nodeState.KindStatus["Endpoint"].Status = make(map[string]bool)
		nodeState.KindStatus["Network"].Status = make(map[string]bool)
		nodeState.KindStatus["NetworkSecurityPolicy"].Status = make(map[string]bool)
		for _, ev := range events {
			if !sm.topics.AggregateTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, "Network", ev) {
				nodeState.KindStatus["Network"].Status[ev.String()] = false
				log.Infof("SmartNic %v, Network not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["Network"].Status[ev.String()] = true
			}

			if !sm.topics.AggregateTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, "Endpoint", ev) {
				nodeState.KindStatus["Endpoint"].Status[ev.String()] = false
				log.Infof("SmartNic %v, Endpoint not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["Endpoint"].Status[ev.String()] = true
			}

			if !sm.topics.AggregateTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, "App", ev) {
				nodeState.KindStatus["App"].Status[ev.String()] = false
				log.Infof("SmartNic %v, App not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["App"].Status[ev.String()] = true
			}

			if !sm.topics.AggregateTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, "NetworkSecurityPolicy", ev) {
				nodeState.KindStatus["NetworkSecurityPolicy"].Status[ev.String()] = false
				log.Infof("SmartNic %v, NetworkSecurityPolicy not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["NetworkSecurityPolicy"].Status[ev.String()] = true
			}
		}
	}
	return pushStatus
}

//GetDBWatchStatus for debugging
func (sm *Statemgr) GetDBWatchStatus(kind string) interface{} {

	dbWatcher, _ := sm.mbus.GetDBWatchers(kind)
	return dbWatcher
}

//StartAppWatch stops App watch, used of testing
func (sm *Statemgr) StartAppWatch() {
	fmt.Printf("Starting App watch\n")
	sm.ctrler.App().Watch(sm)
}

//StartNetworkSecurityPolicyWatch stops App watch, used of testing
func (sm *Statemgr) StartNetworkSecurityPolicyWatch() {
	sm.ctrler.NetworkSecurityPolicy().Watch(sm)
}

//StopAppWatch stops App watch, used of testing
func (sm *Statemgr) StopAppWatch() {
	sm.ctrler.App().StopWatch(sm)
}

//StopNetworkSecurityPolicyWatch stops App watch, used of testing
func (sm *Statemgr) StopNetworkSecurityPolicyWatch() {
	sm.ctrler.NetworkSecurityPolicy().StopWatch(sm)
}
