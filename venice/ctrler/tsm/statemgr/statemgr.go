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
	writer  writer.Writer  // writer to ApiServer

	// Channels to receive events from api server and internal timers
	MirrorSessionWatcher chan kvstore.WatchEvent // mirror session object watcher
	mirrorTimerWatcher   chan MirrorTimerEvent   // mirror session Timer watcher

	// Channels to receive TechSupport events from ApiServer
	// Affected objects can be TechSupportRequests, Controller nodes and SmartNIC nodes
	TechSupportWatcher chan kvstore.WatchEvent

	stopFlag syncFlag

	// Resource allocation/tracking
	Mutex             sync.Mutex
	numMirrorSessions int // total mirror sessions created
}

const (
	watcherQueueLen   = 16
	maxMirrorSessions = 8
)

// FindObject looks up an object in local db
func (sm *Statemgr) FindObject(kind string, ometa *api.ObjectMeta) (memdb.Object, error) {
	// form object key
	// find it in db
	return sm.memDB.FindObject(kind, ometa)
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
	defer sm.waitGrp.Done()
	log.Infof("Mirror Session Watcher running")

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

			log.Infof("Watcher: Got Mirror session  watch event(%s): %v - ver %v", evt.Type, ms.Name, ms.ResourceVersion)

			sm.handleMirrorSessionEvent(evt.Type, ms)

		case evt, ok := <-sm.mirrorTimerWatcher:
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			log.Infof("Watcher: Got Mirror session Timer event(%v) on %v, ver %v", evt.Type, evt.MirrorSessionState.Name, evt.MirrorSessionState.ResourceVersion)
			sm.handleMirrorSessionTimerEvent(evt.Type, evt.MirrorSessionState)
		}
	}
}

// runTechSupportWatcher watches on a channel for changes from api server and internal events
func (sm *Statemgr) runTechSupportWatchers() {
	defer sm.waitGrp.Done()
	log.Infof("TechSupportRequest Watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-sm.TechSupportWatcher:
			// if channel has error, we are done..
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				return
			}
			sm.handleTechSupportEvent(&evt)
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

// MirrorSessionCountAllocate : Increment the active session counter if max is not reached
func (sm *Statemgr) MirrorSessionCountAllocate() bool {
	sm.Mutex.Lock()
	defer sm.Mutex.Unlock()
	if sm.numMirrorSessions < maxMirrorSessions {
		sm.numMirrorSessions++
		log.Infof("Allocated mirror session: count %v", sm.numMirrorSessions)
		return true
	}
	log.Infof("Max mirror session count reached")
	return false
}

// MirrorSessionCountFree : decrement the active session counter
func (sm *Statemgr) MirrorSessionCountFree() {
	sm.Mutex.Lock()
	defer sm.Mutex.Unlock()
	if sm.numMirrorSessions > 0 {
		sm.numMirrorSessions--
		log.Infof("Free mirror session: count %v", sm.numMirrorSessions)
	} else {
		panic("Bug - mirror session free below 0")
	}
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
	close(sm.TechSupportWatcher)
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
		TechSupportWatcher:   make(chan kvstore.WatchEvent, watcherQueueLen),
		stopFlag: syncFlag{
			flag: false,
		},
		numMirrorSessions: 0,
	}

	stateMgr.waitGrp.Add(2)
	go stateMgr.runMirrorSessionWatcher()
	go stateMgr.runTechSupportWatchers()

	return stateMgr, nil
}
