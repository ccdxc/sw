package memkv

import (
	"context"
	"sync"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

type opType int

const (
	// A default op has opType 0, which is invalid.
	tCreate opType = iota + 1
	tUpdate
	tDelete
)

type op struct {
	t   opType
	key string
	val string
	obj runtime.Object
}

func opDelete(key string) op {
	ret := op{t: tDelete, key: key}
	return ret
}
func opCreate(key, val string, obj runtime.Object) op {
	ret := op{t: tCreate, key: key, val: val, obj: obj}
	return ret
}
func opUpdate(key, val string, obj runtime.Object) op {
	ret := op{t: tUpdate, key: key, val: val, obj: obj}
	return ret
}

type txn struct {
	sync.Mutex

	store *memKv // kv store
	cmps  []kvstore.Cmp
	ops   []op
}

// newTxn creates a new transaction.
func (f *memKv) newTxn() kvstore.Txn {
	return &txn{
		store: f,
		cmps:  make([]kvstore.Cmp, 0),
		ops:   make([]op, 0),
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

	t.ops = append(t.ops, opCreate(key, string(value), obj))
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *txn) Delete(key string, cs ...kvstore.Cmp) error {
	t.Lock()
	defer t.Unlock()

	t.cmps = append(t.cmps, cs...)
	t.ops = append(t.ops, opDelete(key))
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

	t.cmps = append(t.cmps, cs...)
	t.ops = append(t.ops, opUpdate(key, string(value), obj))

	return nil
}

// Commit tries to commit the transaction.
func (t *txn) Commit(ctx context.Context) error {
	t.Lock()
	defer t.Unlock()

	err := t.store.commitTxn(t)

	t.cmps = make([]kvstore.Cmp, 0)
	t.ops = make([]op, 0)
	return err
}
