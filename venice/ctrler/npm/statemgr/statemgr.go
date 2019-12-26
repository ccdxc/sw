// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/log"
	memdb "github.com/pensando/sw/venice/utils/memdb"
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

// Topics are the Nimbus message bus topics
type Topics struct {
	AppTopic                   *nimbus.AppTopic
	EndpointTopic              *nimbus.EndpointTopic
	NetworkTopic               *nimbus.NetworkTopic
	SecurityProfileTopic       *nimbus.SecurityProfileTopic
	NetworkSecurityPolicyTopic *nimbus.NetworkSecurityPolicyTopic
	NetworkInterfaceTopic      *nimbus.InterfaceTopic
	AggregateTopic             *nimbus.AggregateTopic
	IPAMPolicyTopic            *nimbus.IPAMPolicyTopic
}

// Statemgr is the object state manager
type Statemgr struct {
	sync.Mutex
	mbus                          *nimbus.MbusServer     // nimbus server
	periodicUpdaterQueue          chan updatable         // queue for periodically writing items back to apiserver
	ctrler                        ctkit.Controller       // controller instance
	topics                        Topics                 // message bus topics
	networkLocks                  map[string]*sync.Mutex // lock for performing network operation
	logger                        log.Logger
	ModuleReactor                 ctkit.ModuleHandler
	TenantReactor                 ctkit.TenantHandler
	SecurityGroupReactor          ctkit.SecurityGroupHandler
	AppReactor                    ctkit.AppHandler
	VirtualRouterReactor          ctkit.VirtualRouterHandler
	NetworkReactor                ctkit.NetworkHandler
	FirewallProfileReactor        ctkit.FirewallProfileHandler
	DistributedServiceCardReactor ctkit.DistributedServiceCardHandler
	HostReactor                   ctkit.HostHandler
	EndpointReactor               ctkit.EndpointHandler
	NetworkSecurityPolicyReactor  ctkit.NetworkSecurityPolicyHandler
	WorkloadReactor               ctkit.WorkloadHandler
	NetworkInterfaceReactor       ctkit.NetworkInterfaceHandler
	IPAMPolicyReactor             ctkit.IPAMPolicyHandler

	SecurityProfileStatusReactor       nimbus.SecurityProfileStatusReactor
	AppStatusReactor                   nimbus.AppStatusReactor
	NetworkStatusReactor               nimbus.NetworkStatusReactor
	NetworkInterfaceStatusReactor      nimbus.InterfaceStatusReactor
	EndpointStatusReactor              nimbus.EndpointStatusReactor
	NetworkSecurityPolicyStatusReactor nimbus.NetworkSecurityPolicyStatusReactor
	IPAMPolicyStatusReactor            nimbus.IPAMPolicyStatusReactor
	AggregateStatusReactor             nimbus.AggStatusReactor
}

// SetSecurityProfileStatusReactor sets the SecurityProfileStatusReactor
func (sm *Statemgr) SetSecurityProfileStatusReactor(handler nimbus.SecurityProfileStatusReactor) {
	sm.SecurityProfileStatusReactor = handler
}

// SetNetworkInterfaceStatusReactor sets the InterfaceStatusReactor
func (sm *Statemgr) SetNetworkInterfaceStatusReactor(handler nimbus.InterfaceStatusReactor) {
	sm.NetworkInterfaceStatusReactor = handler
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
	return sm.ctrler.Stop()
}

func initStatemgr() {
	singletonStatemgr = Statemgr{
		networkLocks: make(map[string]*sync.Mutex),
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

	sm.SetNetworkReactor(reactor)

	sm.SetFirewallProfileReactor(reactor)

	sm.SetDistributedServiceCardReactor(reactor)

	sm.SetHostReactor(reactor)

	sm.SetEndpointReactor(reactor)

	sm.SetNetworkSecurityPolicyReactor(reactor)

	sm.SetWorkloadReactor(reactor)

	sm.SetNetworkInterfaceReactor(reactor)

	sm.SetIPAMPolicyReactor(reactor)

}

// Run calls the feature statemgr callbacks and eastablishes the Watches
func (sm *Statemgr) Run(rpcServer *rpckit.RPCServer, apisrvURL string, rslvr resolver.Interface, mserver *nimbus.MbusServer, logger log.Logger, flags uint32, options ...Option) error {

	sm.mbus = mserver

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

	for name, svc := range featuremgrs {
		logger.Info("svc", name, "complete registration")
		svc.CompleteRegistration(flags)
	}

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
	err = ctrler.Network().Watch(sm.NetworkReactor)
	if err != nil {
		logger.Fatalf("Error watching network")
	}
	err = ctrler.FirewallProfile().Watch(sm.FirewallProfileReactor)
	if err != nil {
		logger.Fatalf("Error watching firewall profile")
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

	err = ctrler.IPAMPolicy().Watch(sm.IPAMPolicyReactor)
	if err != nil {
		logger.Fatalf("Error watching ipam-policy")
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
			for _, obj := range pending {
				obj.Write()
			}
			pending = make(map[string]updatable)
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

// GetWatchFilter returns a filter function to filter Watch Events
func (sm *Statemgr) GetWatchFilter(kind string, ometa *api.ListWatchOptions) func(memdb.Object) bool {
	switch kind {
	case "Endpoint":
		objMac, err := net.ParseMAC(ometa.Name)
		name := objMac.String()
		if err != nil {
			sm.logger.Infof("object meta does not have a valid mac - returning default func [%+v]", ometa)
			return func(memdb.Object) bool {
				return true
			}
		}
		return func(obj memdb.Object) bool {
			ep := obj.(*netproto.Endpoint)
			if inmac, err := net.ParseMAC(ep.Spec.NodeUUID); err == nil {
				return inmac.String() == name
			}
			return true
		}
	}
	return func(memdb.Object) bool {
		return true
	}
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
		nodeState.KindStatus["NetworkSecurityPolicy"] = &evStatus{}
		nodeState.KindStatus["App"].Status = make(map[string]bool)
		nodeState.KindStatus["Endpoint"].Status = make(map[string]bool)
		nodeState.KindStatus["NetworkSecurityPolicy"].Status = make(map[string]bool)
		for _, ev := range events {
			if !sm.topics.AppTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, ev) {
				log.Infof("SmartNic %v, App not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
				nodeState.KindStatus["App"].Status[ev.String()] = false
			} else {
				nodeState.KindStatus["App"].Status[ev.String()] = true
			}
			if !sm.topics.EndpointTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, ev) {
				nodeState.KindStatus["Endpoint"].Status[ev.String()] = false
				log.Infof("SmartNic %v, Endpoint not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["Endpoint"].Status[ev.String()] = true
			}
			if !sm.topics.NetworkSecurityPolicyTopic.WatcherInConfigSync(snic.DistributedServiceCard.Name, ev) {
				nodeState.KindStatus["NetworkSecurityPolicy"].Status[ev.String()] = false
				log.Infof("SmartNic %v, NetworkSecurityPolicy not in sync for ev : %v", snic.DistributedServiceCard.Name, ev)
			} else {
				nodeState.KindStatus["NetworkSecurityPolicy"].Status[ev.String()] = true
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
