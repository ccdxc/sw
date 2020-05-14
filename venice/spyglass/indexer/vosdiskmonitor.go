package indexer

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/rpckit"
	vosinternalprotos "github.com/pensando/sw/venice/vos/protos"
)

const (
	vosDiskMonitorID       = 101
	cleanIntervalInMinutes = 10 // minutes
)

type vosDiskWatcher struct {
	ctx context.Context

	wg *sync.WaitGroup

	isCleanupRunning bool

	lastCleanupTime time.Time

	numFwLogObjectsToDelete int

	logger log.Logger

	// Vos disk update watcher
	vosDiskUpdateWatcher vosinternalprotos.ObjstoreInternalService_WatchDiskThresholdUpdatesClient

	elasticClient elastic.ESClient

	vosFwlogsHTTPClient objstore.Client
}

func newVosDiskWatcher(ctx context.Context,
	wg *sync.WaitGroup,
	logger log.Logger,
	numFwLogObjectsToDelete int,
	vosInternalClient *rpckit.RPCClient,
	elasticClient elastic.ESClient,
	vosFwlogsHTTPClient objstore.Client) (*vosDiskWatcher, error) {
	opts := api.ListWatchOptions{}
	cc := vosinternalprotos.NewObjstoreInternalServiceClient(vosInternalClient.ClientConn)
	watch, err := cc.WatchDiskThresholdUpdates(ctx, &opts)
	if err != nil {
		return nil, err
	}
	return &vosDiskWatcher{ctx: ctx, wg: wg, numFwLogObjectsToDelete: numFwLogObjectsToDelete,
		vosDiskUpdateWatcher: watch, logger: logger, elasticClient: elasticClient,
		vosFwlogsHTTPClient: vosFwlogsHTTPClient, lastCleanupTime: time.Time{},
		isCleanupRunning: false}, nil
}

func (vw *vosDiskWatcher) StopWatchers() {
	if vw.vosDiskUpdateWatcher != nil {
		vw.vosDiskUpdateWatcher.CloseSend()
	}
}

func (vw *vosDiskWatcher) startVosDiskMonitorWatcher(doneCh <-chan bool) error {
	outCh := make(chan *vosinternalprotos.DiskUpdate)
	vw.wg.Add(1)
	go func() {
		defer vw.wg.Done()
		for {
			// Recv is a blocking call. Hence running it in another go routine.
			// Copying functionality from generated code.
			r, err := vw.vosDiskUpdateWatcher.Recv()
			if err != nil {
				vw.logger.ErrorLog("msg", "error on receive", "err", err)
				close(outCh)
				return
			}

			select {
			case outCh <- r:
			case <-vw.vosDiskUpdateWatcher.Context().Done():
				close(outCh)
				return
			}
		}
	}()

	vw.wg.Add(1)
	go func() {
		defer vw.wg.Done()

		for {
			select {
			case <-doneCh:
				vw.vosDiskUpdateWatcher.CloseSend()
				vw.logger.Info("Exiting diskupdate watcher, watcherDone event")
				return
			case <-vw.ctx.Done():
				vw.logger.Info("Exiting diskupdate watcher, Ctx cancelled")
				return
			case event, ok := <-outCh:
				if !ok {
					vw.logger.Info("Exiting diskupdate watcher, channel closed")
					return
				}
				vw.handleVosDiskMonitorUpdate(event)
			}
		}
	}()
	return nil
}

func (vw *vosDiskWatcher) handleVosDiskMonitorUpdate(obj *vosinternalprotos.DiskUpdate) {
	// we are only interetsed in the fwlog bucket notification
	if !strings.Contains(obj.Status.Path, fwlogsBucketName) {
		return
	}

	// Vos is calculating the threshold and generating this message only when the
	// threshold is reached.
	vw.logger.Infof("handling vos disk update %+v", obj)

	if vw.isCleanupRunning {
		vw.logger.Infof("cleanup is already runnin, ignoring vos disk update %+v", obj)
		return
	}

	// Dont cleanup within 10 minute interval, these could be stale messages
	if !vw.lastCleanupTime.IsZero() && time.Now().Sub(vw.lastCleanupTime).Minutes() < cleanIntervalInMinutes {
		vw.logger.Infof("ignoring vos disk update %+v", obj)
		return
	}
	vw.cleanupOldObjects()
}

func (vw *vosDiskWatcher) cleanupOldObjects() {
	vw.isCleanupRunning = true
	defer func() {
		vw.isCleanupRunning = false
	}()

	maxResults := 10000
	if vw.numFwLogObjectsToDelete < 10000 {
		maxResults = vw.numFwLogObjectsToDelete
	}
	objectsFetched := 0

	for {
		if objectsFetched >= vw.numFwLogObjectsToDelete {
			break
		}

		if vw.numFwLogObjectsToDelete-objectsFetched > 10000 {
			maxResults = 10000
		} else {
			maxResults = vw.numFwLogObjectsToDelete - objectsFetched
		}

		objs, num, err := vw.getOldestFwLogObjects(maxResults)
		if err != nil {
			vw.logger.Debugf("error in fetching old object keys from elastic, err: %s", err.Error())
			return
		}

		objectsFetched += num

		vw.logger.Infof("disk used capacity reached, total objects %d", len(objs))

		wg := sync.WaitGroup{}
		erroredObjs := map[string]map[string]struct{}{}
		for tenant, tenantObjs := range objs {
			objectCh := make(chan string)
			errorCh := vw.vosFwlogsHTTPClient.RemoveObjectsWithContext(
				vw.ctx,
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
					vw.logger.Debugf("error while deleting object %s, err %+v", err.ObjectName, err.Err)
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
				vw.logger.Infof("VosDiskMonitor: Calling Bulk Delete Api reached batchsize len: %d",
					len(reqs))
				helper(vw.ctx, vosDiskMonitorID, vw.logger, vw.elasticClient, bulkTimeout, reqs, nil)
			}
		}
	}

	vw.lastCleanupTime = time.Now()
}

func (vw *vosDiskWatcher) getOldestFwLogObjects(maxResults int) (map[string][]*FwLogObjectV1, int, error) {
	query := es.NewMatchAllQuery()
	objectsFetched := 0

	// Per tenant objects
	objs := map[string][]*FwLogObjectV1{}

	// execute query
	result, err := vw.elasticClient.Search(vw.ctx,
		elastic.GetIndex(globals.FwLogsObjects, ""), // index
		"",                // skip the index type
		query,             // query to be executed
		nil,               // no aggregation
		0,                 // from
		int32(maxResults), // to
		"creationts",      // sorting is required
		true)              // sort in desc order

	if err != nil {
		vw.logger.Errorf("failed to query elasticsearch, err: %+v", err)
		return nil, 0, fmt.Errorf("failed to query elasticsearch, err: %+v", err)
	}

	// parse the result
	for _, res := range result.Hits.Hits {
		var obj FwLogObjectV1
		if err := json.Unmarshal(*res.Source, &obj); err != nil {
			vw.logger.Debugf("failed to unmarshal elasticsearch result, err: %+v", err)
			continue
		}
		objs[obj.Tenant] = append(objs[obj.Tenant], &obj)
		objectsFetched++
	}

	vw.logger.Debugf("GetOldestFwLogObjects response: {%+v}", objs)

	return objs, objectsFetched, nil
}
