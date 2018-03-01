// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package alerts

import (
	"context"

	api "github.com/pensando/sw/api"
	apiserver "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// Dummy vars to suppress unused imports message
var _ context.Context
var _ api.ObjectMeta
var _ kvstore.Interface

// AlertDestinationV1AlertDestinationInterface exposes the CRUD methods for AlertDestination
type AlertDestinationV1AlertDestinationInterface interface {
	Create(ctx context.Context, in *AlertDestination) (*AlertDestination, error)
	Update(ctx context.Context, in *AlertDestination) (*AlertDestination, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*AlertDestination, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*AlertDestination, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*AlertDestination, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// AlertDestinationV1Interface exposes objects with CRUD operations allowed by the service
type AlertDestinationV1Interface interface {
	AlertDestination() AlertDestinationV1AlertDestinationInterface
}

// AlertPolicyV1AlertPolicyInterface exposes the CRUD methods for AlertPolicy
type AlertPolicyV1AlertPolicyInterface interface {
	Create(ctx context.Context, in *AlertPolicy) (*AlertPolicy, error)
	Update(ctx context.Context, in *AlertPolicy) (*AlertPolicy, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*AlertPolicy, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*AlertPolicy, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*AlertPolicy, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// AlertPolicyV1Interface exposes objects with CRUD operations allowed by the service
type AlertPolicyV1Interface interface {
	AlertPolicy() AlertPolicyV1AlertPolicyInterface
}
