package kvstore

import (
	"context"

	"github.com/pensando/sw/utils/runtime"
)

// Operation is the type of Kv Store operation.
type Operation uint8

const (
	OperUnknown Operation = iota
	OperUpdate
	OperGet
	OperDelete
)

// TxnResponse is a response from a transaction.
type TxnResponse struct {
	Succeeded bool
	Responses []TxnOpResponse
}

// TxnOpResponse is a response from a operation in a transaction.
type TxnOpResponse struct {
	Oper Operation
	Key  string
	Obj  runtime.Object
}

// Txn is an interface on top of key value store for:
//
// a) creating/updating/deleting keys conditionally
// b) creating/updating/deleting multiple keys atomically
//
// Txn has an all-or-none semantic when Commit is issued.
type Txn interface {
	// Create stages an object creation in a transaction.
	Create(key string, obj runtime.Object) error

	// Delete stages an object deletion in a transaction.
	Delete(key string, cs ...Cmp) error

	// Update stages an object update in a transaction.
	Update(key string, obj runtime.Object, cs ...Cmp) error

	// Commit tries to commit the transaction.
	Commit(ctx context.Context) (TxnResponse, error)

	// IsEmpty returns true if the Txn is empty
	IsEmpty() bool

	// AddComparator adds a condition for the transaction.
	AddComparator(cs ...Cmp)
}
