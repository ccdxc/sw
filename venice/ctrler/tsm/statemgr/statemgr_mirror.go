// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

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
}

/* XXX Uncomment after NAPLES adds support for mirror expiry
func (mss *MirrorSessionState) getExpDuration() time.Duration {
	// format conversion and max duration (2h) is checked by common Venice parameter checker hook
	expDuration, _ := time.ParseDuration(mss.MirrorSession.Spec.StopConditions.ExpiryDuration)
	return expDuration
} */

func (mss *MirrorSessionState) runMsExpTimer() bool {
	return true
	/* XXX Uncomment after NAPLES adds support for mirror expiry
	expDuration := mss.getExpDuration()
	expTime := mss.schTime.Add(expDuration)
	if expTime.After(time.Now()) {
		log.Infof("Expiry Time set to %v for %v\n", expTime, mss.Name)
		mss.expTimer = time.AfterFunc(time.Until(expTime), mss.handleExpTimer)
		return true
	}
	log.Infof("Expiry Time expired in the past %v for %v\n", expTime, mss.Name)
	return false */
}

func (mss *MirrorSessionState) handleExpTimer() {
	log.Infof("Exp Timer Done...%v", mss.MirrorSession.Name)
	mss.Statemgr.mirrorTimerWatcher <- MirrorTimerEvent{
		Type:               mirrorExpTimer,
		MirrorSessionState: mss}
}

func (mss *MirrorSessionState) setMirrorSessionRunning() {
	if !mss.Statemgr.MirrorSessionCountAllocate() {
		mss.State = monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION
		mss.Status.ScheduleState = monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION.String()
	} else {
		mss.State = monitoring.MirrorSessionState_ACTIVE
		mss.Status.ScheduleState = monitoring.MirrorSessionState_ACTIVE.String()
		ts, _ := types.TimestampProto(time.Now())
		mss.Status.StartedAt.Timestamp = *ts
		// create PCAP file URL for sessions with venice collector
		_t, _ := mss.Status.StartedAt.Time()
		log.Infof("Mirror session StartedAt %v", _t)
	}
}

// UpdateMirrorSession : Update an existing mirror session's state
func (sm *Statemgr) UpdateMirrorSession(ms *monitoring.MirrorSession) error {
	// Check the Sepc to see if there was an update from user
	// If no user update, write status back to apiserver
	mss, err := sm.GetMirrorSessionState(ms)
	if err != nil {
		log.Errorf("Update called on non-existing mirror session %v", ms.MakeKey(""))
		return err
	}
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	if _, different := ref.ObjDiff(ms.Spec, mss.MirrorSession.Spec); different {
		if mss.canUpdateMirrorSession(ms) {
			log.Infof("UpdateMirrorSession - %s\n", ms.Name)
			// free the mirror session count - it will be claimed again as needed
			if mss.State == monitoring.MirrorSessionState_ACTIVE {
				sm.MirrorSessionCountFree()
				mss.State = monitoring.MirrorSessionState_NONE
			}
			// stop any running timers
			timersStopped := true
			if mss.schTimer != nil {
				if !mss.schTimer.Stop() {
					log.Infof("Cannot STOP SchTimer for %v", mss.MirrorSession.Name)
					// could not stop timer
					timersStopped = false
				} else {
					mss.schTimer = nil
				}
			}
			if mss.expTimer != nil {
				if !mss.expTimer.Stop() {
					log.Infof("Cannot STOP ExpTimer for %v", mss.MirrorSession.Name)
					timersStopped = false
				} else {
					mss.expTimer = nil
				}
			}
			if !timersStopped {
				// replace the object in the DB
				mss1 := MirrorSessionState{
					MirrorSession: ms,
					schTimer:      nil,
					expTimer:      nil,
					Statemgr:      sm,
				}
				// indicate that mss object is now obsolete
				mss.State = monitoring.MirrorSessionState_STOPPED
				mss1.Mutex.Lock()
				defer mss1.Mutex.Unlock()
				mss1.programMirrorSession(ms)
				sm.memDB.UpdateObject(&mss1)
				sm.writer.WriteMirrorSession(mss1.MirrorSession)
			} else {
				// copy new spec
				mss.MirrorSession.Spec = ms.Spec
				mss.ObjectMeta = ms.ObjectMeta
				mss.programMirrorSession(ms)
				sm.memDB.UpdateObject(mss)
				sm.writer.WriteMirrorSession(mss.MirrorSession)
			}
		}
	} else {
		// status update, grab ResourceVersion
		mss.ObjectMeta = ms.ObjectMeta
	}

	return nil
}

func (mss *MirrorSessionState) canUpdateMirrorSession(*monitoring.MirrorSession) bool {
	// All updates are now supported (until we find any problems)

	switch mss.State {
	case monitoring.MirrorSessionState_STOPPED:
		log.Infof("Update on STOPPED session %v: ver %v", mss.Name, mss.ResourceVersion)
	case monitoring.MirrorSessionState_ACTIVE:
		log.Infof("Update on RUNNING session %v: ver %v", mss.Name, mss.ResourceVersion)
	case monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION:
		// this session could not run earlier, try now
	default:
		// all other cases - program the session
	}
	return true
}

func (mss *MirrorSessionState) programMirrorSession(ms *monitoring.MirrorSession) {
	// common function called for both create and update operations
	// New or previously SCHEDULED session
	if ms.Spec.StartConditions.ScheduleTime != nil {
		schTime, _ := ms.Spec.StartConditions.ScheduleTime.Time()
		if schTime.After(mss.schTime) {
			mss.schTime = schTime
			// start the timer routine only if time is in future
			log.Infof("Schedule time is %v", mss.schTime)
			mss.schTimer = time.AfterFunc(time.Until(mss.schTime), mss.handleSchTimer)
			mss.State = monitoring.MirrorSessionState_SCHEDULED
			mss.Status.ScheduleState = monitoring.MirrorSessionState_SCHEDULED.String()
		} else {
			// schedule time in the past, run it right-away
			log.Warnf("Schedule time %v already passed, starting the mirror-session now - %v\n", schTime, mss.MirrorSession.Name)
			mss.setMirrorSessionRunning()
		}
	} else {
		mss.schTime = time.Now()
		mss.setMirrorSessionRunning()
		log.Infof("Mirror Session  %v, State %v", mss.Name, mss.Status.ScheduleState)
	}
	if !mss.runMsExpTimer() {
		if mss.State == monitoring.MirrorSessionState_ACTIVE {
			mss.Statemgr.MirrorSessionCountFree()
		}
		mss.State = monitoring.MirrorSessionState_STOPPED
		mss.Status.ScheduleState = monitoring.MirrorSessionState_STOPPED.String()
		mss.expTimer = nil
	}
}

// CreateMirrorSession : Create a MirrorSessionState and process the MirrorSession requirements
func (sm *Statemgr) CreateMirrorSession(ms *monitoring.MirrorSession) error {
	// All parameters are validated (using apiserver hooks) by the time we get here
	mss1, err := sm.GetMirrorSessionState(ms)
	if err == nil {
		// This could happen due to apiserver restart, grpc re-connect
		// Turn the create request into update
		log.Infof("Mirror session %v exists (Create->Update)", mss1.MirrorSession.MakeKey(""))
		return sm.UpdateMirrorSession(ms)
	}
	mss := MirrorSessionState{
		MirrorSession: ms,
		schTimer:      nil,
		expTimer:      nil,
		Statemgr:      sm,
	}
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	// If incoming object's state is not "" or NONE, it indicates that the TSM restarted
	// Naples will already have this mirror session, but will send a new create which Naples must handle
	// same as tsm handles apiserver restart XXX
	if ms.Status.ScheduleState != "" && (ms.Status.ScheduleState != monitoring.MirrorSessionState_NONE.String()) {
		mss.State = monitoring.MirrorSessionState(monitoring.MirrorSessionState_value[ms.Status.ScheduleState])
		mss.schTime, _ = mss.Status.StartedAt.Time()
	} else {
		mss.State = monitoring.MirrorSessionState_NONE
		mss.schTime = time.Now()
	}

	mss.programMirrorSession(ms)
	sm.memDB.AddObject(&mss)
	sm.writer.WriteMirrorSession(mss.MirrorSession)
	return nil
}

// DeleteMirrorSession - delete a mirror session
func (sm *Statemgr) DeleteMirrorSession(ms *monitoring.MirrorSession) {
	mss, err := sm.GetMirrorSessionState(ms)
	if err != nil {
		log.Debugf("Error deleting non-existent mirror session {%+v}. Err: %v", ms, err)
		return
	}
	sm.deleteMirrorSession(mss)

	// bring-up any of the mirror session in failed state
	ml, err := sm.ListMirrorSessions()
	if err == nil {
		for _, m := range ml {
			if m.Status.ScheduleState == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION.String() {
				log.Infof("retry session %v in state:%v ", m.Name, m.Status.ScheduleState)
				m.Mutex.Lock()
				m.State = monitoring.MirrorSessionState_SCHEDULED
				m.Mutex.Unlock()
				sm.scheduleMirrorSession(m)
				return
			}
		}
	}
}

func (sm *Statemgr) deleteMirrorSession(mss *MirrorSessionState) {
	log.Infof("Deleting Mirror Session %v", mss.MirrorSession.Name)
	// Mutex - agent grpc thread may be using it
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	// Stop the timers,
	// If Timers cannot be stopped, its ok. Set the state to STOPPED. Timers
	// check the state and will no-op if it is STOPPED
	if mss.schTimer != nil {
		log.Infof("STOP SchTimer for %v", mss.MirrorSession.Name)
		mss.schTimer.Stop()
	}
	if mss.expTimer != nil {
		log.Infof("STOP ExpTimer for %v", mss.MirrorSession.Name)
		mss.expTimer.Stop()
	}
	if mss.State == monitoring.MirrorSessionState_ACTIVE {
		sm.MirrorSessionCountFree()
	}
	mss.State = monitoring.MirrorSessionState_STOPPED
	// XXX delete minio bucket used for packet capture from minio server when we support local captures

	// delete mirror session state from DB
	_ = sm.memDB.DeleteObject(mss)
}

func (sm *Statemgr) scheduleMirrorSession(mss *MirrorSessionState) {
	// Do not check the DB since the object might be replaced, use provided object
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	if mss.State != monitoring.MirrorSessionState_SCHEDULED {
		// some other thread may have updated the session
		mss.schTimer = nil
		return
	}
	mss.setMirrorSessionRunning()
	sm.memDB.UpdateObject(mss)
	log.Infof("scheduleMirrorSession(): Mirror Session %v - %v", mss.Name, mss.Status.ScheduleState)
	mss.schTimer = nil
	if err := sm.writer.WriteMirrorSession(mss.MirrorSession); err != nil {
		log.Errorf("failed to update %v status, %v", mss.Name, err)
	}
}

func (sm *Statemgr) stopMirrorSession(mss *MirrorSessionState) {
	log.Infof("Mirror Session Expired %v", mss.Name)
	mss.Mutex.Lock()
	defer mss.Mutex.Unlock()
	if mss.State == monitoring.MirrorSessionState_STOPPED {
		// already stopped, no need to process further
		mss.expTimer = nil
		return
	}
	if mss.State == monitoring.MirrorSessionState_ACTIVE {
		sm.MirrorSessionCountFree()
	}
	mss.State = monitoring.MirrorSessionState_STOPPED
	mss.Status.ScheduleState = monitoring.MirrorSessionState_STOPPED.String()
	mss.expTimer = nil
	sm.memDB.UpdateObject(mss)
	sm.writer.WriteMirrorSession(mss.MirrorSession)
}

// GetMirrorSessionState : Get the specified mirror sessions state
func (sm *Statemgr) GetMirrorSessionState(ms *monitoring.MirrorSession) (*MirrorSessionState, error) {
	dbMs, err := sm.FindObject("MirrorSession", &ms.ObjectMeta)
	if err != nil {
		return nil, err
	}
	return dbMs.(*MirrorSessionState), nil
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
		err := sm.UpdateMirrorSession(ms)
		if err != nil {
			log.Errorf("Error updating mirror session {%+v}. Err: %v", ms, err)
			return
		}
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

// Equal : Compare keys of mirror sessions
func (mss *MirrorSessionState) Equal(rhs *monitoring.MirrorSession) bool {
	return mss.MirrorSession.MakeKey("") == rhs.MakeKey("")
}

// PurgeDeletedMirrorSessions : Delete mirror session that are no longer in apiserver's DB
func (sm *Statemgr) PurgeDeletedMirrorSessions(msList []*monitoring.MirrorSession) {
	// incoming msList contains MirrorSessions that exists in apiserver. It is possible that
	// some mirror sessions may have been deleted since the grpc connection was lost
	// loss and restore of grpc connection can be due to -
	// 1. apiserver instance (could be more than one) fail-over/restart
	// 2. network problem (connectivity to apiserver)
	// 3. tsm restart
	// This function is needed for cases 1 and 2
	mssList, _ := sm.ListMirrorSessions()

checkNextMss:
	for _, mss := range mssList {
		// objects that are only in local DB and not in incoming list, must be deleted
		for _, ms := range msList {
			if mss.Equal(ms) {
				// goto outer loop
				continue checkNextMss
			}
		}
		// Delete this MirrorSession
		sm.deleteMirrorSession(mss)
	}
}
