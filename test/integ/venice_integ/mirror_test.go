// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"
	"time"

	. "gopkg.in/check.v1"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var timeFormat = "2006-01-02 15:04:05 MST"

const (
	maxMirrorSessions = 8
)

var testMirrorSessions = []monitoring.MirrorSession{
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test-Mirror-Session-1",
			Tenant: "default",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: monitoring.PacketCollectorType_ERSPAN.String(),
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"10.1.1.10"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"ipprotocol/6"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"10.1.2.20"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"TCP/1234"},
					},
				},
			},
		},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test-Mirror-Session-2",
			Tenant: "default",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 10,
					},
				},
			},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: monitoring.PacketCollectorType_ERSPAN.String(),
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
}

// utility functions
func (it *veniceIntegSuite) createMirrorSession(ctx context.Context, activeMs map[string]api.ObjectMeta, ms *monitoring.MirrorSession) error {
	log.Infof("----- Test Create Mirror Session: %v", ms.Name)
	if ms.Spec.StartConditions.ScheduleTime != nil {
		// fix the time
		schAfter := time.Duration(20 * uint64(time.Second))
		schTime := time.Now().Add(schAfter)
		ts, _ := types.TimestampProto(schTime)
		ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
		log.Infof("Setting ScheduleTime to %v\n", schTime.Format(timeFormat))
	}
	_, err := it.restClient.MonitoringV1().MirrorSession().Create(ctx, ms)
	if err != nil {
		log.Infof("Create mirror session %v Error: %v", ms.Name, err)
		return err
	}
	activeMs[makeKey(ms.ObjectMeta)] = ms.ObjectMeta
	return err
}

func makeKey(o api.ObjectMeta) string {
	return fmt.Sprintf("%s/%s", o.Tenant, o.Name)
}

func (it *veniceIntegSuite) deleteMirrorSession(ctx context.Context, activeMs map[string]api.ObjectMeta, o *api.ObjectMeta) error {
	log.Infof("----- Delete Mirror Session %v", o.Name)
	_, err := it.restClient.MonitoringV1().MirrorSession().Delete(ctx, o)
	if err != nil {
		log.Infof("deleteMirrorSession Error: %v", err)
		return err
	}
	delete(activeMs, makeKey(*o))
	return err
}

func (it *veniceIntegSuite) deleteAllMirrorSessions(ctx context.Context, activeMs map[string]api.ObjectMeta) error {
	log.Infof("----- Delete ALL Mirror Sessions")
	msList := []api.ObjectMeta{}
	for _, o := range activeMs {
		msList = append(msList, o)
	}
	for _, o := range msList {
		if err := it.deleteMirrorSession(ctx, activeMs, &o); err != nil {
			return err
		}
	}
	return nil
}

// TestMirrorSessions - creates a mirror session using REST api
func (it *veniceIntegSuite) TestMirrorSessions(c *C) {
	log.Infof("----- TEST START: TestMirrorSessions")
	// This is a dummy test case and will be replaced with more realistic test cases
	// It is used only to iron-out framework and overall code structure of TS controller
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")
	activeMs := make(map[string]api.ObjectMeta)

	for _, ms := range testMirrorSessions {
		err := it.createMirrorSession(ctx, activeMs, &ms)
		Assert(c, err == nil, "Failed to create mirror session")
	}
	var intervals []string
	intervals = []string{"1s", "30s"}

	// make sure all created sessions did run
	AssertEventually(c, func() (bool, interface{}) {
		for _, ms := range testMirrorSessions {
			tms, err := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
			if err != nil {
				log.Infof("Get():MirrorSession %s:%s ... Not Found", ms.Tenant, ms.Name)
				return false, nil
			}
			if tms.Status.ScheduleState != monitoring.MirrorSessionState_ACTIVE.String() && tms.Status.ScheduleState != monitoring.MirrorSessionState_STOPPED.String() {
				log.Infof("Get():MirrorSession %s:%s ... Not Running/Stopped - %v", ms.Tenant, ms.Name, tms.Status.ScheduleState)
				return false, nil
			}
			log.Infof("Successful Get():MirrorSession %s:%s - %v", tms.Tenant, tms.Name, tms.Status.ScheduleState)
		}
		return true, nil
	}, "All Mirror Sessions did not run in TsCtrler", intervals...)

	log.Infof("----- Create Mirror Session[%d] %v again (should fail)", 1, testMirrorSessions[0].Name)
	ms := testMirrorSessions[0]
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err != nil, "Successfully re-created a mirror session - should not be allowed")
	// Delete
	err = it.deleteMirrorSession(ctx, activeMs, &ms.ObjectMeta)
	Assert(c, err == nil, "Unable to delete mirror session %v err=%v", testMirrorSessions[0].Name, err)
	// Recreate
	log.Infof("----- Recreate Mirror Session[1] %v", testMirrorSessions[0].Name)
	ms = testMirrorSessions[0]
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err == nil, "Unable to recreate a mirror session %v err=%v", testMirrorSessions[0].Name, err)
	AssertEventually(c, func() (bool, interface{}) {
		tms, err := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if err != nil {
			log.Infof("Get():MirrorSession %s:%s ... Not Found", ms.Tenant, ms.Name)
			return false, nil
		}
		if tms.Status.ScheduleState != monitoring.MirrorSessionState_ACTIVE.String() && tms.Status.ScheduleState != monitoring.MirrorSessionState_STOPPED.String() {
			log.Infof("Get():MirrorSession %s:%s ... Not Running/Stopped - %v", ms.Tenant, ms.Name, tms.Status.ScheduleState)
			return false, nil
		}
		log.Infof("Successful Get():MirrorSession %s:%s - %v", tms.Tenant, tms.Name, tms.Status.ScheduleState)
		return true, nil
	}, "Recreated Mirror Session did not run in TsCtrler", intervals...)

	for _, ms := range testMirrorSessions {
		log.Infof("----- Delete MirrorSession %s", ms.Name)
		err = it.deleteMirrorSession(ctx, activeMs, &ms.ObjectMeta)
		Assert(c, err == nil, "Failed to delete mirror session")
	}

	// Create MAX sessions
	log.Infof("----- Create Max Mirror Sessions")
	for i := 0; i < maxMirrorSessions; i++ {
		ms := testMirrorSessions[0]
		ms.Name = fmt.Sprintf("MirrorSession_%v", i)
		log.Infof("----- Create (Max) Mirror Sessions %v", ms.Name)
		err = it.createMirrorSession(ctx, activeMs, &ms)
		Assert(c, err == nil, "Unable to create a mirror session - %v", ms.Name)
	}
	mid := maxMirrorSessions - 1
	// delete one - create - pass
	log.Infof("----- Delete one and re-create")
	ms = testMirrorSessions[0]
	ms.Name = fmt.Sprintf("MirrorSession_%v", mid)
	err = it.deleteMirrorSession(ctx, activeMs, &ms.ObjectMeta)
	Assert(c, err == nil, "Failed to delete mirror session")
	// recreate
	log.Infof("----- and re-create")
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err == nil, "Unable to create a mirror session - %v", ms.Name)
	mid++
	ms = testMirrorSessions[0]
	ms.Name = fmt.Sprintf("MirrorSession_%v", mid)
	log.Infof("----- Create (>Max) Mirror Sessions %v", ms.Name)
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err != nil, "created more than max. mirror session - %v", ms.Name)

	log.Infof("----- List mirror sessions")
	msList, err := it.restClient.MonitoringV1().MirrorSession().List(ctx, &api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{Tenant: "default"}})
	Assert(c, err == nil, "Unable to retrieve mirror session list %v", err)
	totalSessions := len(msList)
	Assert(c, totalSessions == mid, "All sessions were not successfully created atual %v, expected %v, %+v", totalSessions, mid, msList)
	for _, ms := range msList {
		log.Infof("----- MirrorSession %v state %v", ms.Name, ms.Status.ScheduleState)
	}
	AssertEventually(c, func() (bool, interface{}) {
		errSessions := 0
		for _, ms := range msList {
			tms, err := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
			if err != nil {
				return false, err
			}
			if tms.Status.ScheduleState != monitoring.MirrorSessionState_ACTIVE.String() {
				errSessions++
			}
		}
		return errSessions == 0, nil
	}, "found mirror session in error state", intervals...)

	// Test updates
	// change errored sessions startTime to get it into scheduled state
	ms = testMirrorSessions[1]
	mid = maxMirrorSessions - 1
	ms.Name = fmt.Sprintf("MirrorSession_%v", mid)
	tm, err := ms.Spec.StartConditions.ScheduleTime.Time()
	Assert(c, err == nil, "Time() errored for %v", ms.Spec.StartConditions.ScheduleTime)
	tm = tm.Add(time.Duration(time.Second * 8))
	ts, _ := types.TimestampProto(tm)
	ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
	tm, _ = ms.Spec.StartConditions.ScheduleTime.Time()
	log.Infof("----- update SchTime to  %v", tm)
	_, err = it.restClient.MonitoringV1().MirrorSession().Update(ctx, &ms)
	Assert(c, err == nil, "Unable to update mirror session %v - %v", ms.Name, err)

	AssertEventually(c, func() (bool, interface{}) {
		tms, err := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if err != nil {
			return false, err
		}
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION.String() {
			return false, "Mirror session in ERR_ state"
		}
		return true, nil
	}, "Scheduled session is not out of ERROR state as expected", intervals...)

	time.Sleep(time.Duration(time.Second * 8))
	// Delete all mirror sessions
	it.deleteAllMirrorSessions(ctx, activeMs)
}

func (it *veniceIntegSuite) TestMirrorSessionUpdate(c *C) {
	log.Infof("----- TEST START: TestMirrorSessionUpdate")
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")
	activeMs := make(map[string]api.ObjectMeta)

	for _, ms := range testMirrorSessions {
		err := it.createMirrorSession(ctx, activeMs, &ms)
		Assert(c, err == nil, "Failed to create mirror session")
	}
	// Test cases -
	// 1. Change scheduled to run immediately
	log.Infof("----- T1: Change scheduled to run immediately")
	intervals := []string{"1s", "30s"}
	ms := testMirrorSessions[1]
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_SCHEDULED.String() {
			return true, nil
		}
		return false, "Not scheduled yet"
	}, "Scheduled session is not found", intervals...)

	ms.Spec.StartConditions.ScheduleTime = nil
	_, err = it.restClient.MonitoringV1().MirrorSession().Update(ctx, &ms)

	AssertOk(c, err, "Error update test1")
	intervals = []string{"1s", "8s"}
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_ACTIVE.String() {
			return true, nil
		}
		return false, "Not running yet"
	}, "Mirror session 2 is not running", intervals...)

	// T2: schedule change while running (add start condition)
	schAfter := time.Duration(15 * uint64(time.Second))
	log.Infof("Duration=%v, Now=%v = %v\n", schAfter, time.Now(), time.Now().Format(timeFormat))
	schTime := time.Now().Add(schAfter)
	ts, _ := types.TimestampProto(schTime)
	ms.Spec.StartConditions.ScheduleTime = &api.Timestamp{}
	ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
	log.Infof("Setting ScheduleTime to %v\n", schTime.Format(timeFormat))

	log.Infof("----- T2: schdule change while running (start condition)")
	_, err = it.restClient.MonitoringV1().MirrorSession().Update(ctx, &ms)
	Assert(c, err == nil, "T2:Successfully updated startCondition of a running mirror session")

	// wait for status to become scheduled
	time.Sleep(time.Duration(time.Second * 5))
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_SCHEDULED.String() {
			return true, nil
		}
		return false, "Not scheduled yet"
	}, "Scheduled session is not found", intervals...)

	// T3: reduce start time of a scheduled session
	log.Infof("----- T3: advance start time of a scheduled session")
	ms = testMirrorSessions[1]
	err = it.deleteMirrorSession(ctx, activeMs, &ms.ObjectMeta)
	Assert(c, err == nil, "Delete failed for mirror session 2")

	ms.Spec.StartConditions.ScheduleTime.Seconds = 20
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err == nil, "Create failed for mirror session 2")

	// wait for status update ???
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_SCHEDULED.String() {
			return true, nil
		}
		return false, "Not scheduled yet"
	}, "Scheduled session is not found", intervals...)

	// reduce the schedule time to Now
	time.Sleep(time.Duration(time.Second * 1))
	schTime = time.Now()
	ts, _ = types.TimestampProto(schTime)
	ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
	log.Infof("---- T3:resetting ScheduleTime to %v\n", schTime.Format(timeFormat))
	_, err = it.restClient.MonitoringV1().MirrorSession().Update(ctx, &ms)
	Assert(c, err == nil, "T3:Failed to update schedule time")

	intervals = []string{"1s", "30s"}
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_ACTIVE.String() {
			return true, nil
		}
		return false, "Not running yet"
	}, "T3:Mirror session 2 is not running", intervals...)

	log.Infof("----- T4: Stop (rightaway) a running session")
	ms = testMirrorSessions[0]
	err = it.deleteMirrorSession(ctx, activeMs, &ms.ObjectMeta)
	Assert(c, err == nil, "T4 delete old session - error")
	ms = testMirrorSessions[0]
	err = it.createMirrorSession(ctx, activeMs, &ms)
	Assert(c, err == nil, "T4 create session - error")
	// make sure it starts running
	AssertEventually(c, func() (bool, interface{}) {
		tms, _ := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
		if tms.Status.ScheduleState == monitoring.MirrorSessionState_ACTIVE.String() {
			return true, nil
		}
		return false, "Not running yet"
	}, "T4: running session is not found", intervals...)

	log.Infof("----- T5: Update non-existing session")
	ms = testMirrorSessions[0]
	ms.Name = "xyz"
	_, err = it.restClient.MonitoringV1().MirrorSession().Update(ctx, &ms)
	Assert(c, err != nil, "T5: update suceeded - should have failed")

	time.Sleep(time.Duration(time.Second * 5))
	// cleanup
	it.deleteAllMirrorSessions(ctx, activeMs)
	// check if mirror session count is accurate - create and delete max sessions
	log.Infof("----- Cleanup check : Create Max Mirror Sessions")
	for i := 0; i < maxMirrorSessions; i++ {
		ms := testMirrorSessions[0]
		ms.Name = fmt.Sprintf("Max_MirrorSession_%v", i)
		log.Infof("----- Create (Max) Mirror Sessions %v", ms.Name)
		err = it.createMirrorSession(ctx, activeMs, &ms)
		Assert(c, err == nil, "Unable to create a mirror session - %v", ms.Name)
	}
	// cleanup
	it.deleteAllMirrorSessions(ctx, activeMs)
}
