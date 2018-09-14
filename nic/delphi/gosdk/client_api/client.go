package clientApi

import "github.com/pensando/sw/nic/delphi/proto/delphi"

// Service Implement this to use the sdk
type Service interface {
	OnMountComplete()
	Name() string
}

// MountListener is the interface clients have to implement if the want to be
// notified of MountComplete
type MountListener interface {
	OnMountComplete()
}

// Client This is the main SDK Client API
type Client interface {
	// Mount a kind to get notifications and/or make changes to the objects.
	// This *MUST* be called before the `Dial` function
	MountKind(kind string, mode delphi.MountMode) error
	// Mount a kind,key to get notifications and/or make changes to the objects.
	// This *MUST* be called before the `Dial` function
	MountKindKey(kind string, key string, mode delphi.MountMode) error
	// Dial establishes connection to the HUB
	Dial() error
	// SetObject notifies about changes to an object. The user doesn't need to
	// call this explicitly. It is getting called automatically when there is a
	// change in any object.
	SetObject(obj BaseObject) error
	// GetObject returns the object of kind `kind` with key `key` if it
	// exists in the local database, else it return nil
	GetObject(kind string, key string) BaseObject
	// Delete object, as it names sugests, deletes an object from the database.
	// Users can use this, or just call <OBJECT>.Delete()
	DeleteObject(obj BaseObject) error
	// WatchKind is used internally by the object to register reactors. Users
	// should not call this directly
	WatchKind(kind string, reactor BaseReactor) error
	// WatchMount allows users to register extra onMount callbacks
	WatchMount(listener MountListener) error
	// List all the objects in the database of a specific kind
	List(kind string) []BaseObject
	// Close, as the name suggests, closes the connection to the hub.
	Close()
	// DumpSubtrees prints the local database state to stderr. It's meant to be
	// used for debugging purposes.
	DumpSubtrees()
}
