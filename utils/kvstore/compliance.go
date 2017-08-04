package kvstore

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/runtime"
)

// kvstore compliance
// A compliant kvstore must pass all the tests in this file in order to be used in
// rest of the code. The test cases are meant to test semantic nature of the APIs
//

// exported constants
const (
	TestKey     = "/nbv"
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

// TestObj is test object
type TestObj struct {
	api.TypeMeta
	api.ObjectMeta
	Counter int
}

// TestObjList is test list object
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

// RunInterfaceTests runs interface tests
func RunInterfaceTests(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	fns := []func(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc){
		TestCreate,
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
		// TestWatchVersion,   this test is state store specific
		TestCancelWatch,
		TestElection,
		TestElectionRestartContender,
		TestCancelElection,
		TestTxn,
		TestMultipleElection,
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

// TestCreate tests create
func TestCreate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	if len(obj.ResourceVersion) == 0 || obj.ResourceVersion == "0" {
		t.Fatalf("Create returned with resource version 0")
	}

	err = store.Get(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Failed Get after Create with error: %v", err)
	}

	t.Logf("Create succeeded with version: %s", obj.ResourceVersion)
}

// TestDuplicateCreate tests duplicate creates
func TestDuplicateCreate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	err = store.Create(context.Background(), TestKey, obj)
	if err == nil || !IsKeyExistsError(err) {
		t.Fatalf("Failed to detect duplicate create: %v", err)
	}

	t.Logf("Duplicate create was detected")
}

// TestDelete tests deletes
func TestDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	err = store.Delete(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Delete failed with error: %v", err)
	}

	if prevVersion != obj.ResourceVersion {
		t.Fatalf("Delete failed, expected version %v, got %v", prevVersion, obj.ResourceVersion)
	}

	err = store.Get(context.Background(), TestKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("After Delete, key is possibly present, error: %v", err)
	}

	t.Logf("Delete of a key succeeded")
}

// TestNonExistentDelete tests non-existent deletes
func TestNonExistentDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Delete(context.Background(), TestKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("Delete failed with error: %v", err)
	}

	t.Logf("Delete of a non existent key failed as expected")
}

// TestAtomicDelete tests atomic deletes
func TestAtomicDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	if err = store.Delete(context.Background(), TestKey, nil, Compare(WithVersion(TestKey), "=", 0)); err == nil {
		t.Fatalf("AtomicDelete failed with incorrect previous version")
	}

	prevVersion := obj.ResourceVersion

	err = store.Delete(context.Background(), TestKey, obj, Compare(WithVersion(TestKey), "=", prevVersion))
	if err != nil {
		t.Fatalf("AtomicDelete failed with error: %v", err)
	}

	if prevVersion != obj.ResourceVersion {
		t.Fatalf("AtomicDelete failed, expected version %v, got %v", prevVersion, obj.ResourceVersion)
	}

	t.Logf("AtomicDelete of a key succeeded")
}

// TestPrefixDelete tests prefix deletes
func TestPrefixDelete(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	testKeys := []string{"/abc/123", "/abc/456", "/abcd"}
	expDelKeys := []string{"/abc/123", "/abc/456"}
	for _, key := range testKeys {
		err := store.Create(context.Background(), key, obj)
		if err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}
	}

	if err := store.PrefixDelete(context.Background(), "/abc"); err != nil {
		t.Fatalf("PrefixDelete failed with error: %v", err)
	}

	// Check that keys that need to be deleted are deleted.
	for _, key := range expDelKeys {
		if err := store.Get(context.Background(), key, nil); err == nil || !IsKeyNotFoundError(err) {
			t.Fatalf("PrefixDelete failed to delete key: %v", err)
		}
	}

	// Check that the remaining key is not deleted.
	if err := store.Get(context.Background(), "/abcd", obj); err != nil {
		t.Fatalf("PrefixDelete failed with error: %v", err)
	}

	t.Logf("PrefixDelete of a key succeeded")
}

// TestUpdate tests updates
func TestUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	if err := store.Update(context.Background(), TestKey, obj); err == nil {
		t.Fatalf("Update of a non existent key passed")
	}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	if err := store.Update(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Update of a key failed")
	}

	if obj.ResourceVersion == prevVersion {
		t.Fatalf("Update of a key without version change")
	}

	t.Logf("Update of a key succeeded")
}

// TestNonExistentUpdate tests non-existent updates
func TestNonExistentUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Update(context.Background(), TestKey, obj)
	if err == nil || !IsKeyNotFoundError(err) {
		t.Fatalf("Update failed with error: %v", err)
	}

	t.Logf("Update of a non existent object failed as expected")
}

// TestAtomicUpdate tests atomic updates
func TestAtomicUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	prevVersion := obj.ResourceVersion

	if err := store.Update(context.Background(), TestKey, obj, Compare(WithVersion(TestKey), "=", 0)); err == nil {
		t.Fatalf("AtomicUpdate passed with incorrect previous version")
	}

	if err := store.Update(context.Background(), TestKey, obj, Compare(WithVersion(TestKey), "=", prevVersion)); err != nil {
		t.Fatalf("AtomicUpdate of a key failed with error: %v", err)
	}

	t.Logf("AtomicUpdate of a key succeeded")
}

// TestConsistentUpdate tests consistent updates
func TestConsistentUpdate(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}
	updateFunc := func(obj runtime.Object) (runtime.Object, error) {
		obj.(*TestObj).Counter++
		return obj, nil
	}

	if err := store.ConsistentUpdate(context.Background(), TestKey, nil, updateFunc); err == nil {
		t.Fatalf("ConsistentUpdate passed when key doesn't exist")
	}

	err := store.Create(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	numUpdates := 10
	ch := make(chan bool, numUpdates)
	for ii := 0; ii < numUpdates; ii++ {
		go func() {
			if err2 := store.ConsistentUpdate(context.Background(), TestKey, &TestObj{}, updateFunc); err2 != nil {
				t.Fatalf("ConsistentUpdate of a key failed with error: %v", err2)
			}
			ch <- true
		}()
	}

	for ii := 0; ii < numUpdates; ii++ {
		select {
		case <-ch:
		}
	}

	err = store.Get(context.Background(), TestKey, obj)
	if err != nil {
		t.Fatalf("Get of the key failed with error: %v", err)
	}

	if obj.Counter != numUpdates {
		t.Fatalf("ConsistentUpdate failed to increment counter correctly, expected %v, got %v", numUpdates, obj.Counter)
	}

	t.Logf("ConsistentUpdate of a key succeeded")
}

// TestList tests lists
func TestList(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	keys := []string{"testObj1", "testObj2"}
	for ii := range keys {
		obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: keys[ii]}}
		err := store.Create(context.Background(), TestKey+"/"+keys[ii], obj)
		if err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}
	}

	// This object should not be returned with List.
	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "abc"}}
	err := store.Create(context.Background(), TestKey+"abc", obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	var listObj1 TestObjList
	err = store.List(context.Background(), TestKey, &listObj1)
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
		err = store.Delete(context.Background(), TestKey+"/"+keys[ii], nil)
		if err != nil {
			t.Fatalf("Failed to delete key %v, err %v", keys[ii], err)
		}
	}

	var listObj2 TestObjList
	err = store.List(context.Background(), TestKey, &listObj2)
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

	w, err := store.Watch(context.Background(), TestKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	if err = store.Create(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	expectWatchEvent(t, evCh, Created, obj)

	obj.Counter = 1
	if err := store.Update(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	expectWatchEvent(t, evCh, Updated, obj)

	if err := store.Delete(context.Background(), TestKey, obj); err != nil {
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

	w, err := store.PrefixWatch(context.Background(), testDir, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	keys := []string{"/key1", "/key2"}
	for ii, key := range keys {
		obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("testObj%d", ii)}, Counter: 0}

		if err = store.Create(context.Background(), testDir+key, obj); err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}

		expectWatchEvent(t, evCh, Created, obj)

		obj.Counter = 1
		if err := store.Update(context.Background(), testDir+key, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectWatchEvent(t, evCh, Updated, obj)

		if err := store.Delete(context.Background(), testDir+key, obj); err != nil {
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
	if err := store.Create(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	w, err := store.Watch(context.Background(), TestKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectWatchEvent(t, w.EventChan(), Created, obj)
	w.Stop()

	// Case 2 - created and updated.
	obj.Counter = 1
	if err = store.Update(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	w, err = store.Watch(context.Background(), TestKey, "0")
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

	if err := store.Create(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	fromVersion := obj.ResourceVersion
	obj.Counter = 1
	if err := store.Update(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Update failed with error: %v", err)
	}

	w, err := store.Watch(context.Background(), TestKey, fromVersion)
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

	w, err := store.PrefixWatch(context.Background(), testDir, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}

	expectedObjs := []expectedObj{}
	for ii := 0; ii < 10; ii++ {
		obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("testObj%d", ii)}, Counter: 0}
		key := fmt.Sprintf("/key%d", ii)

		if err = store.Create(context.Background(), testDir+key, obj); err != nil {
			t.Fatalf("Create failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: Created})

		obj.Counter = 1
		if err := store.Update(context.Background(), testDir+key, obj); err != nil {
			t.Fatalf("Update failed with error: %v", err)
		}

		expectedObjs = append(expectedObjs, expectedObj{testObj: *obj, evType: Updated})

		if err := store.Delete(context.Background(), testDir+key, obj); err != nil {
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

// TestCancelWatch tests that watch stops on cancelling the provided context.
func TestCancelWatch(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, cluster)

	ctx, cancel := context.WithCancel(context.Background())

	w, err := store.Watch(ctx, TestKey, "0")
	if err != nil {
		t.Fatalf("Watch failed with error: %v", err)
	}
	evCh := w.EventChan()

	// If cancelled immediately, watch goroutine will send an error event.
	time.Sleep(time.Millisecond * 100)

	cancel()

	time.Sleep(time.Millisecond * 100)

	obj := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj"}, Counter: 0}

	if err := store.Create(context.Background(), TestKey, obj); err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	select {
	case ev, ok := <-evCh:
		if ok {
			t.Fatalf("Received event on cancelled watch: %+v\n", ev)
		}
	default:
	}
	t.Logf("Cancel watch succeeded")
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
func newContest(ctx context.Context, t *testing.T, store Interface, id string, ttl uint64) Election {
	election, err := store.Contest(ctx, contestName, id, ttl)
	if err != nil {
		t.Fatalf("Contest creation for %v failed with error: %v", id, err)
	}
	t.Logf("Starting election with %v", id)
	return election
}

// addCandidates creates the specified number of candidates.
func addCandidates(ctx context.Context, t *testing.T, sSetup StoreSetupFunc, cluster TestCluster, numCandidates, startID int) []Election {
	contenders := []Election{}
	for ii := 0; ii < numCandidates; ii++ {
		store, err := sSetup(t, cluster)
		if err != nil {
			t.Fatalf("Store creation failed with error: %v", err)
		}
		contenders = append(contenders, newContest(ctx, t, store, fmt.Sprintf("contender-%d", ii+1+startID), minTTL))
	}
	return contenders
}

// setupContest sets up the asked number of candidates.
func setupContest(ctx context.Context, t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, numCandidates int) (TestCluster, []Election) {
	cluster := cSetup(t)
	return cluster, addCandidates(ctx, t, sSetup, cluster, numCandidates, 0)
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

			if strings.Split(e.Leader, "-")[0] != strings.Split(contender.ID(), "-")[0] {
				t.Fatalf("Contender %v got event for some other election with leader %v", contender.ID(), e.Leader)
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
	cluster, contenders := setupContest(context.Background(), t, cSetup, sSetup, 3)
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

	contenders = append(contenders, newContest(context.Background(), t, store, newID, minTTL))

	if leader != contenders[0].Leader() {
		t.Fatalf("Leader changed to %v, expecting %v", contenders[0].Leader(), leader)
	}

	// Clean up
	for _, contender := range contenders {
		contender.Stop()
	}
}

// TestElectionRestartContender checks the following.
// 1) One contender wins an election (among 3).
// 2) Start another contender with the same ID.
// 3) Check that the same ID wins the election.
func TestElectionRestartContender(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	cluster, contenders := setupContest(context.Background(), t, cSetup, sSetup, 3)
	defer cCleanup(t, cluster)

	checkElectionEvents(t, contenders, true)

	t.Logf("Adding another contender with same id %v", contenders[0].Leader())

	store, err := sSetup(t, cluster)
	if err != nil {
		t.Fatalf("Failed to create store with error: %v", err)
	}

	contender := newContest(context.Background(), t, store, contenders[0].Leader(), minTTL)

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
func TestCancelElection(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	t.Logf("Starting CancelElection")
	ctx, cancel := context.WithCancel(context.Background())
	cluster, contenders := setupContest(ctx, t, cSetup, sSetup, 1)
	defer cCleanup(t, cluster)

	checkElectionEvents(t, contenders, true)

	// Add two more candidates.
	newContenders := addCandidates(context.Background(), t, sSetup, cluster, 2, 1)

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

// TestMultipleElection checks that multiple  elections can run with the same KvStore and they dont interfere with one another
func TestMultipleElection(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {
	t.Logf("Starting MultipleElection")

	contenders1 := []Election{}
	contenders1Cancel := []context.CancelFunc{}
	contenders2 := []Election{}
	contenders2Cancel := []context.CancelFunc{}

	cluster := cSetup(t)
	// every 2 contenders share a KV.
	// 2 elections - each with 3 contenders.
	for ii := 1; ii <= 3; ii++ {
		store, err := sSetup(t, cluster)
		if err != nil {
			t.Fatalf("Store creation failed with error: %v", err)
		}

		ctx, cancel := context.WithCancel(context.Background())
		contenders1Cancel = append(contenders1Cancel, cancel)

		election, err := store.Contest(ctx, "contest1", fmt.Sprintf("contender1-%d", ii), minTTL)
		if err != nil {
			t.Fatalf("Store Contest failed with error: %v", err)
		}
		contenders1 = append(contenders1, election)

		ctx, cancel = context.WithCancel(context.Background())
		contenders2Cancel = append(contenders2Cancel, cancel)

		election, err = store.Contest(ctx, "contest2", fmt.Sprintf("contender2-%d", ii), minTTL)
		if err != nil {
			t.Fatalf("Store Contest failed with error: %v", err)
		}
		contenders2 = append(contenders2, election)
	}

	defer cCleanup(t, cluster)

	checkElectionEvents(t, contenders1, true)
	checkElectionEvents(t, contenders2, true)

	// Disrupt the leader from first election and make sure the leader of second election is still ok
	// and not unhappy with disruption
	var secondElectionLeader Election
	for _, v := range contenders2 {
		if v.IsLeader() {
			secondElectionLeader = v
			break
		}
	}
	for k, v := range contenders1 {
		if v.IsLeader() {
			contenders1Cancel[k]()
			break
		}
	}
	time.Sleep(100 * time.Millisecond)
	if !secondElectionLeader.IsLeader() {
		t.Fatalf("Leader of second election %v is not leader after disruption of first election", secondElectionLeader)
	}

	// Clean up
	for _, contender := range contenders1 {
		contender.Stop()
	}
	for _, contender := range contenders2 {
		contender.Stop()
	}

	t.Logf("test of multiple election succeeded")
}

// TestTxn tests creation/deletion/updation of keys in a transanction.
func TestTxn(t *testing.T, cSetup ClusterSetupFunc, sSetup StoreSetupFunc, cCleanup ClusterCleanupFunc) {

	c, store := setupTestCluster(t, cSetup, sSetup)
	defer cCleanup(t, c)

	obj1 := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj1"}}
	obj2 := &TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj2"}}

	txn1 := store.NewTxn()
	if err := txn1.Create(obj1.Name, obj1); err != nil {
		t.Fatalf("Failed to create obj1 in txn with error: %v", err)
	}
	if err := txn1.Create(obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to create obj1 in with error: %v", err)
	}
	if _, err := txn1.Commit(context.Background()); err != nil {
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
	if err := txn2.Update(obj1.Name, obj1, Compare(WithVersion(obj1.Name), "=", obj1.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj1 in with error: %v", err)
	}
	if err := txn2.Update(obj2.Name, obj2, Compare(WithVersion(obj2.Name), "=", obj2.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj2 in with error: %v", err)
	}
	if _, err := txn2.Commit(context.Background()); err != nil {
		t.Fatalf("Failed to commit txn with multiple Updates with error: %v", err)
	} else if obj1.ResourceVersion == oldVersion {
		t.Fatalf("Failed to update version in txn")
	}

	txn3 := store.NewTxn()
	if err := store.Get(context.Background(), obj2.Name, obj2); err != nil {
		t.Fatalf("Failed to get obj2 created in with error: %v", err)
	}
	oldVersion = obj2.ResourceVersion
	if err := txn3.Delete(obj1.Name, Compare(WithVersion(obj1.Name), "=", obj1.ResourceVersion)); err != nil {
		t.Fatalf("Failed to delete obj1 in with error: %v", err)
	}
	if err := txn3.Update(obj2.Name, obj2, Compare(WithVersion(obj2.Name), "=", obj2.ResourceVersion)); err != nil {
		t.Fatalf("Failed to update obj2 in with error: %v", err)
	}
	if resp, err := txn3.Commit(context.Background()); err != nil {
		t.Fatalf("Failed to commit txn with Update+Delete with error: %v", err)
	} else {
		if resp.Succeeded != true {
			t.Fatalf("Transaction failed")
		}
		found := false
		for _, r := range resp.Responses {
			if r.Oper == OperDelete {
				found = true
				robj, ok := r.Obj.(*TestObj)
				if !ok {
					t.Fatalf("Retrieved Object is not right kind")
				}
				robj.ResourceVersion = obj1.ResourceVersion
				if !reflect.DeepEqual(obj1, r.Obj) {
					t.Fatalf("Retrieved object is not the same [%+v] [%+v]", obj1, r.Obj)
				}
			}
		}
		if !found {
			t.Fatalf("Response for delete not found")
		}
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
