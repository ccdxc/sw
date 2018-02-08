// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"

	es "gopkg.in/olivere/elastic.v5"
)

// ESClient interface for elasticsearch client.
type ESClient interface {
	// Ping connects to the elasticsearch instance and fetches info. about the cluster.
	// equivalent to curl http://elasticIP:9200
	Ping(ctx context.Context) error

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

	// Search performs the given query on `index` and type `iType`.
	Search(ctx context.Context, index, iType string, query interface{}) (*es.SearchResult, error)
}
