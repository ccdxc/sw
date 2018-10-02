// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"context"
	"fmt"
	"reflect"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	// maxRetries maximum number of retries for fetching elasticsearch URLs
	// and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second

	// generate elastic mapping for an event object
	eventSkeleton = evtsapi.Event{
		EventAttributes: evtsapi.EventAttributes{
			// Need to make sure pointer fields are valid to
			// generate right mappings using reflect
			ObjectRef: &api.ObjectRef{},
			Source:    &evtsapi.EventSource{},
		},
	}
)

// Option fills the optional params for EventsMgr
type Option func(*EventsManager)

// EventsManager instance of events manager; responsible for all aspects of events
// including management of elastic connections.
type EventsManager struct {
	RPCServer *rpcserver.RPCServer // RPCServer that exposes the server implementation of event manager APIs
	apiClient apiclient.Services   // api client
	memDb     *memdb.MemDb         // memDb to store the alert policies and alerts
	logger    log.Logger           // logger
	esClient  elastic.ESClient     // elastic client
}

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(em *EventsManager) {
		em.esClient = esClient
	}
}

// NewEventsManager returns a events manager/controller instance
func NewEventsManager(serverName, serverURL string, resolverClient resolver.Interface,
	logger log.Logger, opts ...Option) (*EventsManager, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || resolverClient == nil || logger == nil {
		return nil, errors.New("all parameters are required")
	}

	em := &EventsManager{
		logger: logger,
		memDb:  memdb.NewMemDb(),
	}
	for _, opt := range opts {
		if opt != nil {
			opt(em)
		}
	}

	// start watching alert policies and alerts; update the results in mem DB
	go em.watchAPIServerEvents(resolverClient)

	// create elastic client
	if em.esClient == nil {
		result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
			return elastic.NewAuthenticatedClient("", resolverClient, em.logger.WithContext("submodule", "elastic-client"))
		}, retryDelay, maxRetries)
		if err != nil {
			em.logger.Errorf("failed to create elastic client, err: %v", err)
			return nil, err
		}
		em.logger.Debugf("created elastic client")
		em.esClient = result.(elastic.ESClient)
	}

	// create events template; once the template is created, elasticsearch automatically
	// applies the properties for any new indices that's matching the pattern. As the index call
	// automatically creates the index when it does not exists, we don't need to explicitly
	// create daily events index.
	if err := em.createEventsElasticTemplate(em.esClient); err != nil {
		em.logger.Errorf("failed to create events template in elastic, err: %v", err)
		return nil, err
	}

	alertEngine, err := alertengine.NewAlertEngine(em.memDb, logger.WithContext("pkg", "evts-alert-engine"), resolverClient)
	if err != nil {
		em.logger.Errorf("failed to create alert engine, err: %v", err)
		return nil, err
	}

	// create RPC server
	em.RPCServer, err = rpcserver.NewRPCServer(serverName, serverURL, em.esClient, alertEngine)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating RPC server")
	}

	return em, nil
}

// createEventsElasticTemplate helper function to create index template for events.
func (em *EventsManager) createEventsElasticTemplate(esClient elastic.ESClient) error {
	docType := elastic.GetDocType(globals.Events)
	mapping, err := mapper.ElasticMapper(eventSkeleton,
		docType,
		mapper.WithShardCount(3),
		mapper.WithReplicaCount(2),
		mapper.WithMaxInnerResults(globals.SpyglassMaxResults),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)))
	if err != nil {
		em.logger.Errorf("failed get elastic mapping for event object {%v}, err: %v", eventSkeleton, err)
		return err
	}

	// JSON string mapping
	strMapping, err := mapping.JSONString()
	if err != nil {
		em.logger.Errorf("failed to convert elastic mapping {%v} to JSON string", mapping)
		return err
	}

	// create events template
	if err := esClient.CreateIndexTemplate(context.Background(), elastic.GetTemplateName(globals.Events), strMapping); err != nil {
		em.logger.Errorf("failed to create events index template, err: %v", err)
		return err
	}

	return nil
}

// watchAPIServerEvents handles alert policy and alert events
func (em *EventsManager) watchAPIServerEvents(resolverClient resolver.Interface) error {
	for {
		ctx, cancel := context.WithCancel(context.Background())
		defer cancel()

		client, err := utils.ExecuteWithRetry(func() (interface{}, error) {
			return apiclient.NewGrpcAPIClient(globals.EvtsMgr, globals.APIServer, em.logger.WithContext("pkg", "evtsmgr-api-client"),
				rpckit.WithBalancer(balancer.New(resolverClient)))
		}, 2*time.Second, maxRetries)
		if err != nil {
			em.logger.Errorf("failed to create API client, err: %v", err)
			continue
		}

		em.logger.Infof("created API server client")
		em.apiClient = client.(apiclient.Services)

		em.processEvents(ctx)
		em.apiClient.Close()
		if err := ctx.Err(); err != nil {
			em.logger.Errorf("watcher context error: %v", err)
		}

		time.Sleep(2 * time.Second)
	}
}

// processEvents helper function to handle the API server watch events
func (em *EventsManager) processEvents(parentCtx context.Context) error {
	ctx, cancelWatch := context.WithCancel(parentCtx)
	defer cancelWatch()

	opts := &api.ListWatchOptions{}
	watchList := map[int]string{}
	selCases := []reflect.SelectCase{}

	// watch alert policies
	watcher, err := em.apiClient.MonitoringV1().AlertPolicy().Watch(ctx, opts)
	if err != nil {
		em.logger.Errorf("failed to watch alert policy, err: %v", err)
		return err
	}

	watchList[len(selCases)] = "alertPolicy"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// watch alerts
	watcher, err = em.apiClient.MonitoringV1().Alert().Watch(ctx, opts)
	if err != nil {
		em.logger.Errorf("failed to watch alerts, err: %v", err)
		return err
	}

	watchList[len(selCases)] = "alert"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// watch alert destination
	watcher, err = em.apiClient.MonitoringV1().AlertDestination().Watch(ctx, opts)
	if err != nil {
		em.logger.Errorf("failed to watch alerts, err: %v", err)
		return err
	}

	watchList[len(selCases)] = "alertDestination"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// ctx done
	watchList[len(selCases)] = "ctx-canceled"
	selCases = append(selCases, reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ctx.Done())})

	// event loop
	for {
		id, recVal, ok := reflect.Select(selCases)
		if !ok {
			em.logger.Errorf("{%s} channel closed", watchList[id])
			return fmt.Errorf("channel closed")
		}

		event, ok := recVal.Interface().(*kvstore.WatchEvent)
		if !ok {
			em.logger.Errorf("unknown object received from {%s}: %+v", watchList[id], recVal.Interface())
			return fmt.Errorf("unknown object received")
		}

		em.logger.Infof("received watch event %#v", event)
		switch obj := event.Object.(type) {
		case *monitoring.AlertPolicy:
			em.processAlertPolicy(event.Type, obj)
		case *monitoring.Alert:
			em.processAlert(event.Type, obj)
		case *monitoring.AlertDestination:
			em.processAlertDestination(event.Type, obj)
		default:
			em.logger.Errorf("invalid watch event type received from {%s}, %+v", watchList[id], event)
			return fmt.Errorf("invalid watch event type")
		}
	}
}

// helper to process alert policy
func (em *EventsManager) processAlertPolicy(eventType kvstore.WatchEventType, policy *monitoring.AlertPolicy) error {
	em.logger.Infof("processing alert policy watch event: {%s} {%#v} ", eventType, policy)
	switch eventType {
	case kvstore.Created:
		return em.memDb.AddObject(policy)
	case kvstore.Updated:
		return em.memDb.UpdateObject(policy)
	case kvstore.Deleted:
		return em.memDb.DeleteObject(policy)
	default:
		em.logger.Errorf("invalid alert policy watch event, event %s policy %+v", eventType, policy)
		return fmt.Errorf("invalid alert policy watch event")
	}
}

// helper to process alerts
func (em *EventsManager) processAlert(eventType kvstore.WatchEventType, alert *monitoring.Alert) error {
	em.logger.Infof("processing alert watch event: {%s} {%#v} ", eventType, alert)
	switch eventType {
	case kvstore.Created:
		return em.memDb.AddObject(alert)
	case kvstore.Updated:
		return em.memDb.UpdateObject(alert)
	case kvstore.Deleted:
		return em.memDb.DeleteObject(alert)
	default:
		em.logger.Errorf("invalid alert watch event, type %s policy %+v", eventType, alert)
		return fmt.Errorf("invalid alert watch event")
	}
}

// helper to process alert destinations
func (em *EventsManager) processAlertDestination(eventType kvstore.WatchEventType, alertDest *monitoring.AlertDestination) error {
	em.logger.Infof("processing alert destination watch event: {%s} {%#v} ", eventType, alertDest)
	switch eventType {
	case kvstore.Created:
		return em.memDb.AddObject(alertDest)
	case kvstore.Updated:
		return em.memDb.UpdateObject(alertDest)
	case kvstore.Deleted:
		return em.memDb.DeleteObject(alertDest)
	default:
		em.logger.Errorf("invalid alert destination watch event, type %s policy %+v", eventType, alertDest)
		return fmt.Errorf("invalid alert destination watch event")
	}
}
