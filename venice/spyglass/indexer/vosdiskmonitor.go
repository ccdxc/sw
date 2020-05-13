package indexer

import (
	"encoding/json"
	"fmt"
	"strings"
	"sync"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	vosinternalprotos "github.com/pensando/sw/venice/vos/protos"
)

const (
	vosDiskMonitorID = 101
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
	objs, err := idr.getOldestFwLogObjects()
	if err != nil {
		idr.logger.Debugf("error in fetching old object keys from elastic, err: %s", err.Error())
		return
	}
	idr.logger.Infof("disk used capacity reached, total objects %d", len(objs))

	wg := sync.WaitGroup{}
	erroredObjs := map[string]map[string]struct{}{}
	for tenant, tenantObjs := range objs {
		objectCh := make(chan string)
		errorCh := idr.vosFwLogsHTTPClient.RemoveObjectsWithContext(
			idr.ctx,
			tenant+"."+fwlogsBucketName,
			objectCh)

		wg.Add(1)
		go func() {
			defer func() {
				wg.Done()
				close(objectCh)
			}()

			for _, obj := range tenantObjs {
				objectCh <- obj.Key
			}
		}()

		wg.Add(1)
		go func(tenantName string) {
			defer wg.Done()
			for err := range errorCh {
				eObjs, ok := erroredObjs[tenantName]
				if !ok {
					eObjs = map[string]struct{}{}
					erroredObjs[tenantName] = eObjs
				}
				eObjs[err.ObjectName] = struct{}{}
				idr.logger.Debugf("error while deleting object %s, err %+v", err.ObjectName, err.Err)
			}
		}(tenant)
	}

	wg.Wait()

	// Delete the objects from elastic's index as well
	objDeleteReqs := [][]*elastic.BulkRequest{}
	temp := []*elastic.BulkRequest{}
	for tenant, tenantObjs := range objs {
		for _, obj := range tenantObjs {
			if _, ok := erroredObjs[tenant][obj.Key]; !ok {
				// prepare the delete request
				request := &elastic.BulkRequest{
					RequestType: elastic.Delete,
					Index:       elastic.GetIndex(globals.FwLogsObjects, ""),
					IndexType:   elastic.GetDocType(globals.FwLogsObjects),
					ID:          getUUIDForFwlogObject("Object", tenant, fwlogsBucketName, obj.Key),
					Obj:         obj, // req.object
				}

				temp = append(temp, request)

				if len(temp) >= fwLogsElasticBatchSize {
					objDeleteReqs = append(objDeleteReqs, temp)
					temp = []*elastic.BulkRequest{}
				}
			}
		}

		if len(temp) != 0 {
			objDeleteReqs = append(objDeleteReqs, temp)
		}
	}

	for _, reqs := range objDeleteReqs {
		if len(reqs) != 0 {
			idr.logger.Infof("VosDiskMonitor: Calling Bulk Delete Api reached batchsize len: %d",
				len(reqs))
			idr.helper(vosDiskMonitorID, bulkTimeout, reqs, nil)
		}
	}
}

func (idr *Indexer) getOldestFwLogObjects() (map[string][]*FwLogObjectV1, error) {
	query := es.NewMatchAllQuery()
	maxResults := 10000
	if idr.numFwLogObjectsToDelete < 10000 {
		maxResults = idr.numFwLogObjectsToDelete
	}
	objectsFetched := 0

	// Per tenant objects
	objs := map[string][]*FwLogObjectV1{}

	for {
		if objectsFetched >= idr.numFwLogObjectsToDelete {
			break
		}
		if idr.numFwLogObjectsToDelete-objectsFetched > 10000 {
			maxResults = 10000
		} else {
			maxResults = idr.numFwLogObjectsToDelete - objectsFetched
		}

		// execute query
		result, err := idr.elasticClient.Search(idr.ctx,
			elastic.GetIndex(globals.FwLogsObjects, ""), // index
			"",                      // skip the index type
			query,                   // query to be executed
			nil,                     // no aggregation
			int32(objectsFetched+1), // from
			int32(maxResults),       // to
			"creationts",            // sorting is required
			true)                    // sort in desc order

		if err != nil {
			idr.logger.Errorf("failed to query elasticsearch, err: %+v", err)
			return nil, fmt.Errorf("failed to query elasticsearch, err: %+v", err)
		}

		if len(result.Hits.Hits) == 0 {
			break
		}

		// parse the result
		for _, res := range result.Hits.Hits {
			var obj FwLogObjectV1
			if err := json.Unmarshal(*res.Source, &obj); err != nil {
				idr.logger.Debugf("failed to unmarshal elasticsearch result, err: %+v", err)
				continue
			}
			objs[obj.Tenant] = append(objs[obj.Tenant], &obj)
			objectsFetched++
		}
	}

	idr.logger.Debugf("GetOldestFwLogObjects response: {%+v}", objs)

	return objs, nil
}
