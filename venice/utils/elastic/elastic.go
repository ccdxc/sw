// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"encoding/json"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/venice/utils/log"
)

// Client holds the elastic config and client;
type Client struct {
	url    string
	client *es.Client
	logger log.Logger
}

// NewClient create a new elastic client
// XXX: Elasticsearch does not have transactions
func NewClient(elasticURL string, logger log.Logger) (ESClient, error) {
	// TODO: support for credentials
	client, err := es.NewClient(es.SetURL(elasticURL), es.SetSniff(false))
	if err != nil {
		return nil, err
	}

	es.SetTraceLog(logger)

	return &Client{
		url:    elasticURL,
		client: client,
		logger: logger}, nil
}

// Ping elasticsearch server to get version and other info.
func (e *Client) Ping(ctx context.Context) error {
	_, _, err := e.client.Ping(e.url).Do(ctx) // resp, statuscode, err
	return err
}

// Version returns the elasticsearch version installed.
func (e *Client) Version() (string, error) {
	return e.client.ElasticsearchVersion(e.url)
}

// CreateIndex creates the requested index on the elastic cluster.
// settings contains the index settings (shards, replicas) and
// any mapping of the obj fields.
// After creating the index, only number of replicas can be changed
// dynamically but not number of shards.
func (e *Client) CreateIndex(ctx context.Context, index, settings string) error {
	// check if index exists
	exists, err := e.client.IndexExists(index).Do(ctx)
	if err != nil {
		return err
	}

	if exists {
		return NewError(ErrIndexExists, "")
	}

	// create the given index
	resp, err := e.client.CreateIndex(index).BodyString(settings).Do(ctx)
	if err != nil {
		return err
	}

	// createIndex returns only when the primary copies of
	// each shard have been started or request times out
	if !resp.Acknowledged {
		return NewError(ErrRespNotAcknowledged, "")
	}

	return nil
}

// DeleteIndex deletes the given index from elastic cluster.
// we cannot efficiently delete (doc) from an existing index, but deleting an entire index is cheap.
func (e *Client) DeleteIndex(ctx context.Context, index string) error {
	resp, err := e.client.DeleteIndex(index).Do(ctx)
	if err != nil {
		return err
	}

	if !resp.Acknowledged {
		return NewError(ErrRespNotAcknowledged, "")
	}

	return nil
}

// FlushIndex flushes the given index; flushes the data to index storage.
func (e *Client) FlushIndex(ctx context.Context, index string) error {
	if _, err := e.client.Flush().Index(index).Do(ctx); err != nil {
		return err
	}

	return nil
}

// Index indexes the given object.
// Index action will either create the document if it doesn't exist or
// replace it if it exists, but the call will always succeed.
// Whereas the Create action will fail, if the doc exists already.
func (e *Client) Index(ctx context.Context, index, iType, ID string, obj interface{}) error {
	if obj == nil {
		return NewError(ErrEmptyDocument, "")
	}
	// check if index exists
	exists, err := e.client.IndexExists(index).Do(ctx)
	if err != nil {
		return err
	}

	if !exists {
		return NewError(ErrIndexNotExist, "")
	}

	// index the given document(obj)
	if _, err := e.client.Index().
		Index(index). // index document on the given index
		Type(iType).  // on the given type
		Id(ID).       // id of the document
		BodyJson(obj).
		Do(ctx); err != nil {
		return err
	}

	return nil
}

// Bulk indexes the given list of documents using bulk API. Efficient mechanism to do multiple
// operations as fast as possible with as few network round trips as possible.
// The Bulk API does not fail due to failures in one of the actions. If a single action fails for
// whatever reason, it will continue to process the remainder of the actions after it.
// When the bulk API returns, it will provide a status for each action (in the
// same order it was sent in). so that we can check if a specific action failed or not.
// indexed documents are visible for search in near-real time.
// document changes are not visible to search immediately, but will become visible within 1 second.
func (e *Client) Bulk(ctx context.Context, objs []*BulkRequest) (*es.BulkResponse, error) {
	if len(objs) == 0 {
		return nil, NewError(ErrBulkRequestFailed, "No bulk requests to commit")
	}

	bulkReq := e.client.Bulk()

	e.client.Bulk()
	// index the documents in the same order as insertion
	for _, obj := range objs {
		switch obj.RequestType {
		case "index":
			req := es.NewBulkIndexRequest().Index(obj.Index).Type(obj.IndexType).Id(obj.ID).Doc(obj.Obj)
			bulkReq.Add(req)
		}
		// TODO: handle other request types;
		// bulk operation can carry heterogenous requests (index, update, delete, etc.)
	}

	// at this point, total actions on the bulk request should be len(objs)
	// i.e. bulkReq.NumberOfActions() == len(objs)

	// execute the bulk request
	bulkResp, err := bulkReq.Do(ctx)
	if err != nil {
		return nil, err
	}

	// as all the operations are performed, it should be 0 now
	if bulkReq.NumberOfActions() != 0 {
		return bulkResp, NewError(ErrBulkRequestFailed, "")
	}

	return bulkResp, nil
}

// Search executes the given query and returns the result.
// Elasticsearch by default refreshes each shard every 1s,
// so the document will be available to search 1s after indexing it.
// This behavior can be changed by adjusting `index.refresh_interval` in indices settings.
func (e *Client) Search(ctx context.Context, index, iType string, query interface{}) (*es.SearchResult, error) {
	esQuery, ok := query.(es.Query)
	if !ok {
		return nil, NewError(ErrInvalidSearchQuery, "")
	}

	// make sure the query is in correct format; Source() returns the json of the query
	if src, err := esQuery.Source(); err != nil {
		return nil, NewError(ErrInvalidSearchQuery, err.Error())
	} else if _, err := json.Marshal(src); err != nil {
		return nil, NewError(ErrInvalidSearchQuery, err.Error())
	}

	searchResult, err := e.client.Search().
		Index(index).     // search in the given index
		Type(iType).      // search in the given type
		Query(esQuery).   // specify the query
		From(0).Size(10). // take documents 0-9
		Pretty(true).     // pretty print request and response JSON
		Do(ctx)           // execute
	if err != nil {
		return nil, err
	}

	return searchResult, nil
}
