package indexer

import (
	"context"
	"strings"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	vosinternalprotos "github.com/pensando/sw/venice/vos/protos"
)

func (idr *Indexer) createVosDiskMonitorWatcher() error {
	opts := api.ListWatchOptions{}
	cc := vosinternalprotos.NewObjstoreInternalServiceClient(idr.vosInternalClient.ClientConn)
	watch, err := cc.WatchDiskThresholdUpdates(idr.ctx, &opts)
	if err != nil {
		return err
	}
	idr.vosDiskUpdateWatcher = watch
	return nil
}

func (idr *Indexer) startVosDiskMonitorWatcher() error {
	outCh := make(chan *vosinternalprotos.DiskUpdate)
	idr.Add(1)
	go func() {
		for {
			// Recv is a blocking call. Hence running it in another go routine.
			// Copying functionality from generated code.
			r, err := idr.vosDiskUpdateWatcher.Recv()
			if err != nil {
				idr.logger.ErrorLog("msg", "error on receive", "err", err)
				close(outCh)
				return
			}

			select {
			case outCh <- r:
			case <-idr.vosDiskUpdateWatcher.Context().Done():
				close(outCh)
				return
			}
		}
	}()

	idr.Add(1)
	go func() {
		defer idr.Done()

		for {
			select {
			case <-idr.watcherDone:
				idr.vosDiskUpdateWatcher.CloseSend()
				idr.logger.Info("Exiting diskupdate watcher, watcherDone event")
				return
			case <-idr.ctx.Done():
				idr.logger.Info("Exiting diskupdate watcher, Ctx cancelled")
				return
			case event, ok := <-outCh:
				if !ok {
					idr.logger.Info("Exiting diskupdate watcher, channel closed")
					return
				}
				idr.handleVosDiskMonitorUpdate(event)
			}
		}
	}()
	return nil
}

func (idr *Indexer) handleVosDiskMonitorUpdate(obj *vosinternalprotos.DiskUpdate) {
	// we are only interetsed in the fwlog bucket notification
	if !strings.Contains(obj.Status.Path, fwlogsBucketName) {
		return
	}

	// Vos is calculating the threshold and generating this message only when the
	// threshold is reached.
	idr.logger.Infof("handling vos disk update %+v", obj)
	idr.cleanupOldObjects()
}

func (idr *Indexer) cleanupOldObjects() {
	l, err := idr.deleteOldestFwLogObjects()
	if err != nil {
		idr.logger.Debugf("error in deleting old object keys, err: %s", err.Error())
	}
	idr.logger.Infof("disk used capacity reached, total objects deleted %d", l)
}

func (idr *Indexer) deleteOldestFwLogObjects() (int, error) {
	query := es.NewMatchAllQuery()
	maxResults := 10000
	if idr.numFwLogObjectsToDelete < 10000 {
		maxResults = idr.numFwLogObjectsToDelete
	}
	deleted := 0
	for {
		if deleted >= idr.numFwLogObjectsToDelete {
			break
		}
		if idr.numFwLogObjectsToDelete-deleted > 10000 {
			maxResults = 10000
		} else {
			maxResults = idr.numFwLogObjectsToDelete - deleted
		}

		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return idr.elasticClient.DeleteByQuery(idr.ctx,
				elastic.GetIndex(globals.FwLogsObjects, ""), // index
				"",           // skip the index type
				query,        // query to be executed
				maxResults,   // to
				"creationts", // sorting is required
				false)        // sort in asc order
		}, indexRetryIntvl, indexMaxRetries)

		if err != nil {
			idr.logger.Errorf("failed to query elasticsearch, err: %+v", err)
			return deleted, err
		}

		deleted += int(result.(*es.BulkIndexByScrollResponse).Deleted)
	}
	return deleted, nil
}

// Dont remove this commented code.
// func (idr *Indexer) cleanupOldObjects() {
// 	objs, err := idr.getOldestFwLogObjects()
// 	if err != nil {
// 		idr.logger.Debugf("error in fetching old object keys from elastic, err: %s", err.Error())
// 		return
// 	}

// 	idr.logger.Infof("disk used capacity reached, total objects %d", len(objs))

// 	for _, obj := range objs {
// 		// Delete the object from Minio and then delete its index from elastic
// 		err = idr.vosFwLogsHTTPClient.RemoveObject(obj.Key)
// 		if err != nil {
// 			idr.logger.Errorf("error in removing old object from vos, key %s", obj.Key)
// 			continue
// 		}

// 		err = idr.elasticClient.Delete(idr.ctx,
// 			elastic.GetIndex(globals.FwLogsObjects, ""),
// 			elastic.GetDocType(globals.FwLogsObjects),
// 			getUUIDForFwlogObject("Object", "fwlogs", "fwlogs", obj.Key))

// 		if err != nil {
// 			idr.logger.Errorf("error in deleting old object index from elastic, key %s, err %+v", obj.Key, err)
// 		}
// 	}
// }

// func (idr *Indexer) getOldestFwLogObjects() ([]*FwLogObjectV1, error) {
// 	query := es.NewMatchQuery("bucket", "fwlogs")
// 	maxResults := 10000
// 	if idr.numFwLogObjectsToDelete < 10000 {
// 		maxResults = idr.numFwLogObjectsToDelete
// 	}
// 	objectsFetched := 0
// 	objs := []*FwLogObjectV1{}
// 	for {
// 		if objectsFetched >= idr.numFwLogObjectsToDelete {
// 			break
// 		}
// 		if idr.numFwLogObjectsToDelete-objectsFetched > 10000 {
// 			maxResults = 10000
// 		} else {
// 			maxResults = idr.numFwLogObjectsToDelete - objectsFetched
// 		}

// 		// execute query
// 		result, err := idr.elasticClient.Search(idr.ctx,
// 			elastic.GetIndex(globals.FwLogsObjects, ""), // index
// 			"",                // skip the index type
// 			query,             // query to be executed
// 			nil,               // no aggregation
// 			0,                 // from
// 			int32(maxResults), // to
// 			"creationTs",      // sorting is required
// 			true)              // sort in desc order

// 		if err != nil {
// 			idr.logger.Errorf("failed to query elasticsearch, err: %+v", err)
// 			return nil, fmt.Errorf("failed to query elasticsearch, err: %+v", err)
// 		}

// 		if len(result.Hits.Hits) == 0 {
// 			break
// 		}

// 		// parse the result
// 		for _, res := range result.Hits.Hits {
// 			var obj FwLogObjectV1
// 			if err := json.Unmarshal(*res.Source, &obj); err != nil {
// 				idr.logger.Debugf("failed to unmarshal elasticsearch result, err: %+v", err)
// 				continue
// 			}
// 			objs = append(objs, &obj)
// 			objectsFetched++
// 		}
// 	}

// 	idr.logger.Debugf("GetOldestFwLogObjects response: {%+v}", objs)

// 	return objs, nil
// }
