package etcd

import (
	"context"
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
func newContest(t *testing.T, ctx context.Context, store kvstore.Interface, id string, ttl uint64) kvstore.Election {
	election, err := store.Contest(ctx, contestName, id, ttl)
	if err != nil {
		t.Fatalf("Contest creation for %v failed with error: %v", id, err)
	}
	return election
}

// addCandidates creates the specified number of candidates.
func addCandidates(t *testing.T, ctx context.Context, cluster *integration.ClusterV3, numCandidates, startId int) []kvstore.Election {
	s := runtime.NewScheme()
	contenders := []kvstore.Election{}
	for ii := 0; ii < numCandidates; ii++ {
		store, err := newEtcdStore(cluster.NewClient(t), runtime.NewJSONCodec(s))
		if err != nil {
			t.Fatalf("Store creation failed with error: %v", err)
		}
		contenders = append(contenders, newContest(t, ctx, store, fmt.Sprintf("contender-%d", ii+1+startId), minTTL))
	}
	return contenders
}

// setupContest sets up the asked number of candidates.
func setupContest(t *testing.T, ctx context.Context, numCandidates int) (*integration.ClusterV3, []kvstore.Election) {
	cluster := integration.NewClusterV3(t)
	return cluster, addCandidates(t, ctx, cluster, numCandidates, 0)
}

// checkEvents pulls one event out of each contender provided and checks that
// there is atmost one leader (if expLeader is set).
func checkEvents(t *testing.T, contenders []kvstore.Election, expLeader bool) {
	leaderCount := 0
	for _, contender := range contenders {
		select {
		case <-time.After(time.Second):
			t.Fatalf("Timed out waiting for event on leader %v", contender.ID())
		case e := <-contender.EventChan():
			t.Logf("Got event %+v for contender %v", e, contender.ID())
			if e.Leader == contender.ID() {
				if e.Type != kvstore.Elected {
					t.Fatalf("Leader %v with non elected event %v", contender.ID(), e.Type)
				}
				leaderCount++
			} else if e.Type != kvstore.Changed {
				t.Fatalf("Contender %v with non changed event %v", contender.ID(), e.Type)
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
	cluster, contenders := setupContest(t, context.Background(), 3)
	defer cluster.Terminate(t)

	checkEvents(t, contenders, true)

	newID := ""
	for ii, contender := range contenders {
		if contender.IsLeader() {
			newID = contender.ID()
			t.Logf("Stopping leader %v", newID)
			contender.Stop()
			contenders = append(contenders[:ii], contenders[ii+1:]...)
			// Leader event and changed event
			checkEvents(t, contenders, true)
			break
		}
	}

	leader := contenders[0].Leader()
	t.Logf("Adding contender %v with existing leader %v", newID, leader)
	s := runtime.NewScheme()
	store, err := newEtcdStore(cluster.NewClient(t), runtime.NewJSONCodec(s))
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contenders = append(contenders, newContest(t, context.Background(), store, newID, minTTL))

	if leader != contenders[0].Leader() {
		t.Fatalf("Leader changed to %v, expecting %v", contenders[0].Leader(), leader)
	}

	// Clean up
	for _, contender := range contenders {
		contender.Stop()
	}
}

// TestRestart checks the following.
// 1) One contender wins an election (among 3).
// 2) Start another contender with the same ID.
// 3) Check that the same ID wins the election.
func TestRestart(t *testing.T) {
	cluster, contenders := setupContest(t, context.Background(), 3)
	defer cluster.Terminate(t)

	checkEvents(t, contenders, true)

	t.Logf("Adding another contender with same id %v", contenders[0].Leader())

	s := runtime.NewScheme()
	store, err := newEtcdStore(cluster.NewClient(t), runtime.NewJSONCodec(s))
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contender := newContest(t, context.Background(), store, contenders[0].Leader(), minTTL)

	time.Sleep(time.Second)
	if contenders[0].Leader() != contender.Leader() {
		t.Fatalf("Leader changed to %v, expecting %v", contender.Leader(), contenders[0].Leader())
	}

	// Clean up
	contender.Stop()
	for _, contender := range contenders {
		contender.Stop()
	}
}

// TestCancelElection checks that cancelling the election stops the contender.
func TestCancelElection(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cluster, contenders := setupContest(t, ctx, 1)
	defer cluster.Terminate(t)

	checkEvents(t, contenders, true)

	// Add two more candidates.
	newContenders := addCandidates(t, context.Background(), cluster, 2, 1)

	// Check that original candidate is leader.
	if !contenders[0].IsLeader() {
		t.Fatalf("Original candidate is not leader")
	}

	cancel()
	time.Sleep(time.Millisecond * 500)

	// Check that one of the new candidates is leader.
	found := false
	for _, contender := range newContenders {
		if contender.IsLeader() {
			found = true
			break
		}
	}

	if !found {
		t.Fatalf("New candidate did not become leader on cancel of old leader")
	}

	// Clean up
	for _, contender := range contenders {
		contender.Stop()
	}

	t.Logf("Cancel of election succeeded")
}
