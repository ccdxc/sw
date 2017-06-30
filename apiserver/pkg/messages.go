package apisrvpkg

import (
	"context"
	"errors"

	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"

	apisrv "github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/utils/kvstore"
)

var (
	errNotImplemented = grpc.Errorf(codes.Unimplemented, "Operation not implemented")
)

// MessageHdlr is an representation of the message object.
type MessageHdlr struct {
	// transforms is a map of version transform functions indexed
	// by [from-version][to-version].
	transforms map[string]map[string]apisrv.TransformFunc
	// defaulter is the registered defaulting function for the message
	defualter apisrv.DefaulterFunc
	// validater is the registered validating function
	validater apisrv.ValidateFunc
	// keyFunc is the function that generates the key for this object
	keyFunc apisrv.KeyGenFunc
	// kvUpdateFunc handles updating the KV store
	kvUpdateFunc apisrv.UpdateKvFunc
	// txnUpdateFunc handles updating a KV store transaction.
	txnUpdateFunc apisrv.UpdateKvTxnFunc
	// kvGetFunc handles gets from the KV store for the object.
	kvGetFunc apisrv.GetFromKvFunc
	// kvDelFunc handles deleting the object from the KV store.
	kvDelFunc apisrv.DelFromKvFunc
	// txnDelFunc handles deleting the object as part of a KV store transaction.
	txnDelFunc apisrv.DelFromKvTxnFunc
	// Kind holds the kind of object.
	Kind string
}

// NewMessage creates a new message performing all initialization needed.
func NewMessage(kind string) apisrv.Message {
	return &MessageHdlr{Kind: kind, transforms: make(map[string]map[string]apisrv.TransformFunc)}
}

// WithTransform registers a Transform function for the the message. to, and from are the versions
//   involved and fn is the function being registered by the app to perform the trasnform.
func (m *MessageHdlr) WithTransform(from, to string, fn apisrv.TransformFunc) apisrv.Message {
	if v, ok := m.transforms[from]; ok {
		if v != nil {
			v[to] = fn
		}
		return m
	}
	m.transforms[from] = make(map[string]apisrv.TransformFunc)
	m.transforms[from][to] = fn
	return m
}

// WithValidate registers a function for the Validing the contents of the message.
func (m *MessageHdlr) WithValidate(fn apisrv.ValidateFunc) apisrv.Message {
	m.validater = fn
	return m
}

// WithDefaulter registers a function to apply custom defaults to teh message.
func (m *MessageHdlr) WithDefaulter(fn apisrv.DefaulterFunc) apisrv.Message {
	m.defualter = fn
	return m
}

// WithKvUpdater registers a function to handle KV store updates for the object.
//   Usually registered by the generated code.
func (m *MessageHdlr) WithKvUpdater(fn apisrv.UpdateKvFunc) apisrv.Message {
	m.kvUpdateFunc = fn
	return m
}

// WithKvTxnUpdater registers a function to handle KV store updates for the object
//   as part of a transaction. Usually registered by the generated code.
func (m *MessageHdlr) WithKvTxnUpdater(fn apisrv.UpdateKvTxnFunc) apisrv.Message {
	m.txnUpdateFunc = fn
	return m
}

// WithKvGetter registers a function to handle KV store retrieves for the object
//  typically registered by the generated code.
func (m *MessageHdlr) WithKvGetter(fn apisrv.GetFromKvFunc) apisrv.Message {
	m.kvGetFunc = fn
	return m
}

// WithKvDelFunc registers a function to handle KV store deletes for the object
//  typically registered by the generated code.
func (m *MessageHdlr) WithKvDelFunc(fn apisrv.DelFromKvFunc) apisrv.Message {
	m.kvDelFunc = fn
	return m
}

// WithKvTxnDelFunc registers a function to handle KV store deletes for the object
//  via a transaction. Typically registered by the generated code.
func (m *MessageHdlr) WithKvTxnDelFunc(fn apisrv.DelFromKvTxnFunc) apisrv.Message {
	m.txnDelFunc = fn
	return m
}

// GetKind returns the Kind of the object.
func (m *MessageHdlr) GetKind() string {
	return m.Kind
}

// GetKVKey returns the KV store key for the object.
func (m *MessageHdlr) GetKVKey(i interface{}, prefix string) (string, error) {
	if m.keyFunc == nil {
		return "", errNotImplemented
	}
	// TODO(sanjayt): Add validation to generated key (size, allowed characters etc.)
	return m.keyFunc(i, prefix), nil
}

// WriteToKvTxn is a wrapper around txnUpdateFunc to update the object in the KV store via a transaction.
func (m *MessageHdlr) WriteToKvTxn(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
	if m.txnUpdateFunc != nil {
		return m.txnUpdateFunc(ctx, txn, i, prefix, create)
	}
	return errNotImplemented
}

// WriteToKv is a wrapper around kvUpdateFunc to update the object in the KV store.
func (m *MessageHdlr) WriteToKv(ctx context.Context, i interface{}, prefix string, create bool) (interface{}, error) {
	if m.kvUpdateFunc != nil {
		return m.kvUpdateFunc(ctx, singletonAPISrv.kv, i, prefix, create)
	}
	return nil, errNotImplemented
}

// GetFromKv is a wrapper around kvGetFunc to get the object in the KV store.
func (m *MessageHdlr) GetFromKv(ctx context.Context, key string) (interface{}, error) {
	if m.kvGetFunc != nil {
		return m.kvGetFunc(ctx, singletonAPISrv.kv, key)
	}
	return nil, errNotImplemented
}

// DelFromKv is a wrapper around kvDelFunc to delete the object in the KV store.
func (m *MessageHdlr) DelFromKv(ctx context.Context, key string) (interface{}, error) {
	if m.kvDelFunc != nil {
		return m.kvDelFunc(ctx, singletonAPISrv.kv, key)
	}
	return nil, singletonAPISrv.kv.Delete(ctx, key, nil)
}

// DelFromKvTxn is a wrapper around kvDelFunc to delete the object in the KV store.
func (m *MessageHdlr) DelFromKvTxn(ctx context.Context, txn kvstore.Txn, key string) error {
	if m.txnDelFunc != nil {
		return m.txnDelFunc(ctx, txn, key)
	}
	return errNotImplemented
}

// WithKeyGenerator registers a Key Generation function for the omessage.
func (m *MessageHdlr) WithKeyGenerator(fn apisrv.KeyGenFunc) apisrv.Message {
	m.keyFunc = fn
	return m
}

// PrepareMsg applies needed transforms to transform the message to the "to" version. Used
//  for request and response but in opposite directions.
func (m *MessageHdlr) PrepareMsg(from, to string, i interface{}) (interface{}, error) {
	if v, ok := m.transforms[from]; ok {
		if fn, ok := v[to]; ok {
			return fn(from, to, i), nil
		}
	}
	return nil, errors.New("unsupported tranformation")
}

// Default Applies Defaults to the Message if any custom defaulter was registered.
func (m *MessageHdlr) Default(i interface{}) interface{} {
	if m.defualter != nil {
		return m.defualter(i)
	}
	return i
}

// Validate is a wrapper around the validater function registered.
func (m *MessageHdlr) Validate(i interface{}) error {
	if m.validater != nil {
		return m.validater(i)
	}
	return nil
}
