// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
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

	// request gets cancelled after 90s
	contextDeadline = 90 * time.Second
)

type request func() (interface{}, error)

type options struct {
	httpClient *http.Client // Custom HTTP/HTTPS REST client
}

// Option fills the optional params for NewClient
type Option func(opt *options)

// WithHTTPClient passes a custom HTTP REST client
// used, for example, to enable HTTPS
func WithHTTPClient(c *http.Client) Option {
	return func(o *options) {
		o.httpClient = c
	}
}

// Client holds the elastic config and client;
type Client struct {
	sync.Mutex
	urls           []string           // list of elasticsearch URLs
	esClient       *es.Client         // elasticsearch client
	resolverClient resolver.Interface // resolver used to get the updated elasticsearch URLs
	resetCount     int                // number of times the client is reset
	logger         log.Logger
	scheme         string
	options
}

// NewClient create a new elastic client.
// Use resolver based resolution only inside cluster; otherwise use URL based clients (for tests)
//
// sniffing is enabled by default for both URL and resolver based clients
// sniff - will update the connection URLs by sniffing; so, make sure http.transport_host is set properly
// New client is created using `elasticURL` if it is not empty; otherwise, the given resolver
// is used to fetch elastic URLs and the client is created using that.
// either elasticURL or resolverClient is needed to create elastic client.
func NewClient(elasticURL string, resolverClient resolver.Interface, logger log.Logger, opts ...Option) (ESClient, error) {
	var err error
	wrapperClient := &Client{
		logger:         logger.WithContext("submodule", "elastic-client"),
		resolverClient: resolverClient,
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&wrapperClient.options)
		}
	}

	if isHTTPSClient(wrapperClient.options.httpClient) {
		wrapperClient.scheme = "https"
	} else {
		wrapperClient.scheme = "http"
	}

	if !utils.IsEmpty(elasticURL) {
		elasticURL = fmt.Sprintf("%s://%s", wrapperClient.scheme, elasticURL)
		wrapperClient.urls = []string{elasticURL}
	} else if resolverClient != nil {
		// use resolver and find the elasticsearch URLs
		wrapperClient.urls, err = getElasticSearchURLs(wrapperClient.scheme, resolverClient)
		if err != nil {
			return nil, err
		}
	} else {
		return nil, fmt.Errorf("provide URL or resolver to create elasticsearch client")
	}

	// create new elastic client with sniffing and health checks enabled
	wrapperClient.esClient, err = newElasticClient(wrapperClient.urls, wrapperClient.logger, wrapperClient.httpClient)
	if err != nil {
		return nil, err
	}

	return wrapperClient, nil
}

// NewAuthenticatedClient create a new elastic client with TLS certificates to authenticate itself.
func NewAuthenticatedClient(elasticURL string, resolverClient resolver.Interface, logger log.Logger) (ESClient, error) {
	tlsConfig, err := certs.LoadTLSCredentials(globals.ElasticClientAuthDir)
	if err != nil {
		return nil, fmt.Errorf("error accessing client credentials: %v", err)
	}
	tlsConfig.ServerName = globals.ElasticSearch + "-https"
	transport := netutils.CopyHTTPDefaultTransport()
	transport.MaxIdleConnsPerHost = 300
	transport.TLSClientConfig = tlsConfig
	client := &http.Client{Transport: transport}
	return NewClient(elasticURL, resolverClient, logger, WithHTTPClient(client))
}

// IsClusterHealthy ensures the cluster status is `green`
func (e *Client) IsClusterHealthy(ctx context.Context) (bool, error) {
	ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
	defer cancel()

	result, err := e.esClient.ClusterHealth().Do(ctxWithDeadline)
	e.logger.Debugf("ClusterHealth request result %+v,  err: %v", result, err)
	if err != nil {
		return false, err
	}

	if result.Status == "red" {
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

// IndexNames returns the names of indices present on Es
func (e *Client) IndexNames() ([]string, error) {
	return e.esClient.IndexNames()
}

// CreateIndexTemplate creates the requested index template with given setting and name.
// Once the template is created, elasticsearch will automatically apply the properties for
// new indices that matches the index pattern given in template. This helps to
// avoid creating repetitive/daily based indices explicitly as the index call will
// automatically create new indices if it does not exists already. Internally, it will utilize
// the templates to apply properties to new indices.
func (e *Client) CreateIndexTemplate(ctx context.Context, name, settings string) error {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// create index template
			resp, err := e.esClient.IndexPutTemplate(name).BodyString(settings).Do(ctxWithDeadline)
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// create index template
			resp, err := e.esClient.IndexDeleteTemplate(name).Do(ctxWithDeadline)
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// check if index exists
			if indexExistsResp, err := e.esClient.IndexExists(index).Do(ctxWithDeadline); err != nil {
				return indexExistsResp, err
			} else if indexExistsResp {
				return indexExistsResp, NewError(ErrIndexExists, "")
			}

			// create index
			resp, err := e.esClient.CreateIndex(index).BodyString(settings).Do(ctxWithDeadline)
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			deleteIndexResp, err := e.esClient.DeleteIndex(index).Do(ctxWithDeadline)
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
			return e.esClient.Flush().Index(index).Do(ctxWithDeadline)
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

// GetIndexSettings returns the index-features for the list of indices provided
// indices is list of indices that needs to be queried
func (e *Client) GetIndexSettings(ctx context.Context, indices []string) (map[string]SettingsResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool
	response := make(map[string]SettingsResponse)
	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
			indexResp, err := es.NewIndicesGetService(e.esClient).Index(indices...).Do(ctxWithDeadline)
			if err != nil {
				e.logger.Errorf("get indices failed {%v} resp: %+v err: %+v",
					indices, indexResp, err)
				return nil, err
			}
			for index, iResp := range indexResp {
				var sresp SettingsResponse
				var err error
				for k1, v1 := range iResp.Settings["index"].(map[string]interface{}) {
					switch k1 {
					case "codec":
						sresp.Codec = v1.(string)
					case "number_of_shards":
						sresp.NumberOfShards, err = strconv.ParseUint(v1.(string), 10, 64)
					case "provided_name":
						sresp.ProvidedName = v1.(string)
					case "max_inner_result_window":
						sresp.MaxInnerResultWindow, err = strconv.ParseUint(v1.(string), 10, 64)
					case "creation_date":
						// Parse millisecs to Time
						var msInt int64
						msInt, err = strconv.ParseInt(v1.(string), 10, 64)
						if err == nil {
							sresp.CreationDate = time.Unix(0, msInt*int64(time.Millisecond))
						}
					case "number_of_replicas":
						sresp.NumberOfShards, _ = strconv.ParseUint(v1.(string), 10, 64)
					case "version":
						for k2, v2 := range v1.(map[string]interface{}) {
							switch k2 {
							case "created":
								sresp.Version.Created = v2.(string)
							}
						}
					}
					if err != nil {
						e.logger.Errorf("@@@ Error parsing attribute: %s err: %v", k1, err)
					}
				}
				response[index] = sresp
			}
			return response, nil
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)
			retryCount++
			continue
		}

		if rErr != nil {
			return nil, rErr
		}
		return rResp.(map[string]SettingsResponse), nil
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
			// index the given document(obj)
			return e.esClient.Index().Index(index).Type(iType).Id(ID).BodyJson(obj).Do(ctxWithDeadline)
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

	e.logger.Debugf("bulk request length: %d, Index %s, ID %s", len(objs), objs[0].Index, objs[0].ID)

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
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
			bulkResp, err := bulkReq.Do(ctxWithDeadline)
			if err != nil {
				e.logger.Infof("bulk request failed with err: %v, Index %s, ID %s", err, objs[0].Index, objs[0].ID)
				return nil, err
			}

			// as all the operations are performed, it should be 0 now
			if bulkReq.NumberOfActions() != 0 {
				var errs []string
				for index, item := range bulkResp.Items {
					for key, val := range item {
						if val.Error != nil {
							e.logger.Infof("[%+v] request {%s} failed with err: %+v", objs[index], key, val.Error)
							errs = append(errs, val.Error.Reason)
						}
					}
				}
				return nil, NewError(ErrBulkRequestFailed,
					fmt.Sprintf("failed to complete some actions from bulk request, err: %s", strings.Join(errs, ",")))
			}

			// check for partial failures
			// bulkResp.Errors will be true in such cases and bulkResp.Items
			// will have Error details for each entry sent in the bulkRequest.
			// Caller is expected to retry the Bulk operation for failed entries.
			if bulkResp.Errors == true {
				var errs []string
				for index, item := range bulkResp.Items {
					for key, val := range item {
						if val.Error != nil {
							e.logger.Infof("[%+v] request {%s} failed with err: %+v", objs[index], key, val.Error)
							errs = append(errs, val.Error.Reason)
						}
					}
				}
				return bulkResp, NewError(ErrBulkRequestFailed, strings.Join(errs, ","))
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

// DeleteByQuery deletes objects that matches the given query from the given index
func (e *Client) DeleteByQuery(ctx context.Context, index string, iType string, query es.Query,
	size int, sortByField string, sortAsc bool) (*es.BulkIndexByScrollResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
			return e.esClient.DeleteByQuery().Index(index).Type(iType).Query(query).
				Size(size).SortByField(sortByField, sortAsc).Refresh("false").Do(ctxWithDeadline)
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

		searchResult := rResp.(*es.BulkIndexByScrollResponse)
		return searchResult, rErr
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
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()
			return e.esClient.Delete().Index(index).Type(docType).Id(ID).Do(ctxWithDeadline)
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
func (e *Client) Search(ctx context.Context, index, iType string, query es.Query, aggregation es.Aggregation,
	from, size int32, sortByField string, sortAsc bool, options ...SearchOption) (*es.SearchResult, error) {

	// validate index
	if len(strings.TrimSpace(index)) == 0 {
		return nil, NewError(ErrInvalidIndex, "")
	}

	// validate query
	if query != nil && reflect.ValueOf(query).IsValid() {
		// make sure the query is in correct format; Source() returns the json of the query
		if src, err := query.Source(); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		} else if _, err := json.Marshal(src); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		}
	}

	// validate aggregation
	if aggregation != nil && reflect.ValueOf(aggregation).IsValid() {
		// make sure the aggregation is in correct format; Source() returns the json of the aggregation
		if src, err := aggregation.Source(); err != nil {
			return nil, NewError(ErrInvalidSearchAggregation, err.Error())
		} else if _, err := json.Marshal(src); err != nil {
			return nil, NewError(ErrInvalidSearchAggregation, err.Error())
		}
	}

	sOptions := &searchOptions{}
	for _, opt := range options {
		opt(sOptions)
	}

	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool

	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// Construct the search request on a given index
			request := e.esClient.Search().Index(index)

			// Add doc type if valid
			if len(iType) != 0 {
				request = request.Type(iType)
			}

			// Add query if valid
			if query != nil {
				request = request.Query(query)
			}

			// Add aggregation if valid
			if aggregation != nil {
				request = request.Aggregation(TenantAggKey, aggregation)
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

			if sOptions.sourceContext != nil {
				request = request.FetchSourceContext(sOptions.sourceContext)
			}

			// Add sort option if valid
			if !utils.IsEmpty(sortByField) {
				sortInfo := es.SortInfo{
					Field:        sortByField,
					Ascending:    sortAsc,
					Missing:      "_last",
					UnmappedType: "keyword",
				}
				request = request.SortWithInfo(sortInfo)
			}

			// Execute the search request with desired size
			return request.Do(ctxWithDeadline)
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

		// UNPACK AS es.error

		searchResult := rResp.(*es.SearchResult)
		return searchResult, rErr
	}
}

// Scroll performs the given query and iteratively fetches the result
func (e *Client) Scroll(ctx context.Context, index, iType string, query es.Query, size int32) (Scroller, error) {
	return NewScroller(ctx, e, index, iType, query, size)
}

// GetClusterHealth returns cluster health info including indices health if specified
func (e *Client) GetClusterHealth(indices []string) (*es.ClusterHealthResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool
	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(context.Background(), time.Now().Add(contextDeadline))
			defer cancel()

			// Get cluster health service handle
			chs := e.esClient.ClusterHealth()
			if chs == nil {
				return nil, fmt.Errorf("failed to create cluster health service")
			}

			// Add indices if specified
			if len(indices) > 0 {
				chs = chs.Index(indices...)
			}

			return chs.Do(ctxWithDeadline)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, get cluster health")
			retryCount++
			continue
		}

		// request failed
		if rErr != nil {
			return nil, rErr
		}

		// request executed successfully
		return rResp.(*es.ClusterHealthResponse), rErr
	}
}

// GetSearchShards returns the indices and shards that a search request would be executed against.
// Helps to retrive the node IDs belonging to the index shards.
func (e *Client) GetSearchShards(ctx context.Context, indices []string) (*es.SearchShardsResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool
	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// get search shards service handle
			sss := e.esClient.SearchShards(indices...)
			if sss == nil {
				return nil, fmt.Errorf("failed to create search shards service")
			}

			return sss.Do(ctxWithDeadline)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, get cluster health")
			retryCount++
			continue
		}

		// request failed
		if rErr != nil {
			return nil, rErr
		}

		// request executed successfully
		return rResp.(*es.SearchShardsResponse), rErr
	}
}

// GetNodesInfo returns one or more (or all) of the cluster nodes statistics.
// Helps to retrieve the node details and other stats of the given nodeID(s).
func (e *Client) GetNodesInfo(ctx context.Context, nodeIDs []string) (*es.NodesInfoResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool
	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// get nodes info service handle
			nis := e.esClient.NodesInfo()
			if nis == nil {
				return nil, fmt.Errorf("failed to create nodes info service")
			}
			nis = nis.NodeId(nodeIDs...)

			return nis.Do(ctxWithDeadline)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, get cluster health")
			retryCount++
			continue
		}

		// request failed
		if rErr != nil {
			return nil, rErr
		}

		// request executed successfully
		return rResp.(*es.NodesInfoResponse), rErr
	}
}

// GetIndicesStats returns index level stats on different operations happening on an index.
// Helps to retrieve shard details of the given indices.
func (e *Client) GetIndicesStats(ctx context.Context, indices []string) (*es.IndicesStatsResponse, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry bool
	for {
		retry, rResp, rErr = e.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(ctx, time.Now().Add(contextDeadline))
			defer cancel()

			// get index stats service handle
			iss := e.esClient.IndexStats()
			if iss == nil {
				return nil, fmt.Errorf("failed to create index stats service")
			}

			iss = iss.Index(indices...)
			return iss.Do(ctxWithDeadline)
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)

			e.logger.Debug("retrying, get cluster health")
			retryCount++
			continue
		}

		// request failed
		if rErr != nil {
			return nil, rErr
		}

		// request executed successfully
		return rResp.(*es.IndicesStatsResponse), rErr
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

	// handle any elasticsearch error if any
	if er, ok := err.(*es.Error); ok && er.Details != nil {
		var errDetails []es.ErrorDetails
		for _, rtCause := range er.Details.RootCause {
			errDetails = append(errDetails, *rtCause)
		}

		if len(errDetails) > 0 {
			e.logger.Errorf("request failed, root_cause: %+v", errDetails)
		}
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
	elasticURLs, err := getElasticSearchURLs(e.scheme, e.resolverClient)
	if err != nil {
		e.logger.Debug("failed to get elasticsearch URLs from the resolver")
		return err
	}

	if len(elasticURLs) > 0 {
		// update the client
		newClient, err := newElasticClient(elasticURLs, e.logger, e.httpClient)
		if err != nil {
			e.logger.Errorf("failed to create new elastic client, err: %v", err)
			return err
		}

		// check cluster health
		result, err := newClient.ClusterHealth().Do(context.Background())
		if err != nil {
			e.logger.Errorf("failed to get elasticsearch cluster health, err: %v", err)
			return err
		}

		if result.Status == "red" {
			if err = newClient.WaitForGreenStatus("10s"); err != nil {
				e.logger.Errorf("elasticsearch cluster not in green status, err: %v", err)
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

func isHTTPSClient(client *http.Client) bool {
	if client == nil {
		return false
	}
	transport, ok := client.Transport.(*http.Transport)
	return ok && transport.TLSClientConfig != nil
}

// helper function to create a elastic client
func newElasticClient(urls []string, logger log.Logger, httpClient *http.Client) (*es.Client, error) {
	opts := []es.ClientOptionFunc{
		es.SetURL(urls...),
		es.SetSniff(true),
		es.SetTraceLog(logger),
		es.SetErrorLog(logger),
		es.SetInfoLog(logger),
	}

	if httpClient != nil {
		opts = append(opts, es.SetHttpClient(httpClient))
		if isHTTPSClient(httpClient) {
			opts = append(opts, es.SetScheme("https"))
		}
	}

	return es.NewClient(opts...)
}

// getElasticSearchURLs helper function to retrieve the list of elasticsearch URLs using the resolver
func getElasticSearchURLs(scheme string, resolverClient resolver.Interface) ([]string, error) {
	elasticAddrs, err := getElasticSearchAddrs(resolverClient)
	if err != nil {
		return []string{}, err
	}

	// update the urls
	for i := 0; i < len(elasticAddrs); i++ {
		elasticAddrs[i] = fmt.Sprintf("%s://%s", scheme, elasticAddrs[i])
	}

	return elasticAddrs, nil
}

// GetRawClient returns raw client. Only used for testing.
func (e *Client) GetRawClient() *es.Client {
	return e.esClient
}
