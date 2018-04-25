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
	waitGrp         sync.WaitGroup          // wait group to wait on all go routines to exit
	statemgr        *statemgr.Statemgr      // reference to state manager
	netWatcher      chan kvstore.WatchEvent // network object watcher
	vmmEpWatcher    chan kvstore.WatchEvent // vmm endpoint watcher
	sgWatcher       chan kvstore.WatchEvent // sg object watcher
	sgPolicyWatcher chan kvstore.WatchEvent // sg object watcher
	tenantWatcher   chan kvstore.WatchEvent // tenant object watcher
	watchCtx        context.Context         // ctx for watchers
	watchCancel     context.CancelFunc      // cancel for watchers
	stopFlag        syncFlag                // boolean flag to exit the API watchers
	debugStats      *debug.Stats            // debug Stats
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
func (w *Watcher) handleSgPolicyEvent(et kvstore.WatchEventType, sgp *security.Sgpolicy) {
	switch et {
	case kvstore.Created:
		// ask statemgr to create the network
		err := w.statemgr.CreateSgpolicy(sgp)
		if err != nil {
			log.Errorf("Error creating sg policy {%+v}. Err: %v", sgp.ObjectMeta, err)
			return
		}
	case kvstore.Updated:
		// FIXME:
	case kvstore.Deleted:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			log.Errorf("Error deleting sg policy {%+v}. Err: %v", sgp.ObjectMeta, err)
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

			log.Infof("Watcher: Got network watch event(%s): {%+v}", evt.Type, nw.ObjectMeta)

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

			log.Infof("Watcher: Got vmm endpoint watch event(%s): {%+v}", evt.Type, ep.ObjectMeta)

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

			log.Infof("Watcher: Got SecurityGroup watch event(%s): {%+v}", evt.Type, ep.ObjectMeta)

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
			var sgp *security.Sgpolicy
			switch tp := evt.Object.(type) {
			case *security.Sgpolicy:
				sgp = evt.Object.(*security.Sgpolicy)
			default:
				log.Fatalf("sg policy watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got SgPolicy watch event(%s): {%+v}", evt.Type, sgp.ObjectMeta)

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

			log.Infof("Watcher: Got tenant watch event(%s): {%+v}", evt.Type, tn.ObjectMeta)

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
func NewWatcher(statemgr *statemgr.Statemgr, apisrvURL, vmmURL string, resolver resolver.Interface, debugStats *debug.Stats) (*Watcher, error) {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create a watcher
	watcher := &Watcher{
		statemgr:        statemgr,
		netWatcher:      make(chan kvstore.WatchEvent, watcherQueueLen),
		vmmEpWatcher:    make(chan kvstore.WatchEvent, watcherQueueLen),
		sgWatcher:       make(chan kvstore.WatchEvent, watcherQueueLen),
		sgPolicyWatcher: make(chan kvstore.WatchEvent, watcherQueueLen),
		tenantWatcher:   make(chan kvstore.WatchEvent, watcherQueueLen),
		watchCtx:        watchCtx,
		watchCancel:     watchCancel,
		debugStats:      debugStats,
		stopFlag: syncFlag{
			flag: false,
		},
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

func (w *Watcher) stopped() (val bool) {
	w.stopFlag.RLock()

	defer w.stopFlag.RUnlock()
	val = w.stopFlag.flag
	return
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
			EndpointUUID:       epName,
			WorkloadName:       vmName,
			WorkloadUUID:       vmName,
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
			EndpointUUID:   epName,
			WorkloadName:   vmName,
			WorkloadUUID:   vmName,
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
func (w *Watcher) CreateSgpolicy(tenant, namespace, pname string, attachGroups []string, inrules, outrules []security.SGRule) error {
	// build sg object
	sgp := security.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      pname,
		},
		Spec: security.SgpolicySpec{
			AttachGroups: attachGroups,
			InRules:      inrules,
			OutRules:     outrules,
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

// DeleteSgpolicy injects a delete sg policy event to the watcher
func (w *Watcher) DeleteSgpolicy(tenant, pname string) error {
	// build a sg object
	sgp := security.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:   pname,
			Tenant: tenant,
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
			Tenant:    tenant,
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
