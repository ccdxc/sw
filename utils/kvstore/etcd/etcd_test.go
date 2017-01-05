package etcd

import (
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/utils/runtime"
)

const (
	testKey = "/nbv"
)

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

func setupTestCluster(t *testing.T) (*integration.ClusterV3, kvstore.Interface) {
	cluster := integration.NewClusterV3(t)
	s := runtime.NewScheme()
	s.AddKnownTypes(&TestObj{}, &TestObjList{})
	store, _ := newEtcdStore(cluster.Client(), runtime.NewJSONCodec(s))
	return cluster, store
}

func TestCreate(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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

func TestCreateWithTTL(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 1, obj)
	if err != nil {
		t.Fatalf("Create with TTL failed with error: %v", err)
	}

	time.Sleep(time.Second * 2)

	err = store.Get(testKey, obj)
	if err == nil || !kvstore.IsKeyNotFoundError(err) {
		t.Fatalf("TTL key failed to expire")
	}

	t.Logf("Create with TTL succeeded with version: %s", obj.ResourceVersion)
}

func TestDuplicateCreate(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Create(testKey, obj, 0, obj)
	if err != nil {
		t.Fatalf("Create failed with error: %v", err)
	}

	err = store.Create(testKey, obj, 0, obj)
	if err == nil || !kvstore.IsKeyExistsError(err) {
		t.Fatalf("Failed to detect duplicate create: %v", err)
	}

	t.Logf("Duplicate create was detected")
}

func TestDelete(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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
	if err == nil || !kvstore.IsKeyNotFoundError(err) {
		t.Fatalf("After Delete, key is possibly present, error: %v", err)
	}

	t.Logf("Delete of a key succeeded")
}

func TestNonExistentDelete(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

	obj := &TestObj{ObjectMeta: api.ObjectMeta{Name: "testObj"}}

	err := store.Delete(testKey, obj)
	if err == nil || !kvstore.IsKeyNotFoundError(err) {
		t.Fatalf("Delete failed with error: %v", err)
	}

	t.Logf("Delete of a non existent key failed as expected")
}

func TestAtomicDelete(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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

func TestPrefixDelete(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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
		if err := store.Get(key, nil); err == nil || !kvstore.IsKeyNotFoundError(err) {
			t.Fatalf("PrefixDelete failed to delete key: %v", err)
		}
	}

	// Check that the remaining key is not deleted.
	if err := store.Get("/abcd", obj); err != nil {
		t.Fatalf("PrefixDelete failed with error: %v", err)
	}

	t.Logf("PrefixDelete of a key succeeded")
}

func TestUpdate(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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

func TestAtomicUpdate(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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
		t.Fatalf("AtomicUpdate of a key failed")
	}

	t.Logf("AtomicUpdate of a key succeeded")
}

func TestConsistentUpdate(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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

func TestList(t *testing.T) {
	cluster, store := setupTestCluster(t)
	defer cluster.Terminate(t)

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
