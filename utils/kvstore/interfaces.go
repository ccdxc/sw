package kvstore

import (
	"github.com/pensando/sw/utils/runtime"
)

// UpdateFunc is the signature for a function used to update an object. It is
// used with Update to retry updates.
type UpdateFunc func(oldObj runtime.Object) (newObj runtime.Object, err error)

// Interface for a key value store.
type Interface interface {
	// Create instantiates an object with the provided key, if it doesn't
	// already exist. ttl is the time-to-live in seconds, ttl of 0 means the
	// key doesn't expire. If "into" is not nil, it is set to the value
	// returned by the kv store.
	Create(key string, obj runtime.Object, ttl int64, into runtime.Object) error

	// Delete removes a key, if it exists. If "into" is not nil, it is set
	// to the last known value in the kv store.
	Delete(key string, into runtime.Object) error

	// AtomicDelete removes a key, only if it exists with the specified
	// version. If "into" is not nil, it is set to the last known value in
	// the kv store.
	AtomicDelete(key string, prevVersion string, into runtime.Object) error

	// PrefixDelete removes all keys with the matching prefix. Since it is
	// meant to be used for deleting prefixes only, a "/" is added at the
	// end of the prefix if it doesn't exist. For example, a delete with
	// "/abc" prefix would only delete "/abc/123" and "/abc/456", but not
	// "/abcd".
	PrefixDelete(prefix string) error

	// Update modifies an existing object. If the key does not exist, update
	// returns an error. This should only be used if a single writer owns
	// the key.
	Update(key string, obj runtime.Object, ttl int64, into runtime.Object) error

	// AtomicUpdate modifies an existing object, only if the provided
	// previous version matches the existing version of the key. This is
	// useful for implementing elections using a single ttl key. The winner
	// refreshes TTL on the key only if it hasn't been taken over by another
	// node.
	AtomicUpdate(key string, obj runtime.Object, prevVersion string, ttl int64, into runtime.Object) error

	// ConsistentUpdate modifies an existing object by invoking the provided
	// update function. This should be used when there are multiple writers
	// to various parts of the object and the updates need to be done in a
	// consistent manner. "into" parameter is mandatory for this call.
	// Example:
	// Writer1 updates field f1 to v1.
	// Writer2 updates field f2 to v2 at the same time.
	// ConsistentUpdate guarantees that the object lands in a consistent
	// state where f1=v1 and f2=v2.
	ConsistentUpdate(key string, ttl int64, into runtime.Object, updateFunc UpdateFunc) error

	// Get the object corresponding to the provided key, if it exists.
	Get(key string, into runtime.Object) error

	// List the objects corresponding to a prefix. It is assumed that all
	// the keys under this prefix are homogenous. "into" should point to
	// a List object and should have an "Items" slice for individual
	// objects.
	List(prefix string, into runtime.Object) error
}
