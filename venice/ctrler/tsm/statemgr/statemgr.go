// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tsm/writer"
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
	waitGrp sync.WaitGroup // wait group to wait on all go routines to exit
	memDB   *memdb.Memdb   // database of all objects
	writer  writer.Writer  // writer to apiserver

	// Channels to receive events from api server and internal timers
	MirrorSessionWatcher chan kvstore.WatchEvent // mirror session object watcher
	mirrorTimerWatcher   chan MirrorTimerEvent   // mirror session Timer watcher

	stopFlag syncFlag
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
	return sm.memDB.ListObjects(kind)
}

// WatchObjects watches monitoring objects for changes
func (sm *Statemgr) WatchObjects(kind string, watchChan chan memdb.Event) error {
	// just add the channel to the list of watchers
	return sm.memDB.WatchObjects(kind, watchChan)
}

// StopWatchObjects Stops watches of network state
func (sm *Statemgr) StopWatchObjects(kind string, watchChan chan memdb.Event) error {
	// just remove the channel from the list of watchers
	return sm.memDB.StopWatchObjects(kind, watchChan)
}

// runMirrorSessionWatcher watches on a channel for changes from api server and internal events
func (sm *Statemgr) runMirrorSessionWatcher() {
	log.Infof("Mirror Session Watcher running")

	// setup wait group
	sm.waitGrp.Add(1)
	defer sm.waitGrp.Done()

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-sm.MirrorSessionWatcher:
			// if channel has error, we are done..
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}

			// convert to mirror session object
			var ms *monitoring.MirrorSession
			switch tp := evt.Object.(type) {
			case *monitoring.MirrorSession:
				ms = evt.Object.(*monitoring.MirrorSession)
			default:
				log.Fatalf("Mirror session  watcher Found object of invalid type: %v", tp)
				return
			}

			log.Infof("Watcher: Got Mirror session  watch event(%s): %v", evt.Type, ms.Name)

			sm.handleMirrorSessionEvent(evt.Type, ms)

		case evt, ok := <-sm.mirrorTimerWatcher:
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			log.Infof("Watcher: Got Mirror session Timer event(%s) on %v", evt.Type, evt.MirrorSessionState.Name)
			sm.handleMirrorSessionTimerEvent(evt.Type, evt.MirrorSessionState)
		}
	}
}

func (sm *Statemgr) stopped() bool {
	sm.stopFlag.RLock()
	defer sm.stopFlag.RUnlock()
	return sm.stopFlag.flag
}
func (sm *Statemgr) setStop() {
	sm.stopFlag.Lock()
	sm.stopFlag.flag = true
	sm.stopFlag.Unlock()
}

// Stop state manager
func (sm *Statemgr) Stop() {
	log.Infof("Tsm StateMgr STOP\n")
	sm.setStop()
	// Stop timers for all active Mirror session
	mssList, _ := sm.ListMirrorSessions()
	for _, mss := range mssList {
		sm.deleteMirrorSession(mss)
	}

	// close the channels
	close(sm.MirrorSessionWatcher)
	close(sm.mirrorTimerWatcher)
	// wait for all go routines to exit
	log.Debugf("Tsm wait for all go routines")
	sm.waitGrp.Wait()
	log.Debugf("Tsm wait Done for all go routines")
}

// NewStatemgr creates a new state manager object
func NewStatemgr(wr writer.Writer) (*Statemgr, error) {
	// create new statemgr instance
	stateMgr := &Statemgr{
		memDB:                memdb.NewMemdb(),
		writer:               wr,
		MirrorSessionWatcher: make(chan kvstore.WatchEvent, watcherQueueLen),
		mirrorTimerWatcher:   make(chan MirrorTimerEvent, watcherQueueLen),
		stopFlag: syncFlag{
			flag: false,
		},
	}
	go stateMgr.runMirrorSessionWatcher()

	return stateMgr, nil
}
