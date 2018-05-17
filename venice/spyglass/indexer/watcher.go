// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package indexer

import (
	"reflect"
	"sync/atomic"
	"time"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	indexRetryIntvl = (100 * time.Millisecond)
	indexMaxRetries = 5
)

// API server object-kinds of interest for search
// TODO: Using a enumerated slice for now, until we have
// support from infra to watch all or watch by apiGroup.
var apiObjects = []string{

	// app objects
	"App",
	"AppUser",
	"AppUserGrp",

	// auth objects
	"User",
	"AuthenticationPolicy",

	// cmd objects
	"Cluster",
	"Node",
	"SmartNIC",

	// events objects
	"EventPolicy",

	// network objects
	"Endpoint",
	"LbPolicy",
	"Network",
	"SecurityGroup",
	"Service",
	"Sgpolicy",
	"Tenant",

	// networkencryption objects
	"TrafficEncryptionPolicy",

	// telemetry objects
	"StatsPolicy",
	"FwlogPolicy",
	"FlowExportPolicy",

	// x509 objects
	"Certificate",
}

// Create watchers for API-server objects
//
// TODO:
// 1. Need an elegant way/iterator to create watchers
//    per Kind, instead of this manual enumeration
// 2. Cache the resourceVersion per Object Kind and
//    use it when creating watchers when handling
//    failures or restarts.
func (idr *Indexer) createWatchers() error {

	idr.Lock()
	defer idr.Unlock()

	var err error
	opts := api.ListWatchOptions{}

	idr.watchers["App"], err = idr.apiClient.SecurityV1().App().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for app.App object, err: %v", err)
		return err
	}
	idr.channels["App"] = idr.watchers["App"].EventChan()

	idr.watchers["AppUser"], err = idr.apiClient.SecurityV1().AppUser().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for app.AppUser object, err: %v", err)
		return err
	}
	idr.channels["AppUser"] = idr.watchers["AppUser"].EventChan()

	idr.watchers["AppUserGrp"], err = idr.apiClient.SecurityV1().AppUserGrp().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for app.AppUser object, err: %v", err)
		return err
	}
	idr.channels["AppUserGrp"] = idr.watchers["AppUserGrp"].EventChan()

	idr.watchers["User"], err = idr.apiClient.AuthV1().User().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for auth.User object, err: %v", err)
		return err
	}
	idr.channels["User"] = idr.watchers["User"].EventChan()

	idr.watchers["AuthenticationPolicy"], err = idr.apiClient.AuthV1().AuthenticationPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for auth.AuthenticationPolicy object, err: %v", err)
		return err
	}
	idr.channels["AuthenticationPolicy"] = idr.watchers["AuthenticationPolicy"].EventChan()

	idr.watchers["Cluster"], err = idr.apiClient.ClusterV1().Cluster().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for cmd.Cluster object, err: %v", err)
		return err
	}
	idr.channels["Cluster"] = idr.watchers["Cluster"].EventChan()

	idr.watchers["Node"], err = idr.apiClient.ClusterV1().Node().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for cmd.Node object, err: %v", err)
		return err
	}
	idr.channels["Node"] = idr.watchers["Node"].EventChan()

	idr.watchers["SmartNIC"], err = idr.apiClient.ClusterV1().SmartNIC().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for cmd.SmartNIC object, err: %v", err)
		return err
	}
	idr.channels["SmartNIC"] = idr.watchers["SmartNIC"].EventChan()

	idr.watchers["EventPolicy"], err = idr.apiClient.MonitoringV1().EventPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for events.EventPolicy object, err: %v", err)
		return err
	}
	idr.channels["EventPolicy"] = idr.watchers["EventPolicy"].EventChan()

	idr.watchers["Endpoint"], err = idr.apiClient.WorkloadV1().Endpoint().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.Endpoint object, err: %v", err)
		return err
	}
	idr.channels["Endpoint"] = idr.watchers["Endpoint"].EventChan()

	idr.watchers["LbPolicy"], err = idr.apiClient.NetworkV1().LbPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.LbPolicy object, err: %v", err)
		return err
	}
	idr.channels["LbPolicy"] = idr.watchers["LbPolicy"].EventChan()

	idr.watchers["Network"], err = idr.apiClient.NetworkV1().Network().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.Network object, err: %v", err)
		return err
	}
	idr.channels["Network"] = idr.watchers["Network"].EventChan()

	idr.watchers["SecurityGroup"], err = idr.apiClient.SecurityV1().SecurityGroup().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.SecurityGroup object, err: %v", err)
		return err
	}
	idr.channels["SecurityGroup"] = idr.watchers["SecurityGroup"].EventChan()

	idr.watchers["Service"], err = idr.apiClient.NetworkV1().Service().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.Service object, err: %v", err)
		return err
	}
	idr.channels["Service"] = idr.watchers["Service"].EventChan()

	idr.watchers["Sgpolicy"], err = idr.apiClient.SecurityV1().Sgpolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.Sgpolicy object, err: %v", err)
		return err
	}
	idr.channels["Sgpolicy"] = idr.watchers["Sgpolicy"].EventChan()

	idr.watchers["Tenant"], err = idr.apiClient.ClusterV1().Tenant().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for network.Tenant object, err: %v", err)
		return err
	}
	idr.channels["Tenant"] = idr.watchers["Tenant"].EventChan()

	idr.watchers["TrafficEncryptionPolicy"], err = idr.apiClient.SecurityV1().TrafficEncryptionPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for networkencryption.TrafficEncryptionPolicy object, err: %v", err)
		return err
	}
	idr.channels["TrafficEncryptionPolicy"] = idr.watchers["TrafficEncryptionPolicy"].EventChan()

	idr.watchers["StatsPolicy"], err = idr.apiClient.MonitoringV1().StatsPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for telemetry.StatsPolicy object, err: %v", err)
		return err
	}
	idr.channels["StatsPolicy"] = idr.watchers["StatsPolicy"].EventChan()

	idr.watchers["FwlogPolicy"], err = idr.apiClient.MonitoringV1().FwlogPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for telemetry.FwlogPolicy object, err: %v", err)
		return err
	}
	idr.channels["FwlogPolicy"] = idr.watchers["FwlogPolicy"].EventChan()

	idr.watchers["FlowExportPolicy"], err = idr.apiClient.MonitoringV1().FlowExportPolicy().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for telemetry.FlowExportPolicy object, err: %v", err)
		return err
	}
	idr.channels["FlowExportPolicy"] = idr.watchers["FlowExportPolicy"].EventChan()

	idr.watchers["Certificate"], err = idr.apiClient.SecurityV1().Certificate().Watch(idr.ctx, &opts)
	if err != nil {
		idr.logger.Errorf("Error starting watcher for x509.Certificate object, err: %v", err)
		return err
	}
	idr.channels["Certificate"] = idr.watchers["Certificate"].EventChan()

	return nil
}

// Start watch handlers for api-server objects
func (idr *Indexer) startWatchers() {

	go func() {

		// The following code snipped performs Select on a slice of channels
		// Initialize the SelectCase slice, with one channel per Kind and
		// and two additional channel to handle context cancellation and Done
		cases := make([]reflect.SelectCase, len(idr.channels)+2)

		// Add the Done and Ctx.Done() channels to initial offset of the slice
		cases[0] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(idr.done)}
		cases[1] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(idr.ctx.Done())}

		// Add the object watcher channels
		i := 2
		for _, ch := range idr.channels {
			cases[i] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ch)}
			i++
		}

		// Handle event update on any of the channel
		// TODO: make this a utility function that can
		//       be used by other services as well.
		for {
			chosen, value, ok := reflect.Select(cases)

			// First handle special channels related to Done and Ctx
			if chosen == 0 {
				log.Debugf("Exiting watcher, Done ")
				return
			} else if chosen == 1 {
				log.Debugf("Exiting watcher, Ctx cancelled")
				idr.stopWatchers()
				return
			}

			// Handle the events updates from object watchers
			if !ok {
				// If any of the channel is closed, restart the watcher
				idr.stopWatchers()
				go idr.Start()
				return
			}

			log.Debugf(" Reading from channel %d and received event: {%+v} %s",
				chosen, value, value.String())
			event := value.Interface().(*kvstore.WatchEvent)
			idr.handleWatcherEvent(event.Type, event.Object)
		}
	}()
}

// Update object count
func (idr *Indexer) updateCount(increment uint64) {
	atomic.AddUint64(&idr.count, increment)
}

// GetObjectCount returns count of total objects indexed
func (idr *Indexer) GetObjectCount() uint64 {
	return atomic.LoadUint64(&idr.count)
}

// handleWatcherEvent enqueue the watcher-event into requestChannel
func (idr *Indexer) handleWatcherEvent(et kvstore.WatchEventType, obj interface{}) {

	// create request object and enqueue it to the request channel
	req := &indexRequest{
		evType: et,
		object: obj,
	}
	log.Debugf("handleWatcherEvent - writing req: %+v to reqChan", req)

	// TODO: if we see very prolonged or repeated blocking due to slow writers
	// we should wrap this channel send inside a select loop along with timer
	// and threshold counter to do recovery by doing either adaptive backoff or
	// increasing the writer pool size (bounded by NUMPROCS). This is TBD.
	idr.reqChan <- req
}

// Start the Bulk/Batch writer to Elasticseach
func (idr *Indexer) startWriter(id int) {

	// input validation
	if id < 0 || id >= idr.maxWriters {
		log.Debugf("argID: %d out of range [%d .. %d]",
			id, 0, idr.maxWriters)
		return
	}

	log.Debugf("Starting Writer: %d to Elasticsearch", id)
	//idr.reqCount[id] = 0
	idr.requests[id] = make([]*elastic.BulkRequest, 0, idr.batchSize)

	for {

		select {

		// read Index Request from Request Channel
		case req, more := <-idr.reqChan:

			if more == false {
				log.Debugf("Writer: %d Request channel is closed, Done", id)
				return
			}

			log.Debugf("Writer: %d, got request from channel {%+v}", id, req)

			// get the object meta
			ometa, err := runtime.GetObjectMeta(req.object)
			if err != nil {
				log.Errorf("Writer: %d Failed to get obj-meta for object: %+v, err: %+v",
					id, ometa, err)
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

			log.Debugf("Writer: %d processing object: <%s %s> count: %d",
				id, ometa.GetName(), req.evType,
				len(idr.requests[id]))

			// TODO: Once the category is available in Kind attribute or a new Meta
			// attribute we will use it here. Until then, it is derived from this map.
			category := globals.Kind2Category[req.object.(runtime.Object).GetObjectKind()]
			if category == "" {
				category = "default"
			}

			// Insert Category as Label in Meta
			// TODO: Remove this once api-server populates it for all policy objects
			if ometa.Labels == nil {
				ometa.Labels = make(map[string]string)
			}
			ometa.Labels["_category"] = category

			// prepare the index request
			request := &elastic.BulkRequest{
				RequestType: reqType,
				Index:       elastic.GetIndex(globals.Configs, globals.DefaultTenant),
				IndexType:   elastic.GetDocType(globals.Configs),
				ID:          ometa.GetUUID(),
				Obj:         req.object,
			}
			idr.requests[id] = append(idr.requests[id], request)
			log.Debugf("Writer: %d pending-requests len:%d data:%v",
				id, len(idr.requests[id]), idr.requests[id])

			// check if batchSize is reached and call the bulk API
			count := len(idr.requests[id])
			if count >= idr.batchSize {

				// Send a bulk request
				log.Infof("Writer: %d Calling Bulk Api reached batchsize len: %d requests: %v",
					id,
					len(idr.requests[id]),
					idr.requests[id])

				// Perform Bulk index with inline retries
				result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
					return idr.elasticClient.Bulk(idr.ctx, idr.requests[id])
				}, indexRetryIntvl, indexMaxRetries)
				if err != nil {
					log.Errorf("Writer: %d Failed to perform bulk indexing, resp: %+v err: %+v",
						id, result, err)
					// TODO: Need to add recovery for perisisting errors
					//       - Reset connection and restart (tbd)
				}
				idr.updateCount(uint64(count))
				idr.requests[id] = nil
				break
			}

		// timer callback that fires every index-interval
		case <-time.After(idr.indexIntvl):

			// Batch any pending requests.
			count := len(idr.requests[id])
			if count > 0 {

				// Send a bulk request
				log.Infof("Writer: %d Calling Bulk Api len: %d requests: %v",
					id,
					len(idr.requests[id]),
					idr.requests[id])
				resp, err := idr.elasticClient.Bulk(idr.ctx, idr.requests[id])
				if err != nil {
					log.Errorf("Writer: %d Failed to perform bulk indexing, resp: %+v err: %+v",
						id, resp, err)
				}
				idr.updateCount(uint64(count))
				idr.requests[id] = nil
				break
			}

		// handle context cancellation
		case <-idr.ctx.Done():
			log.Debugf("Stopping Writer: %d, ctx cancelled", id)
			return
		}
	}
}

// Stop all the watchers
func (idr *Indexer) stopWatchers() {

	for key, watcher := range idr.watchers {
		if watcher != nil {
			watcher.Stop()
		}
		idr.watchers[key] = nil
	}
	close(idr.done)
	idr.logger.Info("Stopped watchers")
}

// Stop all the Writers
func (idr *Indexer) stopWriters() {

	close(idr.reqChan)
	idr.logger.Info("Stopped writers")
}
