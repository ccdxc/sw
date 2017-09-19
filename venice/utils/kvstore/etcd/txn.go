package etcd

import (
	"context"
	"sync"

	"github.com/coreos/etcd/clientv3"
	pb "github.com/coreos/etcd/etcdserver/etcdserverpb"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
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
	t.ops = append(t.ops, clientv3.OpDelete(key, clientv3.WithPrevKV()))
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
func (t *txn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	t.Lock()
	defer t.Unlock()

	newCtx, cancel := context.WithTimeout(ctx, timeout)
	resp, err := t.store.client.KV.Txn(newCtx).If(t.cmps...).Then(t.ops...).Commit()
	cancel()

	var ret kvstore.TxnResponse
	if err != nil {
		return ret, err
	}

	ret.Succeeded = resp.Succeeded
	for _, r := range resp.Responses {
		switch r.Response.(type) {
		case *pb.ResponseOp_ResponseDeleteRange:
			if item := r.GetResponseDeleteRange(); item != nil {
				for _, kv := range item.PrevKvs {
					into, err := t.store.codec.Decode(kv.Value[:], nil)
					if err != nil {
						return ret, errors.Wrap(err, "Delete failed")
					}
					ret.Responses = append(ret.Responses, kvstore.TxnOpResponse{Oper: kvstore.OperDelete, Key: string(kv.Key[:]), Obj: into})
				}
			}
		case *pb.ResponseOp_ResponsePut:
			if item := r.GetResponsePut(); item != nil {
				if item.PrevKv != nil {
					into, err := t.store.codec.Decode(item.PrevKv.Value[:], nil)
					t.store.objVersioner.SetVersion(into, uint64(item.Header.Revision))
					if err != nil {
						return ret, errors.Wrap(err, "Update failed")
					}
					ret.Responses = append(ret.Responses, kvstore.TxnOpResponse{Oper: kvstore.OperUpdate, Key: string(item.PrevKv.Key[:]), Obj: into})
				}
			}
		case *pb.ResponseOp_ResponseRange:
			if item := r.GetResponseRange(); item != nil {
				for _, kv := range item.Kvs {
					into, err := t.store.codec.Decode(kv.Value[:], nil)
					t.store.objVersioner.SetVersion(into, uint64(kv.Version))
					if err != nil {
						return ret, errors.Wrap(err, "Get failed")
					}
					ret.Responses = append(ret.Responses, kvstore.TxnOpResponse{Oper: kvstore.OperGet, Key: string(kv.Key[:]), Obj: into})
				}
			}
		}
	}

	if !resp.Succeeded {
		return ret, kvstore.NewTxnFailedError()
	}

	for _, obj := range t.objs {
		t.store.objVersioner.SetVersion(obj, uint64(resp.Header.Revision))
	}

	t.cmps = make([]clientv3.Cmp, 0)
	t.ops = make([]clientv3.Op, 0)
	t.objs = make([]runtime.Object, 0)

	return ret, nil
}

// IsEmpty returns true if the transaction is empty.
func (t *txn) IsEmpty() bool {
	return (len(t.cmps) == 0) && (len(t.ops) == 0) && (len(t.objs) == 0)
}

// AddComparator adds comparators to the transaction.
func (t *txn) AddComparator(cs ...kvstore.Cmp) {
	t.cmps = append(t.cmps, translateCmps(cs...)...)
}
