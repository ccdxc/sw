package indexer

import (
	"context"
	"time"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
)

// Start the Bulk/Batch writer to Elasticsearch
// It doesnt maintain any order for sending data to Elastic.
// Its supposed to be used for append only usecases like logs.
func (idr *Indexer) startAppendOnlyWriter(id int,
	batchSize,
	elasticWorkersSize,
	workersBufferSize int,
	requestCreator func(int, *indexRequest, int, *workers) error) {

	// Workers for running elastic writes
	workers := newWorkers(idr.ctx, elasticWorkersSize, workersBufferSize)

	idr.logger.Infof("Starting scaled writer: %d to elasticsearch batchSize: %d", id, batchSize)
	idr.requests[id] = make([]*elastic.BulkRequest, 0, batchSize)

	bulkTimeout := int((1 / (indexRetryIntvl.Seconds() * indexMaxRetries)) * failedWriteTimeout.Seconds())
	for {
		select {

		// handle context cancellation
		case <-idr.ctx.Done():
			idr.logger.Infof("Stopping Writer: %d, ctx cancelled", id)
			return

		// read Index Request from Request Channel
		case req, more := <-idr.reqChan[id]:
			if more == false {
				idr.logger.Infof("Writer: %d Request channel is closed, Done", id)
				return
			}

			if err := requestCreator(id, req, bulkTimeout, workers); err != nil {
				continue
			}

			idr.logger.Debugf("Writer: %d pending-requests len:%d data:%v",
				id, len(idr.requests[id]), idr.requests[id])

			// check if batchSize is reached and call the bulk API
			if len(idr.requests[id]) >= batchSize {
				idr.helper(id, bulkTimeout, idr.requests[id])
				idr.updateIndexer(id)
				idr.requests[id] = make([]*elastic.BulkRequest, 0, batchSize)
			}

		// timer callback that fires every index-interval
		case <-time.After(idr.indexIntvl):
			idr.helper(id, bulkTimeout, idr.requests[id])
			idr.updateIndexer(id)
			idr.requests[id] = make([]*elastic.BulkRequest, 0, batchSize)
		}
	}
}

func (idr *Indexer) helper(id, timeout int, reqs []*elastic.BulkRequest) {
	failedBulkCount := 0
	// Batch any pending requests.
	if len(reqs) > 0 {
		// Send a bulk request
		idr.logger.Debugf("Writer: %d Calling Bulk Api len: %d requests",
			id,
			len(reqs))

		go func(reqs []*elastic.BulkRequest) {
			for {
				if failedBulkCount == timeout {
					idr.logger.Errorf("Writer: %d elastic write failed for %d seconds. so, dropping the request", id, timeout)
					break
				}

				result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
					return idr.elasticClient.Bulk(idr.ctx, reqs)
				}, indexRetryIntvl, indexMaxRetries)

				if err != nil {
					idr.logger.Errorf("Writer: %d Failed to perform bulk indexing, resp: %+v err: %+v",
						id, result, err)
					failedBulkCount++
					continue
				}

				idr.logger.Infof("Writer: %d Bulk request succeeded after (%d) failures", id, failedBulkCount)
				failedBulkCount = 0
				break
			}
		}(reqs)
	}
}
