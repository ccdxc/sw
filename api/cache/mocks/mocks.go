package mocks

import (
	"context"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// FakeCache is mock implementation
type FakeCache struct {
	FakeKvStore
	ListFilteredFn func(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error
	Kvconn         kvstore.Interface
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

// Clear is mock implementation
func (f *FakeCache) Clear() {
}

// GetKvConn is mock implementation
func (f *FakeCache) GetKvConn() kvstore.Interface {
	return f.Kvconn
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
	Lists, Flushes            uint64
	Marks, Sweeps, DelDeleted uint64
	Setfn                     func(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) error
	Getfn                     func(key string) (runtime.Object, error)
	Deletefn                  func(key string, rev uint64, cb apiintf.SuccessCbFunc) (runtime.Object, error)
	Listfn                    func(key string, opts api.ListWatchOptions) []runtime.Object
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

// Clear is mock implementation
func (f *FakeStore) Clear() {
	f.Flushes++
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
	Ops       []FakeOps
	Cmps      []kvstore.Cmp
	Error     error
	CommitOps int
	Empty     bool
	Commitfn  func(ctx context.Context) (kvstore.TxnResponse, error)
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
	return kvstore.TxnResponse{}, nil
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
}

// Start is a mock implementation
func (f *FakeOverlay) Start() error { return nil }

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
func (f *FakeOverlay) UpdatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error {
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

// NewTxn is a mock implementation
func (f *FakeOverlay) NewTxn() kvstore.Txn {
	return &FakeTxn{}
}
