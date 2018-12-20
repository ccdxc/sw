// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/writer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// updatable is an interface all updatable objects have to implement
type updatable interface {
	Write() error
	GetKey() string
}

// Statemgr is the object state manager
type Statemgr struct {
	memDB                *memdb.Memdb            // database of all objects
	writer               writer.Writer           // writer to apiserver
	workloadReactor      *WorkloadReactor        // workload event reactor
	hostReactor          *HostReactor            // host event reactor
	smartNicReactor      *SmartNICReactor        // smart nic event reactor
	fwProfileReactor     *FirewallProfileReactor // firewall profile reactor
	appReactor           *AppReactor             // app reactor
	periodicUpdaterQueue chan updatable          // queue for periodically writing items back to apiserver
}

// ErrIsObjectNotFound returns true if the error is object not found
func ErrIsObjectNotFound(err error) bool {
	return (err == memdb.ErrObjectNotFound)
}

// FindObject looks up an object in local db
func (sm *Statemgr) FindObject(kind, tenant, name string) (memdb.Object, error) {
	// form network key
	ometa := api.ObjectMeta{
		Tenant: tenant,
		Name:   name,
	}

	// find it in db
	return sm.memDB.FindObject(kind, &ometa)
}

// ListObjects list all objects of a kind
func (sm *Statemgr) ListObjects(kind string) []memdb.Object {
	return sm.memDB.ListObjects(kind)
}

// WatchObjects watches network state for changes
func (sm *Statemgr) WatchObjects(kind string, watchChan chan memdb.Event) error {
	// just add the channel to the list of watchers
	return sm.memDB.WatchObjects(kind, watchChan)
}

// StopWatchObjects Stops watches of network state
func (sm *Statemgr) StopWatchObjects(kind string, watchChan chan memdb.Event) error {
	// just remove the channel from the list of watchers
	return sm.memDB.StopWatchObjects(kind, watchChan)
}

// WorkloadReactor returns the workload event reactor
func (sm *Statemgr) WorkloadReactor() WorkloadHandler {
	return sm.workloadReactor
}

// HostReactor returns the host event reactor
func (sm *Statemgr) HostReactor() HostHandler {
	return sm.hostReactor
}

// SmartNICReactor returns the snic event reactor
func (sm *Statemgr) SmartNICReactor() SmartNICHandler {
	return sm.smartNicReactor
}

// FirewallProfileReactor returns the firewall profile event reactor
func (sm *Statemgr) FirewallProfileReactor() FirewallProfileHandler {
	return sm.fwProfileReactor
}

// AppReactor returns the app event reactor
func (sm *Statemgr) AppReactor() AppHandler {
	return sm.appReactor
}

func (sm *Statemgr) smartNICCreated(nic *SmartNICState) {
	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		if _, ok := policy.NodeVersions[nic.Name]; ok == false {
			policy.NodeVersions[nic.Name] = ""
		}
	}

	// store it in local DB
	sm.memDB.AddObject(nic)
}

func (sm *Statemgr) smartNICDeleted(nic *SmartNICState) error {

	// Update SGPolicies
	policies, _ := sm.ListSgpolicies()
	for _, policy := range policies {
		delete(policy.NodeVersions, nic.Name)
	}

	// delete the object
	return sm.memDB.DeleteObject(nic)
}

// NewStatemgr creates a new state manager object
func NewStatemgr(wr writer.Writer) (*Statemgr, error) {
	// create new statemgr instance
	statemgr := &Statemgr{
		memDB:  memdb.NewMemdb(),
		writer: wr,
	}

	statemgr.workloadReactor, _ = NewWorkloadReactor(statemgr)
	statemgr.hostReactor, _ = NewHostReactor(statemgr)
	statemgr.smartNicReactor, _ = NewSmartNICReactor(statemgr)
	statemgr.fwProfileReactor, _ = NewFirewallProfileReactor(statemgr)
	statemgr.appReactor, _ = NewAppReactor(statemgr)

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
