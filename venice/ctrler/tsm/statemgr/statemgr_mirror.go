// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"bufio"
	"fmt"
	"os"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// Default parameters
var defaultExpiryTime = "2h"

//var timeFormat = "2006-01-02 15:04:05 MST"

type fileIo struct {
	fname   string
	fHandle *os.File
	w       *bufio.Writer
}

// MirrorSessionState - Internal state for MirrorSession
type MirrorSessionState struct {
	Mutex sync.Mutex
	*monitoring.MirrorSession
	schTime  time.Time
	schTimer *time.Timer
	expTimer *time.Timer
	*Statemgr
	// Local information
	State              monitoring.MirrorSessionState
	numCapturedPackets int32
	srcPacketsIo       *fileIo
	dstPacketsIo       *fileIo
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
			mss.State = monitoring.MirrorSessionState_RUNNING
			mss.Status.State = monitoring.MirrorSessionState_RUNNING.String()
		}
	} else {
		mss.State = monitoring.MirrorSessionState_RUNNING
		mss.Status.State = monitoring.MirrorSessionState_RUNNING.String()
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
	if mss.srcPacketsIo != nil {
		mss.srcPacketsIo.w.Flush()
		mss.srcPacketsIo.fHandle.Close()
	}
	mss.srcPacketsIo = nil
	if mss.dstPacketsIo != nil {
		mss.dstPacketsIo.w.Flush()
		mss.dstPacketsIo.fHandle.Close()
	}
	mss.dstPacketsIo = nil
	mss.State = monitoring.MirrorSessionState_STOPPED

	// delete mirror session state from DB
	_ = sm.memDB.DeleteObject(mss)
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
	mss1.Status.State = monitoring.MirrorSessionState_RUNNING.String()
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
		// - update match rules, packet filters
		//  - if stopped: Not allowed
		// - update collector list (add/remove/change destination)
		//  - if stopped: Not allowed
		// - update stop condition
		//  - if already stopped?
		// - update start condition
		//  - if running: Not allowed, must be stopped first(?)
		//  - if stopped: apply new start condition
		//  - if scheduled: Stop Schtimer and apply new start condition
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

func (mss *MirrorSessionState) findNICStatus(SmartNIC string) *monitoring.SmartNICMirrorSessionStatus {
	// find a status object for given smartNIC
	var found *monitoring.SmartNICMirrorSessionStatus
	found = nil
	for _, snic := range mss.MirrorSession.Status.NICStatus {
		if snic.SmartNIC == SmartNIC {
			found = &snic
			break
		}
	}
	return found
}

// UpdateNICMirrorSessionsStatus : process status update from a NIC agent
func (sm *Statemgr) UpdateNICMirrorSessionsStatus(mspList *tsproto.MirrorSessionStatusList) {
	for _, msp := range mspList.StatusList {
		mss, err := sm.GetMirrorSessionState(msp.Tenant, msp.Name)
		if err != nil || mss == nil {
			log.Infof("SmartNIC %v captured packets on deleted session %v", mspList.SmartNIC, msp.Name)
			continue
		}
		mss.Mutex.Lock()
		// store packets.. update counts...
		nicStatus := mss.findNICStatus(mspList.SmartNIC)
		if nicStatus == nil {
			nStatus := monitoring.SmartNICMirrorSessionStatus{
				SmartNIC:  mspList.SmartNIC,
				SessionId: msp.Status.SessionId,
			}
			nicStatus = &nStatus
			mss.MirrorSession.Status.NICStatus = append(mss.MirrorSession.Status.NICStatus, *nicStatus)
		}
		mss.Mutex.Unlock()
		sm.writer.WriteMirrorSession(mss.MirrorSession)
	}
}
