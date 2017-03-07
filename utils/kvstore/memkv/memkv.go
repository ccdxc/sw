package memkv

// memkv uses memory as kv store,
// The goal is allow a emulate kvstore without starting an external process (e.g. etcd)
// to help with unit testing any function that depends on state store to validate its functionality
// The package can be used in multi-threaded binary, but not useful for cross nodes kv store
// 

import (
	"encoding/json"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/utils/kvstore/helper"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

type memKvRec struct {
	value		string
	ttl		int64
	revision	int64
	watchers	[]*watcher
}

// memKv is state store interface to a memkv client
type memKv struct {
	sync.Mutex
	cluster       interface{}
	deleted       bool
	codec         runtime.Codec
	objVersioner  runtime.Versioner
	listVersioner runtime.Versioner
	kvs           map[string]*memKvRec
	watchers      map[string]*watcher
}

// NewFakeStore creates a new in memory kv store
func NewMemKv(cluster interface{}, codec runtime.Codec) (kvstore.Interface, error) {
	kvs := make(map[string]*memKvRec)
	watchers := make(map[string]*watcher)
	fkv := &memKv{
		cluster:       cluster,
		deleted:       false,
		codec:         codec,
		objVersioner:  runtime.NewObjectVersioner(),
		listVersioner: runtime.NewListVersioner(),
		kvs:           kvs,
		watchers:      watchers,
	}
	go ttlDecrement(fkv)
	return fkv, nil
}

// ttlDecrement would sleep for a second and decrement ttl for a key in kvstore
// FIXME: this background task must be stopped, perhaps by having kvstore use the context
func ttlDecrement(f *memKv) {
	for ;; {
		time.Sleep(time.Second)
		f.Lock()
		if f.deleted {
			f.Unlock()
			break
		}
		for key, v := range f.kvs {
			if v.ttl == 0 {
				continue
			}
			v.ttl -= 1
			if v.ttl == 0 {
				delete(f.kvs, key)
			}
		}
		f.Unlock()
	}
}

func (f *memKv) deleteAll() {
	f.Lock()
	// delete all kv pairs
	for key, _ := range f.kvs {
		delete (f.kvs, key)
	}

	// delete all watchers
	for key, _ := range f.watchers {
		delete (f.watchers, key)
	}
	f.Unlock()
}

// encode implements the serialization of an object to be stored in memkv.
// TBD: this function is common among all state stores
func (f *memKv) encode(obj runtime.Object) ([]byte, error) {
	// If the object implements the json.Marshaler interface, use it.
	if m, ok := interface{}(obj).(json.Marshaler); ok {
		return m.MarshalJSON()
	}
	// Use the store encoder.
	return f.codec.Encode(obj)
}

// decode implements the de-serialization of an object stored in memkv.
// TBD: this function is common among all state stores
func (f *memKv) decode(value []byte, into runtime.Object, version int64) error {
	if err := helper.ValidObjForDecode(into); err != nil {
		return err
	}

	// If the object implements the json.Unmarshaler interface, use it.
	if m, ok := interface{}(into).(json.Unmarshaler); ok {
		if err := m.UnmarshalJSON(value); err != nil {
			return err
		}
	} else {
		// Use the store decoder.
		if _, err := f.codec.Decode(value, into); err != nil {
			return err
		}
	}

	return f.objVersioner.SetVersion(into, uint64(version))
}

// Create creates a key in memkv with the provided object and ttl. If ttl is 0, it means the key
// does not expire. If "into" is not nil, it is set to the value returned by the kv store.
func (f *memKv) Create(key string, obj runtime.Object, ttl int64, into runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	if _, ok := f.kvs[key]; ok {
		return kvstore.NewKeyExistsError(key, 0)
	}
	if ttl < 0 {
		return kvstore.NewInvalidTTLError(ttl)
	}

	value, err := f.encode(obj)
	if err != nil {
		return err
	}

	v := &memKvRec{ value: string(value), ttl: ttl, revision: 1 }
	f.kvs[key] = v
	f.setupWatchers(key, v)

	if into != nil {
		return f.decode(value, into, v.revision)
	}

	return nil
}

// Delete removes a single key in memKv. If "into" is not nil, it is set to the previous value
// of the key in kv store.
func (f *memKv) Delete(key string, into runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}
	defer delete(f.kvs, key)
	f.sendWatchEvents(key, v, true)

	if into != nil {
		return f.decode([]byte(v.value), into, v.revision)
	}
	
	return nil
}

// AtomicDelete removes a key, only if it exists with the specified version. If "into" is not
// nil, it is set to the last known value in the kv store.
func (f *memKv) AtomicDelete(key string, prevVersion string, into runtime.Object) error {
	version, err := strconv.ParseInt(prevVersion, 10, 64)
	if err != nil {
		return err
	}

	f.Lock()
	defer f.Unlock()

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return f.decode([]byte(v.value), into, v.revision)
	}
	if v.revision != version {
		return kvstore.NewVersionConflictError(key, version)
	}

	f.sendWatchEvents(key, v, true)

	delete(f.kvs, key)
	return nil
}

// PrefixDelete removes all keys with the matching prefix. Since it is meant to be used
// for deleting prefixes only, a "/" is added at the end of the prefix if it doesn't
// exist. For example, a delete with "/abc" prefix would only delete "/abc/123" and
// "/abc/456", but not "/abcd".
func (f *memKv) PrefixDelete(prefix string) error {
	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}

	f.Lock()
	defer f.Unlock()

	for key, v := range f.kvs {
		if strings.HasPrefix(key, prefix) {
			f.sendWatchEvents(key, v, true)
			delete(f.kvs, key)
		}
	}

	return nil
}

// Update modifies an existing object. If the key does not exist, update returns an error. This
// should only be used if a single writer owns the key.
func (f *memKv) Update(key string, obj runtime.Object, ttl int64, into runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}
	value, err := f.encode(obj)
	if err != nil {
		return err
	}

	v.value = string(value)
	v.ttl = ttl
	v.revision += 1

	f.sendWatchEvents(key, v, false)

	if into != nil {
		return f.decode(value, into, v.revision)
	}


	return nil
}

// AtomicUpdate modifies an existing object, only if the provided previous version matches the
// existing version of the key. This is useful for implementing elections using a single ttl key. The
// winner refreshes TTL on the key only if it hasn't been taken over by another node.
func (f *memKv) AtomicUpdate(key string, obj runtime.Object, prevVersion string, ttl int64, into runtime.Object) error {
	version, err := strconv.ParseInt(prevVersion, 10, 64)
	if err != nil {
		return err
	}

	f.Lock()
	defer f.Unlock()

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if v.revision != version {
		return kvstore.NewVersionConflictError(key, version)
	}

	value, err := f.encode(obj)
	if err != nil {
		return err
	}

	v.value = string(value)
	v.ttl = ttl
	v.revision += 1

	f.sendWatchEvents(key, v, false)

	if into != nil {
		return f.decode(value, into, v.revision)
	}

	return nil
}

// ConsistentUpdate modifies an existing object by invoking the provided update function. This should
// be used when there are multiple writers to various parts of the object and the updates need to be
// done in a consistent manner.
// Example:
// Writer1 updates field f1 to v1.
// Writer2 updates field f2 to v2 at the same time.
// ConsistentUpdate guarantees that the object lands in a consistent state where f1=v1 and f2=v2.
func (f *memKv) ConsistentUpdate(key string, ttl int64, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	if into == nil {
		return fmt.Errorf("into parameter is mandatory")
	}
	for {
		// Get the object.
		if err := f.Get(key, into); err != nil {
			return err
		}

		// Use the provided updateFunc to update the object.
		newObj, err := updateFunc(into)
		if err != nil {
			return err
		}

		// Previous version for doing a CAS.
		objMeta, _ := runtime.GetObjectMeta(into)
		version, err := strconv.ParseInt(objMeta.ResourceVersion, 10, 64)
		if err != nil {
			return err
		}

		f.Lock()

		v, ok := f.kvs[key]
		if !ok {
			v = &memKvRec{}
		}
		if v.revision == version {
			value, err := f.encode(newObj)
			if err != nil {
				f.Unlock()
				return err
			}

			v.value = string(value)
			v.ttl = ttl
			v.revision += 1
			f.sendWatchEvents(key, v, false)

			if into != nil {
				f.Unlock()
				return f.decode(value, into, v.revision)
			}
		}
		f.Unlock()
	}
	return nil
}

// Get the object corresponding to a single key
func (f *memKv) Get(key string, into runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return f.decode([]byte(v.value), into, v.revision)
	}
	return nil
}

// List the objects corresponding to a prefix. It is assumed that all the keys under this
// prefix are homogenous. "into" should point to a List object and should have an "Items"
// slice for individual objects.
func (f *memKv) List(prefix string, into runtime.Object) error {
	target, err := helper.ValidListObjForDecode(into)
	if err != nil {
		return err
	}

	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}

	f.Lock()
	defer f.Unlock()

	for key, v := range f.kvs {
		if strings.HasPrefix(key, prefix) {
			obj := reflect.New(target.Type().Elem()).Interface().(runtime.Object)
			if err := f.decode([]byte(v.value), obj, v.revision); err != nil {
				return err
			}
			target.Set(reflect.Append(target, reflect.ValueOf(obj).Elem()))
		}
	}

	return f.listVersioner.SetVersion(into, uint64(0))
}

// Watch the object corresponding to a key. fromVersion is the version to start
// the watch from. If fromVersion is 0, it will return the existing object and
// watch for changes from the returned version.
func (f *memKv) Watch(key string, fromVersion string) (kvstore.Watcher, error) {
	return f.newWatcher(key, fromVersion)
}

// PrefixWatch watches changes on all objects corresponding to a prefix key.
// fromVersion is the version to start the watch from. If fromVersion is 0, it
// will return the existing objects and watch for changes from the returned
// version.
// TODO: Filter objects
func (f *memKv) PrefixWatch(prefix string, fromVersion string) (kvstore.Watcher, error) {
	return f.newPrefixWatcher(prefix, fromVersion)
}

// Contest creates a new contender in an election. name is the name of the
// election. id is the identifier of the contender. When a leader is elected,
// the leader's lease is automatically refreshed. ttl is the timeout for lease
// refresh. If the leader does not update the lease for ttl duration, a new
// election is performed.
func (f *memKv) Contest(name string, id string, ttl uint64) (kvstore.Election, error) {
	return f.newElection(name, id, int(ttl))
}
