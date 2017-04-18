package services

import (
	"testing"
	"time"

	"github.com/pensando/sw/cmd/env"
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

func TestLeaderService(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	env.KVStore = store
	id := "foo"

	l := NewLeaderService(id)
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
}
