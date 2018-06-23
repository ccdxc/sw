// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// Default parameters
var defaultExpiryTime = "2h"

// MirrorSessionState - Internal state for MirrorSession
type MirrorSessionState struct {
	Mutex sync.Mutex
	*monitoring.MirrorSession
	schTime  time.Time
	schTimer *time.Timer
	expTimer *time.Timer
	*Statemgr
	// Local information
	State monitoring.MirrorSessionState
}

type mirrorTimerType int

const (
	mirrorSchTimer mirrorTimerType = iota
	mirrorExpTimer
)

// MirrorTimerEvent - schedule or expiry
type MirrorTimerEvent struct {
	Type mirrorTimerType
	*MirrorSessionState
}

func (mss *MirrorSessionState) handleSchTimer() {
	log.Infof("Sch Timer Done... %v", mss.MirrorSession.Name)
	mss.Statemgr.mirrorTimerWatcher <- MirrorTimerEvent{
		Type:               mirrorSchTimer,
		MirrorSessionState: mss}
	return
}

func (mss *MirrorSessionState) runMsExpTimer() {
	// Exp time is *after* timer starts
	expDuration, _ := time.ParseDuration(defaultExpiryTime)

	if mss.MirrorSession.Spec.StopConditions.ExpiryDuration != "" {
		// format conversion is checked by common Venice parameter checker hook
		mssExpDuration, _ := time.ParseDuration(mss.MirrorSession.Spec.StopConditions.ExpiryDuration)
		if mssExpDuration.Nanoseconds() < expDuration.Nanoseconds() {
			// use user provided value if it is less than max allowed
			expDuration = mssExpDuration
		}
	}
	expTime := mss.schTime.Add(expDuration)
	log.Infof("Expiry Time set to %v for %v\n", expTime, mss.Name)
	mss.expTimer = time.AfterFunc(expDuration, mss.handleExpTimer)
}

func (mss *MirrorSessionState) handleExpTimer() {
	log.Infof("Exp Timer Done...%v", mss.MirrorSession.Name)
	mss.Statemgr.mirrorTimerWatcher <- MirrorTimerEvent{
		Type:               mirrorExpTimer,
		MirrorSessionState: mss}
	return
}

func (mss *MirrorSessionState) hasVeniceCollector() bool {
	for _, c := range mss.Spec.Collectors {
		if c.Type == monitoring.PacketCollectorType_VENICE.String() {
			return true
		}
	}
	return false
}

func (mss *MirrorSessionState) runMirrorSession() {
	mss.State = monitoring.MirrorSessionState_RUNNING
	mss.Status.State = monitoring.MirrorSessionState_RUNNING.String()
	// create PCAP file URL for sessions with venice collector
	if mss.hasVeniceCollector() {
		mss.Status.PcapFileURL = mss.mirrorSessionGetPcapLink()
	}
}

// CreateMirrorSession : Create a MirrorSessionState and process the MirrorSession requirements
func (sm *Statemgr) CreateMirrorSession(ms *monitoring.MirrorSession) error {
	// All parameters are validated (using apiserver hooks) by the time we get here
	_, err := sm.FindObject("MirrorSession", ms.Tenant, ms.Name)
	if err == nil {
		log.Errorf("Mirror session {+%v} exists", ms)
		return fmt.Errorf("Mirror session already exists")
	}

	mss := MirrorSessionState{
		MirrorSession: ms,
		schTimer:      nil,
		expTimer:      nil,
		Statemgr:      sm,
	}

	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	mss.schTime = time.Now()
	// XXX validate parameters -
	// - Atleast 1 collector
	// - atleast 1 valid match rule
	// If not valid, set status to INVALID_PARAMS... but create the MirrorSessionState
	// Params may get fixed using update
	if ms.Spec.StartConditions.ScheduleTime != nil {
		schTime, err := mss.MirrorSession.Spec.StartConditions.ScheduleTime.Time()
		if err != nil {
			log.Fatalf("Bad Schedule Time %v\n", mss.MirrorSession.Spec.StartConditions.ScheduleTime)
		}
		if schTime.After(mss.schTime) {
			// start the timer routine only if time is in future
			mss.schTime = schTime
			log.Infof("Schedule time is %v", mss.schTime)
			mss.schTimer = time.AfterFunc(time.Until(mss.schTime), mss.handleSchTimer)
			mss.State = monitoring.MirrorSessionState_SCHEDULED
			mss.Status.State = monitoring.MirrorSessionState_SCHEDULED.String()
		} else {
			// schedule time in the past, run it right-away
			log.Warnf("Schedule time already passed, strting the mirror-session now - %v\n", mss.MirrorSession.Name)
			mss.runMirrorSession()
		}
	} else {
		mss.runMirrorSession()
		log.Infof("Mirror Session is running %v - %v", mss.Name, mss.Status.State)
	}
	mss.runMsExpTimer()
	sm.memDB.AddObject(&mss)
	sm.writer.WriteMirrorSession(mss.MirrorSession)
	return nil
}

// DeleteMirrorSession - delete a mirror session
func (sm *Statemgr) DeleteMirrorSession(ms *monitoring.MirrorSession) {
	mss, err := sm.GetMirrorSessionState(ms.Tenant, ms.Name)
	if err != nil {
		log.Debugf("Error deleting non-existent mirror session {%+v}. Err: %v", ms, err)
		return
	}
	sm.deleteMirrorSession(mss)
}

func (sm *Statemgr) deleteMirrorSession(mss *MirrorSessionState) {
	log.Infof("Deleting Mirror Session %v", mss.MirrorSession.Name)
	// Mutex - agent grpc thread may be using it
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	if mss.schTimer != nil {
		log.Infof("STOP SchTimer for %v", mss.MirrorSession.Name)
		mss.schTimer.Stop()
		mss.schTimer = nil
	}
	if mss.expTimer != nil {
		log.Infof("STOP ExpTimer for %v", mss.MirrorSession.Name)
		mss.expTimer.Stop()
		mss.expTimer = nil
	}
	mss.State = monitoring.MirrorSessionState_STOPPED
	// delete minio bucket used for packet capture
	if mss.Status.PcapFileURL != "" {
		// XXX delete this bucket from minio server
		mss.Status.PcapFileURL = ""
	}

	// delete mirror session state from DB
	_ = sm.memDB.DeleteObject(mss)
}

func (mss *MirrorSessionState) mirrorSessionGetPcapLink() string {
	// Create a name for packet file directory as -
	//  packet_capture/<tenant-name>/<mirror-session-name>
	// XXX Replace all the not-allowed characters by '_' OR add it to validator
	// so that mirror session name cannot have those (?What about TenantName)
	s := []string{"packet_capture", mss.Tenant, mss.Name}
	return strings.Join(s, "/")
}

func (sm *Statemgr) scheduleMirrorSession(mss *MirrorSessionState) {
	// XXX: check - do we need to release reference to mss so that it can be GC-ed incase
	// original object has changed/replaced ???
	// check that session is not deleted/modified
	mss1, err := sm.GetMirrorSessionState(mss.Tenant, mss.Name)
	if err != nil {
		log.Debugf("Mirror session is not found %v:%v", mss.Tenant, mss.Name)
		return
	}
	mss1.Mutex.Lock()
	defer mss1.Mutex.Unlock()
	if mss1.State != monitoring.MirrorSessionState_SCHEDULED {
		// some other thread may have updated the session
		return
	}
	mss1.State = monitoring.MirrorSessionState_READY_TO_RUN
	mss1.runMirrorSession()
	log.Infof("Mirror Session is running %v - %v", mss1.Name, mss1.Status.State)
	mss1.schTimer = nil
	sm.memDB.UpdateObject(mss1)
	sm.writer.WriteMirrorSession(mss1.MirrorSession)
	return
}

func (sm *Statemgr) stopMirrorSession(mss *MirrorSessionState) {
	log.Infof("Mirror Session Expired %v", mss.Name)
	mss1, err := sm.GetMirrorSessionState(mss.Tenant, mss.Name)
	if err != nil {
		log.Debugf("Mirror session is not active/scheduled anymore %v:%v", mss.Tenant, mss.Name)
		return
	}
	mss1.Mutex.Lock()
	defer mss1.Mutex.Unlock()
	mss1.State = monitoring.MirrorSessionState_STOPPED
	mss1.Status.State = monitoring.MirrorSessionState_STOPPED.String()
	mss1.expTimer = nil
	sm.memDB.UpdateObject(mss1)
	sm.writer.WriteMirrorSession(mss1.MirrorSession)
	return
}

// GetMirrorSessionState : Get the specified mirror sessions state
func (sm *Statemgr) GetMirrorSessionState(tenant, name string) (*MirrorSessionState, error) {
	dbMs, err := sm.FindObject("MirrorSession", tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*MirrorSessionState), nil
}

// GetMirrorSession : Get the specified mirror sessions
func (sm *Statemgr) GetMirrorSession(tenant, name string) (*monitoring.MirrorSession, error) {
	dbMs, err := sm.FindObject("MirrorSession", tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*MirrorSessionState).MirrorSession, nil
}

// ListMirrorSessions : Get the list of mirror sessions
func (sm *Statemgr) ListMirrorSessions() ([]*MirrorSessionState, error) {
	var msList []*MirrorSessionState
	memDbObjs := sm.ListObjects("MirrorSession")
	for _, obj := range memDbObjs {
		msList = append(msList, obj.(*MirrorSessionState))
	}
	return msList, nil
}

func (sm *Statemgr) handleMirrorSessionEvent(et kvstore.WatchEventType, ms *monitoring.MirrorSession) {
	switch et {
	case kvstore.Created:
		log.Infof("CreateMirrorSession - %s\n", ms.Name)
		err := sm.CreateMirrorSession(ms)
		if err != nil {
			log.Errorf("Error creating mirror session {%+v}. Err: %v", ms, err)
			return
		}
	case kvstore.Updated:
		// watcher is set to watch changes to Spec only
		// Supported updates -
		// Validation done in the validation hook
		// - update match rules, packet filters
		//  - stopped: Ok, will take effect if/when restarted
		//  - scheduled: Ok
		//  - running: Ok will take effect for the remaining capture
		// - update collector list (add/remove/change destination)
		//  - stopped: Ok, will take effect if/when restarted
		//  - scheduled: Ok
		//  - running: Ok, will take effect for the remaining
		// - update stop condition
		//  - stopped: Ok, for next run
		//  - scheduled: Ok (stop/restart exp timer as needed)
		//  - running: Ok (reset the stop condition, i.e. ignore elapsed time and restart the timer for the requested duration)
		// - update start condition
		//  - stopped: Ok, apply new start condition (may result in running state or scheduled state)
		//  - running: Not allowed, must be stopped first(?)
		//  - scheduled: Ok, Stop Schtimer and apply new start condition
		log.Infof("UpdateMirrorSession(TBD) - %s\n", ms.Name)

	case kvstore.Deleted:
		sm.DeleteMirrorSession(ms)
	}
}

func (sm *Statemgr) handleMirrorSessionTimerEvent(et mirrorTimerType, mss *MirrorSessionState) {
	switch et {
	case mirrorSchTimer:
		sm.scheduleMirrorSession(mss)
	case mirrorExpTimer:
		sm.stopMirrorSession(mss)
	default:
	}
}
