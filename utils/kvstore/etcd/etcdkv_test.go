package etcd

import (
	"context"
	"strconv"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/utils/runtime"
)

func clusterSetup(t *testing.T) kvstore.TestCluster {
	cluster := integration.NewClusterV3(t)
	return cluster
}

func clusterCleanup(t *testing.T, cluster kvstore.TestCluster) {
	c, ok := cluster.(*integration.ClusterV3)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Terminate(t)
}

func storeSetup(t *testing.T, cluster kvstore.TestCluster) (kvstore.Interface, error) {
	c, ok := cluster.(*integration.ClusterV3)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	s := runtime.NewScheme()
	s.AddKnownTypes(&kvstore.TestObj{}, &kvstore.TestObjList{})
	store, _ := newEtcdStore(c.Client(), runtime.NewJSONCodec(s))

	return store, nil
}

// Run standard compliance tests
func TestRunAll(t *testing.T) {
	kvstore.RunInterfaceTests(t, clusterSetup, storeSetup, clusterCleanup)
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T) {
	c := clusterSetup(t)
	store, _ := storeSetup(t, c)
	defer clusterCleanup(t, c)

	cluster, _ := c.(*integration.ClusterV3)

	obj := &kvstore.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(context.Background(), kvstore.TestKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(context.Background(), kvstore.TestKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	etcdWatcher := cluster.NewClient(t).Watch(context.Background(), kvstore.TestKey)

	if err := store.Delete(context.Background(), kvstore.TestKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.Atoi(ev.Object.(*kvstore.TestObj).ResourceVersion)
	if err != nil {
		t.Fatalf("Failed to parse watch version, error: %v", err)
	}

	res := <-etcdWatcher

	if res.Events[0].Kv.ModRevision != int64(watchVer) {
		t.Fatalf("Version mismatch, expected %v, got %v", res.Events[0].Kv.ModRevision, int64(watchVer))
	}
	t.Logf("Got expected version from watch")
}

// TestTxn tests creation/deletion/updation of keys in a transanction.
func TestTxn(t *testing.T) {
	c := clusterSetup(t)
	store, _ := storeSetup(t, c)
	defer clusterCleanup(t, c)

	obj1 := &kvstore.TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj1"}}
	obj2 := &kvstore.TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj2"}}

	txn1 := store.NewTxn()
	if err := txn1.Create(obj1.Name, obj1); err != nil {
		t.Fatalf("Failed to create obj1 in txn with error: %v", err)
	}
	if err := txn1.Create(obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to create obj1 in with error: %v", err)
	}
	if err := txn1.Commit(context.Background()); err != nil {
		t.Fatalf("Failed to commit txn with multiple Creates with error: %v", err)
	}
	if err := store.Get(context.Background(), obj1.Name, obj1); err != nil {
		t.Fatalf("Failed to get obj1 created in with error: %v", err)
	}
	if err := store.Get(context.Background(), obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to get obj2 created in with error: %v", err)
	}

	txn2 := store.NewTxn()
	oldVersion := obj1.ResourceVersion
	obj1.Counter++
	if err := txn2.Update(obj1.Name, obj1, kvstore.Compare(kvstore.WithVersion(obj1.Name), "=", obj1.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj1 in with error: %v", err)
	}
	if err := txn2.Update(obj2.Name, obj2, kvstore.Compare(kvstore.WithVersion(obj2.Name), "=", obj2.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj2 in with error: %v", err)
	}
	if err := txn2.Commit(context.Background()); err != nil {
		t.Fatalf("Failed to commit txn with multiple Updates with error: %v", err)
	} else if obj1.ResourceVersion == oldVersion {
		t.Fatalf("Failed to update version in txn")
	}

	txn3 := store.NewTxn()
	if err := store.Get(context.Background(), obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to get obj2 created in with error: %v", err)
	}
	oldVersion = obj2.ResourceVersion
	if err := txn3.Delete(obj1.Name, kvstore.Compare(kvstore.WithVersion(obj1.Name), "=", obj1.ResourceVersion)); err != nil {
		t.Fatalf("Failed to delete obj1 in with error: %v", err)
	}
	if err := txn3.Update(obj2.Name, obj2, kvstore.Compare(kvstore.WithVersion(obj2.Name), "=", obj2.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj2 in with error: %v", err)
	}
	if err := txn3.Commit(context.Background()); err != nil {
		t.Fatalf("Failed to commit txn with Update+Delete with error: %v", err)
	}
	if err := store.Get(context.Background(), obj1.Name, obj1); err == nil {
		t.Fatalf("Found obj1 deleted in txn, %+v", obj1)
	}
	if err := store.Get(context.Background(), obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to get obj2 updated in txn with error: %v", err)
	}
	if oldVersion == obj2.ResourceVersion {
		t.Fatalf("Failed to update version in txn")
	}

	t.Logf("TestTxn succeeded")
}
