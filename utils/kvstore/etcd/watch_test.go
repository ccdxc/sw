package etcd

import (
	"context"
	"fmt"
	"reflect"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

const (
	testDir = "/registry"
)

var (
	watchTimeout = time.Second * 1
)

type expectedObj struct {
	testObj TestObj
	evType  kvstore.EventType
}

// TestWatch tests the watch for Created, Updated and Deleted events on an object.
func TestWatch(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	w, err := store.Watch(testKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	if err = store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	expectEvent(t, evCh, kvstore.Created, obj)

	obj.Counter = 1
	if err := store.Update(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	expectEvent(t, evCh, kvstore.Updated, obj)

	if err := store.Delete(testKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	expectEvent(t, evCh, kvstore.Deleted, obj)
	w.Stop()

	t.Logf("Watch of Created, Updated, Deleted events succeeded")
}

// TestPrefixWatch tests the watch events on a prefix.
func TestPrefixWatch(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	w, err := store.PrefixWatch(testDir, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	keys := []string{"/key1", "/key2"}
	for ii, key := range keys {
		obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("testObj%d", ii)}, Counter: 0}

		if err = store.Create(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}

		expectEvent(t, evCh, kvstore.Created, obj)

		obj.Counter = 1
		if err := store.Update(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectEvent(t, evCh, kvstore.Updated, obj)

		if err := store.Delete(testDir+key, obj); err != nil {
			t.Fatalf("Delete failed with error: %v", err)
		}

		expectEvent(t, evCh, kvstore.Deleted, obj)
	}

	w.Stop()
	t.Logf("PrefixWatch of Created, Updated, Deleted events succeeded")
}

// TestWatchExisting tests that a watch established with version "0" finds an
// existing object.
func TestWatchExisting(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	// Case 1 - created but not updated.
	if err := store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	w, err := store.Watch(testKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectEvent(t, w.EventChan(), kvstore.Created, obj)
	w.Stop()

	// Case 2 - created and updated.
	obj.Counter = 1
	if err := store.Update(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	w, err = store.Watch(testKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectEvent(t, w.EventChan(), kvstore.Updated, obj)
	w.Stop()

	t.Logf("Watch of an existing object succeeded")
}

// TestWatchFromVersion tests that a watch established with specified version
// only returns the changes from that version.
func TestWatchFromVersion(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion
	obj.Counter = 1
	if err := store.Update(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	w, err := store.Watch(testKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectEvent(t, w.EventChan(), kvstore.Updated, obj)
	w.Stop()

	t.Logf("Watch from a specified version succeeded")
}

// TestBufferedWatch tests that the watch events are buffered.
func TestBufferedWatch(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	w, err := store.PrefixWatch(testDir, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectedObjs := []expectedObj{}
	for ii := 0; ii < 10; ii++ {
		obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("testObj%d", ii)}, Counter: 0}
		key := fmt.Sprintf("/key%d", ii)

		if err = store.Create(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: kvstore.Created})

		obj.Counter = 1
		if err := store.Update(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: kvstore.Updated})

		if err := store.Delete(testDir+key, obj); err != nil {
			t.Fatalf("Delete failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: kvstore.Deleted})
	}

	for _, expObj := range expectedObjs {
		expectEvent(t, w.EventChan(), expObj.evType, &expObj.testObj)
	}
	w.Stop()
	t.Logf("Buffered Watch succeeded")
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(testKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	etcdWatcher := cluster.NewClient(t).Watch(context.Background(), testKey)

	if err := store.Delete(testKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.Atoi(ev.Object.(*TestObj).ResourceVersion)
	if err != nil {
		t.Fatalf("Failed to parse watch version, error: %v", err)
	}

	res := <-etcdWatcher

	if res.Events[0].Kv.ModRevision != int64(watchVer) {
		t.Fatalf("Version mismatch, expected %v, got %v", res.Events[0].Kv.ModRevision, int64(watchVer))
	}
	t.Logf("Got expected version from watch")
}

func expectEvent(t *testing.T, ch <-chan *kvstore.Event, evType kvstore.EventType, obj runtime.Object) {
	select {
	case ev := <-ch:
		if ev.Type != evType {
			t.Fatalf("Expected event %v, got %v", evType, ev.Type)
		}

		if evType == kvstore.Deleted {
			// Delete returns the last modified version of the object,
			// while Watch returns the version at the time of deletion.
			obj.(*TestObj).ResourceVersion = ev.Object.(*TestObj).ResourceVersion
		}

		if !reflect.DeepEqual(ev.Object, obj) {
			t.Fatalf("Expected obj %+v, got %+v", obj, ev.Object)
		}
	case <-time.After(watchTimeout):
		t.Fatalf("Timed out waiting for watch event")
	}
}
