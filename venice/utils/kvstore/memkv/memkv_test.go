package memkv

import (
	"context"
	"fmt"
	"strconv"

	"math/rand"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	compliance "github.com/pensando/sw/venice/utils/kvstore/compliance"
	"github.com/pensando/sw/venice/utils/runtime"
)

func clusterSetup(t *testing.T) compliance.TestCluster {
	rand.Seed(74)
	return getCluster([]string{"default"})
}

func clusterCleanup(t *testing.T, cluster compliance.TestCluster) {
	c, ok := cluster.(*Cluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	// walk all state stores and delete keys
	c.deleteAll()
}

func storeSetup(t *testing.T, cluster compliance.TestCluster) (kvstore.Interface, error) {
	c, ok := cluster.(*Cluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Lock()
	defer c.Unlock()

	c.clientID++
	clientName := fmt.Sprintf("memkv-%d", c.clientID)

	s := runtime.NewScheme()
	s.AddKnownTypes(&compliance.TestObj{}, &compliance.TestObjList{})
	store, _ := NewMemKv([]string{"default"}, runtime.NewJSONCodec(s))
	cs, ok := store.(*MemKv)
	if !ok {
		t.Fatalf("invalid store")
	}
	c.clients[clientName] = cs

	return store, nil
}

// Run common set of compliance tests
func TestRunAll(t *testing.T) {
	compliance.RunInterfaceTests(t, clusterSetup, storeSetup, clusterCleanup)
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T) {
	cluster := clusterSetup(t)
	store, _ := storeSetup(t, cluster)
	defer clusterCleanup(t, cluster)

	obj := &compliance.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(context.Background(), compliance.TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(context.Background(), compliance.TestKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	into := &compliance.TestObj{}
	err = store.Get(context.Background(), compliance.TestKey, into)
	if err != nil {
		t.Fatalf("Get of the key failed with error: %v", err)
	}

	if err = store.Delete(context.Background(), compliance.TestKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.ParseInt(ev.Object.(*compliance.TestObj).ResourceVersion, 10, 64)
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
