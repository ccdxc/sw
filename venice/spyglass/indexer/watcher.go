// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package indexer

import (
	"context"
	"fmt"
	"net/http"
	"reflect"
	"strings"
	"sync/atomic"
	"time"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	indexRetryIntvl = (100 * time.Millisecond)
	indexMaxRetries = 5
	// Time in minutes to tolerate continuous write failures
	// before we drop the request
	failedWriteTimeout = 1 * time.Minute
)

type service interface {
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
}

// Create watchers for API-server objects
func (idr *Indexer) createWatchers() error {
	idr.Lock()
	defer idr.Unlock()

	idr.watchers = make(map[string]kvstore.Watcher)
	idr.channels = make(map[string]<-chan *kvstore.WatchEvent)
	if idr.writerMap == nil {
		idr.writerMap = make(map[string]*writerMapEntry)
	}

	groupMap := runtime.GetDefaultScheme().Kinds()
	apiClientVal := reflect.ValueOf(idr.apiClient)

	for group := range groupMap {
		// Objstore is handled separately below
		if group == "objstore" || group == "bookstore" {
			continue
		}
		// TODO: Remove hardcoded version
		version := "V1"
		key := strings.Title(group) + version
		groupFunc := apiClientVal.MethodByName(key)
		if !groupFunc.IsValid() {
			idr.logger.Infof("API Group %s does not have a watch method", key)
			continue
		}
		opts := api.ListWatchOptions{}
		if entry, ok := idr.writerMap[key]; ok && entry.resVersion != "" {
			opts.ObjectMeta = api.ObjectMeta{
				ResourceVersion: entry.resVersion,
			}
		}

		serviceGroup := groupFunc.Call(nil)
		watch, err := serviceGroup[0].Interface().(service).Watch(idr.ctx, &opts)
		err = idr.addWatcher(key, watch, err)
		if err != nil {
			return err
		}
	}

	// Handling objstore separately
	// Need to create multiple watches, one for each bucket type
	// Objstore currently does not support resource version
	// There is a possibility of missing a delete if this watch connection goes down briefly.
	// TODO: Add resource version
	if !idr.watchVos {
		idr.logger.Infof("Skipping VOS watchers")
		return nil
	}
	for _, bucket := range objstore.Buckets_name {
		key := fmt.Sprintf("objstore-%s", bucket)
		opts := api.ListWatchOptions{}
		opts.Tenant = globals.DefaultTenant
		// To watch on a bucket, bucket name must be provided as the namespace
		opts.Namespace = bucket
		watch, err := idr.vosClient.ObjstoreV1().Object().Watch(idr.ctx, &opts)
		err = idr.addWatcher(key, watch, err)
		if err != nil {
			// VOS does not support restarting a watch currently. Indexer should shutdown and recreate our index
			// incase we missed delete events
			idr.doneCh <- err
			return err
		}
	}

	return nil
}

func (idr *Indexer) addWatcher(key string, watch kvstore.Watcher, err error) error {
	if err != nil {
		apiErr := apierrors.FromError(err)
		switch apiErr.GetCode() {
		case http.StatusNotImplemented:
			idr.logger.Errorf("Error starting watcher for API Group %s, err unimplemented: %v", key, err)
			return err
		case http.StatusGone:
			// API Server no longer has records for our resource version.
			// Shutdown indexer and have spyglass recreate in case we missed delete events
			idr.logger.Errorf("Error starting watcher for API Group %s, outdated res version: %v", key, err)
			idr.doneCh <- err
			return err
		default:
			idr.logger.Errorf("Error starting watcher for API Group %s, err: %v", key, err)
			return err
		}
	} else {
		idr.logger.Infof("Created watcher for API Group %s", key)
		idr.watchers[key] = watch
		idr.channels[key] = watch.EventChan()
	}
	return nil
}

// Start watch handlers for api-server objects
func (idr *Indexer) startWatchers() {
	idr.logger.Info("Starting watchers")
	activateIndices := false
	idr.Add(1)
	go func() {

		defer idr.Done()

		// The following code snippet performs Select on a slice of channels
		// Initialize the SelectCase slice, with one channel per Kind and
		// and two additional channel to handle context cancellation and Done
		cases := make([]reflect.SelectCase, len(idr.channels)+2)
		apiGroupMappings := make([]string, len(idr.channels)+2)

		// Add the watcherDone and Ctx.Done() channels to initial offset of the slice
		cases[0] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(idr.watcherDone)}
		cases[1] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(idr.ctx.Done())}

		// Add the object watcher channels
		i := 2
		for key, ch := range idr.channels {
			cases[i] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ch)}
			apiGroupMappings[i] = key
			idr.writerMap[key] = &writerMapEntry{
				writerID: (i - 2) % maxWriters,
			}
			i++
		}

		// Handle event update on any of the channel
		// TODO: make this a utility function that can
		//       be used by other services as well.
		for {
			chosen, value, ok := reflect.Select(cases)

			// First handle special channels related to watcherDone and Ctx
			if chosen == 0 {
				idr.logger.Info("Exiting watcher, watcherDone event")
				return
			} else if chosen == 1 {
				idr.logger.Info("Exiting watcher, Ctx cancelled")
				idr.stopWatchers()
				return
			}

			// Handle the events updates from object watchers
			if !ok {
				// If the indexer is stopped or is in the process of being
				// stopped, this is an expected condition and no recovery is
				// needed. In all other cases, it is a watcher error that
				// needs to be re-established.
				if idr.GetRunningStatus() == true {
					err := fmt.Errorf("Channel read not ok for API Group %s", apiGroupMappings[chosen])
					idr.logger.Errorf(err.Error())
					// if its an error for objstore, we restart spyglass since we may have missed an event by the time we come back up
					if strings.Contains(apiGroupMappings[chosen], "Objstore") {
						idr.doneCh <- err
						return
					}
					idr.restartWatchers()
					return
				}

				// remove the channel that failed
				cases = append(cases[:chosen], cases[chosen+1:]...)
				continue
			}

			idr.logger.Debugf("Reading from channel %d and received event: {%+v} %s",
				chosen, value, value.String())
			apiGroup := apiGroupMappings[chosen]
			event := value.Interface().(*kvstore.WatchEvent)
			idr.handleWatcherEvent(apiGroup, event.Type, event.Object)

			// Launch the index refresher function first time
			// when there are objects ready to be indexed.
			// refreshIndices checks if a refresher is already running before
			// starting of a go routine,
			// so the watcher group restarting won't cause a leak
			if activateIndices == false {
				idr.refreshIndices()
				activateIndices = true
			}
		}
	}()
}

// Update write count
func (idr *Indexer) updateWriteCount(increment uint64) {
	atomic.AddUint64(&idr.count, increment)
}

// GetWriteCount returns total count of writes/updates to elastic
func (idr *Indexer) GetWriteCount() uint64 {
	return atomic.LoadUint64(&idr.count)
}

// handleWatcherEvent enqueue the watcher-event into the correct writers requestChannel
// NOTE: All events for an api group must always go to the same writer
func (idr *Indexer) handleWatcherEvent(apiGroup string, et kvstore.WatchEventType, obj interface{}) {
	// create request object and enqueue it to the request channel
	ometa, _ := runtime.GetObjectMeta(obj)
	resVersion := ometa.GetResourceVersion()
	kind := obj.(runtime.Object).GetObjectKind()
	req := &indexRequest{
		evType: et,
		object: obj,
		resVersionUpdate: func() {
			// No need for lock since only one writer will be updating this
			idr.writerMap[apiGroup].resVersion = resVersion
			idr.logger.Debugf("Updated resource version to %s for api group %s", resVersion, apiGroup)
		},
	}

	idr.logger.Debugf("event req for %s-%s : %+v to reqChan", kind, ometa.GetName(), resVersion, req)

	// TODO: if we see very prolonged or repeated blocking due to slow writers
	// we should wrap this channel send inside a select loop along with timer
	// and threshold counter to do recovery by doing either adaptive backoff or
	// increasing the writer pool size (bounded by NUMPROCS). This is TBD.
	idr.reqChan[idr.writerMap[apiGroup].writerID] <- req
}

// Start the Bulk/Batch writer to Elasticseach
func (idr *Indexer) startWriter(id int) {
	// input validation
	if id < 0 || id >= idr.maxWriters {
		idr.logger.Debugf("argID: %d out of range [%d .. %d]",
			id, 0, idr.maxWriters)
		return
	}

	idr.logger.Debugf("Starting Writer: %d to elasticsearch", id)
	idr.requests[id] = make([]*elastic.BulkRequest, 0, idr.batchSize)

	failedBulkCount := 0
	bulkTimeout := int((1 / (indexRetryIntvl.Seconds() * indexMaxRetries)) * failedWriteTimeout.Seconds())
	for {

		// If we fail to write for 5 min, we drop the request
		if failedBulkCount == bulkTimeout {
			idr.logger.Errorf("elastic write failed for %d seconds. so, dropping the request", bulkTimeout)
			failedBulkCount = 0
			idr.resVersionUpdater[id] = nil
			idr.requests[id] = nil
		}

		if len(idr.requests[id]) == idr.batchSize {
			select {
			case <-idr.ctx.Done():
				idr.logger.Infof("Stopping Writer: %d, ctx cancelled", id)
				return
			default:
				if idr.attemptSendBulkRequest(id) != nil {
					failedBulkCount++
				} else {
					failedBulkCount = 0
				}
			}
			continue
		}

		select {

		// handle context cancellation
		case <-idr.ctx.Done():
			idr.logger.Infof("Stopping Writer: %d, ctx cancelled", id)
			return

		// read Index Request from Request Channel
		case req, more := <-idr.reqChan[id]:

			if more == false {
				idr.logger.Debugf("Writer: %d Request channel is closed, Done", id)
				return
			}

			// get the object meta
			ometa, err := runtime.GetObjectMeta(req.object)
			if err != nil {
				idr.logger.Errorf("Writer: %d Failed to get obj-meta for object: %+v, err: %+v",
					id, req.object, err)
				continue
			}
			kind := req.object.(runtime.Object).GetObjectKind()
			uuid := ometa.GetUUID()
			if kind == "Object" && uuid == "" {
				// VOS objects do not have a UUID
				// Artificially create one using Objstore-Object-<tenant>-<namespace>-<meta.name>
				uuid = fmt.Sprintf("Objstore-%s-%s-%s-%s", kind, ometa.GetTenant(), ometa.GetNamespace(), ometa.GetName())
			}
			idr.logger.Infof("Writer %d, processing object: <%s %s %v %v>", id, kind, ometa.GetName(), uuid, req.evType)
			idr.logger.Debugf("Writer %d, processing object: <%s %s %v, %+v>", id, kind, ometa.GetName(), uuid, req)
			if uuid == "" {
				idr.logger.Errorf("Writer %d, object %s %s has no uuid", id, kind, ometa.GetName())
				// Skip indexing as write is guaranteed to fail without uuid
				continue
			}

			// determine the operation type based on event-type
			var reqType string
			switch req.evType {
			case kvstore.Created:
				reqType = elastic.Index
			case kvstore.Updated:
				reqType = elastic.Update
			case kvstore.Deleted:
				reqType = elastic.Delete
			}

			// Update Policy cache for SGpolicy objects
			switch kind {
			case "NetworkSecurityPolicy":
				idr.logger.Infof("Policy cache update: %s/%s/%s evType: %v",
					ometa.Tenant,
					ometa.Namespace,
					ometa.Name,
					req.evType)
				idr.updatePolicyCache(req.object, req.evType)
			}

			// TODO: Once the category is available in Kind attribute or a new Meta
			// attribute we will use it here. Until then, it is derived from this map.
			category := globals.Kind2Category(kind)
			if category == "" {
				idr.logger.Errorf("Couldn't find category for %s, setting default", kind)
				category = "default"
			}

			// Insert Category as Label in Meta
			// TODO: Remove this once api-server populates it for all policy objects
			if ometa.Labels == nil {
				ometa.Labels = make(map[string]string)
			}
			ometa.Labels[globals.CategoryLabel] = category

			// If the tenant is empty, add a default value to enable
			// aggregation by tenants. Elastic skips objects that have
			// empty strings in any of aggregatable-fields.
			if ometa.Tenant == "" {
				ometa.Tenant = globals.DefaultTenant
			}

			// prepare the index request
			request := &elastic.BulkRequest{
				RequestType: reqType,
				Index:       elastic.GetIndex(globals.Configs, globals.DefaultTenant),
				IndexType:   elastic.GetDocType(globals.Configs),
				ID:          uuid,
				Obj:         req.object,
			}
			idr.requests[id] = append(idr.requests[id], request)

			if idr.resVersionUpdater[id] == nil {
				idr.resVersionUpdater[id] = req.resVersionUpdate
			} else {
				currFunc := idr.resVersionUpdater[id]
				idr.resVersionUpdater[id] = func() {
					// Order is important. We must run previous updates before newer ones
					currFunc()
					req.resVersionUpdate()
				}
			}

			idr.logger.Debugf("Writer: %d pending-requests len:%d data:%v",
				id, len(idr.requests[id]), idr.requests[id])

			// check if batchSize is reached and call the bulk API
			count := len(idr.requests[id])
			if count >= idr.batchSize {

				// Send a bulk request
				idr.logger.Infof("Writer: %d Calling Bulk Api reached batchsize len: %d requests: %v",
					id,
					len(idr.requests[id]),
					idr.requests[id])

				if idr.attemptSendBulkRequest(id) != nil {
					failedBulkCount++
				} else {
					failedBulkCount = 0
				}
				break
			}

		// timer callback that fires every index-interval
		case <-time.After(idr.indexIntvl):

			// Batch any pending requests.
			count := len(idr.requests[id])
			if count > 0 {

				// Send a bulk request
				idr.logger.Infof("Writer: %d Calling Bulk Api len: %d requests: %v",
					id,
					len(idr.requests[id]),
					idr.requests[id])
				if idr.attemptSendBulkRequest(id) != nil {
					failedBulkCount++
				} else {
					failedBulkCount = 0
				}
				break
			}
		}
	}
}

// Attempts to send the buffered requests for the writer with the given id
func (idr *Indexer) attemptSendBulkRequest(id int) error {
	result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		return idr.elasticClient.Bulk(idr.ctx, idr.requests[id])
	}, indexRetryIntvl, indexMaxRetries)
	if err != nil {
		idr.logger.Errorf("Writer: %d Failed to perform bulk indexing, resp: %+v err: %+v",
			id, result, err)
		return err
	}

	idr.updateWriteCount(uint64(len(idr.requests[id])))

	updateFunc := idr.resVersionUpdater[id]
	updateFunc()

	idr.resVersionUpdater[id] = nil
	idr.requests[id] = nil
	return nil
}

// Stop all the watchers
func (idr *Indexer) stopWatchers() {
	select {
	case <-idr.watcherDone:
		idr.logger.Errorf("Stop watchers called but watcher channel is already closed")
		return
	default:
	}

	idr.Lock()
	defer idr.Unlock()

	for key, watcher := range idr.watchers {
		if watcher != nil {
			watcher.Stop()
		}
		idr.watchers[key] = nil
	}
	idr.watchers = nil
	idr.channels = nil
	close(idr.watcherDone)
	idr.logger.Info("Stopped watchers")
}

// Update the policy cache
func (idr *Indexer) updatePolicyCache(obj interface{}, evType kvstore.WatchEventType) error {

	var err error
	switch evType {
	case kvstore.Created:
		fallthrough
	case kvstore.Updated:
		err = idr.cache.UpdateObject(obj)
	case kvstore.Deleted:
		err = idr.cache.DeleteObject(obj)
	}

	return err
}
