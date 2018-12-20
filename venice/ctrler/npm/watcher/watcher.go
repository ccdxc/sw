// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// length of watcher channel
const watcherQueueLen = 1000

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// Watcher watches api server for changes
type Watcher struct {
	waitGrp          sync.WaitGroup                  // wait group to wait on all go routines to exit
	statemgr         *statemgr.Statemgr              // reference to state manager
	netWatcher       chan kvstore.WatchEvent         // network object watcher
	vmmEpWatcher     chan kvstore.WatchEvent         // vmm endpoint watcher
	sgWatcher        chan kvstore.WatchEvent         // sg object watcher
	sgPolicyWatcher  chan kvstore.WatchEvent         // sg object watcher
	tenantWatcher    chan kvstore.WatchEvent         // tenant object watcher
	hostWatcher      kvstore.Watcher                 // host watcher
	snicWatcher      kvstore.Watcher                 // smart nic watcher
	workloadWatcher  kvstore.Watcher                 // workload watcher
	appWatcher       kvstore.Watcher                 // app object watcher
	fwprofileWatcher kvstore.Watcher                 // firewall profile watcher
	workloadHandler  statemgr.WorkloadHandler        // workload event handler
	hostHandler      statemgr.HostHandler            // host event handler
	snicHandler      statemgr.SmartNICHandler        // smart nic event handler
	appHandler       statemgr.AppHandler             // app event handler
	fwprofileHandler statemgr.FirewallProfileHandler // firewall profile event handler
	watchCtx         context.Context                 // ctx for watchers
	watchCancel      context.CancelFunc              // cancel for watchers
	stopFlag         syncFlag                        // boolean flag to exit the API watchers
	debugStats       *debug.Stats                    // debug Stats
}

// handleNetworkEvent handles network event
func (w *Watcher) handleNetworkEvent(et kvstore.WatchEventType, nw *network.Network) {
	switch et {
	case kvstore.Created:
		w.debugStats.Increment("CreateNetwork")
		// ask statemgr to create the network
		err := w.statemgr.CreateNetwork(nw)
		if err != nil {
			log.Errorf("Error creating network {%+v}. Err: %v", nw.ObjectMeta, err)
			w.debugStats.Increment("CreateNetworkFails")
			return
		}

	case kvstore.Updated:
		// FIXME:
	case kvstore.Deleted:

		w.debugStats.Increment("DeleteNetwork")
		// ask statemgr to delete the network
		err := w.statemgr.DeleteNetwork(nw.Tenant, nw.Name)
		if err != nil {
			log.Errorf("Error deleting network {%+v}. Err: %v", nw.ObjectMeta, err)
			w.debugStats.Increment("DeleteNetworkFails")
			return
		}

	}
}

// handleEndpointEvent handles endpoint event
func (w *Watcher) handleEndpointEvent(et kvstore.WatchEventType, ep *workload.Endpoint) {
	// find the network
	nw, err := w.statemgr.FindNetwork(ep.ObjectMeta.Tenant, ep.Status.Network)
	if err != nil {
		log.Errorf("Could not find network %s|%s", ep.ObjectMeta.Tenant, ep.Status.Network)
		return
	}

	// handle based on event type
	switch et {
	case kvstore.Updated:
		fallthrough
	case kvstore.Created:
		// ask statemgr to create the endpoint
		_, err = nw.CreateEndpoint(ep)
		if err != nil {
			log.Errorf("Error creating endpoint {%+v}. Err: %v", ep.ObjectMeta, err)
			return
		}
	case kvstore.Deleted:
		// ask statemgr to delete the endpoint
		_, err := nw.DeleteEndpoint(&ep.ObjectMeta)
		if err != nil {
			log.Errorf("Error deleting endpoint {%+v}. Err: %v", ep.ObjectMeta, err)
			return
		}
	}
}

// handleSgEvent handles security group events
func (w *Watcher) handleSgEvent(et kvstore.WatchEventType, sg *security.SecurityGroup) {
	switch et {
	case kvstore.Created:
		// ask statemgr to create the network
		err := w.statemgr.CreateSecurityGroup(sg)
		if err != nil {
			log.Errorf("Error creating sg {%+v}. Err: %v", sg.ObjectMeta, err)
			return
		}
	case kvstore.Updated:
		// FIXME:
	case kvstore.Deleted:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteSecurityGroup(sg.Tenant, sg.Name)
		if err != nil {
			log.Errorf("Error deleting sg {%+v}. Err: %v", sg.ObjectMeta, err)
			return
		}
	}
}

// handleSgPolicyEvent handles sg policy events
func (w *Watcher) handleSgPolicyEvent(et kvstore.WatchEventType, sgp *security.SGPolicy) {
	switch et {
	case kvstore.Created:
		// ask statemgr to create the network
		err := w.statemgr.CreateSgpolicy(sgp)
		if err != nil {
			log.Errorf("Error creating sg policy {%+v}. Err: %v", sgp, err)
			return
		}
	case kvstore.Updated:
		err := w.statemgr.UpdateSgPolicy(sgp)
		if err != nil {
			log.Errorf("Error updating sg policy {%+v}. Err: %v", sgp, err)
			return
		}
	case kvstore.Deleted:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			log.Errorf("Error deleting sg policy {%+v}. Err: %v", sgp, err)
			return
		}
	}
}

// handleTenantEvent handles tenant event
func (w *Watcher) handleTenantEvent(et kvstore.WatchEventType, tn *cluster.Tenant) {
	switch et {
	case kvstore.Created:
		w.debugStats.Increment("CreateTenant")
		// ask statemgr to create the tenant
		err := w.statemgr.CreateTenant(tn)
		if err != nil {
			log.Errorf("Error creating tenant {%+v}. Err: %v", tn.ObjectMeta, err)
			w.debugStats.Increment("CreateTenantFails")
			return
		}

	case kvstore.Updated:
		// FIXME:
	case kvstore.Deleted:

		w.debugStats.Increment("DeleteTenant")
		// ask statemgr to delete the tenant
		err := w.statemgr.DeleteTenant(tn.Tenant)
		if err != nil {
			log.Errorf("Error deleting tenant {%+v}. Err: %v", tn.ObjectMeta, err)
			w.debugStats.Increment("DeleteTenantFails")
			return
		}

	}
}

// handleWorkloadEvent handles workload events from watcher
func (w *Watcher) handleWorkloadEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *workload.Workload:
		wrload := evt.Object.(*workload.Workload)

		log.Infof("Watcher: Got Workload watch event(%s): {%+v}", evt.Type, wrload)

		// handle based on event type
		switch evt.Type {
		case kvstore.Updated:
			fallthrough
		case kvstore.Created:
			// Call the event reactor
			err := w.workloadHandler.CreateWorkload(*wrload)
			if err != nil {
				log.Errorf("Error creating workload %+v. Err: %v", wrload, err)
			}
		case kvstore.Deleted:
			// Call the event reactor
			err := w.workloadHandler.DeleteWorkload(*wrload)
			if err != nil {
				log.Errorf("Error deleting workload %+v. Err: %v", wrload, err)
			}
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on workload watch channel", tp)
		return
	}
}

// handleHostEvent handles host events from watcher
func (w *Watcher) handleHostEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *cluster.Host:
		host := evt.Object.(*cluster.Host)

		log.Infof("Watcher: Got Host watch event(%s): {%+v}", evt.Type, host)

		// handle based on event type
		switch evt.Type {
		case kvstore.Updated:
			fallthrough
		case kvstore.Created:
			// Call the event reactor
			err := w.hostHandler.CreateHost(*host)
			if err != nil {
				log.Errorf("Error creating host %+v. Err: %v", host, err)
			}
		case kvstore.Deleted:
			// Call the event reactor
			err := w.hostHandler.DeleteHost(*host)
			if err != nil {
				log.Errorf("Error deleting host %+v. Err: %v", host, err)
			}
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on host watch channel", tp)
		return
	}
}

// handleSnicEvent handles workload events from watcher
func (w *Watcher) handleSnicEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *cluster.SmartNIC:
		snic := evt.Object.(*cluster.SmartNIC)

		log.Infof("Watcher: Got SmartNIC watch event(%s): {%+v}", evt.Type, snic)

		// handle based on event type
		switch evt.Type {
		case kvstore.Updated:
			fallthrough
		case kvstore.Created:
			// Call the event reactor
			err := w.snicHandler.CreateSmartNIC(*snic)
			if err != nil {
				log.Errorf("Error creating smart nic %+v. Err: %v", snic, err)
			}
		case kvstore.Deleted:
			// Call the event reactor
			err := w.snicHandler.DeleteSmartNIC(*snic)
			if err != nil {
				log.Errorf("Error deleting smart nic %+v. Err: %v", snic, err)
			}
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on smart nic watch channel", tp)
		return
	}
}

// handleAppEvent handles app events from watcher
func (w *Watcher) handleAppEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *security.App:
		app := evt.Object.(*security.App)

		log.Infof("Watcher: Got App watch event(%s): {%+v}", evt.Type, app)

		// handle based on event type
		switch evt.Type {
		case kvstore.Updated:
			fallthrough
		case kvstore.Created:
			// Call the event reactor
			err := w.appHandler.CreateApp(*app)
			if err != nil {
				log.Errorf("Error creating app %+v. Err: %v", app, err)
			}
		case kvstore.Deleted:
			// Call the event reactor
			err := w.appHandler.DeleteApp(*app)
			if err != nil {
				log.Errorf("Error deleting app %+v. Err: %v", app, err)
			}
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on app watch channel", tp)
		return
	}
}

// handleFwprofileEvent handles workload events from watcher
func (w *Watcher) handleFwprofileEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *security.FirewallProfile:
		fwp := evt.Object.(*security.FirewallProfile)

		log.Infof("Watcher: Got FirewallProfile watch event(%s): {%+v}", evt.Type, fwp)

		// handle based on event type
		switch evt.Type {
		case kvstore.Updated:
			fallthrough
		case kvstore.Created:
			// Call the event reactor
			err := w.fwprofileHandler.CreateFirewallProfile(*fwp)
			if err != nil {
				log.Errorf("Error creating firewall profile %+v. Err: %v", fwp, err)
			}
		case kvstore.Deleted:
			// Call the event reactor
			err := w.fwprofileHandler.DeleteFirewallProfile(*fwp)
			if err != nil {
				log.Errorf("Error deleting firewall profile %+v. Err: %v", fwp, err)
			}
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on workload watch channel", tp)
		return
	}
}

// runNetwatcher watches on a channel for changes from api server
func (w *Watcher) runNetwatcher() {
	log.Infof("Network watcher running")

	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.netWatcher:
			// if channel has error, we are done..
			if !ok {
				if w.stopped() {
					return
				}
				log.Infof("Restarting network watcher")
				w.netWatcher = make(chan kvstore.WatchEvent, watcherQueueLen)
				go w.runNetwatcher()
				return
			}

			// convert to network object
			var nw *network.Network
			switch tp := evt.Object.(type) {
			case *network.Network:
				nw = evt.Object.(*network.Network)
			default:
				log.Fatalf("network watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got network watch event(%s): {%+v}", evt.Type, nw)

			// TODO make sure we honor the order of events when handling events in parallel
			go w.handleNetworkEvent(evt.Type, nw)
		}
	}
}

// runVmmEpwatcher watches on a channel for changes from VMM mgr
func (w *Watcher) runVmmEpwatcher() {
	log.Infof("VMM watcher watcher running")

	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.vmmEpWatcher:
			// if channel has error, we are done..
			if !ok {
				if w.stopped() {
					return
				}
				log.Infof("Restarting vmm ep watcher")
				w.vmmEpWatcher = make(chan kvstore.WatchEvent, watcherQueueLen)
				go w.runVmmEpwatcher()
				return
			}

			// convert to endpoint object
			var ep *workload.Endpoint
			switch tp := evt.Object.(type) {
			case *workload.Endpoint:
				ep = evt.Object.(*workload.Endpoint)
			default:
				log.Fatalf("vmm watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got vmm endpoint watch event(%s): {%+v}", evt.Type, ep)

			// TODO process each event in its own go routine
			w.handleEndpointEvent(evt.Type, ep)
		}
	}
}

// runSgwatcher watches on a channel for changes from api server
func (w *Watcher) runSgwatcher() {
	log.Infof("SecurityGroup watcher running")

	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.sgWatcher:
			// if channel has error, we are done..
			if !ok {
				if w.stopped() {
					return
				}
				log.Infof("Restarting security group watcher")
				w.sgWatcher = make(chan kvstore.WatchEvent, watcherQueueLen)
				go w.runSgwatcher()
				return
			}

			// convert to sg object
			var ep *security.SecurityGroup
			switch tp := evt.Object.(type) {
			case *security.SecurityGroup:
				ep = evt.Object.(*security.SecurityGroup)
			default:
				log.Fatalf("sg watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got SecurityGroup watch event(%s): {%+v}", evt.Type, ep)

			// TODO process each event in its own go routine
			w.handleSgEvent(evt.Type, ep)
		}
	}
}

// runSgPolicyWatcher watches on a channel for changes from api server
func (w *Watcher) runSgPolicyWatcher() {
	log.Infof("SgPolicy watcher running")
	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.sgPolicyWatcher:
			// if channel has error, we are done..
			if !ok {
				if w.stopped() {
					return
				}
				log.Infof("Restarting security group policy watcher")
				w.sgPolicyWatcher = make(chan kvstore.WatchEvent, watcherQueueLen)
				go w.runSgPolicyWatcher()
				return
			}

			// convert to sg policy object
			var sgp *security.SGPolicy
			switch tp := evt.Object.(type) {
			case *security.SGPolicy:
				sgp = evt.Object.(*security.SGPolicy)
			default:
				log.Fatalf("sg policy watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got SgPolicy watch event(%s): {%+v}", evt.Type, sgp)

			// TODO process each event in its own go routine
			w.handleSgPolicyEvent(evt.Type, sgp)
		}
	}
}

// runTenantwatcher watches on a channel for changes from api server
func (w *Watcher) runTenantwatcher() {

	log.Infof("Tenant watcher running")

	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.tenantWatcher:
			// if channel has error, we are done..
			if !ok {
				if w.stopped() {
					return
				}
				log.Infof("Restarting tenant watcher")
				w.tenantWatcher = make(chan kvstore.WatchEvent, watcherQueueLen)
				go w.runTenantwatcher()
				return
			}

			// convert to tenant object
			var tn *cluster.Tenant
			switch tp := evt.Object.(type) {
			case *cluster.Tenant:
				tn = evt.Object.(*cluster.Tenant)
			default:
				log.Fatalf("tenant watcher Found object of invalid type: %v, %v", tp, evt)
				return
			}

			log.Infof("Watcher: Got tenant watch event(%s): {%+v}", evt.Type, tn)

			// TODO process each event in its own go routine
			w.handleTenantEvent(evt.Type, tn)
		}
	}
}

// Stop watcher
func (w *Watcher) Stop() {
	// stop the context
	w.stop()
	w.watchCancel()

	// close the channels
	close(w.netWatcher)
	close(w.sgPolicyWatcher)
	close(w.sgWatcher)
	close(w.vmmEpWatcher)
	close(w.tenantWatcher)

	// wait for all goroutines to exit
	w.waitGrp.Wait()
}

// NewWatcher returns a new watcher object
func NewWatcher(sm *statemgr.Statemgr, apisrvURL, vmmURL string, resolver resolver.Interface, debugStats *debug.Stats) (*Watcher, error) {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create a watcher
	watcher := &Watcher{
		statemgr:         sm,
		netWatcher:       make(chan kvstore.WatchEvent, watcherQueueLen),
		vmmEpWatcher:     make(chan kvstore.WatchEvent, watcherQueueLen),
		sgWatcher:        make(chan kvstore.WatchEvent, watcherQueueLen),
		sgPolicyWatcher:  make(chan kvstore.WatchEvent, watcherQueueLen),
		tenantWatcher:    make(chan kvstore.WatchEvent, watcherQueueLen),
		workloadHandler:  sm.WorkloadReactor(),
		hostHandler:      sm.HostReactor(),
		snicHandler:      sm.SmartNICReactor(),
		fwprofileHandler: sm.FirewallProfileReactor(),
		appHandler:       sm.AppReactor(),

		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		debugStats:  debugStats,
		stopFlag:    syncFlag{flag: false},
	}

	// start a go routine to handle messages coming on watcher channel
	go watcher.runNetwatcher()
	go watcher.runVmmEpwatcher()
	go watcher.runSgwatcher()
	go watcher.runSgPolicyWatcher()
	go watcher.runTenantwatcher()

	// handle api watchers
	go watcher.runApisrvWatcher(watchCtx, apisrvURL, resolver)
	go watcher.runVmmWatcher(watchCtx, vmmURL, resolver)

	return watcher, nil
}

func (w *Watcher) stopped() bool {
	w.stopFlag.RLock()

	defer w.stopFlag.RUnlock()
	return w.stopFlag.flag
}

func (w *Watcher) stop() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// ******************* Functions for testing and emulating *********************

// CreateNetwork injects a create network event on the watcher
func (w *Watcher) CreateNetwork(tenant, namespace, net, subnet, gw string) error {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: namespace,
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &nw,
	}
	// inject the object into the network watcher
	w.netWatcher <- evt

	return nil
}

// DeleteNetwork injects a delete network event to the watcher
func (w *Watcher) DeleteNetwork(tenant, net string) error {
	// create a dummy network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:   net,
			Tenant: tenant,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &nw,
	}
	// inject the object into the network watcher
	w.netWatcher <- evt

	return nil
}

// CreateEndpoint injects an endpoint create event
func (w *Watcher) CreateEndpoint(tenant, namespace, net, epName, vmName, macAddr, hostName, hostAddr string, attr map[string]string, usegVlan uint32) error {
	epInfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      epName,
			Tenant:    tenant,
			Namespace: namespace,
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			Network:            net,
			WorkloadName:       vmName,
			WorkloadAttributes: attr,
			MacAddress:         macAddr,
			HomingHostAddr:     hostAddr,
			HomingHostName:     hostName,
			MicroSegmentVlan:   usegVlan,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &epInfo,
	}
	// inject the object into the network watcher
	w.vmmEpWatcher <- evt

	return nil
}

// DeleteEndpoint injects an endpoint delete event
func (w *Watcher) DeleteEndpoint(tenant, net, epName, vmName, macAddr, hostName, hostAddr string) error {
	epInfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:   epName,
			Tenant: tenant,
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			Network:        net,
			WorkloadName:   vmName,
			MacAddress:     macAddr,
			HomingHostAddr: hostAddr,
			HomingHostName: hostName,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &epInfo,
	}
	// inject the object into the network watcher
	w.vmmEpWatcher <- evt

	log.Infof("Injected endpoint Event %s. Ep: {%+v}", evt.Type, epInfo.ObjectMeta)

	return nil
}

// CreateSecurityGroup injects a create sg event on the watcher
func (w *Watcher) CreateSecurityGroup(tenant, namespace, sgname string, selector *labels.Selector) error {
	// build sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &sg,
	}
	// inject the object into the sg watcher
	w.sgWatcher <- evt

	return nil
}

// DeleteSecurityGroup injects a delete sg event to the watcher
func (w *Watcher) DeleteSecurityGroup(tenant, sgname string) error {
	// build a sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Name:   sgname,
			Tenant: tenant,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &sg,
	}

	// inject the object into the sg watcher
	w.sgWatcher <- evt

	return nil
}

// CreateSgpolicy injects a create sg policy event on the watcher
func (w *Watcher) CreateSgpolicy(tenant, namespace, pname string, attachTenant bool, attachGroups []string, rules []security.SGRule) error {
	// build sg object
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      pname,
		},
		Spec: security.SGPolicySpec{
			AttachTenant: attachTenant,
			AttachGroups: attachGroups,
			Rules:        rules,
		},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &sgp,
	}

	// inject the object into the sg policy watcher
	w.sgPolicyWatcher <- evt

	return nil
}

// SgpolicyEvent injects an sgp event
func (w *Watcher) SgpolicyEvent(etype kvstore.WatchEventType, sgp *security.SGPolicy) error {
	evt := kvstore.WatchEvent{
		Type:   etype,
		Object: sgp,
	}

	// inject the object into the sg policy watcher
	w.sgPolicyWatcher <- evt

	return nil
}

// DeleteSgpolicy injects a delete sg policy event to the watcher
func (w *Watcher) DeleteSgpolicy(tenant, namespace, pname string) error {
	// build a sg object
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      pname,
			Namespace: namespace,
			Tenant:    tenant,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &sgp,
	}

	// inject the object into the sg watcher
	w.sgPolicyWatcher <- evt

	return nil
}

// CreateTenant injects a create tenant event on the watcher
func (w *Watcher) CreateTenant(tenant, namespace string) error {
	// build network object
	tn := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Namespace: namespace,
			Name:      tenant,
		},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &tn,
	}
	// inject the object into the network watcher
	w.tenantWatcher <- evt

	return nil
}

// DeleteTenant injects a delete network event to the watcher
func (w *Watcher) DeleteTenant(tenant string) error {
	// create a dummy tenant object
	tn := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name:   tenant,
			Tenant: tenant,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &tn,
	}
	// inject the object into the tenant watcher
	w.tenantWatcher <- evt

	return nil
}

// CreateWorkload creates a workload
func (w *Watcher) CreateWorkload(tenant, namespace, name, host, macAddr string, usegVlan, extVlan uint32) error {
	// build workload object
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: namespace,
			Tenant:    tenant,
		},
		Spec: workload.WorkloadSpec{
			HostName: host,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   macAddr,
					MicroSegVlan: usegVlan,
					ExternalVlan: extVlan,
				},
			},
		},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &wr,
	}

	// handle workload event
	w.handleWorkloadEvent(&evt)

	return nil
}

// DeleteWorkload deletes a workload
func (w *Watcher) DeleteWorkload(tenant, namespace, name, host, macAddr string, usegVlan, extVlan uint32) error {
	// create a dummy workload object
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: namespace,
			Tenant:    tenant,
		},
		Spec: workload.WorkloadSpec{
			HostName: host,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   macAddr,
					MicroSegVlan: usegVlan,
					ExternalVlan: extVlan,
				},
			},
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &wr,
	}
	// ihandle workload event
	w.handleWorkloadEvent(&evt)

	return nil
}

// CreateHost creates a host and an associated smart nic
func (w *Watcher) CreateHost(name, macAddr string) error {
	// smartNic params
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      name + "-NIC",
			Namespace: "",
			Tenant:    "default",
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     macAddr,
		},
	}

	snicEvt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &snic,
	}

	// handle snic event
	w.handleSnicEvent(&snicEvt)

	// build host object
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: macAddr,
				},
			},
		},
		Status: cluster.HostStatus{},
	}

	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &host,
	}

	// handle host event
	w.handleHostEvent(&evt)

	return nil
}

// DeleteHost deletes a workload
func (w *Watcher) DeleteHost(name string) error {
	// create a dummy tenant object
	wr := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}

	// create the watch event
	evt := kvstore.WatchEvent{
		Type:   kvstore.Deleted,
		Object: &wr,
	}
	// ihandle host event
	w.handleHostEvent(&evt)

	// smartNic object
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      name + "-NIC",
			Namespace: "",
			Tenant:    "default",
		},
	}

	snicEvt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &snic,
	}

	// handle snic event
	w.handleSnicEvent(&snicEvt)
	return nil
}
