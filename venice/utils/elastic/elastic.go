// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	defaultStartOffset = int(0)
	defaultMaxResults  = int(10)

	// the retry mechanism will try to reset the client `maxResetRetries` no.of.times
	// before failing the client reset operation.
	maxResetRetries = 10

	// delay between reset retries
	resetInterval = 1 * time.Second

	// the caller retries `maxCallerRetries` no.of. times before failing the request.
	maxCallerRetries = 5

	// initial delay between request retries
	initialRetryInterval = 100 * time.Millisecond

	// max retry delay between request retries
	maxRetryInterval = 500 * time.Millisecond
)

type request func() (interface{}, error)

// Client holds the elastic config and client;
type Client struct {
	sync.Mutex
	urls           []string           // list of elasticsearch URLs
	esClient       *es.Client         // elasticsearch client
	resolverClient resolver.Interface // resolver used to get the updated elasticsearch URLs
	resetCount     int                // number of times the client is reset
	logger         log.Logger
}

// TODO
// 1. support for credentials

// NewClient create a new elastic client.
// Use resolver based resolution only inside cluster; otherwise use URL based clients (for tests)
//
// sniffing is enabled by default for both URL and resolver based clients
// sniff - will update the connection URLs by sniffing; so, make sure http.transport_host is set properly
// New client is created using `elasticURL` if it is not empty; otherwise, the given resolver
// is used to fetch elastic URLs and the client is created using that.
// either elasticURL or resolverClient is needed to create elastic client.
func NewClient(elasticURL string, resolverClient resolver.Interface, logger log.Logger) (ESClient, error) {
	var err error
	wrapperClient := &Client{
		logger:         logger.WithContext("submodule", "elastic-client"),
		resolverClient: resolverClient,
	}

	if !utils.IsEmpty(elasticURL) {
		elasticURL = fmt.Sprintf("http://%s", elasticURL)
		wrapperClient.urls = []string{elasticURL}
	} else if resolverClient != nil {
		// use resolver and find the elasticsearch URLs
		wrapperClient.urls, err = getElasticSearchURLs(resolverClient)
		if err != nil {
			return nil, err
		}
	} else {
		return nil, fmt.Errorf("provide URL or resolver to create elasticsearch client")
	}

	// create new elastic client with sniffing and health checks enabled
	wrapperClient.esClient, err = newElasticClient(wrapperClient.urls, wrapperClient.logger)
	if err != nil {
		return nil, err
	}

	return wrapperClient, nil
}

// IsClusterHealthy ensures the cluster status is `green`
func (e *Client) IsClusterHealthy(ctx context.Context) (bool, error) {
	result, err := e.esClient.ClusterHealth().Do(ctx)
	if err != nil {
		return false, err
	}

	if result.Status != "green" {
		if err = e.esClient.WaitForGreenStatus("10s"); err != nil {
			return false, err
		}
	}

	return true, nil
}

// Version returns the elasticsearch version installed
func (e *Client) Version() (string, error) {
	for _, url := range e.urls {
		result, err := e.esClient.ElasticsearchVersion(url)
		if err != nil {
			continue
		}

		return result, nil
	}

	return "", fmt.Errorf("failed to get elasticsearch version")
}

// CreateIndexTemplate creates the requested index template with given setting and name.
// Once the template is created, elasticsearch will automatically apply the properties for
// new indices that matches the index pattern given in template. This helps to
// avoid creating repeative/daily based indices explicitly as the index call will
// automatically create new indices if it does not exists already. Internally, it will utlize
// the templates to apply properties to new indices.
func (e *Client) CreateIndexTemplate(ctx context.Context, name, settings string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// create index template
			resp, err := e.esClient.IndexPutTemplate(name).BodyString(settings).Do(ctx)
			if err != nil {
				return resp, err
			} else if !resp.Acknowledged {
				return resp, NewError(ErrRespNotAcknowledged, "")
			}

			return resp, err
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, create template {%s}", settings)
			retryCount++
			continue
		}

		return rErr
	}
}

// DeleteIndexTemplate deletes the index template identified by given name.
// Templates are only applied at index creation time. So, changing/deleting a template will
// have no impact on existing indices.
func (e *Client) DeleteIndexTemplate(ctx context.Context, name string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// create index template
			resp, err := e.esClient.IndexDeleteTemplate(name).Do(ctx)
			if err != nil {
				return resp, err
			} else if !resp.Acknowledged {
				return resp, NewError(ErrRespNotAcknowledged, "")
			}

			return resp, err
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, delete template {%s}", name)
			retryCount++
			continue
		}

		return rErr
	}
}

// CreateIndex creates the requested index on elastic cluster. Settings contains the index
// settings (shards, replicas) and any mapping of the obj fields.
// After creating the index, only number of replicas can be changed dynamically but not number of shards.
func (e *Client) CreateIndex(ctx context.Context, index, settings string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// check if index exists
			if indexExistsResp, err := e.esClient.IndexExists(index).Do(ctx); err != nil {
				return indexExistsResp, err
			} else if indexExistsResp {
				return indexExistsResp, NewError(ErrIndexExists, "")
			}

			// create index
			resp, err := e.esClient.CreateIndex(index).BodyString(settings).Do(ctx)
			if err != nil {
				return resp, err
			} else if !resp.Acknowledged {
				// createIndex returns only when the primary copies of
				// each shard have been started or request times out
				return resp, NewError(ErrRespNotAcknowledged, "")
			}

			return resp, err
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, create index {%s}", index)
			retryCount++
			continue
		}

		return rErr
	}
}

// DeleteIndex deletes the given index from elastic cluster.
// we cannot efficiently delete (all the docs) from an existing index, but deleting an entire index is cheap.
func (e *Client) DeleteIndex(ctx context.Context, index string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			deleteIndexResp, err := e.esClient.DeleteIndex(index).Do(ctx)
			if err != nil {
				return deleteIndexResp, err
			} else if !deleteIndexResp.Acknowledged {
				return deleteIndexResp, NewError(ErrRespNotAcknowledged, "")
			}

			return deleteIndexResp, nil
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, delete index {%s}", index)
			retryCount++
			continue
		}

		return rErr
	}
}

// FlushIndex flushes the given index; flushes the data to index storage.
func (e *Client) FlushIndex(ctx context.Context, index string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			return e.esClient.Flush().Index(index).Do(ctx)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, flush index {%s}", index)
			retryCount++
			continue
		}

		return rErr
	}
}

// Index indexes the given object. Index action will either create the document if it doesn't
// exist or replace it if it exists, but the call will always succeed.
// Whereas the Create action will fail, if the doc exists already.
func (e *Client) Index(ctx context.Context, index, iType, ID string, obj interface{}) error {
	if obj == nil {
		return NewError(ErrEmptyDocument, "")
	}

	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// index the given document(obj)
			return e.esClient.Index().Index(index).Type(iType).Id(ID).BodyJson(obj).Do(ctx)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, index operation on document {%s}", ID)
			retryCount++
			continue
		}

		return rErr
	}
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

	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// construct bulk request
			bulkReq := e.esClient.Bulk()

			// index the documents in the same order as insertion
			for _, obj := range objs {
				switch obj.RequestType {
				case Index:
					req := es.NewBulkIndexRequest().Index(obj.Index).Type(obj.IndexType).Id(obj.ID).Doc(obj.Obj)
					bulkReq.Add(req)
				case Update: // update the doc identified by obj.ID
					req := es.NewBulkUpdateRequest().Index(obj.Index).Type(obj.IndexType).Id(obj.ID).Doc(obj.Obj)
					bulkReq.Add(req)
				case Delete: // delete the doc identified by obj.ID
					req := es.NewBulkDeleteRequest().Index(obj.Index).Type(obj.IndexType).Id(obj.ID)
					bulkReq.Add(req)
				}
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
				return nil, NewError(ErrBulkRequestFailed, "")
			}

			// check for partial failures
			// bulkResp.Errors will be true in such cases and bulkResp.Items
			// will have Error details for each entry sent in the bulkRequest.
			// Caller is expected to retry the Bulk operation for failed entries.
			if bulkResp.Errors == true {
				return bulkResp, NewError(ErrBulkRequestFailed, "Partial-failure")
			}

			return bulkResp, nil
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, bulk operation")
			retryCount++
			continue
		}

		// request failed
		if rErr != nil {
			return nil, rErr
		}

		// request executed successfully
		return rResp.(*es.BulkResponse), rErr
	}
}

// Delete deletes the given object in the index and docType provided
func (e *Client) Delete(ctx context.Context, index, docType, ID string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			return e.esClient.Delete().Index(index).Type(docType).Id(ID).Do(ctx)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, delete index {%s}", index)
			retryCount++
			continue
		}

		return rErr
	}
}

// Search executes the given query and returns the result.
// Elasticsearch by default refreshes each shard every 1s,
// so the document will be available to search 1s after indexing it.
// This behavior can be changed by adjusting `index.refresh_interval` in indices settings.
func (e *Client) Search(ctx context.Context, index, iType string, query interface{}, aggregation interface{},
	from, size int32, sortBy string) (*es.SearchResult, error) {
	var esQuery es.Query
	var esAgg es.Aggregation
	var ok bool

	// validate index
	if len(strings.TrimSpace(index)) == 0 {
		return nil, NewError(ErrInvalidIndex, "")
	}

	// validate query
	esQuery = nil
	if query != nil {
		// assert elastic query type
		esQuery, ok = query.(es.Query)
		if !ok {
			return nil, NewError(ErrInvalidSearchQuery, "")
		}

		// make sure the query is in correct format; Source() returns the json of the query
		if src, err := esQuery.Source(); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		} else if _, err := json.Marshal(src); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		}
	}

	// validate aggregation
	esAgg = nil
	if aggregation != nil {
		// assert elastic aggregation type
		esAgg, ok = aggregation.(es.Aggregation)
		if !ok {
			return nil, NewError(ErrInvalidSearchAggregation, "")
		}

		// make sure the aggregation is in correct format; Source() returns the json of the aggregation
		if src, err := esAgg.Source(); err != nil {
			return nil, NewError(ErrInvalidSearchAggregation, err.Error())
		} else if _, err := json.Marshal(src); err != nil {
			return nil, NewError(ErrInvalidSearchAggregation, err.Error())
		}
	}

	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			// Construct the search request on a given index
			request := e.esClient.Search().Index(index)

			// Add doc type if valid
			if len(iType) != 0 {
				request = request.Type(iType)
			}

			// Add query if valid
			if esQuery != nil {
				request = request.Query(esQuery)
			}

			// Add aggregation if valid
			if esAgg != nil {
				request = request.Aggregation(TenantAggKey, esAgg)
			}

			// Set from or start offset for the results
			if from > 0 {
				request = request.From(int(from))
			} else {
				request = request.From(int(defaultStartOffset))
			}

			// Set size or maxResults desired
			if size > 0 {
				request = request.Size(int(size))
			} else {
				request = request.Size(int(defaultMaxResults))
			}

			// Add sort option if valid
			if len(sortBy) != 0 {
				request = request.Sort(sortBy, true)
			}

			// Execute the search request with desired size
			return request.Do(ctx)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debugf("retrying, search on index {%s}", index)
			retryCount++
			continue
		}

		// search failed
		if rErr != nil {
			return nil, rErr
		}

		searchResult := rResp.(*es.SearchResult)
		return searchResult, rErr
	}
}

// Close the elastic client
func (e *Client) Close() error {
	e.esClient.Stop()
	return nil
}

// GetResetCount returns the number of times the client is reset
func (e *Client) GetResetCount() int {
	return e.resetCount
}

// Perform executes the given request (function); and resets the client if the request
// failed on connection issue. It notifies the caller to retry once client is reset.
func (e *Client) Perform(req request, retryCount int, res interface{}, err error) (bool, interface{}, error) {
	if retryCount > maxCallerRetries {
		e.logger.Errorf("exhausted caller retries(%d)", maxCallerRetries)
		return false, res, err
	}

	res, err = req()

	// try to reset the client incase of connection failures
	if err != nil && IsConnRefused(err) {
		resetErr := e.resetClient()
		if resetErr == nil { // client reset successfully
			e.logger.Debug("client reset successfully, let the caller retry the request")
			return true, res, err
		}

		// failed to reset client
		e.logger.Errorf("failed to reset the client, err: %v", resetErr)
		return false, res, err
	}

	// request executed and there is no connection failure
	return false, res, err
}

// resetClient tries to reset the client
// error == nil, indicates the connection is reset successfully and the caller can retry the request.
// otherwise, it failed to reset the client.
func (e *Client) resetClient() error {
	e.Lock()
	defer e.Unlock()

	// check cluster health before stopping the client;
	// if it is healthy, there is no need to reset the client
	// healthy cluster could be a result of the earlier reset.
	if healthy, _ := e.IsClusterHealthy(context.Background()); healthy {
		return nil
	}

	for i := 0; i < maxResetRetries; i++ {
		if rErr := e.resetClientHelper(); rErr != nil {
			time.Sleep(resetInterval)
			e.logger.Debugf("failed to reset elastic client, err: %v, retrying", rErr)
			continue
		}

		// update the number of times the client has been reset successfully
		e.resetCount++
		return nil
	}

	return fmt.Errorf("exhausted reset retries (%d)", maxResetRetries)
}

// resetClientHelper creates a new client using the URL from resolver and replaces it with the old client
func (e *Client) resetClientHelper() error {
	if e.resolverClient == nil {
		e.logger.Error("could not find the resolver to fetch elastic address")
		return fmt.Errorf("could not find the resolver to fetch elastic address")
	}

	e.logger.Debug("trying to reset the client")
	// try to get the new list of URLs and re-create client
	elasticURLs, err := getElasticSearchURLs(e.resolverClient)
	if err != nil {
		e.logger.Debug("failed to get elasticsearch URLs from the resolver")
		return err
	}

	if len(elasticURLs) > 0 {
		// update the client
		newClient, err := newElasticClient(elasticURLs, e.logger)
		if err != nil {
			e.logger.Errorf("failed to reset elastic client, err: %v", err)
			return err
		}

		// check cluster health
		result, err := newClient.ClusterHealth().Do(context.Background())
		if err != nil {
			e.logger.Errorf("elasticsearch cluster not healthy, err: %v", err)
			return err
		}

		if result.Status != "green" {
			if err = newClient.WaitForGreenStatus("10s"); err != nil {
				e.logger.Errorf("elasticsearch cluster not healthy, err: %v", err)
				return err
			}
		}

		// stop the old client; stops all the sniffer and health check go routines
		e.esClient.Stop()

		// update elastic client and URLs
		e.esClient = newClient
		e.urls = elasticURLs
		e.logger.Debug("elasticsearch client reset successfully")
		return nil
	}

	return fmt.Errorf("failed to reset elastic client")
}

// helper function to create a elastic client
func newElasticClient(urls []string, logger log.Logger) (*es.Client, error) {
	return es.NewClient(
		es.SetURL(urls...),
		es.SetSniff(true),
		es.SetTraceLog(logger),
	)
}

// getElasticSearchURLs helper function to retrieve the list of elasticsearch URLs using the resolver
func getElasticSearchURLs(resolverClient resolver.Interface) ([]string, error) {
	elasticAddrs, err := getElasticSearchAddrs(resolverClient)
	if err != nil {
		return []string{}, err
	}

	// update the urls
	for i := 0; i < len(elasticAddrs); i++ {
		elasticAddrs[i] = fmt.Sprintf("http://%s", elasticAddrs[i])
	}

	return elasticAddrs, nil
}
