// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"

	es "gopkg.in/olivere/elastic.v5"
)

// ESClient interface for elasticsearch client.
type ESClient interface {
	// IsClusterHealthy checks the health of the elasticsearch cluster.
	IsClusterHealthy(ctx context.Context) (bool, error)

	// Version returns the version of the running elasticsearch instance.
	Version() (string, error)

	// CreateIndex creates the given index with the given settings.
	CreateIndex(ctx context.Context, index, settings string) error

	// DeleteIndex deletes the given index from elasticsearch.
	DeleteIndex(ctx context.Context, index string) error

	// FlushIndex flushes the given index. This ensures all the writes are flushed to the shard.
	FlushIndex(ctx context.Context, index string) error

	// Index indexes the single document (obj) on the given `index` and
	// type `iType` with the given ID.
	Index(ctx context.Context, index, iType, ID string, obj interface{}) error

	// Bulk performs the bulk request against elasticsearch. Each of the request
	// in bulk operation can be heterogeneous.
	Bulk(ctx context.Context, objs []*BulkRequest) (*es.BulkResponse, error)

	// Delete removes a single document (obj) from the given `index` and
	// type `iType` with the given ID.
	Delete(ctx context.Context, index, iType, ID string) error

	// Search performs the given query on `index` and type `iType` and limits
	// the number of search results to desired 'size' with start offset specified
	// by `from`.
	Search(ctx context.Context, index, iType string, query interface{}, aggregation interface{}, from, size int32) (*es.SearchResult, error)

	// Close the elastic client
	Close() error

	// returns the number of times the underlying elastic client is reset
	GetResetCount() int
}
