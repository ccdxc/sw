package etcd

import (
	"context"
	"sync"

	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

// txn holds state for a transaction.
type txn struct {
	sync.Mutex

	store *etcdStore // kv store
	cmps  []clientv3.Cmp
	ops   []clientv3.Op
	objs  []runtime.Object
}

// newTxn creates a new transaction.
func (e *etcdStore) newTxn() kvstore.Txn {
	return &txn{
		store: e,
		cmps:  make([]clientv3.Cmp, 0),
		ops:   make([]clientv3.Op, 0),
		objs:  make([]runtime.Object, 0),
	}
}

// Create stages an object creation in a transaction.
func (t *txn) Create(key string, obj runtime.Object) error {
	t.Lock()
	defer t.Unlock()

	value, err := t.store.encode(obj)
	if err != nil {
		return err
	}

	t.cmps = append(t.cmps, clientv3.Compare(clientv3.ModRevision(key), "=", 0))
	t.ops = append(t.ops, clientv3.OpPut(key, string(value)))
	t.objs = append(t.objs, obj)
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *txn) Delete(key string, cs ...kvstore.Cmp) error {
	t.Lock()
	defer t.Unlock()

	t.cmps = append(t.cmps, translateCmps(cs...)...)
	t.ops = append(t.ops, clientv3.OpDelete(key))
	return nil
}

// Update stages an object update in a transaction.
func (t *txn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	t.Lock()
	defer t.Unlock()

	value, err := t.store.encode(obj)
	if err != nil {
		return err
	}

	t.cmps = append(t.cmps, translateCmps(cs...)...)
	t.ops = append(t.ops, clientv3.OpPut(key, string(value)))
	t.objs = append(t.objs, obj)

	return nil
}

// Commit tries to commit the transaction.
func (t *txn) Commit(ctx context.Context) error {
	t.Lock()
	defer t.Unlock()

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := t.store.client.KV.Txn(newCtx).If(t.cmps...).Then(t.ops...).Commit()
	cancel()

	if err != nil {
		return err
	}

	if !resp.Succeeded {
		return kvstore.NewTxnFailedError()
	}

	for _, obj := range t.objs {
		t.store.objVersioner.SetVersion(obj, uint64(resp.Header.Revision))
	}

	t.cmps = make([]clientv3.Cmp, 0)
	t.ops = make([]clientv3.Op, 0)
	t.objs = make([]runtime.Object, 0)

	return nil
}
