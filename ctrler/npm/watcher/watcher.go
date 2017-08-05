// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"github.com/Sirupsen/logrus"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	"github.com/pensando/sw/utils/runtime"
)

// length of watcher channel
const watcherQueueLen = 1000

// EventType watch event types
type EventType string

// defined event types
const (
	CreateEvent EventType = "Create"
	UpdateEvent EventType = "Update"
	DeleteEvent EventType = "Delete"
)

// WatchEvent is the watch event message
type WatchEvent struct {
	EventType EventType // type of the event
	runtime.Object
}

// Watcher watches api server for changes
type Watcher struct {
	statemgr        *statemgr.Statemgr // reference to network manager
	netWatcher      chan WatchEvent    // network object watcher
	vmmEpWatcher    chan WatchEvent    // vmm endpoint watcher
	sgWatcher       chan WatchEvent    // sg object watcher
	sgPolicyWatcher chan WatchEvent    // sg object watcher

}

// handleNetworkEvent handles network event
func (w *Watcher) handleNetworkEvent(et EventType, nw *network.Network) {
	switch et {
	case CreateEvent:
		// ask statemgr to create the network
		err := w.statemgr.CreateNetwork(nw)
		if err != nil {
			logrus.Errorf("Error creating network {%+v}. Err: %v", nw, err)
			return
		}
	case UpdateEvent:
		// FIXME:
	case DeleteEvent:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteNetwork(nw.Tenant, nw.Name)
		if err != nil {
			logrus.Errorf("Error deleting network {%+v}. Err: %v", nw, err)
			return
		}
	}
}

// handleEndpointEvent handles endpoint event
func (w *Watcher) handleEndpointEvent(et EventType, ep *network.Endpoint) {
	// find the network
	nw, err := w.statemgr.FindNetwork(ep.ObjectMeta.Tenant, ep.Status.Network)
	if err != nil {
		logrus.Errorf("Could not find network %s|%s", ep.ObjectMeta.Tenant, ep.Status.Network)
		return
	}

	// handle based on event type
	switch et {
	case CreateEvent:
		// ask statemgr to create the endpoint
		_, err = nw.CreateEndpoint(ep)
		if err != nil {
			logrus.Errorf("Error creating endpoint {%+v}. Err: %v", ep, err)
			return
		}
	case UpdateEvent:
		// FIXME:
	case DeleteEvent:
		// ask statemgr to delete the endpoint
		_, err := nw.DeleteEndpoint(&ep.ObjectMeta)
		if err != nil {
			logrus.Errorf("Error deleting endpoint {%+v}. Err: %v", ep, err)
			return
		}
	}
}

// handleSgEvent handles security group events
func (w *Watcher) handleSgEvent(et EventType, sg *network.SecurityGroup) {
	switch et {
	case CreateEvent:
		// ask statemgr to create the network
		err := w.statemgr.CreateSecurityGroup(sg)
		if err != nil {
			logrus.Errorf("Error creating sg {%+v}. Err: %v", sg, err)
			return
		}
	case UpdateEvent:
		// FIXME:
	case DeleteEvent:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteSecurityGroup(sg.Tenant, sg.Name)
		if err != nil {
			logrus.Errorf("Error deleting sg {%+v}. Err: %v", sg, err)
			return
		}
	}
}

// handleSgPolicyEvent handles sg policy events
func (w *Watcher) handleSgPolicyEvent(et EventType, sgp *network.Sgpolicy) {
	switch et {
	case CreateEvent:
		// ask statemgr to create the network
		err := w.statemgr.CreateSgpolicy(sgp)
		if err != nil {
			logrus.Errorf("Error creating sg policy {%+v}. Err: %v", sgp, err)
			return
		}
	case UpdateEvent:
		// FIXME:
	case DeleteEvent:
		// ask statemgr to delete the network
		err := w.statemgr.DeleteSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			logrus.Errorf("Error deleting sg policy {%+v}. Err: %v", sgp, err)
			return
		}
	}
}

// runNetwatcher watches on a channel for changes from api server
func (w *Watcher) runNetwatcher() {
	logrus.Infof("Network watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.netWatcher:
			// if channel has error, we are done..
			if !ok {
				logrus.Infof("Exiting network watcher(channel closed)")
				return
			}

			// convert to network object
			var nw *network.Network
			switch tp := evt.Object.(type) {
			case *network.Network:
				nw = evt.Object.(*network.Network)
			default:
				logrus.Fatalf("network watcher Found object of invalid type: %v", tp)
				return
			}

			logrus.Infof("Watcher: Got network watch event: {%+v}", nw)

			// process each event in its own go routine
			go w.handleNetworkEvent(evt.EventType, nw)
		}
	}
}

// runVmmEpwatcher watches on a channel for changes from VMM mgr
func (w *Watcher) runVmmEpwatcher() {
	logrus.Infof("VMM watcher watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.vmmEpWatcher:
			// if channel has error, we are done..
			if !ok {
				logrus.Infof("Exiting network watcher(channel closed)")
				return
			}

			// convert to network object
			var ep *network.Endpoint
			switch tp := evt.Object.(type) {
			case *network.Endpoint:
				ep = evt.Object.(*network.Endpoint)
			default:
				logrus.Fatalf("vmm watcher Found object of invalid type: %v", tp)
				return
			}

			logrus.Infof("Watcher: Got vmm endpoint watch event(%s): {%+v}", evt.EventType, ep)

			// process each event in its own go routine
			go w.handleEndpointEvent(evt.EventType, ep)
		}
	}
}

// runSgwatcher watches on a channel for changes from api server
func (w *Watcher) runSgwatcher() {
	logrus.Infof("SecurityGroup watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.sgWatcher:
			// if channel has error, we are done..
			if !ok {
				logrus.Infof("Exiting sg watcher(channel closed)")
				return
			}

			// convert to sg object
			var ep *network.SecurityGroup
			switch tp := evt.Object.(type) {
			case *network.SecurityGroup:
				ep = evt.Object.(*network.SecurityGroup)
			default:
				logrus.Fatalf("sg watcher Found object of invalid type: %v", tp)
				return
			}

			logrus.Infof("Watcher: Got SecurityGroup watch event(%s): {%+v}", evt.EventType, ep)

			// process each event in its own go routine
			go w.handleSgEvent(evt.EventType, ep)
		}
	}
}

// runSgPolicyWatcher watches on a channel for changes from api server
func (w *Watcher) runSgPolicyWatcher() {
	logrus.Infof("SgPolicy watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-w.sgPolicyWatcher:
			// if channel has error, we are done..
			if !ok {
				logrus.Infof("Exiting sg policy watcher(channel closed)")
				return
			}

			// convert to sg object
			var sgp *network.Sgpolicy
			switch tp := evt.Object.(type) {
			case *network.Sgpolicy:
				sgp = evt.Object.(*network.Sgpolicy)
			default:
				logrus.Fatalf("sg policy watcher Found object of invalid type: %v", tp)
				return
			}

			logrus.Infof("Watcher: Got SgPolicy watch event(%s): {%+v}", evt.EventType, sgp)

			// process each event in its own go routine
			go w.handleSgPolicyEvent(evt.EventType, sgp)
		}
	}
}

// NewWatcher returns a new watcher object
func NewWatcher(statemgr *statemgr.Statemgr) (*Watcher, error) {
	// create a watcher
	// FIXME: we need to hook this to api server watch once its ready.
	watcher := &Watcher{
		statemgr:        statemgr,
		netWatcher:      make(chan WatchEvent, watcherQueueLen),
		vmmEpWatcher:    make(chan WatchEvent, watcherQueueLen),
		sgWatcher:       make(chan WatchEvent, watcherQueueLen),
		sgPolicyWatcher: make(chan WatchEvent, watcherQueueLen),
	}

	// start a go routine to handle messages coming on watcher channel
	go watcher.runNetwatcher()
	go watcher.runVmmEpwatcher()
	go watcher.runSgwatcher()
	go watcher.runSgPolicyWatcher()

	return watcher, nil
}

// ******************* Functions for testing and emulating *********************

// CreateNetwork injects a create network event on the watcher
func (w *Watcher) CreateNetwork(tenant, net, subnet, gw string) error {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	evt := WatchEvent{
		EventType: CreateEvent,
		Object:    &nw,
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
	evt := WatchEvent{
		EventType: DeleteEvent,
		Object:    &nw,
	}
	// inject the object into the network watcher
	w.netWatcher <- evt

	return nil
}

// CreateEndpoint injects an endpoint create event
func (w *Watcher) CreateEndpoint(tenant, net, epName, vmName, macAddr, hostName, hostAddr string, attr []string, usegVlan uint32) error {
	epInfo := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:   epName,
			Tenant: tenant,
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
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
	evt := WatchEvent{
		EventType: CreateEvent,
		Object:    &epInfo,
	}
	// inject the object into the network watcher
	w.vmmEpWatcher <- evt

	return nil
}

// DeleteEndpoint injects an endpoint delete event
func (w *Watcher) DeleteEndpoint(tenant, net, epName, vmName, macAddr, hostName, hostAddr string) error {
	epInfo := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:   epName,
			Tenant: tenant,
		},
		Spec: network.EndpointSpec{},
		Status: network.EndpointStatus{
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
	evt := WatchEvent{
		EventType: DeleteEvent,
		Object:    &epInfo,
	}
	// inject the object into the network watcher
	w.vmmEpWatcher <- evt

	logrus.Infof("Injected endpoint Event %s. Ep: {%+v}", evt.EventType, epInfo)

	return nil
}

// CreateSecurityGroup injects a create sg event on the watcher
func (w *Watcher) CreateSecurityGroup(tenant, sgname string, selectors []string) error {
	// build sg object
	sg := network.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Name:   sgname,
			Tenant: tenant,
		},
		Spec: network.SecurityGroupSpec{
			WorkloadSelector: selectors,
		},
	}

	evt := WatchEvent{
		EventType: CreateEvent,
		Object:    &sg,
	}
	// inject the object into the sg watcher
	w.sgWatcher <- evt

	return nil
}

// DeleteSecurityGroup injects a delete sg event to the watcher
func (w *Watcher) DeleteSecurityGroup(tenant, sgname string) error {
	// build a sg object
	sg := network.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Name:   sgname,
			Tenant: tenant,
		},
	}

	// create the watch event
	evt := WatchEvent{
		EventType: DeleteEvent,
		Object:    &sg,
	}

	// inject the object into the sg watcher
	w.sgWatcher <- evt

	return nil
}

// CreateSgpolicy injects a create sg policy event on the watcher
func (w *Watcher) CreateSgpolicy(tenant, pname string, attachGroups []string, inrules, outrules []network.SGRule) error {
	// build sg object
	sgp := network.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:   pname,
			Tenant: tenant,
		},
		Spec: network.SgpolicySpec{
			AttachGroups: attachGroups,
			InRules:      inrules,
			OutRules:     outrules,
		},
	}

	evt := WatchEvent{
		EventType: CreateEvent,
		Object:    &sgp,
	}

	// inject the object into the sg policy watcher
	w.sgPolicyWatcher <- evt

	return nil
}

// DeleteSgpolicy injects a delete sg policy event to the watcher
func (w *Watcher) DeleteSgpolicy(tenant, pname string) error {
	// build a sg object
	sgp := network.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:   pname,
			Tenant: tenant,
		},
	}

	// create the watch event
	evt := WatchEvent{
		EventType: DeleteEvent,
		Object:    &sgp,
	}

	// inject the object into the sg watcher
	w.sgPolicyWatcher <- evt

	return nil
}
