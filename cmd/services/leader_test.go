package services

import (
	"testing"
	"time"

	"fmt"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/runtime"
)

func setupTestCluster(t *testing.T) (*integration.ClusterV3, kvstore.Interface) {
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
	defer cluster.Terminate(t)

	id := "foo"

	l := NewLeaderService(store, "TestLeaderService", id)
	m := &mockObserver{}
	l.Register(m)

	go l.Start()

	for ii := 0; ii < 5; ii++ {
		if l.Leader() == id {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if l.Leader() != id {
		t.Fatalf("Failed to become leader")
	}
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
	for ii := 0; ii < 5; ii++ {
		if l.Leader() == "" {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if leader := l.Leader(); leader != "" {
		t.Fatalf("Found leader %v when not expected", leader)
	}
	time.Sleep(100 * time.Millisecond)
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
	defer cluster.Terminate(t)

	id := "foo"
	l := NewLeaderService(store, "TestLeaderServiceWithObserverError", id)

	m1 := &mockObserver{}
	l.Register(m1)
	m1.ForceError = true

	go l.Start()
	defer l.Stop()

	for ii := 0; ii < 5; ii++ {
		if l.Leader() == id {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if l.Leader() == id {
		t.Fatalf("Became leader when it shouldn't")
	}

	m1.ForceError = false

	for ii := 0; ii < 5; ii++ {
		if l.Leader() == id {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if l.Leader() != id {
		t.Fatalf("Failed to become leader")
	}

}

func TestLeaderRegisterService(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	id := "TestLeaderRegisterService"

	l := NewLeaderService(store, "TestLeaderRegisterService", id)
	m := &mockObserver{}
	l.Register(m)

	go l.Start()

	for ii := 0; ii < 5; ii++ {
		if l.Leader() == id {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if l.Leader() != id {
		t.Fatalf("Failed to become leader")
	}
	if m.LeaderStartCount != 1 {
		t.Fatalf("Expected LeaderStartCount of 1. Got %d", m.LeaderStartCount)
	}
	if m.LeaderStopCount != 0 {
		t.Fatalf("Expected LeaderStopCount of 0. Got %d", m.LeaderStopCount)
	}
	if m.LeaderChangeCount != 0 {
		t.Fatalf("Expected LeaderStartCount of 0. Got %d", m.LeaderChangeCount)
	}
	l.UnRegister(m)

	l.Stop()
	for ii := 0; ii < 5; ii++ {
		if l.Leader() == "" {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if leader := l.Leader(); leader != "" {
		t.Fatalf("Found leader %v when not expected", leader)
	}
	if m.LeaderStartCount != 1 || m.LeaderStopCount != 0 || m.LeaderChangeCount != 0 {
		t.Fatalf("Got notified of Leadership events event after Unregister. Value: %+v", m)
	}
}
