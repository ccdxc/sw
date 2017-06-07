package memkv

import (
	"context"
	"fmt"
	"strconv"

	"math/rand"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

func clusterSetup(t *testing.T) kvstore.TestCluster {
	rand.Seed(74)
	return &memkvCluster{
		elections: make(map[string]*memkvElection),
		clientId:  0,
		stores:    make(map[string]*memKv),
	}
}

func clusterCleanup(t *testing.T, cluster kvstore.TestCluster) {
	c, ok := cluster.(*memkvCluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Lock()
	defer c.Unlock()

	// walk all state stores and delete keys
	for memkvKey, f := range c.stores {
		f.deleteAll()
		delete(c.stores, memkvKey)
	}
}

func storeSetup(t *testing.T, cluster kvstore.TestCluster) (kvstore.Interface, error) {
	c, ok := cluster.(*memkvCluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Lock()
	defer c.Unlock()

	c.clientId++
	clientName := fmt.Sprintf("memkv-%d", c.clientId)

	s := runtime.NewScheme()
	s.AddKnownTypes(&kvstore.TestObj{}, &kvstore.TestObjList{})
	store, _ := NewMemKv(c, runtime.NewJSONCodec(s))
	cs, ok := store.(*memKv)
	if !ok {
		t.Fatalf("invalid store")
	}
	c.stores[clientName] = cs

	return store, nil
}

// Run common set of compliance tests
func TestRunAll(t *testing.T) {
	kvstore.RunInterfaceTests(t, clusterSetup, storeSetup, clusterCleanup)
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T) {
	cluster := clusterSetup(t)
	store, _ := storeSetup(t, cluster)
	defer clusterCleanup(t, cluster)

	obj := &kvstore.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(context.Background(), kvstore.TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(context.Background(), kvstore.TestKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	into := &kvstore.TestObj{}
	err = store.Get(context.Background(), kvstore.TestKey, into)
	if err != nil {
		t.Fatalf("Get of the key failed with error: %v", err)
	}

	if err := store.Delete(context.Background(), kvstore.TestKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.ParseInt(ev.Object.(*kvstore.TestObj).ResourceVersion, 10, 64)
	if err != nil {
		t.Fatalf("Failed to parse watch version, error: %v", err)
	}

	objMeta, _ := runtime.GetObjectMeta(into)
	objVersion, err := strconv.ParseInt(objMeta.ResourceVersion, 10, 64)
	if err != nil {
		t.Fatalf("Unable to parse the object's version")
	}
	if objVersion != watchVer {
		t.Fatalf("Version mismatch, expected %v, got %v", int64(objVersion), int64(watchVer))
	}

	t.Logf("Got expected version from watch")
}
