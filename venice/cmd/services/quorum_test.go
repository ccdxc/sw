package services

import (
	"fmt"
	"reflect"
	"testing"

	cmd "github.com/pensando/sw/api/generated/cluster"
	evt "github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/quorum/mock"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func stringToUintID(s string) uint64 {
	var id uint64
	fmt.Sscanf(s, "%x", &id)
	return id
}

func validateQuorumEvents(events []mockevtsrecorder.Event, et evt.EventType, expCount int) error {
	count := 0
	for _, ev := range events {
		if ev.EventType == et.String() {
			count++
		}
	}
	if count != expCount {
		return fmt.Errorf("Found unexpected number of events of type %s. Have: %d, want: %d", et.String(), count, expCount)
	}
	return nil
}

func validateQuorumStatus(quorumSvc quorum.Interface, quorumStatus *cmd.QuorumStatus) error {
	if quorumStatus == nil {
		return fmt.Errorf("Quorum status should not be nil")
	}
	quorumSvcMembers, _ := quorumSvc.List()
	if len(quorumSvcMembers) != len(quorumStatus.Members) {
		return fmt.Errorf("Mismatch in number of members. Map: %d, obj: %d", len(quorumSvcMembers), len(quorumStatus.Members))
	}
	quorumSvcMap := make(map[uint64]quorum.Member)
	for _, q := range quorumSvcMembers {
		quorumSvcMap[q.ID] = q
	}

	for _, quorumStatusMember := range quorumStatus.Members {
		id := stringToUintID(quorumStatusMember.ID)
		quorumSvcMember := quorumSvcMap[id]
		if id != quorumSvcMember.ID {
			return fmt.Errorf("ID mismatch. Have: %v, want: %v", quorumStatusMember.ID, quorumSvcMember.ID)
		}
		if quorumStatusMember.Name != quorumSvcMember.Name {
			return fmt.Errorf("Name mismatch. Have: %v, want: %v", quorumStatusMember.Name, quorumSvcMember.Name)
		}
		if quorumStatusMember.Status != "started" {
			return fmt.Errorf("Unexpected status. Have: %v, want: %v", quorumStatusMember.Status, "started")
		}
		hc := getQuorumMemberCondition(quorumStatusMember.Name, cmd.QuorumMemberCondition_HEALTHY, quorumStatus)
		if hc == nil {
			return fmt.Errorf("Health condition not found for member %+v", quorumStatusMember)
		}
		expHealth, _ := quorumSvc.GetHealth(&quorumSvcMember)
		if (expHealth == true && hc.Status == cmd.ConditionStatus_FALSE.String()) ||
			(expHealth == false && hc.Status == cmd.ConditionStatus_TRUE.String()) {
			return fmt.Errorf("Unexpected health status %s for member %+v", hc.Status, quorumStatusMember)
		}
		if hc.LastTransitionTime == nil {
			return fmt.Errorf("LastTransitionTime not set on cond %+v for member %+v", hc, quorumStatusMember)
		}
	}
	return nil
}

func TestQuorumStatus(t *testing.T) {
	logger = log.WithContext("module", "quorum_test")
	// create mock events recorder
	mr := mockevtsrecorder.NewRecorder("quorum_test", logger)
	_ = recorder.Override(mr)

	numMembers := 5

	q := mock.NewQuorum()
	for i := 0; i < numMembers; i++ {
		err := q.Add(&quorum.Member{ID: uint64(i), Name: fmt.Sprintf("M%d", i)})
		AssertOk(t, err, "Error adding quorum member %d", i)
	}

	clusterObj := &cmd.Cluster{}
	update := updateQuorumStatus(q, clusterObj)
	Assert(t, update, "updateQuorumStatus did not return true as expected")
	AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")

	// call updateQuorumStatus again, there should be no changes
	refClusterObj := ref.DeepCopy(clusterObj).(*cmd.Cluster)
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, !update, "updateQuorumStatus did not return false as expected")
	Assert(t, reflect.DeepEqual(refClusterObj, clusterObj), "updateQuorumStatus should not have modified cluster object.\nBefore: %#v\nAfter: %#v", refClusterObj, clusterObj)

	// Mark all members unhealthy one by one and validate after each step
	for i := 0; i < numMembers; i++ {
		err := q.SetHealth(&quorum.Member{ID: uint64(i)}, false)
		AssertOk(t, err, "Error setting health")
		update = updateQuorumStatus(q, clusterObj)
		Assert(t, update, "updateQuorumStatus did not return true as expected")
		AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")
	}
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, !update, "updateQuorumStatus did not return false as expected")

	// Check expected events
	ev := mr.GetEvents()
	AssertOk(t, validateQuorumEvents(ev, evt.QUORUM_MEMBER_UNHEALTHY, numMembers), "")
	AssertOk(t, validateQuorumEvents(ev, evt.QUORUM_UNHEALTHY, globals.MinSupportedQuorumSize), "")
	mr.ClearEvents()

	// Mark 1 member as healthy again
	err := q.SetHealth(&quorum.Member{ID: uint64(3)}, true)
	AssertOk(t, err, "Error setting health")
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, update, "updateQuorumStatus did not return true as expected")
	AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")

	// Check expected events
	ev = mr.GetEvents()
	AssertOk(t, validateQuorumEvents(ev, evt.QUORUM_MEMBER_HEALTHY, 1), "")
	mr.ClearEvents()

	// Remove 1 member
	q.Remove(3)
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, update, "updateQuorumStatus did not return true as expected")
	AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")

	// Add back 3 and two new ones
	for _, i := range []uint64{3, 6, 9} {
		err := q.Add(&quorum.Member{ID: uint64(i), Name: fmt.Sprintf("M%d", i)})
		AssertOk(t, err, "Error adding quorum member %d", i)
		update = updateQuorumStatus(q, clusterObj)
		Assert(t, update, "updateQuorumStatus did not return true as expected")
		AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")
	}
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, !update, "updateQuorumStatus did not return false as expected")

	// Check expected events
	ev = mr.GetEvents()
	AssertOk(t, validateQuorumEvents(ev, evt.QUORUM_MEMBER_HEALTHY, 3), "")
	mr.ClearEvents()

	// remove all
	ms, err := q.List()
	AssertOk(t, err, "Error listing quorum members")
	for _, m := range ms {
		err := q.Remove(m.ID)
		AssertOk(t, err, "Error removing quorum member %d", m.ID)
	}
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, update, "updateQuorumStatus did not return true as expected")
	AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, !update, "updateQuorumStatus did not return false as expected")

	// add back
	for i := 0; i < numMembers; i++ {
		err := q.Add(&quorum.Member{ID: uint64(i), Name: fmt.Sprintf("M%d", i)})
		AssertOk(t, err, "Error adding quorum member %d", i)
		update = updateQuorumStatus(q, clusterObj)
		Assert(t, update, "updateQuorumStatus did not return true as expected")
		AssertOk(t, validateQuorumStatus(q, clusterObj.Status.QuorumStatus), "Error validating quorum status")
	}
	update = updateQuorumStatus(q, clusterObj)
	Assert(t, !update, "updateQuorumStatus did not return false as expected")

	// Check expected events
	ev = mr.GetEvents()
	AssertOk(t, validateQuorumEvents(ev, evt.QUORUM_MEMBER_HEALTHY, numMembers), "")
	mr.ClearEvents()
}
