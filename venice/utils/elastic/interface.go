// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"io"

	es "github.com/olivere/elastic"
)

// ESClient interface for elasticsearch client.
type ESClient interface {
	// IsClusterHealthy checks the health of the elasticsearch cluster.
	IsClusterHealthy(ctx context.Context) (bool, error)

	// Version returns the version of the running elasticsearch instance.
	Version() (string, error)

	// CreateIndexTemplate creates a template with given name and settings
	CreateIndexTemplate(ctx context.Context, name, settings string) error

	// DeleteIndexTemplate deletes the template identified by given name
	DeleteIndexTemplate(ctx context.Context, name string) error

	// CreateIndex creates the given index with the given settings.
	CreateIndex(ctx context.Context, index, settings string) error

	// DeleteIndex deletes the given index from elasticsearch.
	DeleteIndex(ctx context.Context, index string) error

	// FlushIndex flushes the given index. This ensures all the writes are flushed to the shard.
	FlushIndex(ctx context.Context, index string) error

	// GetIndexSettings returns a map of index name and its associated settings
	// for the given list of indices or a index patterns provided as input argument.
	GetIndexSettings(ctx context.Context, indices []string) (map[string]SettingsResponse, error)

	// Index indexes the single document (obj) on the given `index` and
	// type `iType` with the given ID.
	Index(ctx context.Context, index, iType, ID string, obj interface{}) error

	// Bulk performs the bulk request against elasticsearch. Each of the request
	// in bulk operation can be heterogeneous.
	Bulk(ctx context.Context, objs []*BulkRequest) (*es.BulkResponse, error)

	// Delete removes a single document (obj) from the given `index` and
	// type `iType` with the given ID.
	Delete(ctx context.Context, index, iType, ID string) error

	// DeleteByQuery deletes objects that matches the given query from the given index
	DeleteByQuery(ctx context.Context,
		index string, iType string, query es.Query,
		size int, sortByField string, sortAsc bool) (*es.BulkIndexByScrollResponse, error)

	// Search performs the given query on `index` and type `iType` and limits
	// the number of search results to desired 'size' with start offset specified
	// by `from`.
	Search(ctx context.Context, index, iType string, query es.Query, aggregation es.Aggregation,
		from, size int32, sortByField string, sortAsc bool, options ...SearchOption) (*es.SearchResult, error)

	// Scroll performs the given query and iteratively fetches the result
	Scroll(ctx context.Context, index, iType string, query es.Query, size int32) (Scroller, error)

	// Close the elastic client
	Close() error

	// returns the number of times the underlying elastic client is reset
	GetResetCount() int

	// returns the cluster and indices health info
	GetClusterHealth(indices []string) (*es.ClusterHealthResponse, error)

	// returns the indices and shards that a search request would be executed against
	GetSearchShards(ctx context.Context, indices []string) (*es.SearchShardsResponse, error)

	// returns one or more (or all) of the cluster nodes statistics
	GetNodesInfo(ctx context.Context, nodeIDs []string) (*es.NodesInfoResponse, error)

	// returns index level stats on different operations happening on an index
	GetIndicesStats(ctx context.Context, indices []string) (*es.IndicesStatsResponse, error)

	// IndexNames returns name of indices currently present in elastic search
	IndexNames() ([]string, error)

	// GetRawClient - used for testing purposes only
	GetRawClient() *es.Client
}

// Scroller provides io.Reader interface to scrolling in Elastic. This is not thread safe.
type Scroller interface {
	io.Reader
}
