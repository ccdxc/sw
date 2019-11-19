package apiintf

import (
	"context"
	"io"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Operations allowed by the API server on objects
const (
	CreateOper  APIOperType = "create"
	UpdateOper  APIOperType = "update"
	GetOper     APIOperType = "get"
	DeleteOper  APIOperType = "delete"
	ListOper    APIOperType = "list"
	WatchOper   APIOperType = "watch"
	UnknownOper APIOperType = "unknown"
)

// APIOperType defines possible options on an API object
type APIOperType string

// Types of References
const (
	NamedReference    ReferenceType = "NamedRef"
	WeakReference     ReferenceType = "WeakRef"
	SelectorReference ReferenceType = "SelectorRef"
)

// ObjRelation defines a relation between objects
type ObjRelation struct {
	Type  ReferenceType
	To    string
	Field string
}

// ReferenceType defines types of object references
type ReferenceType string

// Snapshot related constants
const (
	InvalidSnapShotHandle = 0
)

// ObjectStat gives information about a object key in the cache
type ObjectStat struct {
	Key        string
	Valid      bool
	InOverlay  bool
	Revision   uint64
	LastUpd    time.Time
	TypeMeta   api.TypeMeta
	ObjectMeta api.ObjectMeta
}

// SuccessCbFunc is called when a Store operation succeeds. The call happens in
// the critical path hence the function should the min needed.
type SuccessCbFunc func(key string, obj, prev runtime.Object)

// CacheInterface is the interface satisfied by any API cache
type CacheInterface interface {
	kvstore.Interface
	Stat(ctx context.Context, keys []string) []ObjectStat
	StatKind(group string, kind string) ([]ObjectStat, error)
	Start() error
	Restore() error
	Clear()
	GetKvConn() kvstore.Interface
	DebugAction(action string, params []string) string
	StartSnapshot() uint64
	DeleteSnapshot(uint64) error
	SnapshotReader(uint64 uint64, include bool, kinds []string) (io.ReadCloser, error)
	SnapshotWriter(reader io.Reader) SnapshotWriter
	Rollback(ctx context.Context, rev uint64, kvs kvstore.Interface) error
}

// SnapshotWriter is a interface to write a configuration snapshot to kvstore.
type SnapshotWriter interface {
	Write(ctx context.Context, kvs kvstore.Interface) error
}

// OverlayStatus is status of Overlay verification including objects in the overlay
// and verification results.
type OverlayStatus struct {
	Items  []OverlayItem
	Failed []FailedVerification
}

// OverlayItem is each item in the Overlay
type OverlayItem struct {
	Key    OverlayKey
	Object runtime.Object
}

// OverlayKey identifies a overlay opject
type OverlayKey struct {
	Oper string
	URI  string
}

// FailedVerification is Object that failed verification in the staging buffer
type FailedVerification struct {
	Key    OverlayKey
	Errors []error
}

// ReferenceObj represents a Reference from an API object to another.
type ReferenceObj struct {
	RefType ReferenceType
	RefKind string
	Refs    []string
}

// OverlayInterface is Cache Interface with overlay functionality overlayed.
type OverlayInterface interface {
	CacheInterface
	CreatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object) error
	UpdatePrimary(ctx context.Context, service, method, uri, key string, resVer string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error
	DeletePrimary(ctx context.Context, service, method, uri, key string, orig, into runtime.Object) error
	Commit(ctx context.Context, items []OverlayKey) error
	ClearBuffer(ctx context.Context, items []OverlayKey) error
	Verify(ctx context.Context) (OverlayStatus, error)
	GetRequirements() RequirementSet
	NewWrappedTxn(ctx context.Context) kvstore.Txn
}

// ReqType specifies the type of requirement
type ReqType int

// Requirement types
const (
	Reference        ReqType = 1
	ConsistentUpdate ReqType = 2
)

// String returns a human friendly string
func (r ReqType) String() string {
	switch r {
	case Reference:
		return "Reference"
	case ConsistentUpdate:
		return "ConsistentUpdate"
	default:
		return "unknown"
	}
}

// RequirementSet is a collection Requirement objects
type RequirementSet interface {
	// Check checks if the requirement has been met by all requirements in the collection
	Check(ctx context.Context) []error
	// CheckOne checks if requirements have been met for one requiement in the collection
	CheckOne(ctx context.Context, key string) (errors []error, found bool)
	// Apply applies the requirements to the transaction provided.
	//  All the requirements in the collection are called to Apply
	Apply(ctx context.Context, txn kvstore.Txn, cache CacheInterface) []error
	// Finalize call Requirement s in the set to the Finalize their requirements
	Finalize(ctx context.Context) []error
	// Clear clears all accumulated requirements in the set
	Clear(ctx context.Context)

	// NewRefRequirement creates a new reference related requirement and adds it to the list of requirements.
	NewRefRequirement(oper APIOperType, key string, reqs map[string]ReferenceObj) Requirement

	// NewConsUpdateRequirement creates a new consistent update requirement and adds it to the list of requirements
	//  the consistent update operations are applied in the order speciefied in the slice passed in
	NewConsUpdateRequirement(reqs []ConstUpdateItem) Requirement

	// AddRequirement adds a new requirement to the set
	AddRequirement(tpe ReqType, key string, req Requirement)

	// String gives out a string representation of the requirements for debug/display purposes
	String() string
}

// Requirement defines the interface for a API requirement
type Requirement interface {
	// Check checks if the requirement has been met.
	Check(ctx context.Context) []error
	// Apply applies the requirements to the transaction provided.
	//  The implementation would add all kvstore operations dictated by the
	//  requirement to the transaction.
	Apply(ctx context.Context, txn kvstore.Txn, cache CacheInterface) error
	// Finalize is call for the implementation to take action after the requirements
	//  have been applied.
	Finalize(ctx context.Context) error

	// String gives out a string representation of the requirements for debug/display purposes
	String() string
}

// ConstUpdateItem is an item for the consistent update requirement
type ConstUpdateItem struct {
	Key             string
	Func            kvstore.UpdateFunc
	Into            runtime.Object
	ResourceVersion string
}

// Store is the interface for the Local Object Store
type Store interface {
	Set(key string, rev uint64, obj runtime.Object, cb SuccessCbFunc) error
	Get(key string) (runtime.Object, error)
	GetFromSnapshot(rev uint64, key string) (runtime.Object, error)
	Delete(key string, rev uint64, cb SuccessCbFunc) (runtime.Object, error)
	List(key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error)
	ListFromSnapshot(rev uint64, key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error)
	Mark(key string)
	Sweep(key string, cb SuccessCbFunc)
	PurgeDeleted(past time.Duration)
	Stat(key []string) []ObjectStat
	StatAll(prefix string) []ObjectStat
	StartSnapshot() uint64
	DeleteSnapshot(uint64) error
	ListSnapshotWithCB(pfix string, rev uint64, cbfunc func(key string, cur, revObj runtime.Object, deleted bool) error) error
	Clear()
}

// EventHandlerFn is for handling watch events
type EventHandlerFn func(inctx context.Context, evType kvstore.WatchEventType, item, prev runtime.Object)
