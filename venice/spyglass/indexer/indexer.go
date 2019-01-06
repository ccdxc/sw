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
	elastic "github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	elasticWaitIntvl  = time.Second
	maxElasticRetries = 200
	apiSrvWaitIntvl   = time.Second
	maxAPISrvRetries  = 200
	indexBatchSize    = 100
	indexBatchIntvl   = 5 * time.Second
	indexRefreshIntvl = 60 * time.Second
	maxWriters        = 8 // Max concurrent writers
	indexMaxBuffer    = (maxWriters * indexBatchSize)
)

// Option fills the optional params for Finder
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

	// ElasticDB client object
	elasticClient elastic.ESClient

	// Map of KV store watchers per Object Kind
	watchers map[string]kvstore.Watcher

	// Map of KV store recv channel per Object kind
	// to receive WatchEvent
	// TODO: Move to single consolidated watcher once
	//       apiserver supports it.
	channels map[string]<-chan *kvstore.WatchEvent

	// Fan-in channel of index request objects.
	// The watcher events are fanned to a single
	// requestChannel. A pool of Elastic Writers
	// listen on this channel and divide the work
	// of indexing among them using a Worker-Pool
	// pattern
	reqChan chan *indexRequest

	// Max number of objects indexed using
	// Bulk/Batch API
	batchSize int

	// Max number of Elastic writers acting
	// as worker-pool to index incoming objects
	maxWriters int

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

	// Channel to stop processing
	done chan bool

	// Total count of objects indexed
	count uint64

	// Running status of the indexer
	// Used by Set/GetRunningStatus() functions
	runningStatus uint32

	// Policy cache
	cache cache.Interface
}

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(fdr *Indexer) {
		fdr.elasticClient = esClient
	}
}

// WatchHandler is handler func for watch events on API-server objects
type WatchHandler func(et kvstore.WatchEventType, obj interface{})

// indexRequest entry
type indexRequest struct {
	evType kvstore.WatchEventType
	object interface{}
}

// NewIndexer instantiates a new indexer
func NewIndexer(ctx context.Context, apiServerAddr string, rsr resolver.Interface, cache cache.Interface, logger log.Logger, opts ...Option) (Interface, error) {

	log.Debugf("Creating Indexer, apiserver-addr: %s", apiServerAddr)

	newCtx, cancelFunc := context.WithCancel(ctx)
	indexer := Indexer{
		ctx:               newCtx,
		cancelFunc:        cancelFunc,
		apiServerAddr:     apiServerAddr,
		logger:            logger,
		watchers:          make(map[string]kvstore.Watcher),
		channels:          make(map[string]<-chan *kvstore.WatchEvent),
		reqChan:           make(chan *indexRequest, indexMaxBuffer),
		batchSize:         indexBatchSize,
		indexIntvl:        indexBatchIntvl,
		indexRefreshIntvl: indexRefreshIntvl,
		maxWriters:        maxWriters,
		requests:          make([][]*elastic.BulkRequest, maxWriters),
		done:              make(chan bool),
		count:             0,
		cache:             cache,
	}

	for _, opt := range opts {
		if opt != nil {
			opt(&indexer)
		}
	}

	if indexer.elasticClient == nil {
		// Initialize elastic client
		result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
			return elastic.NewAuthenticatedClient("", rsr, logger.WithContext("submodule", "elastic"))
		}, elasticWaitIntvl, maxElasticRetries)
		if err != nil {
			log.Errorf("Failed to create elastic client, err: %v", err)
			return nil, err
		}
		log.Debugf("Created elastic client")
		indexer.elasticClient = result.(elastic.ESClient)
	}

	// Initialize api client
	result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return apiservice.NewGrpcAPIClient(globals.Spyglass, apiServerAddr, logger, rpckit.WithBalancer(balancer.New(rsr)))
	}, apiSrvWaitIntvl, maxAPISrvRetries)
	if err != nil {
		log.Errorf("Failed to create api client, addr: %s err: %v",
			apiServerAddr, err)
		indexer.elasticClient.Close()
		return nil, err
	}
	log.Debugf("Created API client")
	indexer.apiClient = result.(apiservice.Services)

	log.Infof("Created new indexer: {%+v}", &indexer)
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

	// initialize the watchers
	// TODO: need to reinitialize done channel to take care of Stop/Start scenario
	//       that can happen when watchers are reset and reestablished due to error.
	err = idr.createWatchers()
	if err != nil {
		idr.logger.Errorf("Failed to create watchers, err: %v", err)
		// stop and cleanup watchers
		idr.stopWatchers()
		idr.watchers = nil
		return err
	}

	// start the watchers
	idr.SetRunningStatus(true)
	idr.startWatchers()

	// start the Elastic Writer Pool
	for i := 0; i < idr.maxWriters; i++ {
		idr.Add(1)
		go func(id int) {
			defer idr.Done()
			idr.startWriter(id)
		}(i)
	}

	return nil
}

// Stop stops all the watchers for API-server objects
func (idr *Indexer) Stop() {
	if idr.GetRunningStatus() == false {
		return
	}

	idr.SetRunningStatus(false)
	idr.cancelFunc()
	idr.Wait()         // wait for all the watchers and writers to stop (<-ctx.Done())
	close(idr.reqChan) // close the channel where the writers were receiving the request from
	idr.elasticClient.Close()
	idr.apiClient.Close()
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
		return errors.New("Args mismatch")
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

	// Create index and mapping for Policy objects
	index := elastic.GetIndex(globals.Configs, globals.DefaultTenant)
	mapping, err := idr.getIndexMapping(globals.Configs)
	if err != nil {
		idr.logger.Errorf("Failed to get index mapping, err: %v", err)
		return err
	}
	if err := idr.elasticClient.CreateIndex(idr.ctx, index, mapping); err != nil && !elastic.IsIndexExists(err) {
		idr.logger.Errorf("Failed to create index: %s, err: %v, %v", index, err, elastic.IsIndexExists(err))
		return err
	}

	return nil
}

// Function to keep the elastic indices warm in memory
// by periodically querying in the background. This helps
// reduce the intital search latency by 2-5x order of
// magnitude.
func (idr *Indexer) refreshIndices() {

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
		case <-idr.done:
			idr.logger.Infof("Stopping refreshIndices(), indexer stopped")
			return
		}
	}
}
