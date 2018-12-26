// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/netagent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/runtime"
)

// updatable is an interface all updatable objects have to implement
type updatable interface {
	Write() error
	GetKey() string
}

// Topics are the Nimbus message bus topics
type Topics struct {
	AppTopic             *nimbus.AppTopic
	EndpointTopic        *nimbus.EndpointTopic
	NetworkTopic         *nimbus.NetworkTopic
	SecurityProfileTopic *nimbus.SecurityProfileTopic
	SecurityGroupTopic   *nimbus.SecurityGroupTopic
	SGPolicyTopic        *nimbus.SGPolicyTopic
}

// Statemgr is the object state manager
type Statemgr struct {
	mbus                 *nimbus.MbusServer // nimbus server
	periodicUpdaterQueue chan updatable     // queue for periodically writing items back to apiserver
	ctrler               ctkit.Controller   // controller instance
	topics               Topics             // message bus topics
}

// ErrIsObjectNotFound returns true if the error is object not found
func ErrIsObjectNotFound(err error) bool {
	return (err == memdb.ErrObjectNotFound) || strings.Contains(err.Error(), "not found")
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

func (sm *Statemgr) smartNICCreated(nic *SmartNICState) {
	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		if _, ok := policy.NodeVersions[nic.SmartNIC.Name]; ok == false {
			policy.NodeVersions[nic.SmartNIC.Name] = ""
		}
	}
}

func (sm *Statemgr) smartNICDeleted(nic *SmartNICState) error {

	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		delete(policy.NodeVersions, nic.SmartNIC.Name)
	}

	return nil
}

// Stop stops the watchers
func (sm *Statemgr) Stop() error {
	return sm.ctrler.Stop()
}

// NewStatemgr creates a new state manager object
func NewStatemgr(apisrvURL string, rslvr resolver.Interface, mserver *nimbus.MbusServer) (*Statemgr, error) {
	// create new statemgr instance
	statemgr := &Statemgr{
		mbus: mserver,
	}

	// create controller instance
	ctrler, err := ctkit.NewController(globals.Npm, apisrvURL, rslvr)
	if err != nil {
		log.Fatalf("Error creating controller. Err: %v", err)
	}
	statemgr.ctrler = ctrler

	// start all object watches
	err = ctrler.Tenant().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching sg policy")
	}
	err = ctrler.SGPolicy().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching sg policy")
	}
	err = ctrler.SecurityGroup().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching security group")
	}
	err = ctrler.App().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching app")
	}
	err = ctrler.Network().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching network")
	}
	err = ctrler.FirewallProfile().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching firewall profile")
	}
	err = ctrler.Host().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching host")
	}
	err = ctrler.SmartNIC().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching smartnic")
	}
	err = ctrler.Workload().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching workloads")
	}
	err = ctrler.Endpoint().Watch(statemgr)
	if err != nil {
		log.Fatalf("Error watching endpoint")
	}

	// create all topics on the message bus
	statemgr.topics.EndpointTopic, err = nimbus.AddEndpointTopic(mserver, statemgr)
	if err != nil {
		log.Errorf("Error starting endpoint RPC server")
		return nil, err
	}
	statemgr.topics.AppTopic, err = nimbus.AddAppTopic(mserver, nil)
	if err != nil {
		log.Errorf("Error starting App RPC server")
		return nil, err
	}
	statemgr.topics.SecurityProfileTopic, err = nimbus.AddSecurityProfileTopic(mserver, nil)
	if err != nil {
		log.Errorf("Error starting SecurityProfile RPC server")
		return nil, err
	}
	statemgr.topics.SecurityGroupTopic, err = nimbus.AddSecurityGroupTopic(mserver, nil)
	if err != nil {
		log.Errorf("Error starting SG RPC server")
		return nil, err
	}
	statemgr.topics.SGPolicyTopic, err = nimbus.AddSGPolicyTopic(mserver, statemgr)
	if err != nil {
		log.Errorf("Error starting SG policy RPC server")
		return nil, err
	}
	statemgr.topics.NetworkTopic, err = nimbus.AddNetworkTopic(mserver, nil)
	if err != nil {
		log.Errorf("Error starting network RPC server")
		return nil, err
	}

	// newPeriodicUpdater creates a new go subroutines
	// Given that objects returned by `NewStatemgr` should live for the duration
	// of the process, we don't have to worry about leaked go subroutines
	statemgr.periodicUpdaterQueue = newPeriodicUpdater()

	return statemgr, nil
}

// runPeriodicUpdater runs periodic and write objects back
func runPeriodicUpdater(queue chan updatable) {
	ticker := time.NewTicker(time.Second)
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
	updateChan := make(chan updatable)
	go runPeriodicUpdater(updateChan)
	return updateChan
}

// PeriodicUpdaterPush enqueues an object to the periodic updater
func (sm *Statemgr) PeriodicUpdaterPush(obj updatable) {
	sm.periodicUpdaterQueue <- obj
}
