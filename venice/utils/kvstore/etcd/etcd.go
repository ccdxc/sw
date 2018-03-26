package etcd

import (
	"context"
	"encoding/json"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"time"

	"github.com/coreos/etcd/clientv3"
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/runtime"

	opentracing "github.com/opentracing/opentracing-go"
	otext "github.com/opentracing/opentracing-go/ext"

	tracing "github.com/pensando/sw/venice/utils/trace"
)

const (
	timeout            = time.Second * 5
	maxCallSendMsgSize = 10 * 1024 * 1024
)

// etcdStore implements a KVStore using etcd as storage.
type etcdStore struct {
	client        *clientv3.Client
	codec         runtime.Codec
	objVersioner  runtime.Versioner
	listVersioner runtime.Versioner
}

// NewEtcdStore creates a new etcdStore.
func NewEtcdStore(servers []string, codec runtime.Codec, grpcOpts ...grpc.DialOption) (kvstore.Interface, error) {
	config := clientv3.Config{
		Endpoints:          servers,
		DialTimeout:        timeout,
		DialOptions:        grpcOpts,
		MaxCallSendMsgSize: maxCallSendMsgSize,
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
		if _, err := e.codec.Decode(value, into); err != nil {
			return err
		}
	}

	return e.objVersioner.SetVersion(into, uint64(version))
}

func traceOper(ctx context.Context, operationName string, key string) (context.Context, opentracing.Span) {
	if !tracing.IsEnabled() {
		return ctx, nil
	}
	var clientSpan opentracing.Span
	if parentSpan := opentracing.SpanFromContext(ctx); parentSpan != nil {
		clientSpan = opentracing.StartSpan(operationName, opentracing.ChildOf(parentSpan.Context()))
	} else {
		clientSpan = opentracing.StartSpan(operationName)
	}
	otext.SpanKindRPCClient.Set(clientSpan)
	clientSpan.SetTag("kvkey", key)
	return opentracing.ContextWithSpan(ctx, clientSpan), clientSpan
}

// putHelper updates a key in etcd with the provided object and comparators.
func (e *etcdStore) putHelper(ctx context.Context, key string, obj runtime.Object, cs ...clientv3.Cmp) ([]byte, *clientv3.TxnResponse, error) {
	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore put", key)
	if span != nil {
		defer span.Finish()
	}

	value, err := e.encode(obj)
	if err != nil {
		return nil, nil, err
	}

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := e.client.KV.Txn(newCtx).If(
		cs...,
	).Then(
		clientv3.OpPut(key, string(value)),
	).Commit()
	cancel()

	return value, resp, err
}

// Create creates a key in etcd with the provided object.
func (e *etcdStore) Create(ctx context.Context, key string, obj runtime.Object) error {
	_, resp, err := e.putHelper(ctx, key, obj, clientv3.Compare(clientv3.ModRevision(key), "=", 0))
	if err != nil {
		return err
	}

	if !resp.Succeeded {
		return kvstore.NewKeyExistsError(key, 0)
	}

	return e.objVersioner.SetVersion(obj, uint64(resp.Responses[0].GetResponsePut().Header.Revision))
}

// Delete removes a single key in etcd. If "into" is not nil, it is set to the previous value
// of the key in kv store. "cs" are comparators to allow for conditional deletes.
func (e *etcdStore) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore delete", key)
	if span != nil {
		defer span.Finish()
	}

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := e.client.KV.Txn(newCtx).If(translateCmps(cs...)...).Then(
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

// PrefixDelete removes all keys with the matching prefix. Since it is meant to be used
// for deleting prefixes only, a "/" is added at the end of the prefix if it doesn't
// exist. For example, a delete with "/abc" prefix would only delete "/abc/123" and
// "/abc/456", but not "/abcd".
func (e *etcdStore) PrefixDelete(ctx context.Context, prefix string) error {
	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore prefixdelete", prefix)
	if span != nil {
		defer span.Finish()
	}

	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}
	newCtx, cancel := context.WithTimeout(ctx, timeout)
	_, err := e.client.KV.Txn(newCtx).If().Then(
		clientv3.OpDelete(prefix, clientv3.WithPrefix()),
	).Commit()
	cancel()
	return err
}

// Update modifies an existing object. If the key does not exist, update returns an error. This
// can be used without comparators if a single writer owns the key. "cs" are comparators to allow
// for conditional updates, including parallel updates.
func (e *etcdStore) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	version := int64(-1)
	for _, c := range cs {
		if c.Target == kvstore.Version {
			version = c.Version
		}
	}

	cmps := []clientv3.Cmp{}
	if len(cs) == 0 {
		cmps = append(cmps, clientv3.Compare(clientv3.ModRevision(key), ">", 0))
	} else {
		cmps = append(cmps, translateCmps(cs...)...)
	}

	_, resp, err := e.putHelper(ctx, key, obj, cmps...)
	if err != nil {
		return err
	}

	if !resp.Succeeded {
		if version != -1 {
			return kvstore.NewVersionConflictError(key, version)
		}
		return kvstore.NewKeyNotFoundError(key, 0)
	}

	return e.objVersioner.SetVersion(obj, uint64(resp.Responses[0].GetResponsePut().Header.Revision))
}

// ConsistentUpdate modifies an existing object by invoking the provided update function. This should
// be used when there are multiple writers to various parts of the object and the updates need to be
// done in a consistent manner.
// Example:
// Writer1 updates field f1 to v1.
// Writer2 updates field f2 to v2 at the same time.
// ConsistentUpdate guarantees that the object lands in a consistent state where f1=v1 and f2=v2.
func (e *etcdStore) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	if into == nil {
		return fmt.Errorf("into parameter is mandatory")
	}

	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore consUpdate", key)
	if span != nil {
		defer span.Finish()
	}

	for {
		// Get the object.
		if err := e.Get(ctx, key, into); err != nil {
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
		value, resp, err := e.putHelper(ctx, key, newObj, clientv3.Compare(clientv3.ModRevision(key), "=", version))
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
func (e *etcdStore) Get(ctx context.Context, key string, into runtime.Object) error {
	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore get", key)
	if span != nil {
		defer span.Finish()
	}

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := e.client.KV.Get(newCtx, key)
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
func (e *etcdStore) List(ctx context.Context, prefix string, into runtime.Object) error {
	v, err := helper.ValidListObjForDecode(into)
	if err != nil {
		return err
	}

	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}
	var span opentracing.Span
	ctx, span = traceOper(ctx, "kvstore list", prefix)
	if span != nil {
		defer span.Finish()
	}

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := e.client.KV.Get(newCtx, prefix, clientv3.WithPrefix())
	cancel()
	if err != nil {
		return err
	}

	ptr := false
	elem := v.Type().Elem()
	if elem.Kind() == reflect.Ptr {
		ptr = true
		elem = elem.Elem()
	}
	if elem.Kind() == reflect.Interface {
		ptr = true
	}

	for _, kv := range resp.Kvs {
		obj, err := e.codec.Decode(kv.Value, nil)
		if err != nil {
			return err
		}
		e.objVersioner.SetVersion(obj, uint64(kv.ModRevision))
		if ptr {
			v.Set(reflect.Append(v, reflect.ValueOf(obj)))
		} else {
			v.Set(reflect.Append(v, reflect.ValueOf(obj).Elem()))
		}
	}

	return e.listVersioner.SetVersion(into, uint64(resp.Header.Revision))
}

// Watch the object corresponding to a key. fromVersion is the version to start
// the watch from. If fromVersion is 0, it will return the existing object and
// watch for changes from the returned version.
func (e *etcdStore) Watch(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
	return e.newWatcher(ctx, key, fromVersion)
}

// PrefixWatch watches changes on all objects corresponding to a prefix key.
// fromVersion is the version to start the watch from. If fromVersion is 0, it
// will return the existing objects and watch for changes from the returned
// version.
// TODO: Filter objects
func (e *etcdStore) PrefixWatch(ctx context.Context, prefix string, fromVersion string) (kvstore.Watcher, error) {
	return e.newPrefixWatcher(ctx, prefix, fromVersion)
}

// Contest creates a new contender in an election. name is the name of the
// election. id is the identifier of the contender. When a leader is elected,
// the leader's lease is automatically refreshed. ttl is the timeout for lease
// refresh. If the leader does not update the lease for ttl duration, a new
// election is performed.
func (e *etcdStore) Contest(ctx context.Context, name string, id string, ttl uint64) (kvstore.Election, error) {
	return e.newElection(ctx, name, id, int(ttl))
}

// NewTxn creates a new transaction object.
func (e *etcdStore) NewTxn() kvstore.Txn {
	return e.newTxn()
}

// runLeaseLoop runs lease keepalive loop
func (e *etcdStore) runLeaseLoop(ctx context.Context, key string, leaseCh <-chan *clientv3.LeaseKeepAliveResponse, eventCh chan kvstore.LeaseEvent) {
	for {
		select {
		case <-ctx.Done():
			log.Infof("Context cancelled. stopping lease reneval for %s", key)
			eventCh <- kvstore.LeaseCancelled
			close(eventCh)
			return
		case resp, ok := <-leaseCh:
			if !ok {
				log.Errorf("Error receiving from lease ch for key %s.", key)
				eventCh <- kvstore.LeaseError
				close(eventCh)
				return
			}

			// check if we lost the lease
			if resp.ID == 0 {
				eventCh <- kvstore.LeaseLost
				close(eventCh)
				return
			}
		}
	}
}

// Lease takes a lease on a key with TTL and keeps it alive
func (e *etcdStore) Lease(ctx context.Context, key string, obj runtime.Object, ttl uint64) (chan kvstore.LeaseEvent, error) {
	// create a new lease
	resp, err := e.client.Lease.Grant(ctx, int64(ttl))
	if err != nil {
		log.Errorf("Error getting a grant: Err: %v", err)
		return nil, err
	}

	// encode the object
	value, err := e.encode(obj)
	if err != nil {
		return nil, err
	}

	// put the object using lease
	_, err = e.client.KV.Put(context.TODO(), key, string(value), clientv3.WithLease(resp.ID))
	if err != nil {
		log.Errorf("Error writing key %s to kvstore. Err: %v", key, err)
		return nil, err
	}

	// keep the lease alive
	leaseCh, err := e.client.Lease.KeepAlive(ctx, resp.ID)
	if err != nil {
		log.Errorf("Error during keepalive. Err: %v", err)
		return nil, err
	}

	// channel to send out events
	eventCh := make(chan kvstore.LeaseEvent, outCount)

	// watch the lease channel
	go e.runLeaseLoop(ctx, key, leaseCh, eventCh)

	return eventCh, nil
}

// Close closes client connection to store
func (e *etcdStore) Close() {
	if e.client != nil {
		e.client.Close()
		e.client = nil
	}
}
