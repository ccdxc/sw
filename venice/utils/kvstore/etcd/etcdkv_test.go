package etcd

import (
	"context"
	"os"
	"strconv"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	compliance "github.com/pensando/sw/venice/utils/kvstore/compliance"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/runtime"
)

func clusterSetup(t *testing.T) compliance.TestCluster {
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(t)
	return cluster
}

func clusterCleanup(t *testing.T, cluster compliance.TestCluster) {
	c, ok := cluster.(*integration.ClusterV3)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Terminate(t)
}

func storeSetup(t *testing.T, cluster compliance.TestCluster) (kvstore.Interface, error) {
	c, ok := cluster.(*integration.ClusterV3)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	s := runtime.NewScheme()
	s.AddKnownTypes(&compliance.TestObj{}, &compliance.TestObjList{})
	store, _ := newEtcdStore(c.Client(), runtime.NewJSONCodec(s))

	return store, nil
}

// Run standard compliance tests
func TestRunAll(t *testing.T) {
	compliance.RunInterfaceTests(t, clusterSetup, storeSetup, clusterCleanup)
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T) {
	c := clusterSetup(t)
	store, _ := storeSetup(t, c)
	defer clusterCleanup(t, c)

	cluster, _ := c.(*integration.ClusterV3)

	obj := &compliance.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(context.Background(), compliance.TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(context.Background(), compliance.TestKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	client := cluster.Client()
	etcdWatcher := client.Watch(context.Background(), compliance.TestKey)

	if err = store.Delete(context.Background(), compliance.TestKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.Atoi(ev.Object.(*compliance.TestObj).ResourceVersion)
	if err != nil {
		t.Fatalf("Failed to parse watch version, error: %v", err)
	}

	res := <-etcdWatcher

	if res.Events[0].Kv.ModRevision != int64(watchVer) {
		t.Fatalf("Version mismatch, expected %v, got %v", res.Events[0].Kv.ModRevision, int64(watchVer))
	}

	w.Stop()
	t.Logf("Got expected version from watch")
}
