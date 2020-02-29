// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/venice/utils/events"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/ctrler/rollout/writer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// Statemgr is the object state manager
type Statemgr struct {
	waitGrp      sync.WaitGroup  // wait group to wait on all go routines to exit
	memDB        *memdb.Memdb    // database of all objects
	writer       writer.Writer   // writer to apiserver
	evtsRecorder events.Recorder //events recorder for rollout

	// Channels to receive events from api server and internal timers
	RolloutWatcher  chan kvstore.WatchEvent // rolllout object watcher
	NodeWatcher     chan kvstore.WatchEvent // Node object watcher
	SmartNICWatcher chan kvstore.WatchEvent // smartNIC object watcher

	stopFlag syncFlag
	rollout  bool

	smartNICWG sync.WaitGroup //  Wait group to wait on all go routines dealing with smartNICs for the current rollout
}

const watcherQueueLen = 16

// FindObject looks up an object in local db
func (sm *Statemgr) FindObject(kind, tenant, name string) (memdb.Object, error) {
	// form object key
	ometa := api.ObjectMeta{
		Tenant: tenant,
		Name:   name,
	}

	// find it in db
	return sm.memDB.FindObject(kind, &ometa)
}

// ListObjects list all objects of a kind
func (sm *Statemgr) ListObjects(kind string) []memdb.Object {
	return sm.memDB.ListObjects(kind, nil)
}

// WatchObjects watches rollout objects for changes
func (sm *Statemgr) WatchObjects(kind string, watcher *memdb.Watcher) error {
	// just add the channel to the list of watchers
	return sm.memDB.WatchObjects(kind, watcher)
}

// StopWatchObjects Stops watches of network state
func (sm *Statemgr) StopWatchObjects(kind string, watcher *memdb.Watcher) error {
	// just remove the channel from the list of watchers
	return sm.memDB.StopWatchObjects(kind, watcher)
}

// runWatcher watches on a channel for changes from api server and internal events
func (sm *Statemgr) runWatcher() {
	log.Infof("statemgr Watcher running")

	// setup wait group
	defer sm.waitGrp.Done()
	defer log.Infof("returning from statemgr Watcher")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-sm.RolloutWatcher:
			// if channel has error, we are done..
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			ro := evt.Object.(*roproto.Rollout)
			log.Infof("Watcher: Got rollout  watch event(%s): %v", evt.Type, ro.Name)
			sm.handleRolloutEvent(evt.Type, ro)

		case evt, ok := <-sm.NodeWatcher:
			// if channel has error, we are done..
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			node := evt.Object.(*cluster.Node)
			sm.handleNodeEvent(evt.Type, node)

		case evt, ok := <-sm.SmartNICWatcher:
			// if channel has error, we are done..
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			sn := evt.Object.(*cluster.DistributedServiceCard)

			log.Infof("Watcher: Got SmartNIC  watch event(%s): %v", evt.Type, sn.Name)
			sm.handleSmartNICEvent(evt.Type, sn)
		}
	}
}

func (sm *Statemgr) setStop() {
	sm.stopFlag.Lock()
	sm.stopFlag.flag = true
	sm.stopFlag.Unlock()
}

// Stop state manager
func (sm *Statemgr) Stop() {
	log.Infof("rolloutmgr StateMgr STOP\n")
	sm.setStop()

	// close the channels
	close(sm.RolloutWatcher)
	close(sm.NodeWatcher)
	close(sm.SmartNICWatcher)

	sm.waitGrp.Wait()
	log.Infof("rollout Statemgr stopped")
}

// NewStatemgr creates a new state manager object
func NewStatemgr(wr writer.Writer, er events.Recorder) (*Statemgr, error) {
	// create new statemgr instance
	stateMgr := &Statemgr{
		memDB:           memdb.NewMemdb(),
		writer:          wr,
		evtsRecorder:    er,
		RolloutWatcher:  make(chan kvstore.WatchEvent, watcherQueueLen),
		NodeWatcher:     make(chan kvstore.WatchEvent, watcherQueueLen),
		SmartNICWatcher: make(chan kvstore.WatchEvent, watcherQueueLen),

		stopFlag: syncFlag{
			flag: false,
		},
		rollout: false,
	}
	stateMgr.waitGrp.Add(1)
	go stateMgr.runWatcher()

	return stateMgr, nil
}

func (sm *Statemgr) deleteRollouts() {
	veniceROs, err := sm.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error %v listing VeniceRollouts", err)
	} else {
		for _, v := range veniceROs {
			sm.DeleteVeniceRolloutState(v.VeniceRollout)
		}
	}

	serviceROs, err := sm.ListServiceRollouts()
	if err != nil {
		log.Errorf("Error %v listing ServiceRollouts", err)
	} else {
		for _, v := range serviceROs {
			sm.DeleteServiceRolloutState(v.ServiceRollout)
		}
	}

	smartNICROs, err := sm.ListDSCRollouts()
	if err != nil {
		log.Errorf("Error %v listing DSCRollouts", err)
	} else {
		for _, v := range smartNICROs {
			sm.DeleteDSCRolloutState(v.DSCRollout)
		}
	}

}

type byVROSName []*VeniceRolloutState

func (a byVROSName) Len() int           { return len(a) }
func (a byVROSName) Less(i, j int) bool { return a[i].Name < a[j].Name }
func (a byVROSName) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

type bySROSName []*ServiceRolloutState

func (a bySROSName) Len() int           { return len(a) }
func (a bySROSName) Less(i, j int) bool { return a[i].Name < a[j].Name }
func (a bySROSName) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

type bySNICROSName []*DSCRolloutState

func (a bySNICROSName) Len() int           { return len(a) }
func (a bySNICROSName) Less(i, j int) bool { return a[i].Name < a[j].Name }
func (a bySNICROSName) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
