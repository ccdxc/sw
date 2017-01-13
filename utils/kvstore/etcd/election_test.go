package etcd

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/utils/runtime"
)

const (
	contestName = "master"
)

// newContest creates a new contender.
func newContest(t *testing.T, store kvstore.Interface, id string, ttl uint64) kvstore.Election {
	election, err := store.Contest(contestName, id, ttl)
	if err != nil {
		t.Fatalf("Contest creation for %v failed with error: %v", id, err)
	}
	return election
}

// setupContest sets up the asked number of candidates.
func setupContest(t *testing.T, numCandidates int) (*integration.ClusterV3, []kvstore.Election) {
	cluster := integration.NewClusterV3(t)
	s := runtime.NewScheme()
	contenders := []kvstore.Election{}
	for ii := 0; ii < numCandidates; ii++ {
		store, err := newEtcdStore(cluster.NewClient(t), runtime.NewJSONCodec(s))
		if err != nil {
			t.Fatalf("Store creation failed with error: %v", err)
		}
		contenders = append(contenders, newContest(t, store, fmt.Sprintf("contender-%d", ii+1), minTTL))
	}
	return cluster, contenders
}

// checkEvents pulls one event out of each contender provided and checks that
// there is atmost one leader (if expLeader is set).
func checkEvents(t *testing.T, contenders []kvstore.Election, expLeader bool) {
	leaderCount := 0
	for _, contender := range contenders {
		select {
		case <-time.After(time.Second):
			t.Fatalf("Timed out waiting for event on leader %v", contender.Id())
		case e := <-contender.EventChan():
			t.Logf("Got event %+v for contender %v", e, contender.Id())
			if e.Leader == contender.Id() {
				if e.Type != kvstore.Elected {
					t.Fatalf("Leader %v with non elected event %v", contender.Id(), e.Type)
				}
				leaderCount++
			} else if e.Type != kvstore.Changed {
				t.Fatalf("Contender %v with non changed event %v", contender.Id(), e.Type)
			}
		}
	}
	if expLeader && leaderCount != 1 {
		t.Fatalf("Expected one leader, got %v", leaderCount)
	}
}

// TestElection checks the following.
// 1) One contender wins an election (among 3).
// 2) Stopping the contest on the leader results in another election+winner.
func TestElection(t *testing.T) {
	cluster, contenders := setupContest(t, 3)
	defer cluster.Terminate(t)

	checkEvents(t, contenders, true)

	newId := ""
	for ii, contender := range contenders {
		if contender.IsLeader() {
			newId = contender.Id()
			t.Logf("Stopping contender %v", newId)
			contender.Stop()
			contenders = append(contenders[:ii], contenders[ii+1:]...)
			// Changed events first
			checkEvents(t, contenders, false)
			// Leader event and changed event
			checkEvents(t, contenders, true)
			break
		}
	}

	leader := contenders[0].Leader()
	t.Logf("Adding contender %v with existing leader %v", newId, leader)
	s := runtime.NewScheme()
	store, err := newEtcdStore(cluster.NewClient(t), runtime.NewJSONCodec(s))
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contenders = append(contenders, newContest(t, store, newId, minTTL))

	if leader != contenders[0].Leader() {
		t.Fatalf("Leader changed to %v, expecting %v", contenders[0].Leader(), leader)
	}
}
