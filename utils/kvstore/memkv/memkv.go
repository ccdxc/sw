package memkv

// memkv uses memory as kv store,
// The goal is allow a emulate kvstore without starting an external process (e.g. etcd)
// to help with unit testing any function that depends on state store to validate its functionality
// The package can be used in multi-threaded binary, but not useful for cross nodes kv store
//

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/helper"
	"github.com/pensando/sw/utils/runtime"
)

const (
	defaultMinTTL = 2
)

type memKvRec struct {
	value    string
	ttl      int64
	revision int64
	watchers []*watcher
}

// MemKv is state store interface to a memkv client
type MemKv struct {
	sync.Mutex
	cluster       interface{}
	deleted       bool
	codec         runtime.Codec
	objVersioner  runtime.Versioner
	listVersioner runtime.Versioner
	kvs           map[string]*memKvRec
	watchers      map[string]*watcher
	returnErr     bool // when set, all interface methods will return an error
}

// NewMemKv creates a new in memory kv store
func NewMemKv(cluster interface{}, codec runtime.Codec) (kvstore.Interface, error) {
	kvs := make(map[string]*memKvRec)
	watchers := make(map[string]*watcher)
	fkv := &MemKv{
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
func ttlDecrement(f *MemKv) {
	for {
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
			v.ttl--
			if v.ttl == 0 {
				delete(f.kvs, key)
			}
		}
		f.Unlock()
	}
}

func (f *MemKv) deleteAll() {
	f.Lock()
	// delete all kv pairs
	for key := range f.kvs {
		delete(f.kvs, key)
	}

	// delete all watchers
	for key := range f.watchers {
		delete(f.watchers, key)
	}
	f.Unlock()
}

// encode implements the serialization of an object to be stored in memkv.
// TBD: this function is common among all state stores
func (f *MemKv) encode(obj runtime.Object) ([]byte, error) {
	// If the object implements the json.Marshaler interface, use it.
	if m, ok := interface{}(obj).(json.Marshaler); ok {
		return m.MarshalJSON()
	}
	// Use the store encoder.
	return f.codec.Encode(obj)
}

// decode implements the de-serialization of an object stored in memkv.
// TBD: this function is common among all state stores
func (f *MemKv) decode(value []byte, into runtime.Object, version int64) error {
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

// Create creates a key in memkv with the provided object.
func (f *MemKv) Create(ctx context.Context, key string, obj runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	if f.returnErr {
		return errors.New("returnErr set")
	}

	if _, ok := f.kvs[key]; ok {
		return kvstore.NewKeyExistsError(key, 0)
	}

	value, err := f.encode(obj)
	if err != nil {
		return err
	}

	v := &memKvRec{value: string(value), ttl: 0, revision: 1}
	f.kvs[key] = v
	f.setupWatchers(key, v)

	return f.objVersioner.SetVersion(obj, uint64(v.revision))
}

// MUST be called with the Lock HELD
func compCheck(f *MemKv, cs ...kvstore.Cmp) error {
	for _, cmp := range cs {
		v, ok := f.kvs[cmp.Key]
		if !ok {
			if cmp.Target == kvstore.Version &&
				((cmp.Operator == "=" && cmp.Version == 0) || (cmp.Operator == "<" && cmp.Version == 1)) {
				return nil
			}
			return kvstore.NewKeyNotFoundError(cmp.Key, 0)
		}

		switch cmp.Target {
		case kvstore.Version:
			switch cmp.Operator {
			case "=":
				if v.revision != cmp.Version {
					return kvstore.NewVersionConflictError(cmp.Key, cmp.Version)
				}
			case "!=":
				if v.revision == cmp.Version {
					return kvstore.NewVersionConflictError(cmp.Key, cmp.Version)
				}
			case "<":
				if v.revision >= cmp.Version {
					return kvstore.NewVersionConflictError(cmp.Key, cmp.Version)
				}
			case ">":
				if v.revision <= cmp.Version {
					return kvstore.NewVersionConflictError(cmp.Key, cmp.Version)
				}
			}
		}
	}
	return nil
}

// Delete removes a single key in MemKv. If "into" is not nil, it is set to the previous value
// of the key in kv store. "cs" are comparators to allow for conditional deletes.
func (f *MemKv) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	f.Lock()
	defer f.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}
	err := compCheck(f, cs...)
	if err != nil {
		return err
	}

	defer delete(f.kvs, key)
	f.sendWatchEvents(key, v, true)

	if into != nil {
		return f.decode([]byte(v.value), into, v.revision)
	}

	return nil
}

// PrefixDelete removes all keys with the matching prefix. Since it is meant to be used
// for deleting prefixes only, a "/" is added at the end of the prefix if it doesn't
// exist. For example, a delete with "/abc" prefix would only delete "/abc/123" and
// "/abc/456", but not "/abcd".
func (f *MemKv) PrefixDelete(ctx context.Context, prefix string) error {
	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}

	f.Lock()
	defer f.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	for key, v := range f.kvs {
		if strings.HasPrefix(key, prefix) {
			f.sendWatchEvents(key, v, true)
			delete(f.kvs, key)
		}
	}

	return nil
}

// Update modifies an existing object. If the key does not exist, update returns an error. This
// can be used without comparators if a single writer owns the key. "cs" are comparators to allow
// for conditional updates, including parallel updates.
func (f *MemKv) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	f.Lock()
	defer f.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	v, ok := f.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	err := compCheck(f, cs...)
	if err != nil {
		return err
	}

	value, err := f.encode(obj)
	if err != nil {
		return err
	}

	v.value = string(value)
	v.ttl = 0
	v.revision++

	f.sendWatchEvents(key, v, false)

	return f.objVersioner.SetVersion(obj, uint64(v.revision))
}

// ConsistentUpdate modifies an existing object by invoking the provided update function. This should
// be used when there are multiple writers to various parts of the object and the updates need to be
// done in a consistent manner.
// Example:
// Writer1 updates field f1 to v1.
// Writer2 updates field f2 to v2 at the same time.
// ConsistentUpdate guarantees that the object lands in a consistent state where f1=v1 and f2=v2.
func (f *MemKv) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	if into == nil {
		return fmt.Errorf("into parameter is mandatory")
	}

	if f.returnErr {
		return errors.New("returnErr set")
	}

	for {
		// Get the object.
		if err := f.Get(ctx, key, into); err != nil {
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
			v.ttl = 0
			v.revision++
			f.sendWatchEvents(key, v, false)

			if into != nil {
				f.Unlock()
				return f.decode(value, into, v.revision)
			}
		}
		f.Unlock()
	}
}

// Get the object corresponding to a single key
func (f *MemKv) Get(ctx context.Context, key string, into runtime.Object) error {
	f.Lock()
	defer f.Unlock()

	if f.returnErr {
		return errors.New("returnErr set")
	}

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
func (f *MemKv) List(ctx context.Context, prefix string, into runtime.Object) error {

	if f.returnErr {
		return errors.New("returnErr set")
	}

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
func (f *MemKv) Watch(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
	if f.returnErr {
		return nil, errors.New("returnErr set")
	}

	return f.newWatcher(ctx, key, fromVersion)
}

// PrefixWatch watches changes on all objects corresponding to a prefix key.
// fromVersion is the version to start the watch from. If fromVersion is 0, it
// will return the existing objects and watch for changes from the returned
// version.
// TODO: Filter objects
func (f *MemKv) PrefixWatch(ctx context.Context, prefix string, fromVersion string) (kvstore.Watcher, error) {
	if f.returnErr {
		return nil, errors.New("returnErr set")
	}

	return f.newPrefixWatcher(ctx, prefix, fromVersion)
}

// Contest creates a new contender in an election. name is the name of the
// election. id is the identifier of the contender. When a leader is elected,
// the leader's lease is automatically refreshed. ttl is the timeout for lease
// refresh. If the leader does not update the lease for ttl duration, a new
// election is performed.
func (f *MemKv) Contest(ctx context.Context, name string, id string, ttl uint64) (kvstore.Election, error) {
	if f.returnErr {
		return nil, errors.New("returnErr set")
	}

	return f.newElection(ctx, name, id, int(ttl))
}

// NewTxn creates a new transaction object.
func (f *MemKv) NewTxn() kvstore.Txn {
	return f.newTxn()
}

func (f *MemKv) commitTxn(t *txn) (kvstore.TxnResponse, error) {
	f.Lock()
	defer f.Unlock()
	ret := kvstore.TxnResponse{}

	if f.returnErr {
		return ret, errors.New("returnErr set")
	}

	err := compCheck(f, t.cmps...)
	if err != nil {
		return ret, err
	}

	// other checks based on the ops
	for _, o := range t.ops {
		switch o.t {
		case tCreate:
			if _, ok := f.kvs[o.key]; ok {
				return ret, kvstore.NewKeyExistsError(o.key, 0)
			}
		case tUpdate:
			_, ok := f.kvs[o.key]
			if !ok {
				return ret, kvstore.NewKeyNotFoundError(o.key, 0)
			}
		case tDelete:
			_, ok := f.kvs[o.key]
			if !ok {
				return ret, kvstore.NewKeyNotFoundError(o.key, 0)
			}
		}
	}
	ret.Succeeded = true

	// actual operations - Point of no return
	for _, o := range t.ops {
		switch o.t {
		case tCreate:
			v := &memKvRec{value: o.val, revision: 1}
			f.kvs[o.key] = v
			f.setupWatchers(o.key, v)
			f.objVersioner.SetVersion(o.obj, uint64(v.revision))
			opresp := kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: o.key, Obj: nil}
			ret.Responses = append(ret.Responses, opresp)
		case tUpdate:
			v := f.kvs[o.key]
			v.value = o.val
			v.revision++
			f.sendWatchEvents(o.key, v, false)
			f.objVersioner.SetVersion(o.obj, uint64(v.revision))
			opresp := kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: o.key, Obj: nil}
			ret.Responses = append(ret.Responses, opresp)
		case tDelete:
			v, ok := f.kvs[o.key]
			if ok {
				into, err := f.codec.Decode([]byte(v.value), nil)
				if err == nil {
					opresp := kvstore.TxnOpResponse{Oper: kvstore.OperDelete, Key: o.key, Obj: into}
					defer delete(f.kvs, o.key)
					f.sendWatchEvents(o.key, v, true)
					ret.Responses = append(ret.Responses, opresp)
				}
			}
		}
	}
	return ret, nil
}

// InjectWatchEvent injects the given event to the watcher of the specified prefix
// Useful for testing
func (f *MemKv) InjectWatchEvent(prefix string, e *kvstore.WatchEvent, timeOutSec int) error {
	w := f.watchers[prefix]
	if w == nil {
		return errors.New("No watcher on the prefix")
	}

	select {
	case w.outCh <- e:
		return nil
	case <-time.After(time.Duration(timeOutSec) * time.Second):
		return errors.New("Time out")
	}

}

// IsWatchActive tests the presence of a watcher on a given prefix
// Useful for testing
func (f *MemKv) IsWatchActive(prefix string) bool {
	w := f.watchers[prefix]
	return w != nil
}

// CloseWatch closes the watch channel on a given prefix
// Useful for testing
func (f *MemKv) CloseWatch(prefix string) {
	w := f.watchers[prefix]
	if w != nil {
		close(w.outCh)
		w.f.deleteWatchers(w)
	}
}

// SetErrorState sets the returnErr flag
func (f *MemKv) SetErrorState(state bool) {
	f.returnErr = state
}
