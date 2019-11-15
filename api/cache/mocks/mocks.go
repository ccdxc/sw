package mocks

import (
	"context"
	"fmt"
	"io"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// FakeCache is mock implementation
type FakeCache struct {
	FakeKvStore
	ListFilteredFn    func(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error
	StatFn            func(keys []string) []apiintf.ObjectStat
	Kvconn            kvstore.Interface
	StartSnapshotFn   func() uint64
	RetSnapshotReader io.ReadCloser
	RetSnapshotWriter apiintf.SnapshotWriter
	StatKindFn        func(group string, kind string) ([]apiintf.ObjectStat, error)
	SnapWriter        apiintf.SnapshotWriter
	RollbackCalls     int
}

// ListFiltered is mock implementation
func (f *FakeCache) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	if f.ListFilteredFn != nil {
		return f.ListFilteredFn(ctx, prefix, into, opts)
	}
	return nil
}

// Watchfiltered is mock implementation
func (f *FakeCache) Watchfiltered(ctx context.Context, key string, opts api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}

// Start is mock implementation
func (f *FakeCache) Start() error {
	return nil
}

// Restore is mock implementation
func (f *FakeCache) Restore() error {
	return nil
}

// Stat is mock implementation
func (f *FakeCache) Stat(tx context.Context, keys []string) []apiintf.ObjectStat {
	if f.StatFn != nil {
		return f.StatFn(keys)
	}
	return nil
}

// StatKind is mock implementation
func (f *FakeCache) StatKind(group string, kind string) ([]apiintf.ObjectStat, error) {
	if f.StatKindFn != nil {
		return f.StatKindFn(group, kind)
	}
	return nil, fmt.Errorf("unimplemented")
}

// DebugAction is a mock implementation
func (f *FakeCache) DebugAction(action string, params []string) string {
	return ""
}

// Clear is mock implementation
func (f *FakeCache) Clear() {
}

// GetKvConn is mock implementation
func (f *FakeCache) GetKvConn() kvstore.Interface {
	return f.Kvconn
}

// StartSnapshot is a mock implementation
func (f *FakeCache) StartSnapshot() uint64 {
	if f.StartSnapshotFn != nil {
		return f.StartSnapshotFn()
	}
	return 0
}

// DeleteSnapshot is a mock implementation
func (f *FakeCache) DeleteSnapshot(uint64) error {
	return nil
}

// SnapshotReader is a mock implementation
func (f *FakeCache) SnapshotReader(uint64 uint64, include bool, kinds []string) (io.ReadCloser, error) {
	return f.RetSnapshotReader, nil
}

// SnapshotWriter is a mock implementation
func (f *FakeCache) SnapshotWriter(reader io.Reader) apiintf.SnapshotWriter {
	return f.SnapWriter
}

// Rollback is a mock implementation
func (f *FakeCache) Rollback(ctx context.Context, rev uint64, kvs kvstore.Interface) error {
	f.RollbackCalls++
	return nil
}

// FakeKvStore is mock implementation
type FakeKvStore struct {
	Creates, Deletes               uint64
	PrefixDeletes, Updates         uint64
	Consupdates, Gets, Lists       uint64
	Closes, Watches, Prefixwatches uint64
	Contests, Leases, Newtxns      uint64
	Watchfiltereds, Listfiltereds  uint64
	Createfn                       func(ctx context.Context, key string, obj runtime.Object) error
	Deletefn                       func(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error
	Updatefn                       func(ctx context.Context, key string, obj runtime.Object) error
	Watchfn                        func(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error)
	Fwatchfn                       func(ctx context.Context, key string, opts api.ListWatchOptions) kvstore.Watcher
	Getfn                          func(ctx context.Context, key string, into runtime.Object) error
	Listfn                         func(ctx context.Context, prefix string, into runtime.Object) error
	Txn                            *FakeTxn
}

// Create is mock implementation
func (f *FakeKvStore) Create(ctx context.Context, key string, obj runtime.Object) error {
	f.Creates++
	if f.Createfn != nil {
		return f.Createfn(ctx, key, obj)
	}
	return nil
}

// Delete is mock implementation
func (f *FakeKvStore) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	f.Deletes++
	if f.Deletefn != nil {
		return f.Deletefn(ctx, key, into, cs...)
	}
	return nil
}

// PrefixDelete is mock implementation
func (f *FakeKvStore) PrefixDelete(ctx context.Context, prefix string) error {
	f.PrefixDeletes++
	return nil
}

// Update is mock implementation
func (f *FakeKvStore) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	f.Updates++
	if f.Updatefn != nil {
		return f.Updatefn(ctx, key, obj)
	}
	return nil
}

// ConsistentUpdate is mock implementation
func (f *FakeKvStore) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	f.Consupdates++
	if f.Updatefn != nil {
		return f.Updatefn(ctx, key, into)
	}
	return nil
}

// Get is mock implementation
func (f *FakeKvStore) Get(ctx context.Context, key string, into runtime.Object) error {
	f.Gets++
	if f.Getfn != nil {
		return f.Getfn(ctx, key, into)
	}
	return nil
}

// List is mock implementation
func (f *FakeKvStore) List(ctx context.Context, prefix string, into runtime.Object) error {
	f.Lists++
	if f.Listfn != nil {
		return f.Listfn(ctx, prefix, into)
	}
	return nil
}

// ListFiltered is mock implementation
func (f *FakeKvStore) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	return nil
}

// Watch is mock implementation
func (f *FakeKvStore) Watch(ctx context.Context, key string, fromVersion string) (kvstore.Watcher, error) {
	f.Watches++
	if f.Watchfn != nil {
		return f.Watchfn(ctx, key, fromVersion)
	}
	return nil, nil
}

// WatchFiltered is mock implementation
func (f *FakeKvStore) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) (kvstore.Watcher, error) {
	f.Watchfiltereds++
	if f.Fwatchfn != nil {
		return f.Fwatchfn(ctx, key, opts), nil
	}
	return nil, nil
}

// PrefixWatch is mock implementation
func (f *FakeKvStore) PrefixWatch(ctx context.Context, prefix string, fromVersion string) (kvstore.Watcher, error) {
	f.Prefixwatches++
	if f.Watchfn != nil {
		return f.Watchfn(ctx, prefix, fromVersion)
	}
	return nil, nil
}

// Contest is mock implementation
func (f *FakeKvStore) Contest(ctx context.Context, name string, id string, ttl uint64) (kvstore.Election, error) {
	f.Contests++
	return nil, nil
}

// Lease is mock implementation
func (f *FakeKvStore) Lease(ctx context.Context, key string, obj runtime.Object, ttl uint64) (chan kvstore.LeaseEvent, error) {
	f.Leases++
	ch := make(chan kvstore.LeaseEvent)
	return ch, nil
}

// NewTxn is mock implementation
func (f *FakeKvStore) NewTxn() kvstore.Txn {
	f.Newtxns++
	return f.Txn
}

// Close is mock implementation
func (f *FakeKvStore) Close() {
	f.Closes++
}

// Reset is mock implementation
func (f *FakeKvStore) Reset() {
	f.Creates = 0
	f.Deletes = 0
	f.PrefixDeletes = 0
	f.Updates = 0
	f.Consupdates = 0
	f.Gets = 0
	f.Lists = 0
	f.Watches = 0
	f.Prefixwatches = 0
	f.Contests = 0
	f.Leases = 0
	f.Newtxns = 0
	f.Closes = 0
}

// FakeStore is mock implementation
type FakeStore struct {
	Sets, Gets, Deletes       uint64
	Lists, Flushes, Stats     uint64
	Marks, Sweeps, DelDeleted uint64
	Setfn                     func(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) error
	Getfn                     func(key string) (runtime.Object, error)
	Deletefn                  func(key string, rev uint64, cb apiintf.SuccessCbFunc) (runtime.Object, error)
	Listfn                    func(key string, opts api.ListWatchOptions) []runtime.Object
	ListFromSnapshotFn        func(rev uint64, key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error)
}

// Set is mock implementation
func (f *FakeStore) Set(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) error {
	f.Sets++
	if f.Setfn != nil {
		return f.Setfn(key, rev, obj, cb)
	}
	return nil
}

// Get is mock implementation
func (f *FakeStore) Get(key string) (runtime.Object, error) {
	f.Gets++
	if f.Getfn != nil {
		return f.Getfn(key)
	}
	return nil, nil
}

// Delete is mock implementation
func (f *FakeStore) Delete(key string, rev uint64, cb apiintf.SuccessCbFunc) (runtime.Object, error) {
	f.Deletes++
	if f.Deletefn != nil {
		f.Deletefn(key, rev, cb)
	}
	return nil, nil
}

// List is mock implementation
func (f *FakeStore) List(key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	f.Lists++
	if f.Listfn != nil {
		return f.Listfn(key, opts), nil
	}
	var ret []runtime.Object
	return ret, nil
}

// Mark is mock implementation
func (f *FakeStore) Mark(key string) {
	f.Marks++
}

// Sweep is mock implementation
func (f *FakeStore) Sweep(key string, cb apiintf.SuccessCbFunc) {
	f.Sweeps++
}

// PurgeDeleted is mock implementation
func (f *FakeStore) PurgeDeleted(past time.Duration) {
	f.DelDeleted++
}

// Stat is a mock implementation
func (f *FakeStore) Stat(keys []string) []apiintf.ObjectStat {
	f.Stats++
	return nil
}

// StatAll is a mock implementation
func (f *FakeStore) StatAll(prefix string) []apiintf.ObjectStat {
	return nil
}

// Clear is mock implementation
func (f *FakeStore) Clear() {
	f.Flushes++
}

// GetFromSnapshot is a mock implementation
func (f *FakeStore) GetFromSnapshot(rev uint64, key string) (runtime.Object, error) {
	return nil, fmt.Errorf("not implemented")
}

// ListFromSnapshot is a mock implementation
func (f *FakeStore) ListFromSnapshot(rev uint64, key, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	if f.ListFromSnapshotFn != nil {
		return f.ListFromSnapshotFn(rev, key, kind, opts)
	}
	return nil, fmt.Errorf("not implemented")
}

// StartSnapshot is a mock implementation
func (f *FakeStore) StartSnapshot() uint64 {
	return 0
}

// DeleteSnapshot is a mock implementation
func (f *FakeStore) DeleteSnapshot(uint64) error {
	return nil
}

// ListSnapshotWithCB is a mock implementation
func (f *FakeStore) ListSnapshotWithCB(pfix string, rev uint64, cbfunc func(key string, cur, revObj runtime.Object, deleted bool) error) error {
	return nil
}

// Reset is mock implementation
func (f *FakeStore) Reset() {
	f.Sets = 0
	f.Gets = 0
	f.Deletes = 0
	f.Lists = 0
	f.Flushes = 0
	f.Marks = 0
	f.Sweeps = 0
}

// FakeOps is mock implementation
type FakeOps struct {
	Op  string
	Key string
	Obj runtime.Object
}

// FakeTxn is mock implementation
type FakeTxn struct {
	Ops                 []FakeOps
	Cmps                []kvstore.Cmp
	Error               error
	CommitOps, TouchOps int
	Empty               bool
	Commitfn            func(ctx context.Context) (kvstore.TxnResponse, error)
}

// Create stages an object creation in a transaction.
func (f *FakeTxn) Create(key string, obj runtime.Object) error {
	f.Ops = append(f.Ops, FakeOps{Op: "create", Key: key, Obj: obj})
	return f.Error
}

// Delete stages an object deletion in a transaction.
func (f *FakeTxn) Delete(key string, cs ...kvstore.Cmp) error {
	f.Ops = append(f.Ops, FakeOps{Op: "delete", Key: key, Obj: nil})
	return f.Error
}

// Update stages an object update in a transaction.
func (f *FakeTxn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	f.Ops = append(f.Ops, FakeOps{Op: "update", Key: key, Obj: obj})
	return f.Error
}

// Commit tries to commit the transaction.
func (f *FakeTxn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	f.CommitOps++
	if f.Commitfn != nil {
		return f.Commitfn(ctx)
	}
	return kvstore.TxnResponse{Succeeded: true}, nil
}

// Touch updates version without changing contents
func (f *FakeTxn) Touch(key string) error {
	f.TouchOps++
	return nil
}

// IsEmpty returns true if the Txn is empty
func (f *FakeTxn) IsEmpty() bool {
	return f.Empty
}

// AddComparator adds a condition for the transaction.
func (f *FakeTxn) AddComparator(cs ...kvstore.Cmp) {
	f.Cmps = append(f.Cmps, cs...)
}

// FakeOverlay is a mock implementation
type FakeOverlay struct {
	kvstore.Interface
	Kvconn                                                                       kvstore.Interface
	CreatePrimaries, UpdatePrimaries, DeletePrimaries, Commits, Clears, Verifies int
	VerifyFunc                                                                   func(ctx context.Context) (apiintf.OverlayStatus, error)
	CommitFunc                                                                   func(ctx context.Context, items []apiintf.OverlayKey) error
	ClearBufferFunc                                                              func(ctx context.Context, items []apiintf.OverlayKey) error
	Reqs                                                                         apiintf.RequirementSet
	Txn                                                                          *FakeTxn
	StatFn                                                                       func(keys []string) []apiintf.ObjectStat
}

// Stat is a mock implementation
func (f *FakeOverlay) Stat(tx context.Context, keys []string) []apiintf.ObjectStat {
	if f.StatFn != nil {
		return f.StatFn(keys)
	}
	return nil
}

// StatKind is a mock implementation
func (f *FakeOverlay) StatKind(group string, kind string) ([]apiintf.ObjectStat, error) {
	return nil, fmt.Errorf("unimplemented")
}

// Start is a mock implementation
func (f *FakeOverlay) Start() error { return nil }

// Restore is a mock implementation
func (f *FakeOverlay) Restore() error { return nil }

// Clear is a mock implementation
func (f *FakeOverlay) Clear() {}

// GetKvConn is a mock implementation
func (f *FakeOverlay) GetKvConn() kvstore.Interface { return f.Kvconn }

// CreatePrimary is a mock implementation
func (f *FakeOverlay) CreatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object) error {
	f.CreatePrimaries++
	return nil
}

// UpdatePrimary is a mock implementation
func (f *FakeOverlay) UpdatePrimary(ctx context.Context, service, method, uri, key, resVer string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error {
	f.UpdatePrimaries++
	return nil
}

// DeletePrimary is a mock implementation
func (f *FakeOverlay) DeletePrimary(ctx context.Context, service, method, uri, key string, orig, into runtime.Object) error {
	f.DeletePrimaries++
	orig.Clone(into)
	return nil
}

// Commit is a mock implementation
func (f *FakeOverlay) Commit(ctx context.Context, items []apiintf.OverlayKey) error {
	f.Commits++
	if f.CommitFunc != nil {
		return f.CommitFunc(ctx, items)
	}
	return nil
}

// ClearBuffer is a mock implementation
func (f *FakeOverlay) ClearBuffer(ctx context.Context, items []apiintf.OverlayKey) error {
	f.Clears++
	if f.ClearBufferFunc != nil {
		return f.ClearBufferFunc(ctx, items)
	}
	return nil
}

// Verify is a mock implementation
func (f *FakeOverlay) Verify(ctx context.Context) (apiintf.OverlayStatus, error) {
	f.Verifies++
	if f.VerifyFunc != nil {
		return f.VerifyFunc(ctx)
	}
	return apiintf.OverlayStatus{}, nil
}

// GetRequirements is a mock implementation
func (f *FakeOverlay) GetRequirements() apiintf.RequirementSet {
	return f.Reqs
}

// NewTxn is a mock implementation
func (f *FakeOverlay) NewTxn() kvstore.Txn {
	if f.Txn != nil {
		return f.Txn
	}
	return &FakeTxn{}
}

// NewWrappedTxn is a mock implementation
func (f *FakeOverlay) NewWrappedTxn(ctx context.Context) kvstore.Txn {
	return f.NewTxn()
}

// DebugAction is a mock implementation
func (f *FakeOverlay) DebugAction(action string, params []string) string {
	return ""
}

// StartSnapshot is a mock implementation
func (f *FakeOverlay) StartSnapshot() uint64 {
	return 0
}

// DeleteSnapshot is a mock implementation
func (f *FakeOverlay) DeleteSnapshot(uint64) error {
	return nil
}

// SnapshotReader is a mock implementation
func (f *FakeOverlay) SnapshotReader(uint64 uint64, include bool, kinds []string) (io.ReadCloser, error) {
	return nil, nil
}

// SnapshotWriter is a mock implementation
func (f *FakeOverlay) SnapshotWriter(reader io.Reader) apiintf.SnapshotWriter {
	return nil
}

// Rollback is a mock implementation
func (f *FakeOverlay) Rollback(ctx context.Context, rev uint64, kvs kvstore.Interface) error {
	return nil
}

// FakeRequirement is a mock implementation
type FakeRequirement struct {
	CheckCalled, ApplyCalled, FinalizeCalled int
	RetErr                                   error
	Data                                     interface{}
}

// Check is a mock implementation
func (f *FakeRequirement) Check(ctx context.Context) []error {
	f.CheckCalled++
	if f.RetErr != nil {
		return []error{f.RetErr}
	}
	return nil
}

// Apply is a mock implementation
func (f *FakeRequirement) Apply(ctx context.Context, txn kvstore.Txn, cache apiintf.CacheInterface) error {
	f.ApplyCalled++
	if f.RetErr != nil {
		return f.RetErr
	}
	return nil
}

// Finalize is a mock implementation
func (f *FakeRequirement) Finalize(ctx context.Context) error {
	f.FinalizeCalled++
	if f.RetErr != nil {
		return f.RetErr
	}
	return nil
}

// String produces a string for printing
func (f *FakeRequirement) String() string {
	return "FAKE"
}

// FakeRequirementSet is a mock implementation
type FakeRequirementSet struct {
	CheckCalled, ApplyCalled, FinalizeCalled, ClearCalled, RefsCalled int
	RefReq                                                            map[string]map[string]apiintf.ReferenceObj
	ApplyFn                                                           func(txn kvstore.Txn) []error
	ConsUpdates                                                       []FakeRequirement
}

// Check checks if the requirement has been met by all requirements in the collection
func (f *FakeRequirementSet) Check(ctx context.Context) []error {
	f.CheckCalled++
	return nil
}

// Apply applies the requirements to the transaction provided.
//  All the requirements in the collection are called to Apply
func (f *FakeRequirementSet) Apply(ctx context.Context, txn kvstore.Txn, cache apiintf.CacheInterface) []error {
	f.ApplyCalled++
	return nil
}

// Finalize call Requirement s in the set to the Finalize their requirements
func (f *FakeRequirementSet) Finalize(ctx context.Context) []error {
	f.FinalizeCalled++
	return nil
}

// Clear clears all accumulated requirements in the set
func (f *FakeRequirementSet) Clear(ctx context.Context) {
	f.ClearCalled++
}

// String produces a string for printing
func (f *FakeRequirementSet) String() string {
	return "FAKE"
}

// NewRefRequirement creates a new reference related requirement and adds it to the list of requirements.
func (f *FakeRequirementSet) NewRefRequirement(oper apiintf.APIOperType, key string, reqs map[string]apiintf.ReferenceObj) apiintf.Requirement {
	f.RefsCalled++
	if f.RefReq == nil {
		f.RefReq = make(map[string]map[string]apiintf.ReferenceObj)
	}
	f.RefReq[key] = reqs
	return &FakeRequirement{}
}

// NewConsUpdateRequirement creates a new consistent update requirement and adds it to the list of requirements
//  the consistent update operations are applied in the order speciefied in the slice passed in
func (f *FakeRequirementSet) NewConsUpdateRequirement(reqs []apiintf.ConstUpdateItem) apiintf.Requirement {
	req := FakeRequirement{Data: reqs}
	f.ConsUpdates = append(f.ConsUpdates, req)
	return &req
}

// AddRequirement adds a new requirement to the set
func (f *FakeRequirementSet) AddRequirement(requirement apiintf.Requirement) {}

// FakeGraphInterface is a mock implementations
type FakeGraphInterface struct {
	UpdateNodes, DeleteNodes, ReferecesCalled                 int
	RefereesCalled, TreeCalled, IsIsolatedCalled, CloseCalled int
	Refs                                                      map[string][]string
	WeakRefs                                                  map[string][]string
	SelectorRefs                                              map[string][]string
	Verts                                                     map[string][]*graph.Vertex
	WVerts                                                    map[string][]*graph.Vertex
}

// UpdateNode is a mock implementations
func (f *FakeGraphInterface) UpdateNode(in *graph.Node) error {
	f.UpdateNodes++
	return nil
}

// DeleteNode is a mock implementations
func (f *FakeGraphInterface) DeleteNode(node string) error {
	f.DeleteNodes++
	return nil
}

// References is a mock implementations
func (f *FakeGraphInterface) References(in string) *graph.Node {
	f.ReferecesCalled++
	ret := &graph.Node{}
	if f.Refs != nil || f.WeakRefs != nil || f.SelectorRefs != nil {
		ret.This = in
		ret.Dir = graph.RefOut
		ret.Refs = f.Refs
		ret.WeakRefs = f.WeakRefs
		ret.SelectorRefs = f.SelectorRefs
		return ret
	}
	return nil
}

// Referrers is a mock implementations
func (f *FakeGraphInterface) Referrers(in string) *graph.Node {
	f.RefereesCalled++
	ret := &graph.Node{}
	if f.Refs != nil || f.WeakRefs != nil || f.SelectorRefs != nil {
		ret.This = in
		ret.Dir = graph.RefIn
		ret.Refs = f.Refs
		ret.WeakRefs = f.WeakRefs
		ret.SelectorRefs = f.SelectorRefs
		return ret
	}
	return nil
}

// Tree is a mock implementations
func (f *FakeGraphInterface) Tree(in string, dir graph.Direction) *graph.Vertex {
	f.TreeCalled++
	if f.Refs != nil {
		return &graph.Vertex{
			This:         in,
			Dir:          graph.RefIn,
			Refs:         f.Verts,
			WeakRefs:     f.WVerts,
			SelectorRefs: f.SelectorRefs,
		}
	}
	return nil
}

// IsIsolated is a mock implementations
func (f *FakeGraphInterface) IsIsolated(in string) bool {
	f.IsIsolatedCalled++
	return true
}

// Close is a mock implementations
func (f *FakeGraphInterface) Close() {
	f.CloseCalled++
}

// Dump is a mock interface
func (f *FakeGraphInterface) Dump(string) string {
	return ""
}
