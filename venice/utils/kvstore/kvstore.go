package kvstore

import (
	"context"

	"github.com/pensando/sw/venice/utils/runtime"
)

// UpdateFunc is the signature for a function used to update an object. It is
// used with Update to retry updates.
type UpdateFunc func(oldObj runtime.Object) (newObj runtime.Object, err error)

// Interface for a key value store.
type Interface interface {
	// Create instantiates an object with the provided key, if it doesn't
	// already exist. ResourceVersion in obj is set to the value returned
	// by the kv store.
	Create(ctx context.Context, key string, obj runtime.Object) error

	// Delete removes a key, if it exists. If "into" is not nil, it is set
	// to the last known value in the kv store. "cs" are comparators to
	// allow for conditional deletes.
	Delete(ctx context.Context, key string, into runtime.Object, cs ...Cmp) error

	// PrefixDelete removes all keys with the matching prefix. Since it is
	// meant to be used for deleting prefixes only, a "/" is added at the
	// end of the prefix if it doesn't exist. For example, a delete with
	// "/abc" prefix would only delete "/abc/123" and "/abc/456", but not
	// "/abcd".
	PrefixDelete(ctx context.Context, prefix string) error

	// Update modifies an existing object. If the key does not exist, update
	// returns an error. This can be used without comparators if a single
	// writer owns the key. "cs" are comparators to allow for conditional
	// updates, including parallel updates.
	Update(ctx context.Context, key string, obj runtime.Object, cs ...Cmp) error

	// ConsistentUpdate modifies an existing object by invoking the provided
	// update function. This should be used when there are multiple writers
	// to various parts of the object and the updates need to be done in a
	// consistent manner. "into" parameter is mandatory for this call.
	// Example:
	// Writer1 updates field f1 to v1.
	// Writer2 updates field f2 to v2 at the same time.
	// ConsistentUpdate guarantees that the object lands in a consistent
	// state where f1=v1 and f2=v2.
	ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc UpdateFunc) error

	// Get the object corresponding to the provided key, if it exists.
	Get(ctx context.Context, key string, into runtime.Object) error

	// List the objects corresponding to a prefix. It is assumed that all
	// the keys under this prefix are homogenous. "into" should point to
	// a List object and should have an "Items" slice for individual
	// objects.
	List(ctx context.Context, prefix string, into runtime.Object) error

	// Watch the object corresponding to a key. fromVersion is the version
	// to start the watch from. If fromVersion is 0, it will return the
	// existing object and watch for changes from the returned version.
	Watch(ctx context.Context, key string, fromVersion string) (Watcher, error)

	// PrefixWatch watches changes on all objects corresponding to a prefix
	// key. fromVersion is the version to start the watch from. If
	// fromVersion is 0, it will return the existing objects and watch for
	// changes from the returned version.
	// TODO: Filter objects
	PrefixWatch(ctx context.Context, prefix string, fromVersion string) (Watcher, error)

	// Contest creates a new contender in an election. name is the name of
	// the election. id is the identifier of the contender. When a leader is
	// elected, the leader's lease is automatically refreshed. ttl is the
	// timeout for lease refresh. If the leader does not update the lease
	// for ttl duration, a new election is performed.
	Contest(ctx context.Context, name string, id string, ttl uint64) (Election, error)

	// NewTxn creates a transaction object.
	NewTxn() Txn

	// Close closes any persistent connection for good cleanup
	Close()
}
