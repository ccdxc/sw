package services

import (
	"fmt"
	"time"

	etcd "github.com/coreos/etcd/clientv3"
	gogotypes "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
)

var (
	jt                     = NewDefragJobTicker()
	quorumClientNumRetries = 3
)

//update Ticker with new duration
func (jt DefragJobTicker) updateDefragJobTicker() {
	log.Infof("updating  DefragJobTicker")
	jt.t.Reset(getNextTickDuration())
}

// do defragment on Quorum member
func performQuorumDefrag(start bool) {

	if start {

		for {
			<-jt.t.C
			if env.Quorum != nil {
				if members, err := env.Quorum.List(); err == nil {
					for _, member := range members {
						env.Quorum.Defrag(&member)
						time.Sleep(time.Minute * 5)
					}
				}
			}
			jt.updateDefragJobTicker()
		}
	} else {
		jt.t.Stop()
	}

}

func getQuorumMemberCondition(memberName string, condType cmd.QuorumMemberCondition_ConditionType, qs *cmd.QuorumStatus) *cmd.QuorumMemberCondition {
	if qs != nil {
		for _, m := range qs.Members {
			if m.Name == memberName {
				for _, c := range m.Conditions {
					if c.Type == condType.String() {
						return c
					}
				}
			}
		}
	}
	return nil
}

func updateQuorumMemberCondition(memberName string, condType cmd.QuorumMemberCondition_ConditionType, qs *cmd.QuorumStatus, updCond *cmd.QuorumMemberCondition) error {
	if qs != nil {
		// search existing members first
		for _, m := range qs.Members {
			if m.Name == memberName {
				for _, c := range m.Conditions {
					if c.Type == condType.String() {
						*c = *updCond
						return nil
					}
				}
				// condition not found, append
				m.Conditions = append(m.Conditions, updCond)
				return nil
			}
		}
		return fmt.Errorf("Member %s not found", memberName)
	}
	return fmt.Errorf("NIL quorum status")
}

func deleteQuorumMember(memberName string, qs *cmd.QuorumStatus) {
	if qs != nil {
		for i, m := range qs.Members {
			if m.Name == memberName {
				qs.Members = append(qs.Members[:i], qs.Members[i+1:]...)
				return
			}
		}
	}
}

func addQuorumMember(member *cmd.QuorumMemberStatus, qs *cmd.QuorumStatus) {
	if qs != nil {
		for _, cond := range member.Conditions {
			cond.LastTransitionTime = &api.Timestamp{Timestamp: *gogotypes.TimestampNow()}
		}
		qs.Members = append(qs.Members, member)
	}
}

func getNumHealthyQuorumMembers(qs *cmd.QuorumStatus) int {
	var result int
	if qs != nil {
		for _, m := range qs.Members {
			for _, c := range m.Conditions {
				if c.Type == cmd.QuorumMemberCondition_HEALTHY.String() && c.Status == cmd.ConditionStatus_TRUE.String() {
					result++
					break
				}
			}
		}
	}
	return result
}

func getQuorumStatus(q quorum.Interface) *cmd.QuorumStatus {
	if q == nil {
		log.Errorf("Quorum is not available")
		return nil
	}

	var members []quorum.Member
	var err error
	var i int

	for i = 0; i < quorumClientNumRetries; i++ {
		members, err = q.List()
		if err == nil {
			break
		}
		log.Errorf("Error getting quorum members list: %v", err)
	}
	if i == quorumClientNumRetries {
		log.Errorf("Error getting quorum members list, retries exhausted")
		return nil
	}

	qs := &cmd.QuorumStatus{}
	for _, m := range members {
		qms := &cmd.QuorumMemberStatus{
			Name: m.Name,
			ID:   fmt.Sprintf("%x", m.ID),
			Conditions: []*cmd.QuorumMemberCondition{
				{
					Type: cmd.QuorumMemberCondition_HEALTHY.String(),
				},
			},
		}

		if len(m.Name) == 0 { // this logic follows etcdctl v3 makeMemberListTable()
			qms.Status = "unstarted"
		} else {
			qms.Status = "started"
		}

		for i = 0; i < quorumClientNumRetries; i++ {
			if statusResp, err := q.GetStatus(&m); err == nil {
				status := statusResp.(*etcd.StatusResponse).Header
				qms.Term = fmt.Sprintf("%d", status.RaftTerm)
				break
			}
			log.Errorf("Error getting status for quorum member %+v: %v", m, err)
		}
		if i == quorumClientNumRetries {
			log.Errorf("Error getting status for quorum member %+v, retries exhausted", m)
			// do not return
		}

		for i = 0; i < quorumClientNumRetries; i++ {
			if healthy, err := q.GetHealth(&m); err == nil {
				if healthy {
					qms.Conditions[0].Status = cmd.ConditionStatus_TRUE.String()
				} else {
					qms.Conditions[0].Status = cmd.ConditionStatus_FALSE.String()
				}
				break
			}
			log.Errorf("Error getting health for quorum member %+v: %v", m, err)
		}
		if i == quorumClientNumRetries {
			log.Errorf("Error getting health for quorum member %+v, retries exhausted", m)
			qms.Conditions[0].Status = cmd.ConditionStatus_UNKNOWN.String()
			// do not return
		}

		log.Infof("Quorum Member: %+v, status: %+v", m, qms)
		qs.Members = append(qs.Members, qms)
	}
	return qs
}

func generateQuorumEvent(memberName string, cond *cmd.QuorumMemberCondition, clusterObj *cmd.Cluster) {
	var evStr string
	if cond.Status == cmd.ConditionStatus_TRUE.String() {
		evStr = fmt.Sprintf("Quorum member %s is now healthy", memberName)
		recorder.Event(eventtypes.QUORUM_MEMBER_HEALTHY, evStr, clusterObj)
	} else if cond.Status == cmd.ConditionStatus_FALSE.String() {
		evStr = fmt.Sprintf("Quorum member %s is now unhealthy", memberName)
		recorder.Event(eventtypes.QUORUM_MEMBER_UNHEALTHY, evStr, clusterObj)
	} else {
		evStr = fmt.Sprintf("Quorum member %s is now disconnected", memberName)
		recorder.Event(eventtypes.QUORUM_MEMBER_UNHEALTHY, evStr, clusterObj)
	}
	log.Infof(evStr)
}

func updateQuorumStatus(quorum quorum.Interface, clusterObj *cmd.Cluster) bool {
	update := false

	newQuorumStatus := getQuorumStatus(quorum)
	if newQuorumStatus == nil {
		errStr := fmt.Sprintf("Unable to get quorum status")
		log.Errorf(errStr)
		recorder.Event(eventtypes.QUORUM_UNHEALTHY, errStr, clusterObj)
		return false
	}

	if clusterObj.Status.QuorumStatus == nil {
		clusterObj.Status.QuorumStatus = newQuorumStatus
		for _, member := range newQuorumStatus.Members {
			for _, cond := range member.Conditions {
				cond.LastTransitionTime = &api.Timestamp{Timestamp: *gogotypes.TimestampNow()}
			}
		}
		update = true
	}
	oldQuorumStatus := clusterObj.Status.QuorumStatus

	oldNumHealthyMembers := getNumHealthyQuorumMembers(oldQuorumStatus)
	newNumHealthyMembers := getNumHealthyQuorumMembers(newQuorumStatus)
	newQuorumSize := len(newQuorumStatus.Members)
	minNumHealthyMembers := (newQuorumSize / 2) + 1
	if oldNumHealthyMembers != newNumHealthyMembers && newNumHealthyMembers < minNumHealthyMembers {
		errStr := fmt.Sprintf("Quorum does not have enough healthy members, have: %d, want: %d", newNumHealthyMembers, minNumHealthyMembers)
		log.Errorf(errStr)
		recorder.Event(eventtypes.QUORUM_UNHEALTHY, errStr, clusterObj)
	}

	// remove members that are no longer there
	tmp := make([]*cmd.QuorumMemberStatus, len(oldQuorumStatus.Members))
	copy(tmp, oldQuorumStatus.Members)
	for _, oldMember := range tmp {
		found := false
		for _, newMember := range newQuorumStatus.Members {
			if newMember.Name == oldMember.Name {
				found = true
				break
			}
		}
		if !found {
			deleteQuorumMember(oldMember.Name, clusterObj.Status.QuorumStatus)
			update = true
		}
	}

	// add new members, if any
	for _, newMember := range newQuorumStatus.Members {
		found := false
		for _, oldMember := range oldQuorumStatus.Members {
			if newMember.Name == oldMember.Name {
				found = true
				break
			}
		}
		if !found {
			addQuorumMember(newMember, clusterObj.Status.QuorumStatus)
			healthCond := getQuorumMemberCondition(newMember.Name, cmd.QuorumMemberCondition_HEALTHY, clusterObj.Status.QuorumStatus)
			if healthCond != nil {
				generateQuorumEvent(newMember.Name, healthCond, clusterObj)
			}
			update = true
		}
	}

	// update existing members if needed
	for _, m := range newQuorumStatus.Members {
		oldCond := getQuorumMemberCondition(m.Name, cmd.QuorumMemberCondition_HEALTHY, oldQuorumStatus)
		newCond := getQuorumMemberCondition(m.Name, cmd.QuorumMemberCondition_HEALTHY, newQuorumStatus)

		if newCond != nil && (oldCond == nil || oldCond.Status != newCond.Status) {
			update = true
			newCond.LastTransitionTime = &api.Timestamp{Timestamp: *gogotypes.TimestampNow()}
			err := updateQuorumMemberCondition(m.Name, cmd.QuorumMemberCondition_HEALTHY, clusterObj.Status.QuorumStatus, newCond)
			if err != nil {
				log.Errorf("Error updating health condition %+v for member %s: %v", newCond, m.Name, err)
			}
			generateQuorumEvent(m.Name, newCond, clusterObj)
		}
	}

	return update
}
