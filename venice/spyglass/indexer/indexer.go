// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package indexer

import (
	"context"
	"errors"
	"sync"
	"time"

	apiservice "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
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
	maxWriters        = 8 // Max concurrent writers
	indexMaxBuffer    = (maxWriters * indexBatchSize)
)

// Indexer is an implementation of the indexer.Interface
type Indexer struct {
	sync.RWMutex
	sync.WaitGroup
	ctx    context.Context
	logger log.Logger

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

	// Index poll interval. If the batchSize
	// is not met, this timer kicks in to perform
	// the bulk-index on accumulated objects
	indexIntvl time.Duration

	// Slice of Pending objects per Elastic Writer
	requests [][]*elastic.BulkRequest

	// Channel to stop processing
	done chan bool

	// Total count of objects indexed
	count uint64
}

// WatchHandler is handler func for watch events on API-server objects
type WatchHandler func(et kvstore.WatchEventType, obj interface{})

// indexRequest entry
type indexRequest struct {
	evType kvstore.WatchEventType
	object interface{}
}

// NewIndexer instantiates a new indexer
func NewIndexer(ctx context.Context, apiServerAddr string, rsr resolver.Interface, logger log.Logger) (Interface, error) {

	log.Debugf("Creating Indexer, apiserver-addr: %s", apiServerAddr)

	// Initialize elastic client
	result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return elastic.NewClient("", rsr, logger.WithContext("submodule", "elastic"))
	}, elasticWaitIntvl, maxElasticRetries)
	if err != nil {
		logger.Errorf("Failed to create elastic client, err: %v", err)
		return nil, err
	}
	logger.Debugf("Created elastic client")
	esClient := result.(elastic.ESClient)

	// Initialize api client
	result, err = utils.ExecuteWithRetry(func() (interface{}, error) {
		return apiservice.NewGrpcAPIClient(globals.Spyglass, apiServerAddr, logger, rpckit.WithBalancer(balancer.New(rsr)))
	}, apiSrvWaitIntvl, maxAPISrvRetries)
	if err != nil {
		log.Errorf("Failed to create api client, addr: %s err: %v",
			apiServerAddr, err)
		esClient.Close()
		return nil, err
	}
	log.Debugf("Created API client")
	apiClient := result.(apiservice.Services)

	indexer := Indexer{
		ctx:           ctx,
		apiServerAddr: apiServerAddr,
		elasticClient: esClient,
		apiClient:     apiClient,
		logger:        logger,
		watchers:      make(map[string]kvstore.Watcher),
		channels:      make(map[string]<-chan *kvstore.WatchEvent),
		reqChan:       make(chan *indexRequest, indexMaxBuffer),
		batchSize:     indexBatchSize,
		indexIntvl:    indexBatchIntvl,
		maxWriters:    maxWriters,
		requests:      make([][]*elastic.BulkRequest, maxWriters),
		done:          make(chan bool),
		count:         0,
	}

	log.Debugf("Created new indexer: {%+v}", &indexer)
	return &indexer, nil
}

// Start starts the watchers for API-server objects for Indexing
func (idr *Indexer) Start() error {

	idr.logger.Infof("Starting indexer")

	// initialize indexes
	err := idr.initSearchDB()
	if err != nil {
		log.Errorf("Failed to setup indices for search, err: %v", err)
		return err
	}

	// initialize the watchers
	err = idr.createWatchers()
	if err != nil {
		log.Errorf("Failed to create watchers, err: %v", err)
		// stop and cleanup watchers
		idr.stopWatchers()
		idr.watchers = nil
		return err
	}

	// start the watchers
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

	idr.stopWatchers()
	idr.stopWriters()
	idr.Wait()
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
