package memkv

import (
	"context"

	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

// TODO: Need memkv implementation for a txn. Stubs to help compile.
// txn holds state for a transaction.
type txn struct {
}

// newTxn creates a new transaction.
func (f *memKv) newTxn() kvstore.Txn {
	return &txn{}
}

// Create stages an object creation in a transaction.
func (t *txn) Create(key string, obj runtime.Object) error {
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *txn) Delete(key string, cs ...kvstore.Cmp) error {
	return nil
}

// Update stages an object update in a transaction.
func (t *txn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	return nil
}

// Commit tries to commit the transaction.
func (t *txn) Commit(ctx context.Context) error {
	return nil
}
