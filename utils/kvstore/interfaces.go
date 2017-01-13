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

	// Watch the object corresponding to a key. fromVersion is the version
	// to start the watch from. If fromVersion is 0, it will return the
	// existing object and watch for changes from the returned version.
	Watch(key string, fromVersion string) (Watcher, error)

	// PrefixWatch watches changes on all objects corresponding to a prefix
	// key. fromVersion is the version to start the watch from. If
	// fromVersion is 0, it will return the existing objects and watch for
	// changes from the returned version.
	// TODO: Filter objects
	PrefixWatch(prefix string, fromVersion string) (Watcher, error)

	// Contest creates a new contender in an election. name is the name of
	// the election. id is the identifier of the contender. When a leader is
	// elected, the leader's lease is automatically refreshed. ttl is the
	// timeout for lease refresh. If the leader does not update the lease
	// for ttl duration, a new election is performed.
	Contest(name string, id string, ttl uint64) (Election, error)
}

// WatchEventType defines possible types of events for a watch.
type WatchEventType string

const (
	// Created is an event to indicate an object is created
	Created WatchEventType = "Created"
	// Updated is an event to indicate an object is updated.
	Updated WatchEventType = "Updated"
	// Deleted is an event to indicate an object is deleted.
	Deleted WatchEventType = "Deleted"
	// WatcherError is an event to indicate an error with watch. Watch must
	// be re-established when this happens.
	WatcherError WatchEventType = "Error"
)

// WatchEvent contains information about a single event on watched object(s)
type WatchEvent struct {
	// Type of the watch event.
	Type WatchEventType

	// For a Deleted event, this is previous version of the object. For an
	// Error event, it is undefined. For all other events, it is the latest
	// version of the object.
	Object runtime.Object
}

// Watcher is an interface that can be implemented to keep track of changes to
// objects in a key value store. Any watcher that implements this interface must
// start the watch when the watcher is created.
type Watcher interface {
	// EventChan returns the channel to receive events on. If there is an
	// error with the watch or when Stop is called, this channel will be
	// closed.
	EventChan() <-chan *WatchEvent

	// Stop stops the watch and closes the channel returned by EventChan().
	Stop()
}

// ElectionEventType defines possible types of events for an election.
type ElectionEventType string

const (
	// Elected is an event to indicate this node won the election.
	Elected ElectionEventType = "Elected"
	// Lost is an event to indicate this node lost the election after
	// having won it.
	Lost ElectionEventType = "Lost"
	// Changed is an event to indicate the leader has changed..
	Changed ElectionEventType = "Changed"
	// ElectionError is an event to indicate there is an error with the leader
	// election.
	ElectionError ElectionEventType = "Error"
)

// ElectionEvent contains information about a single event with leader election.
type ElectionEvent struct {
	// Type of the election event.
	Type ElectionEventType

	// Leader is populated with the current leader, if there is one.
	Leader string
}

// Election is an interface that can be implemented to elect a leader using the
// key value store. A contender becomes a leader by successful atomic creation
// of a key.
//
// Any implementation of this interface must do the following:
// - Start contending when the object is created.
// - Renew the lease for the leader automatically.
type Election interface {
	// EventChan returns results of leader election.
	EventChan() <-chan *ElectionEvent

	// Stop participating in leader election.
	Stop()

	// Id returns the identifier of the contender.
	Id() string

	// Leader returns the id of the leader.
	Leader() string

	// IsLeader returns true if the current node is the leader, false
	// otherwise.
	IsLeader() bool
}
