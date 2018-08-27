package apiintf

import (
	"context"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SuccessCbFunc is called when a Store operation succeeds. The call happens in
// the critical path hence the function should the min needed.
type SuccessCbFunc func(key string, obj, prev runtime.Object)

// CacheInterface is the interface satisfied by any API cache
type CacheInterface interface {
	kvstore.Interface
	Start() error
	Clear()
	GetKvConn() kvstore.Interface
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

// OverlayInterface is Cache Interface with overlay functionality overlayed.
type OverlayInterface interface {
	CacheInterface
	CreatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object) error
	UpdatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error
	DeletePrimary(ctx context.Context, service, method, uri, key string, orig, into runtime.Object) error
	Commit(ctx context.Context, items []OverlayKey) error
	ClearBuffer(ctx context.Context, items []OverlayKey) error
	Verify(ctx context.Context) (OverlayStatus, error)
}
