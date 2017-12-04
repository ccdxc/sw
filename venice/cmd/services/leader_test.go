package services

import (
	"os"
	"testing"

	"fmt"

	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func setupTestCluster(t *testing.T) (*integration.ClusterV3, kvstore.Interface) {
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(t)
	s := runtime.NewScheme()
	store, err := store.New(store.Config{
		Type:    store.KVStoreTypeEtcd,
		Servers: []string{cluster.ClientURL()},
		Codec:   runtime.NewJSONCodec(s),
	})
	if err != nil {
		t.Fatalf("Failed to create store, error: %v", err)
	}
	return cluster, store
}

func cleanupTestCluster(t *testing.T, c *integration.ClusterV3, s kvstore.Interface) {
	s.Close()
	c.Terminate(t)
}

type mockObserver struct {
	LeaderStartCount, LeaderChangeCount, LeaderStopCount int
	ForceError                                           bool
}

func (m *mockObserver) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	switch e.Evt {
	case types.LeaderEventChange:
		m.LeaderChangeCount++
	case types.LeaderEventWon:
		m.LeaderStartCount++
	case types.LeaderEventLost:
		m.LeaderStopCount++
	}
	if m.ForceError {
		return fmt.Errorf("Error set")
	}
	return nil

}

func TestLeaderService(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cleanupTestCluster(t, cluster, store)

	id := "foo"

	l := NewLeaderService(store, "TestLeaderService", id)
	m := &mockObserver{}
	l.Register(m)

	go l.Start()
	defer l.Stop()

	AssertEventually(t, func() (bool, []interface{}) {
		return l.Leader() == id, []interface{}{"Unexpected leader: ", l.Leader()}
	}, "Failed to become leader", "10ms", "2s")

	if m.LeaderStartCount != 1 {
		t.Fatalf("Expected LeaderStartCount of 1. Got %d", m.LeaderStartCount)
	}
	if m.LeaderStopCount != 0 {
		t.Fatalf("Expected LeaderStopCount of 0. Got %d", m.LeaderStopCount)
	}
	if m.LeaderChangeCount != 0 {
		t.Fatalf("Expected LeaderStartCount of 0. Got %d", m.LeaderChangeCount)
	}

	l.Stop()
	AssertEventually(t, func() (bool, []interface{}) {
		return l.Leader() == "", []interface{}{l.Leader()}
	}, "Failed to give up leadership", "10ms", "1s")

	if m.LeaderStartCount != 1 {
		t.Fatalf("Expected LeaderStartCount of 1. Got %d", m.LeaderStartCount)
	}
	if m.LeaderStopCount != 1 {
		t.Fatalf("Expected LeaderStopCount of 1. Got %d", m.LeaderStopCount)
	}
	if m.LeaderChangeCount != 0 {
		t.Fatalf("Expected LeaderStartCount of 0. Got %d", m.LeaderChangeCount)
	}
}

func TestLeaderServiceWithObserverError(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cleanupTestCluster(t, cluster, store)

	id := "foo"
	l := NewLeaderService(store, "TestLeaderServiceWithObserverError", id)

	m1 := &mockObserver{}
	l.Register(m1)
	m1.ForceError = true // this will fail observer's OnNotifyLeaderEvent() and thus the leader event

	go l.Start()
	defer l.Stop()

	AssertConsistently(t, func() (bool, []interface{}) {
		return l.Leader() != id, nil
	}, "Became leader when it shouldn't", "10ms", "100ms")

	m1.ForceError = false

	AssertEventually(t, func() (bool, []interface{}) {
		return l.Leader() == id, nil
	}, "Failed to become leader", "10ms", "2s")
}

func TestLeaderRegisterService(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cleanupTestCluster(t, cluster, store)

	id := "TestLeaderRegisterService"

	l := NewLeaderService(store, "TestLeaderRegisterService", id)
	m := &mockObserver{}
	l.Register(m)

	go l.Start()
	defer l.Stop()

	AssertEventually(t, func() (bool, []interface{}) {
		return l.Leader() == id, nil
	}, "Failed to become leader", "10ms", "2s")

	if m.LeaderStartCount != 1 {
		t.Fatalf("Expected LeaderStartCount of 1. Got %d", m.LeaderStartCount)
	}
	if m.LeaderStopCount != 0 {
		t.Fatalf("Expected LeaderStopCount of 0. Got %d", m.LeaderStopCount)
	}
	if m.LeaderChangeCount != 0 {
		t.Fatalf("Expected LeaderStartCount of 0. Got %d", m.LeaderChangeCount)
	}
	l.UnRegister(m) // observer will be no more notified of the leader events

	l.Stop()

	AssertEventually(t, func() (bool, []interface{}) {
		return l.Leader() == "", nil
	}, "Failed to give up leadership", "10ms", "2s")

	if m.LeaderStartCount != 1 || m.LeaderStopCount != 0 || m.LeaderChangeCount != 0 {
		t.Fatalf("Got notified of Leadership events event after Unregister. Value: %+v", m)
	}
}
