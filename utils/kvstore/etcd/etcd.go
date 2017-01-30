package etcd

import (
	"context"
	"encoding/json"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

const (
	timeout = time.Second * 5
)

// etcdStore implements a KVStore using etcd as storage.
type etcdStore struct {
	client        *clientv3.Client
	codec         runtime.Codec
	objVersioner  runtime.Versioner
	listVersioner runtime.Versioner
}

// NewEtcdStore creates a new etcdStore.
func NewEtcdStore(servers []string, codec runtime.Codec) (kvstore.Interface, error) {
	config := clientv3.Config{
		Endpoints:   servers,
		DialTimeout: timeout,
	}

	client, err := clientv3.New(config)
	if err != nil {
		return nil, err
	}

	return newEtcdStore(client, codec)
}

func newEtcdStore(client *clientv3.Client, codec runtime.Codec) (kvstore.Interface, error) {
	return &etcdStore{
		client:        client,
		codec:         codec,
		objVersioner:  runtime.NewObjectVersioner(),
		listVersioner: runtime.NewListVersioner(),
	}, nil
}

// encode implements the serialization of an object to be stored in etcd.
func (e *etcdStore) encode(obj runtime.Object) ([]byte, error) {
	// If the object implements the json.Marshaler interface, use it.
	if m, ok := interface{}(obj).(json.Marshaler); ok {
		return m.MarshalJSON()
	}
	// Use the store encoder.
	return e.codec.Encode(obj)
}

// decode implements the de-serialization of an object stored in etcd.
func (e *etcdStore) decode(value []byte, into runtime.Object, version int64) error {
	if err := validObjForDecode(into); err != nil {
		return err
	}

	// If the object implements the json.Unmarshaler interface, use it.
	if m, ok := interface{}(into).(json.Unmarshaler); ok {
		if err := m.UnmarshalJSON(value); err != nil {
			return err
		}
	} else {
		// Use the store decoder.
		if _, err := e.codec.Decode(value, into); err != nil {
			return err
		}
	}

	return e.objVersioner.SetVersion(into, uint64(version))
}

// ttlOpts creates a client option for setting a lease/ttl against a key.
func (e *etcdStore) ttlOpts(ctx context.Context, ttl int64) (clientv3.LeaseID, []clientv3.OpOption, error) {
	if ttl == 0 {
		return 0, nil, nil
	}

	ctxT, cancel := context.WithTimeout(ctx, timeout)
	lcr, err := e.client.Lease.Grant(ctxT, ttl)
	cancel()

	if err != nil {
		return 0, nil, err
	}
	return lcr.ID, []clientv3.OpOption{clientv3.WithLease(clientv3.LeaseID(lcr.ID))}, nil
}

// putHelper updates a key in etcd with the provided object, ttl and comparators.
func (e *etcdStore) putHelper(key string, obj runtime.Object, ttl int64, cs ...clientv3.Cmp) ([]byte, *clientv3.TxnResponse, error) {
	value, err := e.encode(obj)
	if err != nil {
		return nil, nil, err
	}

	_, opts, err := e.ttlOpts(context.Background(), ttl)
	if err != nil {
		return nil, nil, err
	}

	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	resp, err := e.client.KV.Txn(ctx).If(
		cs...,
	).Then(
		clientv3.OpPut(key, string(value), opts...),
	).Commit()
	cancel()

	return value, resp, err
}

// Create creates a key in etcd with the provided object and ttl. If ttl is 0, it means the key
// does not expire. If "into" is not nil, it is set to the value returned by the kv store.
func (e *etcdStore) Create(key string, obj runtime.Object, ttl int64, into runtime.Object) error {
	value, resp, err := e.putHelper(key, obj, ttl, clientv3.Compare(clientv3.ModRevision(key), "=", 0))
	if err != nil {
		return err
	}

	if !resp.Succeeded {
		return kvstore.NewKeyExistsError(key, 0)
	}

	if into != nil {
		return e.decode(value, into, resp.Responses[0].GetResponsePut().Header.Revision)
	}
	return nil
}

// delete removes a single key in etcd, if the comparisons match.
func (e *etcdStore) delete(key string, into runtime.Object, cs ...clientv3.Cmp) error {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	resp, err := e.client.KV.Txn(ctx).If(cs...).Then(
		clientv3.OpGet(key),
		clientv3.OpDelete(key),
	).Commit()
	cancel()
	if err != nil {
		return err
	}

	// delete with wrong prevVersion will return with no error but Succeeded set
	// to false.
	if !resp.Succeeded {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	kvs := resp.Responses[0].GetResponseRange().Kvs
	if len(kvs) == 0 {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return e.decode(kvs[0].Value, into, kvs[0].ModRevision)
	}
	return nil
}

// Delete removes a single key in etcd. If "into" is not nil, it is set to the previous value
// of the key in kv store.
func (e *etcdStore) Delete(key string, into runtime.Object) error {
	return e.delete(key, into)
}

// AtomicDelete removes a key, only if it exists with the specified version. If "into" is not
// nil, it is set to the last known value in the kv store.
func (e *etcdStore) AtomicDelete(key string, prevVersion string, into runtime.Object) error {
	version, err := strconv.ParseInt(prevVersion, 10, 64)
	if err != nil {
		return err
	}
	return e.delete(key, into, clientv3.Compare(clientv3.ModRevision(key), "=", version))
}

// PrefixDelete removes all keys with the matching prefix. Since it is meant to be used
// for deleting prefixes only, a "/" is added at the end of the prefix if it doesn't
// exist. For example, a delete with "/abc" prefix would only delete "/abc/123" and
// "/abc/456", but not "/abcd".
func (e *etcdStore) PrefixDelete(prefix string) error {
	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	_, err := e.client.KV.Txn(ctx).If().Then(
		clientv3.OpDelete(prefix, clientv3.WithPrefix()),
	).Commit()
	cancel()
	return err
}

// Update modifies an existing object. If the key does not exist, update returns an error. This
// should only be used if a single writer owns the key.
func (e *etcdStore) Update(key string, obj runtime.Object, ttl int64, into runtime.Object) error {
	value, resp, err := e.putHelper(key, obj, ttl, clientv3.Compare(clientv3.ModRevision(key), ">", 0))
	if err != nil {
		return err
	}

	if !resp.Succeeded {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return e.decode(value, into, resp.Responses[0].GetResponsePut().Header.Revision)
	}
	return nil
}

// AtomicUpdate modifies an existing object, only if the provided previous version matches the
// existing version of the key. This is useful for implementing elections using a single ttl key. The
// winner refreshes TTL on the key only if it hasn't been taken over by another node.
func (e *etcdStore) AtomicUpdate(key string, obj runtime.Object, prevVersion string, ttl int64, into runtime.Object) error {
	version, err := strconv.ParseInt(prevVersion, 10, 64)
	if err != nil {
		return err
	}

	value, resp, err := e.putHelper(key, obj, ttl, clientv3.Compare(clientv3.ModRevision(key), "=", version))
	if err != nil {
		return err
	}

	if !resp.Succeeded {
		return kvstore.NewVersionConflictError(key, version)
	}

	if into != nil {
		return e.decode(value, into, resp.Responses[0].GetResponsePut().Header.Revision)
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
func (e *etcdStore) ConsistentUpdate(key string, ttl int64, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	if into == nil {
		return fmt.Errorf("into parameter is mandatory")
	}
	for {
		// Get the object.
		if err := e.Get(key, into); err != nil {
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

		// CAS with the read version. Return if there is no error.
		value, resp, err := e.putHelper(key, newObj, ttl, clientv3.Compare(clientv3.ModRevision(key), "=", version))
		if err != nil {
			return err
		}

		if !resp.Succeeded {
			log.Infof("CAS failure for %v, retrying update...", key)
			continue
		}

		return e.decode(value, into, resp.Responses[0].GetResponsePut().Header.Revision)
	}
}

// Get the object corresponding to a single key in etcd.
func (e *etcdStore) Get(key string, into runtime.Object) error {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	resp, err := e.client.KV.Get(ctx, key)
	cancel()
	if err != nil {
		return err
	}

	if len(resp.Kvs) == 0 {
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	if into != nil {
		return e.decode(resp.Kvs[0].Value, into, resp.Kvs[0].ModRevision)
	}
	return nil
}

// List the objects corresponding to a prefix. It is assumed that all the keys under this
// prefix are homogenous. "into" should point to a List object and should have an "Items"
// slice for individual objects.
func (e *etcdStore) List(prefix string, into runtime.Object) error {
	v, err := validListObjForDecode(into)
	if err != nil {
		return err
	}

	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}

	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	resp, err := e.client.KV.Get(ctx, prefix, clientv3.WithPrefix())
	cancel()
	if err != nil {
		return err
	}

	for _, kv := range resp.Kvs {
		obj := reflect.New(v.Type().Elem()).Interface().(runtime.Object)
		if err := e.decode(kv.Value, obj, kv.ModRevision); err != nil {
			return err
		}
		v.Set(reflect.Append(v, reflect.ValueOf(obj).Elem()))
	}

	return e.listVersioner.SetVersion(into, uint64(resp.Header.Revision))
}

// Watch the object corresponding to a key. fromVersion is the version to start
// the watch from. If fromVersion is 0, it will return the existing object and
// watch for changes from the returned version.
func (e *etcdStore) Watch(key string, fromVersion string) (kvstore.Watcher, error) {
	return e.newWatcher(key, fromVersion)
}

// PrefixWatch watches changes on all objects corresponding to a prefix key.
// fromVersion is the version to start the watch from. If fromVersion is 0, it
// will return the existing objects and watch for changes from the returned
// version.
// TODO: Filter objects
func (e *etcdStore) PrefixWatch(prefix string, fromVersion string) (kvstore.Watcher, error) {
	return e.newPrefixWatcher(prefix, fromVersion)
}

// Contest creates a new contender in an election. name is the name of the
// election. id is the identifier of the contender. When a leader is elected,
// the leader's lease is automatically refreshed. ttl is the timeout for lease
// refresh. If the leader does not update the lease for ttl duration, a new
// election is performed.
func (e *etcdStore) Contest(name string, id string, ttl uint64) (kvstore.Election, error) {
	return e.newElection(name, id, int(ttl))
}
