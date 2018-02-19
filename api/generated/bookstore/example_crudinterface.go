// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package bookstore

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

// BookInterface exposes the CRUD methods for Book
type BookInterface interface {
	Create(ctx context.Context, in *Book) (*Book, error)
	Update(ctx context.Context, in *Book) (*Book, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Book, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Book, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Book, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// OrderInterface exposes the CRUD methods for Order
type OrderInterface interface {
	Create(ctx context.Context, in *Order) (*Order, error)
	Update(ctx context.Context, in *Order) (*Order, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Order, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Order, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Order, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// PublisherInterface exposes the CRUD methods for Publisher
type PublisherInterface interface {
	Create(ctx context.Context, in *Publisher) (*Publisher, error)
	Update(ctx context.Context, in *Publisher) (*Publisher, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Publisher, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Publisher, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Publisher, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// StoreInterface exposes the CRUD methods for Store
type StoreInterface interface {
	Create(ctx context.Context, in *Store) (*Store, error)
	Update(ctx context.Context, in *Store) (*Store, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Store, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Store, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Store, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// BookstoreV1Interface exposes objects with CRUD operations allowed by the service
type BookstoreV1Interface interface {
	Order() OrderInterface
	Book() BookInterface
	Publisher() PublisherInterface
	Store() StoreInterface
}
