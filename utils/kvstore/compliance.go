package kvstore

import (
	// "context"
	"fmt"
	"reflect"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/runtime"
)

// kvstore compliance
// A compliant kvstore must pass all the tests in this file in order to be used in
// rest of the code. The test cases are meant to test semantic nature of the APIs
//

const (
	testKey     = "/nbv"
	testDir     = "/registry"
	contestName = "master"
	minTTL      = 5
)

var (
	watchTimeout = time.Second * 1
)

type expectedObj struct {
	testObj TestObj
	evType  WatchEventType
}

type TestObj struct {
	api.TypeMeta
	api.ObjectMeta
	Counter int
}

type TestObjList struct {
	api.TypeMeta
	api.ListMeta
	Items []TestObj
}

// TestCluster is an abstraction of a distributed state store
type TestCluster interface{}

// ClusterSetupFunc sets up a cluster i.e. initializes it
type ClusterSetupFunc func(t *testing.T) TestCluster

// StoreSetupFunc crates a state store client within the specified cluster
type StoreSetupFunc func(t *testing.T, cluster TestCluster) (Interface, error)

// ClusterCleanupFunc cleans up (terminate, destroy objects, etc.) the specified cluster
type ClusterCleanupFunc func(t *testing.T, cluster TestCluster)

func RunInterfaceTests(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	fns := []func(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc){
		TestCreate,
		TestCreateWithTTL,
		TestCreateWithInvalidTTL,
		TestDuplicateCreate,
		TestDelete,
		TestNonExistentDelete,
		TestAtomicDelete,
		TestPrefixDelete,
		TestUpdate,
		TestNonExistentUpdate,
		TestAtomicUpdate,
		TestConsistentUpdate,
		TestList,
		TestWatch,
		TestPrefixWatch,
		TestWatchExisting,
		TestWatchFromVersion,
		TestBufferedWatch,
		TestWatchVersion,
		TestElection,
		TestElectionRestartContender,
	}

	for _, fn := range fns {
		fn(t, cSetup, sSetup, cCleanup)
	}
}

func setupTestCluster(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc) (TestCluster, Interface) {
	cluster := cSetup(t)
	store, _ := sSetup(t, cluster)
	return cluster, store
}

func TestCreate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	if len(obj.ResourceVersion) == 0 || obj.ResourceVersion == "0" {
		t.Fatalf("Create returned with resource version 0")
	}

	err = store.Get(testKey, obj)
	if err != nil {
		t.Fatalf("Failed Get after Create with error: %v", err)
	}

	t.Logf("Create succeeded with version: %s", obj.ResourceVersion)
}

func TestCreateWithTTL(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 1, obj)
	if err != nil {
		t.Fatalf("Create with TTL failed with error: %v", err)
	}

	time.Sleep(time.Second * 2)

	err = store.Get(testKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("TTL key failed to expire")
	}

	t.Logf("Create with TTL succeeded with version: %s", obj.ResourceVersion)
}

func TestCreateWithInvalidTTL(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, -1, obj)
	if err == nil {
		t.Fatalf("Create with Invalid TTL succeeded error: %v", err)
	}

	err = store.Get(testKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("object found with the key")
	}

	t.Logf("Create with Invalid TTL succeeded")
}

func TestDuplicateCreate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	err = store.Create(testKey, obj, 0, obj)
	if err == nil || !IsKeyExistsError(err) {
		t.Fatalf("Failed to detect duplicate create: %v", err)
	}

	t.Logf("Duplicate create was detected")
}

func TestDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	err = store.Delete(testKey, obj)
	if err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	if prevVersion != obj.ResourceVersion {
		t.Fatalf("Delete failed, expected version %v, got %v", prevVersion, obj.ResourceVersion)
	}

	err = store.Get(testKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("After Delete, key is possibly present, error: %v", err)
	}

	t.Logf("Delete of a key succeeded")
}

func TestNonExistentDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Delete(testKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("Delete failed with error: %v", err)
	}

	t.Logf("Delete of a non existent key failed as expected")
}

func TestAtomicDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	if err := store.AtomicDelete(testKey, "0", nil); err == nil {
		t.Fatalf("AtomicDelete failed with incorrect previous version")
	}

	prevVersion := obj.ResourceVersion

	err = store.AtomicDelete(testKey, prevVersion, obj)
	if err != nil {
		t.Fatalf("AtomicDelete failed with error: %v", err)
	}

	if prevVersion != obj.ResourceVersion {
		t.Fatalf("AtomicDelete failed, expected version %v, got %v", prevVersion, obj.ResourceVersion)
	}

	t.Logf("AtomicDelete of a key succeeded")
}

func TestPrefixDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	testKeys := []string{"/abc/123", "/abc/456", "/abcd"}
	expDelKeys := []string{"/abc/123", "/abc/456"}
	for _, key := range testKeys {
		err := store.Create(key, obj, 0, nil)
		if err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}
	}

	if err := store.PrefixDelete("/abc"); err != nil {
		t.Fatalf("PrefixDelete failed with error: %v", err)
	}

	// Check that keys that need to be deleted are deleted.
	for _, key := range expDelKeys {
		if err := store.Get(key, nil); err == nil || !IsKeyNotFoundError(err) {
			t.Fatalf("PrefixDelete failed to delete key: %v", err)
		}
	}

	// Check that the remaining key is not deleted.
	if err := store.Get("/abcd", obj); err != nil {
		t.Fatalf("PrefixDelete failed with error: %v", err)
	}

	t.Logf("PrefixDelete of a key succeeded")
}

func TestUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	if err := store.Update(testKey, obj, 0, obj); err == nil {
		t.Fatalf("Update of a non existent key passed")
	}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	if err := store.Update(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Update of a key failed")
	}

	if obj.ResourceVersion == prevVersion {
		t.Fatalf("Update of a key without version change")
	}

	t.Logf("Update of a key succeeded")
}

func TestNonExistentUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Update(testKey, obj, 0, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("Update failed with error: %v", err)
	}

	t.Logf("Update of a non existent object failed as expected")
}

func TestAtomicUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	if err := store.AtomicUpdate(testKey, obj, "0", 0, obj); err == nil {
		t.Fatalf("AtomicUpdate passed with incorrect previous version")
	}

	if err := store.AtomicUpdate(testKey, obj, prevVersion, 0, obj); err != nil {
		t.Fatalf("AtomicUpdate of a key failed with error: %v", err)
	}

	t.Logf("AtomicUpdate of a key succeeded")
}

func TestConsistentUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}
	updateFunc := func(obj runtime.Object) (runtime.Object, error) {
		obj.(*TestObj).Counter++
		return obj, nil
	}

	if err := store.ConsistentUpdate(testKey, 0, nil, updateFunc); err == nil {
		t.Fatalf("ConsistentUpdate passed when key doesn't exist")
	}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	numUpdates := 10
	ch := make(chan bool, numUpdates)
	for ii := 0; ii < numUpdates; ii++ {
		go func() {
			if err := store.ConsistentUpdate(testKey, 0, &TestObj{}, updateFunc); err != nil {
				t.Fatalf("ConsistentUpdate of a key failed with error: %v", err)
			}
			ch <- true
		}()
	}

	for ii := 0; ii < numUpdates; ii++ {
		select {
		case <-ch:
		}
	}

	err = store.Get(testKey, obj)
	if err != nil {
		t.Fatalf("Get of the key failed with error: %v", err)
	}

	if obj.Counter != numUpdates {
		t.Fatalf("ConsistentUpdate failed to increment counter correctly, expected %v, got %v", numUpdates, obj.Counter)
	}

	t.Logf("ConsistentUpdate of a key succeeded")
}

func TestList(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	keys := []string{"testObj1", "testObj2"}
	for ii := range keys {
		obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: keys[ii]}}
		err := store.Create(testKey+"/"+keys[ii], obj, 0, obj)
		if err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}
	}

	// This object should not be returned with List.
	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "abc"}}
	err := store.Create(testKey+"abc", obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	var listObj1 TestObjList
	err = store.List(testKey, &listObj1)
	if err != nil {
		t.Fatalf("List failed with error: %v", err)
	}

	if len(listObj1.Items) != len(keys) {
		t.Fatalf("List returned invalid number of entries, expected %v, got %v", len(keys), len(listObj1.Items))
	}

	m := make(map[string]struct{})
	for ii := range listObj1.Items {
		key := listObj1.Items[ii].Name
		_, ok := m[key]
		if ok {
			t.Fatalf("List found duplicate entries for %v", key)
		}
		m[key] = struct{}{}
	}

	for ii := range keys {
		err := store.Delete(testKey+"/"+keys[ii], nil)
		if err != nil {
			t.Fatalf("Failed to delete key %v, err %v", keys[ii], err)
		}
	}

	var listObj2 TestObjList
	err = store.List(testKey, &listObj2)
	if err != nil {
		t.Fatalf("List failed with error: %v", err)
	}

	if len(listObj2.Items) != 0 {
		t.Fatalf("List returned invalid number of entries, expected 0, got %v", len(listObj2.Items))
	}

	t.Logf("List succeeded")
}

// TestWatch tests the watch for Created, Updated and Deleted events on an object.
func TestWatch(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	w, err := store.Watch(testKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	if err = store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	expectWatchEvent(t, evCh, Created, obj)

	obj.Counter = 1
	if err := store.Update(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	expectWatchEvent(t, evCh, Updated, obj)

	if err := store.Delete(testKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	expectWatchEvent(t, evCh, Deleted, obj)
	w.Stop()

	t.Logf("Watch of Created, Updated, Deleted events succeeded")
}

// TestPrefixWatch tests the watch events on a prefix.
func TestPrefixWatch(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

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

		expectWatchEvent(t, evCh, Created, obj)

		obj.Counter = 1
		if err := store.Update(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectWatchEvent(t, evCh, Updated, obj)

		if err := store.Delete(testDir+key, obj); err != nil {
			t.Fatalf("Delete failed with error: %v", err)
		}

		expectWatchEvent(t, evCh, Deleted, obj)
	}

	w.Stop()
	t.Logf("PrefixWatch of Created, Updated, Deleted events succeeded")
}

// TestWatchExisting tests that a watch established with version "0" finds an
// existing object.
func TestWatchExisting(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	// Case 1 - created but not updated.
	if err := store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	w, err := store.Watch(testKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectWatchEvent(t, w.EventChan(), Created, obj)
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

	expectWatchEvent(t, w.EventChan(), Updated, obj)
	w.Stop()

	t.Logf("Watch of an existing object succeeded")
}

// TestWatchFromVersion tests that a watch established with specified version
// only returns the changes from that version.
func TestWatchFromVersion(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

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

	expectWatchEvent(t, w.EventChan(), Updated, obj)
	w.Stop()

	t.Logf("Watch from a specified version succeeded")
}

// TestBufferedWatch tests that the watch events are buffered.
func TestBufferedWatch(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

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

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: Created})

		obj.Counter = 1
		if err := store.Update(testDir+key, obj, 0, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: Updated})

		if err := store.Delete(testDir+key, obj); err != nil {
			t.Fatalf("Delete failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: Deleted})
	}

	for _, expObj := range expectedObjs {
		expectWatchEvent(t, w.EventChan(), expObj.evType, &expObj.testObj)
	}
	w.Stop()
	t.Logf("Buffered Watch succeeded")
}

// TestWatchVersion tests that watch returns the same version as the underlying store.
func TestWatchVersion(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(testKey, obj, 0, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion

	w, err := store.Watch(testKey, fromVersion)
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	// etcdWatcher := cluster.NewClient(t).Watch(context.Background(), testKey)
	into := &TestObj{}
	err = store.Get(testKey, into)
	if err != nil {
		t.Fatalf("Get of the key failed with error: %v", err)
	}

	if err := store.Delete(testKey, obj); err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	ev := <-w.EventChan()
	watchVer, err := strconv.ParseInt(ev.Object.(*TestObj).ResourceVersion, 10, 64)
	if err != nil {
		t.Fatalf("Failed to parse watch version, error: %v", err)
	}

	// res := <-etcdWatcher
	//
	// if res.Events[0].Kv.ModRevision != int64(watchVer) {
	//	t.Fatalf("Version mismatch, expected %v, got %v", res.Events[0].Kv.ModRevision, int64(watchVer))
	//}
	objMeta, _ := runtime.GetObjectMeta(into)
	objVersion, err := strconv.ParseInt(objMeta.ResourceVersion, 10, 64)
	if err != nil {
		t.Fatalf("Unable to parse the object's version")
	}
	if objVersion != watchVer {
		t.Fatalf("Version mismatch, expected %v, got %v", int64(objVersion), int64(watchVer))
	}
	// upto here

	t.Logf("Got expected version from watch")
}

func expectWatchEvent(t *testing.T, ch <-chan *WatchEvent, evType WatchEventType, obj runtime.Object) {
	select {
	case ev := <-ch:
		if ev.Type != evType {
			t.Fatalf("Expected event %v, got %v", evType, ev.Type)
		}

		if evType == Deleted {
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

// newContest creates a new contender.
func newContest(t *testing.T, store Interface, id string, ttl uint64) Election {
	election, err := store.Contest(contestName, id, ttl)
	if err != nil {
		t.Fatalf("Contest creation for %v failed with error: %v", id, err)
	}
	return election
}

// setupContest sets up the asked number of candidates.
func setupContest(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, numCandidates int) (TestCluster, []Election) {
	cluster := cSetup(t)
	contenders := []Election{}
	for ii := 0; ii < numCandidates; ii++ {
		store, err := sSetup(t, cluster)
		if err != nil {
			t.Fatalf("Store creation failed with error: %v", err)
		}
		contenders = append(contenders, newContest(t, store, fmt.Sprintf("contender-%d", ii+1), minTTL))
	}
	return cluster, contenders
}

// checkElectionEvents pulls one event out of each contender provided and checks that
// there is atmost one leader (if expLeader is set).
func checkElectionEvents(t *testing.T, contenders []Election, expLeader bool) {
	leaderCount := 0
	for _, contender := range contenders {
		select {
		case <-time.After(time.Second):
			t.Fatalf("Timed out waiting for event on leader %v", contender.ID())
		case e := <-contender.EventChan():
			t.Logf("Got event %+v for contender %v", e, contender.ID())
			if e.Leader == contender.ID() {
				if e.Type != Elected {
					t.Fatalf("Leader %v with non elected event %v", contender.ID(), e.Type)
				}
				leaderCount++
			} else if e.Type != Changed {
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
func TestElection(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, contenders := setupContest(t, cSetup, sSetup, 3)
	defer cCleanup(t, cluster)

	checkElectionEvents(t, contenders, true)

	newID := ""
	for ii, contender := range contenders {
		if contender.IsLeader() {
			newID = contender.ID()
			t.Logf("Stopping leader %v", newID)
			contender.Stop()
			contenders = append(contenders[:ii], contenders[ii+1:]...)
			// Leader event and changed event
			checkElectionEvents(t, contenders, true)
			break
		}
	}

	leader := contenders[0].Leader()
	t.Logf("Adding contender %v with existing leader %v", newID, leader)
	store, err := sSetup(t, cluster)
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contenders = append(contenders, newContest(t, store, newID, minTTL))

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
func TestElectionRestartContender(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, contenders := setupContest(t, cSetup, sSetup, 3)
	defer cCleanup(t, cluster)

	checkElectionEvents(t, contenders, true)

	t.Logf("Adding another contender with same id %v", contenders[0].Leader())

	store, err := sSetup(t, cluster)
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contender := newContest(t, store, contenders[0].Leader(), minTTL)

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
