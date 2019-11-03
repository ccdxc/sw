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

// maxUpdateChannelSize is the size of the update pending channel
const maxUpdateChannelSize = 1000

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
	SecurityGroupTopic         *nimbus.SecurityGroupTopic
	NetworkSecurityPolicyTopic *nimbus.NetworkSecurityPolicyTopic
	NetworkInterfaceTopic      *nimbus.InterfaceTopic
	AggregateTopic             *nimbus.AggregateTopic
}

// Statemgr is the object state manager
type Statemgr struct {
	sync.Mutex
	mbus                 *nimbus.MbusServer     // nimbus server
	periodicUpdaterQueue chan updatable         // queue for periodically writing items back to apiserver
	ctrler               ctkit.Controller       // controller instance
	topics               Topics                 // message bus topics
	networkLocks         map[string]*sync.Mutex // lock for performing network operation
	logger               log.Logger
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

// ListObjects list all objects of a kind
func (sm *Statemgr) ListObjects(kind string) []runtime.Object {
	return sm.ctrler.ListObjects(kind)
}

// Stop stops the watchers
func (sm *Statemgr) Stop() error {
	return sm.ctrler.Stop()
}

// NewStatemgr creates a new state manager object
func NewStatemgr(rpcServer *rpckit.RPCServer, apisrvURL string, rslvr resolver.Interface, mserver *nimbus.MbusServer, logger log.Logger, options ...Option) (*Statemgr, error) {
	// create new statemgr instance
	statemgr := &Statemgr{
		mbus:         mserver,
		networkLocks: make(map[string]*sync.Mutex),
		logger:       logger,
	}

	// create controller instance
	ctrler, err := ctkit.NewController(globals.Npm, rpcServer, apisrvURL, rslvr, logger)
	if err != nil {
		logger.Fatalf("Error creating controller. Err: %v", err)
	}
	statemgr.ctrler = ctrler

	for _, o := range options {
		o(statemgr)
	}

	// newPeriodicUpdater creates a new go subroutines
	// Given that objects returned by `NewStatemgr` should live for the duration
	// of the process, we don't have to worry about leaked go subroutines
	statemgr.periodicUpdaterQueue = newPeriodicUpdater()

	// start all object watches
	// there is a specific order we do these watches to meet dependency requirements
	// 1. apps before sgpolicies
	// 2. endpoints before workload
	// 3. smartnics before hosts
	// 4. security-groups before sgpolicies
	err = ctrler.Module().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching module object")
	}
	err = ctrler.Tenant().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching sg policy")
	}

	err = ctrler.SecurityGroup().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching security group")
	}
	err = ctrler.App().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching app")
	}

	err = ctrler.Network().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching network")
	}
	err = ctrler.FirewallProfile().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching firewall profile")
	}
	err = ctrler.DistributedServiceCard().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching smartnic")
	}
	err = ctrler.Host().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching host")
	}
	err = ctrler.Endpoint().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching endpoint")
	}
	err = ctrler.NetworkSecurityPolicy().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching sg policy")
	}
	err = ctrler.Workload().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching workloads")
	}
	err = ctrler.NetworkInterface().Watch(statemgr)
	if err != nil {
		logger.Fatalf("Error watching network-interface")
	}

	//Remove state endpoints after we start the watch
	//Start watch would synchronosly does diff of all workload and endpoints.
	//Stale endpoints would then be deleted.
	statemgr.RemoveStaleEndpoints()

	// create all topics on the message bus
	statemgr.topics.EndpointTopic, err = nimbus.AddEndpointTopic(mserver, statemgr)
	if err != nil {
		logger.Errorf("Error starting endpoint RPC server")
		return nil, err
	}
	statemgr.topics.AppTopic, err = nimbus.AddAppTopic(mserver, nil)
	if err != nil {
		logger.Errorf("Error starting App RPC server")
		return nil, err
	}
	statemgr.topics.SecurityProfileTopic, err = nimbus.AddSecurityProfileTopic(mserver, statemgr)
	if err != nil {
		logger.Errorf("Error starting SecurityProfile RPC server")
		return nil, err
	}
	statemgr.topics.SecurityGroupTopic, err = nimbus.AddSecurityGroupTopic(mserver, nil)
	if err != nil {
		logger.Errorf("Error starting SG RPC server")
		return nil, err
	}
	statemgr.topics.NetworkSecurityPolicyTopic, err = nimbus.AddNetworkSecurityPolicyTopic(mserver, statemgr)
	if err != nil {
		logger.Errorf("Error starting SG policy RPC server")
		return nil, err
	}
	statemgr.topics.NetworkTopic, err = nimbus.AddNetworkTopic(mserver, nil)
	if err != nil {
		logger.Errorf("Error starting network RPC server")
		return nil, err
	}
	statemgr.topics.NetworkInterfaceTopic, err = nimbus.AddInterfaceTopic(mserver, statemgr)
	if err != nil {
		log.Errorf("Error starting network interface RPC server")
		return nil, err
	}

	statemgr.topics.AggregateTopic, err = nimbus.AddAggregateTopic(mserver, statemgr)
	if err != nil {
		log.Errorf("Error starting Aggregate RPC server")
		return nil, err
	}

	return statemgr, nil
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
func (sm *Statemgr) GetWatchFilter(kind string, ometa *api.ObjectMeta) func(memdb.Object) bool {
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
