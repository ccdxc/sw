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

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	defaultMinTTL = 2
)

type memKvRec struct {
	value     string
	ttl       int64
	revision  int64
	createRev int64
	watchers  []*watcher
}

// RevisionMode defines the way revision numbers are generated for objects
type RevisionMode string

const (
	// ClusterRevision mode is when there is a cluster wide revision akin to etcd
	ClusterRevision RevisionMode = "clusterRevision"
	// ObjectRevision generates revisions that are specific to the object
	ObjectRevision RevisionMode = "objectRevision"
)

// MemKv is state store interface to a memkv client
type MemKv struct {
	sync.Mutex
	cluster       *Cluster
	deleted       bool
	codec         runtime.Codec
	objVersioner  runtime.Versioner
	listVersioner runtime.Versioner
	returnErr     bool // when set, all interface methods will return an error
	revMode       RevisionMode
	maxTxnOps     int
}

// Cluster memkv cluster (equivalent to an etcd backend cluster)
type Cluster struct {
	sync.Mutex
	elections       map[string]*memkvElection // current elections
	clientID        int                       // current id of the store
	clusterRevMutex sync.Mutex
	clusterRevision int64
	clients         map[string]*MemKv // all client stores
	kvs             map[string]*memKvRec
	watchers        map[string][]*watcher
}

// db of clusters
var clusters = make(map[string]*Cluster)

// lock to protect access to clusters
var globalLock sync.Mutex

// NewMemKv creates a new in memory kv store
func NewMemKv(cluster []string, codec runtime.Codec) (kvstore.Interface, error) {
	// create the client
	fkv := &MemKv{
		cluster:       getCluster(cluster),
		deleted:       false,
		codec:         codec,
		objVersioner:  runtime.NewObjectVersioner(),
		listVersioner: runtime.NewListVersioner(),
	}
	return fkv, nil
}

// NewCluster creates a new cluster
func NewCluster() *Cluster {
	cl := &Cluster{
		elections: make(map[string]*memkvElection),
		clientID:  0,
		clients:   make(map[string]*MemKv),
		kvs:       make(map[string]*memKvRec),
		watchers:  make(map[string][]*watcher),
	}

	go ttlDecrement(cl)

	return cl
}

// getCluster gets a cluster by url
func getCluster(cluster []string) *Cluster {
	globalLock.Lock()
	defer globalLock.Unlock()

	// see if we have a cluster already
	cl, ok := clusters[fmt.Sprintf("%v", cluster)]
	if ok {
		return cl
	}

	// create new cluster
	cl = NewCluster()
	clusters[fmt.Sprintf("%v", cluster)] = cl

	return cl
}

// DeleteClusters deletes all clusters, used in tests
func DeleteClusters() {
	globalLock.Lock()
	defer globalLock.Unlock()

	for key, cl := range clusters {
		cl.deleteAll()
		delete(clusters, key)
	}
}

// ttlDecrement would sleep for a second and decrement ttl for a key in kvstore
// FIXME: this background task must be stopped, perhaps by having kvstore use the context
func ttlDecrement(cluster *Cluster) {
	for {
		time.Sleep(time.Second)
		cluster.Lock()

		for key, v := range cluster.kvs {
			if v.ttl == 0 {
				continue
			}
			v.ttl--
			if v.ttl == 0 {
				delete(cluster.kvs, key)
			}
		}
		cluster.Unlock()
	}
}

func (cluster *Cluster) deleteAll() {
	cluster.Lock()
	defer cluster.Unlock()

	// delete all kv pairs
	for key := range cluster.kvs {
		delete(cluster.kvs, key)
	}

	// delete all watchers
	for key := range cluster.watchers {
		delete(cluster.watchers, key)
	}

	// delete all clients
	for key := range cluster.clients {
		delete(cluster.clients, key)
	}
}

func (cluster *Cluster) getNextRevision() int64 {
	defer cluster.clusterRevMutex.Unlock()
	cluster.clusterRevMutex.Lock()
	cluster.clusterRevision++
	return cluster.clusterRevision
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
	f.cluster.Lock()
	defer f.cluster.Unlock()

	if f.returnErr {
		return errors.New("returnErr set")
	}

	if _, ok := f.cluster.kvs[key]; ok {
		return kvstore.NewKeyExistsError(key, 0)
	}

	value, err := f.encode(obj)
	if err != nil {
		return err
	}
	var rev int64 = 1
	if f.revMode == ClusterRevision {
		rev = f.cluster.getNextRevision()
	}
	v := &memKvRec{value: string(value), ttl: 0, revision: rev}
	v.createRev = v.revision
	f.cluster.kvs[key] = v
	f.setupWatchers(key, v)

	return f.objVersioner.SetVersion(obj, uint64(v.revision))
}

// MUST be called with the Lock HELD
func compCheck(f *MemKv, cs ...kvstore.Cmp) error {
	for _, cmp := range cs {
		v, ok := f.cluster.kvs[cmp.Key]
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
	f.cluster.Lock()
	defer f.cluster.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	v, ok := f.cluster.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}
	err := compCheck(f, cs...)
	if err != nil {
		return err
	}
	if f.revMode == ClusterRevision {
		// Increment the cluster revision
		v.revision = f.cluster.getNextRevision()
	}
	defer delete(f.cluster.kvs, key)
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

	f.cluster.Lock()
	defer f.cluster.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	for key, v := range f.cluster.kvs {
		if strings.HasPrefix(key, prefix) {
			f.sendWatchEvents(key, v, true)
			delete(f.cluster.kvs, key)
		}
	}

	return nil
}

// Update modifies an existing object. If the key does not exist, update returns an error. This
// can be used without comparators if a single writer owns the key. "cs" are comparators to allow
// for conditional updates, including parallel updates.
func (f *MemKv) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	f.cluster.Lock()
	defer f.cluster.Unlock()
	if f.returnErr {
		return errors.New("returnErr set")
	}

	v, ok := f.cluster.kvs[key]
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
	if f.revMode == ClusterRevision {
		v.revision = f.cluster.getNextRevision()
	} else {
		v.revision++
	}

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
		// Get the object into a new Object
		scratch, err := runtime.NewEmpty(into)
		if err != nil {
			return err
		}

		if scratch == nil {
			return errors.New("failed to create object")
		}

		if err := f.Get(ctx, key, scratch); err != nil {
			return err
		}

		// Use the provided updateFunc to update the object.
		newObj, err := updateFunc(scratch)
		if err != nil {
			return err
		}

		// Previous version for doing a CAS.
		objMeta, _ := runtime.GetObjectMeta(scratch)
		version, err := strconv.ParseInt(objMeta.ResourceVersion, 10, 64)
		if err != nil {
			return err
		}

		f.cluster.Lock()

		v, ok := f.cluster.kvs[key]
		if !ok {
			v = &memKvRec{}
		}
		if v.revision == version {
			value, err := f.encode(newObj)
			if err != nil {
				f.cluster.Unlock()
				return err
			}

			v.value = string(value)
			v.ttl = 0
			if f.revMode == ClusterRevision {
				v.revision = f.cluster.getNextRevision()
			} else {
				v.revision++
			}

			f.sendWatchEvents(key, v, false)
			f.cluster.Unlock()
			return f.decode(value, into, v.revision)
		}
		f.cluster.Unlock()
	}
}

// Get the object corresponding to a single key
func (f *MemKv) Get(ctx context.Context, key string, into runtime.Object) error {
	f.cluster.Lock()
	defer f.cluster.Unlock()

	if f.returnErr {
		return errors.New("returnErr set")
	}

	v, ok := f.cluster.kvs[key]
	if !ok {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return f.decode([]byte(v.value), into, v.revision)
	}
	return nil
}

// List the objects corresponding to a prefix. It is assumed that all the keys under this
// prefix are homogeneous. "into" should point to a List object and should have an "Items"
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

	f.cluster.Lock()
	defer f.cluster.Unlock()

	ptr := false
	elem := target.Type().Elem()
	if elem.Kind() == reflect.Ptr {
		ptr = true
		elem = elem.Elem()
	}
	if elem.Kind() == reflect.Interface {
		ptr = true
	}

	for key, v := range f.cluster.kvs {
		if strings.HasPrefix(key, prefix) {
			obj, err := f.codec.Decode([]byte(v.value), nil)
			if err != nil {
				log.ErrorLog("msg", "unable to decode", "error", err, "key", key)
				continue
			}
			f.objVersioner.SetVersion(obj, uint64(v.revision))
			if ptr {
				target.Set(reflect.Append(target, reflect.ValueOf(obj)))
			} else {
				target.Set(reflect.Append(target, reflect.ValueOf(obj).Elem()))
			}
		}
	}

	return f.listVersioner.SetVersion(into, uint64(0))
}

// ListFiltered lists objects corresponding to a prefix after applying
// the filter specified by opts. It is assumed that all keys under the
// prefix are homogeneous.
func (f *MemKv) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	// Filtering is not supported, fallback to List
	return f.List(ctx, prefix, into)
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

// WatchFiltered watches changes on all objects with filters specified
// by opts applied.
func (f *MemKv) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) (kvstore.Watcher, error) {
	// Filtering is no supported. Fallback to PrefixWatch
	return f.PrefixWatch(ctx, key, opts.ResourceVersion)
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

// Close and cleanup network resources
func (f *MemKv) Close() {
}

func (f *MemKv) commitTxn(t *txn) (kvstore.TxnResponse, error) {
	f.cluster.Lock()
	defer f.cluster.Unlock()
	ret := kvstore.TxnResponse{}

	if f.returnErr {
		return ret, errors.New("returnErr set")
	}

	// Check for max ops
	if f.maxTxnOps > 0 && (len(t.ops)+len(t.cmps) > f.maxTxnOps) {
		return ret, errors.New("too many operations in transaction")
	}

	err := compCheck(f, t.cmps...)
	if err != nil {
		return ret, err
	}
	// other checks based on the ops
	for _, o := range t.ops {
		switch o.t {
		case tCreate:
			if _, ok := f.cluster.kvs[o.key]; ok {
				return ret, kvstore.NewKeyExistsError(o.key, 0)
			}
		case tUpdate:
			_, ok := f.cluster.kvs[o.key]
			if !ok {
				return ret, kvstore.NewKeyNotFoundError(o.key, 0)
			}
		case tDelete:
			_, ok := f.cluster.kvs[o.key]
			if !ok {
				return ret, kvstore.NewKeyNotFoundError(o.key, 0)
			}
		}
	}
	ret.Succeeded = true

	// actual operations - Point of no return
	var rev int64
	if f.revMode == ClusterRevision {
		rev = f.cluster.getNextRevision()
	}
	ret.Revision = rev
	for _, o := range t.ops {
		switch o.t {
		case tCreate:
			if f.revMode != ClusterRevision {
				rev = 1
			}
			v := &memKvRec{value: o.val, revision: rev}
			v.createRev = v.revision
			f.cluster.kvs[o.key] = v
			f.setupWatchers(o.key, v)
			f.objVersioner.SetVersion(o.obj, uint64(v.revision))
			opresp := kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: o.key, Obj: nil}
			ret.Responses = append(ret.Responses, opresp)
		case tUpdate:
			v := f.cluster.kvs[o.key]
			if !o.ignoreValue {
				v.value = o.val
			}
			if f.revMode == ClusterRevision {
				v.revision = rev
			} else {
				v.revision++
			}
			f.sendWatchEvents(o.key, v, false)
			f.objVersioner.SetVersion(o.obj, uint64(v.revision))
			opresp := kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: o.key, Obj: nil}
			ret.Responses = append(ret.Responses, opresp)
		case tDelete:
			v, ok := f.cluster.kvs[o.key]
			if ok {
				if f.revMode == ClusterRevision {
					v.revision = rev
				} else {
					v.revision++
				}
				into, err := f.codec.Decode([]byte(v.value), nil)
				if err == nil {
					f.objVersioner.SetVersion(into, uint64(v.revision))
					opresp := kvstore.TxnOpResponse{Oper: kvstore.OperDelete, Key: o.key, Obj: into}
					defer delete(f.cluster.kvs, o.key)
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
	f.cluster.Lock()
	wl := f.cluster.watchers[prefix]
	f.cluster.Unlock()
	if wl == nil {
		return errors.New("No watcher on the prefix")
	}

	for _, w := range wl {
		select {
		default:
			w.outCh <- e
		case <-time.After(time.Duration(timeOutSec) * time.Second):
			return errors.New("Time out")
		}
	}

	return nil
}

// IsWatchActive tests the presence of a watcher on a given prefix
// Useful for testing
func (f *MemKv) IsWatchActive(prefix string) bool {
	f.cluster.Lock()
	defer f.cluster.Unlock()
	w := f.cluster.watchers[prefix]
	return w != nil
}

// CloseWatch closes the watch channel on a given prefix
// Useful for testing
func (f *MemKv) CloseWatch(prefix string) {
	f.cluster.Lock()
	wl := f.cluster.watchers[prefix]
	f.cluster.Unlock()
	for _, w := range wl {
		close(w.outCh)
		w.f.deleteWatchers(w)
	}
}

// SetErrorState sets the returnErr flag
func (f *MemKv) SetErrorState(state bool) {
	f.returnErr = state
}

// SetRevMode sets the revision mode
func (f *MemKv) SetRevMode(val RevisionMode) {
	f.revMode = val
}

// SetMaxTxnOps sets the maximum number of elements in the txn allowed
func (f *MemKv) SetMaxTxnOps(val int) {
	f.maxTxnOps = val
}

// Lease takes a lease on a key and renews in background
func (f *MemKv) Lease(ctx context.Context, key string, obj runtime.Object, ttl uint64) (chan kvstore.LeaseEvent, error) {
	// create the object
	err := f.Create(ctx, key, obj)
	if err != nil {
		return nil, err
	}

	// channel to send out events
	eventCh := make(chan kvstore.LeaseEvent, outCount)

	// wait till the context is cancelled
	go func() {
		for {
			select {
			case <-ctx.Done():
				// delete the object
				f.Delete(context.Background(), key, nil)

				// send cancelled event
				eventCh <- kvstore.LeaseCancelled
				close(eventCh)
				return
			}
		}
	}()

	return eventCh, nil
}
