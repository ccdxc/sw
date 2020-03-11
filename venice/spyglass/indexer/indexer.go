// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package indexer

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"sync/atomic"
	"time"

	es "github.com/olivere/elastic"

	apiservice "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

/**
 * Indexer has two components:
 *   1. Watcher - responsible for maintaining a watch to API Server and giving events to writers
 *   2. Writers - responsible for creating bulk elastic requests and writing them to elastic
 *
 * Watchers establish a watch on all API Groups. It maintains in memory a resource version count so that if the
 * connection is broken, it can pick up the watch from where it left off. If API server no longer supports the
 * resource version we have, it returns a 410 HTTP Gone. In this case, or if we get any other errors during watcher creation
 * for a prolonged period of time, we shutdown indexer. Spyglass should restart indexer, and
 * on indexer start we delete and recreate the elastic index.
 *
 * Since we need to maintain a resource version for each watch, an API Group's events should always
 * be processed by the same writer.
 *
 * If a writer fails to write to elastic while it's buffer is full for a prolonged period of time,
 * it will shutdown the indexer.
 */

const (
	elasticWaitIntvl                    = time.Second
	maxElasticRetries                   = 200
	apiSrvWaitIntvl                     = time.Second
	maxAPISrvRetries                    = 200
	indexBatchSize                      = 100
	indexBatchIntvl                     = 5 * time.Second
	indexRefreshIntvl                   = 60 * time.Second
	fwLogsElasticBatchSize              = 100
	fwLogsElasticWriteWorkerSize        = 10
	fwLogsElasticWriterWorkerBufferSize = 1000
)

// Option fills the optional params for Indexer
type Option func(*Indexer)

// Indexer is an implementation of the indexer.Interface
type Indexer struct {
	sync.RWMutex
	sync.WaitGroup
	ctx        context.Context
	cancelFunc context.CancelFunc
	logger     log.Logger

	// API-server address and API-client object
	apiServerAddr string
	apiClient     apiservice.Services

	// VOS client
	vosClient           apiservice.Services
	vosFwLogsHTTPClient objstore.Client

	// ElasticDB client object
	elasticClient elastic.ESClient

	// Map of KV store watchers per API Group
	watchers map[string]kvstore.Watcher

	// Map from API Group to the writer responsible for it
	writerMap map[string]*writerMapEntry

	// Map of KV store recv channel per Object kind
	// to receive WatchEvent
	channels map[string]<-chan *kvstore.WatchEvent

	// Fan-in channels of index request objects.
	// The watcher events are fanned to a group of
	// requestChannels with length maxWriters .
	// Index request goes to either ordered writer
	// or append-only-writer (unordered), but not both.
	// As of now only firewall logs go to unordered writer.
	// A pool of Elastic Writers each listen to one channel.
	reqChan map[int]chan *indexRequest

	// Max number of objects indexed using
	// Bulk/Batch API
	batchSize int

	// Bulk indexing interval. If the batchSize
	// is not met, this timer kicks in to perform
	// the bulk-index on accumulated objects
	indexIntvl time.Duration

	// Index cache refresh interval to keep the elastic
	// indices warm in memory. A background query is
	// executed peridically per this interval.
	indexRefreshIntvl time.Duration

	// Slice of Pending objects per Elastic Writer
	requests [][]*elastic.BulkRequest

	// Slice of Pending functions to update resource version
	// Each index belongs to a specific writer
	resVersionUpdater []func()

	// Channel to stop watchers
	watcherDone chan bool

	// Channel to block indexer start on
	doneCh chan error

	// Running status of refreshIndices
	refreshIndicesRunning bool

	// Total count of writes made to elastic
	count uint64

	// Running status of the indexer
	// Used by Set/GetRunningStatus() functions
	runningStatus uint32

	// Policy cache
	cache cache.Interface

	// Limits
	maxOrderedWriters    int
	maxAppendOnlyWriters int // Used for firewall logs
	maxWriters           int // Max concurrent writers
	indexMaxBuffer       int

	// Whether or not to watch VOS objects
	watchVos       bool
	WatchAPIServer bool // exported because its set using Option closure.

	// running tests, its exported so that it can be set
	// using options from test package
	// The test fields are set using the Option closure.
	VosTest bool

	VosTestGrpcURL string
}

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(idr *Indexer) {
		idr.elasticClient = esClient
	}
}

// DisableVOSWatcher disables indexing VOS objects
func DisableVOSWatcher() Option {
	return func(idr *Indexer) {
		idr.watchVos = false
	}
}

// DisableApiserverWatcher disables indexing APIServer objects
func DisableApiserverWatcher() Option {
	return func(idr *Indexer) {
		idr.WatchAPIServer = false
	}
}

// WatchHandler is handler func for watch events on API-server objects
type WatchHandler func(et kvstore.WatchEventType, obj interface{})

// indexRequest entry
type indexRequest struct {
	evType           kvstore.WatchEventType
	object           interface{}
	resVersionUpdate func()
}

type writerMapEntry struct {
	writerID   int
	resVersion string
}

// NewIndexer instantiates a new indexer
func NewIndexer(ctx context.Context,
	apiServerAddr string, rsr resolver.Interface, cache cache.Interface,
	logger log.Logger, maxOrderedWriters int, maxAppendOnlyWriters int,
	opts ...Option) (Interface, error) {
	newCtx, cancelFunc := context.WithCancel(ctx)
	indexer := Indexer{
		ctx:                  newCtx,
		cancelFunc:           cancelFunc,
		apiServerAddr:        apiServerAddr,
		logger:               logger,
		batchSize:            indexBatchSize,
		indexIntvl:           indexBatchIntvl,
		indexRefreshIntvl:    indexRefreshIntvl,
		doneCh:               make(chan error),
		count:                0,
		cache:                cache,
		maxOrderedWriters:    maxOrderedWriters,
		maxAppendOnlyWriters: maxAppendOnlyWriters,
		maxWriters:           maxOrderedWriters + maxAppendOnlyWriters,
		indexMaxBuffer:       (maxOrderedWriters + maxAppendOnlyWriters) * indexBatchSize,
		watchVos:             true,
		WatchAPIServer:       true,
		VosTest:              false,
		VosTestGrpcURL:       "", // 127.0.0.1:9051
	}

	for _, opt := range opts {
		if opt != nil {
			opt(&indexer)
		}
	}

	logger.Infof("Creating Indexer, apiserver-addr: %s, watchAPIServer %d, watchVos %d",
		apiServerAddr, indexer.WatchAPIServer, indexer.watchVos)

	if indexer.elasticClient == nil {
		// Initialize elastic client
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			if indexer.VosTest {
				return elastic.NewClient("", rsr, logger.WithContext("submodule", "elastic"))
			}
			return elastic.NewAuthenticatedClient("", rsr, logger.WithContext("submodule", "elastic"))
		}, elasticWaitIntvl, maxElasticRetries)
		if err != nil {
			logger.Errorf("Failed to create elastic client, err: %v", err)
			return nil, err
		}
		logger.Debugf("Created elastic client")
		indexer.elasticClient = result.(elastic.ESClient)
	}

	if indexer.WatchAPIServer {
		// Initialize api client
		apiClientBalancer := balancer.New(rsr)
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return apiservice.NewGrpcAPIClient(globals.Spyglass, globals.APIServer, logger, rpckit.WithBalancer(apiClientBalancer))
		}, apiSrvWaitIntvl, maxAPISrvRetries)
		if err != nil {
			logger.Errorf("Failed to create api client, addr: %s err: %v",
				apiServerAddr, err)
			indexer.elasticClient.Close()
			apiClientBalancer.Close()
			return nil, err
		}

		logger.Debugf("Created API client")
		indexer.apiClient = result.(apiservice.Services)
	}

	if indexer.watchVos {
		// Create objstrore http client for fwlogs
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return createBucketClient(ctx, rsr, globals.ReservedFwLogsTenantName, fwlogsBucketName)
		}, apiSrvWaitIntvl, maxAPISrvRetries)
		if err != nil {
			logger.Errorf("Failed to create objstore client for fwlogs")
			return nil, err
		}
		indexer.vosFwLogsHTTPClient = result.(objstore.Client)

		// create grpc client with vos
		vosGrpcURL := globals.Vos
		if indexer.VosTest {
			vosGrpcURL = indexer.VosTestGrpcURL
		}
		vosClientBalancer := balancer.New(rsr)
		result, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return apiservice.NewGrpcAPIClient(globals.Spyglass, vosGrpcURL, logger, rpckit.WithBalancer(vosClientBalancer))
		}, apiSrvWaitIntvl, maxAPISrvRetries)
		if err != nil {
			logger.Errorf("Failed to create vos client, addr: %s err: %v",
				apiServerAddr, err)
			indexer.elasticClient.Close()
			indexer.apiClient.Close()
			vosClientBalancer.Close()
			return nil, err
		}

		logger.Debugf("Created Vos API client")
		indexer.vosClient = result.(apiservice.Services)
	}

	logger.Infof("Created new indexer: {%+v}", &indexer)
	return &indexer, nil
}

// Start starts the watchers for API-server objects for Indexing
func (idr *Indexer) Start() error {

	idr.logger.Infof("Starting indexer")

	// initialize indexes
	err := idr.initSearchDB()
	if err != nil {
		idr.logger.Errorf("Failed to setup indices for search, err: %v", err)
		return err
	}

	k := 0
	idr.reqChan = make(map[int]chan *indexRequest)
	for i := 0; i < idr.maxOrderedWriters; i++ {
		idr.reqChan[k] = make(chan *indexRequest, idr.indexMaxBuffer)
		k++
	}
	for i := 0; i < idr.maxAppendOnlyWriters; i++ {
		idr.reqChan[k] = make(chan *indexRequest, idr.indexMaxBuffer)
		k++
	}

	idr.requests = make([][]*elastic.BulkRequest, idr.maxWriters)
	idr.resVersionUpdater = make([]func(), idr.maxWriters)

	go func() {
		idr.Add(1)
		idr.initializeAndStartWatchers()
	}()

	// start the Elastic Writer Pool
	k = 0
	idr.logger.Infof("Starting ordered %d writers", idr.maxOrderedWriters)
	for i := 0; i < idr.maxOrderedWriters; i++ {
		idr.Add(1)
		go func(id int) {
			defer idr.Done()
			idr.startOrderedWriter(id)
		}(k)
		k++
	}

	// start append only writers
	idr.logger.Infof("Starting append only %d writers", idr.maxAppendOnlyWriters)
	for i := 0; i < idr.maxAppendOnlyWriters; i++ {
		idr.Add(1)
		go func(id int) {
			defer idr.Done()
			// TODO: Generalize if needed. For now directly feeding fwlog parameters
			// because append-only-writer is only working for fwlogs.
			idr.startAppendOnlyWriter(id,
				fwLogsElasticBatchSize,
				fwLogsElasticWriteWorkerSize,
				fwLogsElasticWriterWorkerBufferSize,
				idr.fwlogsRequestCreator)
		}(k)
		k++
	}

	// Block on the done channel
	err = <-idr.doneCh
	idr.logger.Errorf("received err from done ch, err: %v", err)
	idr.Stop()
	return err
}

func (idr *Indexer) initializeAndStartWatchers() {
	defer idr.Done()

	if err := idr.initialize(); err != nil {
		idr.logger.Errorf("failed to create watchers, err: %v", err)
		if idr.GetRunningStatus() == true {
			idr.doneCh <- err // context cancelled
		}
		return
	}

	// start the watchers
	idr.SetRunningStatus(true)
	idr.startWatchers()
}

func (idr *Indexer) initialize() error {
	for {
		select {
		case <-idr.ctx.Done():
			err := errors.New("context cancelled, exiting initialize watchers")
			idr.logger.Error(err)
			return err
		default:
			_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				return func() (interface{}, error) {
					if idr.ctx.Err() != nil { // context canceled; indexer stopped
						return nil, fmt.Errorf("context canceled, returning from create watcher retry loop")
					}

					idr.Lock()
					idr.watcherDone = make(chan bool)
					idr.Unlock()

					return nil, idr.createWatchers()
				}()
			}, apiSrvWaitIntvl, maxAPISrvRetries)

			if err == nil {
				return nil
			}
			idr.logger.Errorf("Failed to create watchers, err: %v", err)
			idr.stopWatchers()
			time.Sleep(10 * time.Second)
		}
	}
}

func (idr *Indexer) restartWatchers() {
	idr.logger.Info("Restarting watchers")
	idr.stopWatchers()
	go func() {
		idr.Add(1)
		idr.initializeAndStartWatchers()
	}()
}

// Stop stops all the watchers and writers for API-server objects
func (idr *Indexer) Stop() {
	idr.logger.Info("Stopping indexer...")
	if idr.GetRunningStatus() == false {
		return
	}

	idr.SetRunningStatus(false)
	idr.cancelFunc()
	idr.Wait() // wait for all the watchers and writers to stop (<-ctx.Done())
	// close the channel where the writers were receiving the request from
	for i := range idr.reqChan {
		close(idr.reqChan[i])
	}
	idr.elasticClient.Close()
	idr.apiClient.Close()
	if idr.vosClient != nil {
		idr.vosClient.Close()
	}
	idr.logger.Info("Stopped indexer")
}

// CreateIndex creates the given index with the given settings in SearchDB.
func (idr *Indexer) CreateIndex(index, settings string) error {
	// Create index and setup index mapping
	if err := idr.elasticClient.CreateIndex(idr.ctx, index, settings); err != nil && !elastic.IsIndexExists(err) {
		idr.logger.Errorf("Failed to create index: %s, err: %v, %v", index, err, elastic.IsIndexExists(err))
		return err
	}

	return nil
}

// DeleteIndex deletes the given index from SearchDB.
func (idr *Indexer) DeleteIndex(index string) error {

	// Delete index
	if err := idr.elasticClient.DeleteIndex(idr.ctx, index); err != nil && !elastic.IsIndexExists(err) {
		idr.logger.Errorf("Failed to create index: %s, err: %v, %v", index, err, elastic.IsIndexExists(err))
		return err
	}

	return nil
}

// FlushIndex flushes the given index. This ensures all the writes are flushed to the shard.
func (idr *Indexer) FlushIndex(index string) error {

	// Flush index
	if err := idr.elasticClient.FlushIndex(idr.ctx, index); err != nil {
		idr.logger.Errorf("Failed to flush index: %s err: %+v", index, err)
		return err
	}

	return nil
}

// Index indexes the single document (obj) on the given `index` and
// type `docType` with the given ID.
func (idr *Indexer) Index(index, docType, ID string, obj interface{}) error {

	// Index an object into elastic
	if err := idr.elasticClient.Index(idr.ctx, index, docType, ID, obj); err != nil {
		idr.logger.Errorf("Failed to index object: %+v err: %+v", obj, err)
		return err
	}

	return nil
}

// Bulk performs the bulk request against SearchDB.
// Each of the request in bulk operation can be heterogeneous.
func (idr *Indexer) Bulk(index, docType string, IDs []string, objects []interface{}) error {

	if len(IDs) != len(objects) {
		idr.logger.Errorf("Arg mismatch, len of IDs and objects should match")
		return errors.New("args mismatch")
	}

	// Add objects to the Bulk-Request slice
	var requests []*elastic.BulkRequest
	for i, obj := range objects {

		requests = append(requests,
			&elastic.BulkRequest{
				RequestType: elastic.Index,
				Index:       index,
				IndexType:   docType,
				ID:          IDs[i],
				Obj:         obj,
			})
	}

	// perform bulk operation
	resp, err := idr.elasticClient.Bulk(idr.ctx, requests)
	if err != nil {
		idr.logger.Errorf("Failed to perform bulk indexing, err: %v resp: %+v", err, resp)
		return err
	}

	return nil
}

// Delete removes a single document (obj) on the given `index` and
// type `docType` with the given ID.
func (idr *Indexer) Delete(index, docType, ID string) error {

	// Index an object into elastic
	if err := idr.elasticClient.Delete(idr.ctx, index, docType, ID); err != nil {
		idr.logger.Errorf("Failed to delete object type: %s ID: %s err: %+v",
			docType, ID, err)
		return err
	}

	return nil
}

// SetRunningStatus updates the running status
func (idr *Indexer) SetRunningStatus(status bool) {
	if status {
		atomic.StoreUint32(&idr.runningStatus, 1)
	} else {
		atomic.StoreUint32(&idr.runningStatus, 0)
	}
}

// GetRunningStatus returns the current running status
func (idr *Indexer) GetRunningStatus() bool {
	if atomic.LoadUint32(&idr.runningStatus) == 1 {
		return true
	}
	return false
}

// Initialize the searchDB with indices
// required for Venice search
func (idr *Indexer) initSearchDB() error {
	if idr.WatchAPIServer {
		// Delete current index since we may have missed delete events while spyglass was down
		if err := idr.deleteIndexHelper(globals.Configs, globals.DefaultTenant); err != nil {
			return err
		}

		// Create index and mapping for Policy objects
		if err := idr.createIndexHelper(globals.Configs, globals.DefaultTenant); err != nil {
			return err
		}
	}

	if idr.watchVos {
		// Create index and mapping for Firewall logs
		if err :=
			idr.createIndexHelper(globals.FwLogs, globals.ReservedFwLogsTenantName); err != nil && !elastic.IsIndexExists(err) {
			return err
		}

		// Create index and mapping for Firewall logs
		if err := idr.createIndexHelper(globals.FwLogsObjects, ""); err != nil && !elastic.IsIndexExists(err) {
			return err
		}
	}

	return nil
}

// Function to keep the elastic indices warm in memory
// by periodically querying in the background. This helps
// reduce the initial search latency by 2-5x order of
// magnitude.
func (idr *Indexer) refreshIndices() {
	idr.Lock()
	defer idr.Unlock()
	if idr.refreshIndicesRunning {
		return
	}
	idr.refreshIndicesRunning = true
	idr.logger.Infof("Launching index refresher")
	idr.Add(1)
	go func() {
		defer idr.Done()
		query := func() {
			idr.logger.Debugf("Executing query to keep indices warm in cache")
			idr.elasticClient.Search(idr.ctx,
				fmt.Sprintf("%s.*", elastic.ExternalIndexPrefix),
				"",
				es.NewTermQuery("kind.keyword", "Node"),
				nil,
				0,
				10,
				"",
				true)
		}

		// Query once right away
		query()

		// Loop to query in background
		for {
			select {
			// Periodic timer callback to keep the indices warm
			// by periodically querying in the background
			case <-time.After(idr.indexRefreshIntvl):
				query()

			// Handle indexer stop/done event
			case <-idr.ctx.Done():
				idr.logger.Infof("Stopping refreshIndices(), indexer stopped")
				idr.Lock()
				idr.refreshIndicesRunning = false
				idr.Unlock()
				return
			}
		}
	}()
}

func (idr *Indexer) createIndexHelper(indexMapper globals.DataType, tenantName string) error {
	index := elastic.GetIndex(indexMapper, tenantName)
	mapping, err := idr.getIndexMapping(indexMapper)
	if err != nil {
		idr.logger.Errorf("Failed to get index mapping for index %s, mapper %s, err: %v", index, indexMapper, err)
		return err
	}
	if err := idr.elasticClient.CreateIndex(idr.ctx, index, mapping); err != nil {
		idr.logger.Errorf("Failed to create index: %s, err: %v", index, err)
		return err
	}
	idr.logger.Info("Created index ", index)
	return nil
}

func (idr *Indexer) deleteIndexHelper(indexMapper globals.DataType, tenantName string) error {
	index := elastic.GetIndex(indexMapper, tenantName)
	idr.elasticClient.DeleteIndex(idr.ctx, index)
	if err := idr.elasticClient.DeleteIndex(idr.ctx, index); err != nil && !elastic.IsIndexNotExists(err) {
		if elasticErr, ok := err.(*es.Error); ok {
			idr.logger.Errorf("Failed to delete index: %s, err: %+v", index, elasticErr)
		}
		return err
	}
	idr.logger.Info("Deleted index ", index)
	return nil
}

func createBucketClient(ctx context.Context, resolver resolver.Interface, tenantName string, bucketName string) (objstore.Client, error) {
	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		return nil, fmt.Errorf("Error getting tls provider (%s)", err)
	}

	if tlsp == nil {
		return objstore.NewClient(tenantName, bucketName, resolver)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		return nil, fmt.Errorf("Error getting tls client (%s)", err)
	}
	tlsc.ServerName = globals.Vos

	return objstore.NewClient(tenantName, bucketName, resolver, objstore.WithTLSConfig(tlsc))
}
