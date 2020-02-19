// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package staging

import (
	"context"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// Dummy vars to suppress unused imports message
var _ context.Context
var _ api.ObjectMeta
var _ kvstore.Interface

// StagingV1BufferInterface exposes the CRUD methods for Buffer
type StagingV1BufferInterface interface {
	Create(ctx context.Context, in *Buffer) (*Buffer, error)
	Update(ctx context.Context, in *Buffer) (*Buffer, error)
	UpdateStatus(ctx context.Context, in *Buffer) (*Buffer, error)
	Label(ctx context.Context, in *api.Label) (*Buffer, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Buffer, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Buffer, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Buffer, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
	Commit(ctx context.Context, in *CommitAction) (*CommitAction, error)
	Clear(ctx context.Context, in *ClearAction) (*ClearAction, error)
}

// StagingV1Interface exposes objects with CRUD operations allowed by the service
type StagingV1Interface interface {
	Buffer() StagingV1BufferInterface
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
}
