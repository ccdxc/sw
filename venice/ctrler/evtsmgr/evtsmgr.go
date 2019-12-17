// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pkg/errors"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/apiclient"
	emmemdb "github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
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

	// default garbage collection configs
	defaultGCInterval                    = 30 * time.Minute
	defaultResolvedAlertsRetentionPeriod = 24 * time.Hour
)

// Option fills the optional params for EventsMgr
type Option func(*EventsManager)

// EventsManager instance of events manager; responsible for all aspects of events
// including management of elastic connections.
type EventsManager struct {
	RPCServer      *rpcserver.RPCServer     // RPCServer that exposes the server implementation of event manager APIs
	configWatcher  *apiclient.ConfigWatcher // api client
	memDb          *emmemdb.MemDb           // memDb to store the alert policies and alerts
	logger         log.Logger               // logger
	esClient       elastic.ESClient         // elastic client
	alertEngine    alertengine.Interface    // alert engine
	ctx            context.Context          // context
	cancelFunc     context.CancelFunc       // cancel func
	diagSvc        diagnostics.Service
	moduleWatcher  module.Watcher
	AlertsGCConfig *AlertsGCConfig
	wg             sync.WaitGroup // for GC routine
}

// AlertsGCConfig contains GC related config
// Note: this needs to be moved to a common place once we start supporting object-based & threshold-based alerts
type AlertsGCConfig struct {
	Interval                      time.Duration // garbage collect every x seconds
	ResolvedAlertsRetentionPeriod time.Duration // delete resolved alerts after this retention period
}

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(em *EventsManager) {
		em.esClient = esClient
	}
}

// WithDiagnosticsService passes a custom diagnostics service
func WithDiagnosticsService(diagSvc diagnostics.Service) Option {
	return func(em *EventsManager) {
		em.diagSvc = diagSvc
	}
}

// WithModuleWatcher passes a module watcher
func WithModuleWatcher(moduleWatcher module.Watcher) Option {
	return func(em *EventsManager) {
		em.moduleWatcher = moduleWatcher
	}
}

// WithAlertsGCConfig passes a GC config
func WithAlertsGCConfig(config *AlertsGCConfig) Option {
	return func(em *EventsManager) {
		if config != nil {
			em.AlertsGCConfig = config
		}
	}
}

// NewEventsManager returns a events manager/controller instance
func NewEventsManager(serverName, serverURL string, resolverClient resolver.Interface,
	logger log.Logger, opts ...Option) (*EventsManager, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || resolverClient == nil || logger == nil {
		return nil, errors.New("all parameters are required")
	}

	ctx, cancel := context.WithCancel(context.Background())

	em := &EventsManager{
		logger:     logger,
		memDb:      emmemdb.NewMemDb(),
		ctx:        ctx,
		cancelFunc: cancel,
		AlertsGCConfig: &AlertsGCConfig{
			Interval:                      defaultGCInterval,
			ResolvedAlertsRetentionPeriod: defaultResolvedAlertsRetentionPeriod,
		},
	}
	for _, opt := range opts {
		if opt != nil {
			opt(em)
		}
	}

	// update GC config if required
	if em.AlertsGCConfig.Interval == 0 {
		em.AlertsGCConfig.Interval = defaultGCInterval
	}
	if em.AlertsGCConfig.ResolvedAlertsRetentionPeriod == 0 {
		em.AlertsGCConfig.ResolvedAlertsRetentionPeriod = defaultResolvedAlertsRetentionPeriod
	}

	// create elastic client
	if em.esClient == nil {
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
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

	var err error

	// create new config watcher
	em.configWatcher = apiclient.NewConfigWatcher(ctx, resolverClient, em.memDb, logger)

	em.alertEngine, err = alertengine.NewAlertEngine(ctx, em.memDb, em.configWatcher, logger.WithContext("pkg",
		"evts-alert-engine"), resolverClient)
	if err != nil {
		em.logger.Errorf("failed to create alert engine, err: %v", err)
		return nil, err
	}

	// create RPC server
	em.RPCServer, err = rpcserver.NewRPCServer(serverName, serverURL, em.esClient, em.alertEngine, em.memDb, logger, em.diagSvc)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating RPC server")
	}

	em.configWatcher.StartWatcher()

	em.wg.Add(1)
	go em.gcAlerts()

	return em, nil
}

// Stop stops events manager
func (em *EventsManager) Stop() {
	em.cancelFunc()
	em.wg.Wait() // wait for alerts garbage collector to stop

	if em.moduleWatcher != nil {
		em.moduleWatcher.Stop()
	}

	if em.diagSvc != nil {
		em.diagSvc.Stop()
	}

	if em.alertEngine != nil {
		em.alertEngine.Stop()
		em.alertEngine = nil
	}

	if em.esClient != nil {
		em.esClient.Close()
		em.esClient = nil
	}

	if em.configWatcher != nil {
		em.configWatcher.Stop()
		em.configWatcher = nil
	}

	if em.RPCServer != nil {
		em.RPCServer.Stop()
		em.RPCServer = nil
	}
}

// gcAlerts garbage collects alerts that are in resolved state for over 24 hrs (default)
func (em *EventsManager) gcAlerts() {
	defer em.wg.Done()

	ticker := time.NewTicker(em.AlertsGCConfig.Interval)
	defer ticker.Stop()

	em.logger.Infof("{GC alerts}: starting GC routine to delete resolved alerts")

	for {
		select {
		case <-em.ctx.Done():
			em.logger.Infof("{GC alerts}: context cancelled, returning")
			return
		case <-ticker.C:
			em.GCAlerts(em.AlertsGCConfig.ResolvedAlertsRetentionPeriod)
		}
	}
}

// GCAlerts garbage collects alerts that are in resolved state for a given duration `retentionPeriod`
func (em *EventsManager) GCAlerts(retentionPeriod time.Duration) {
	if retentionPeriod == 0 {
		em.logger.Errorf("{GC alerts}: invalid retention period")
		return
	}

	if em.configWatcher == nil {
		em.logger.Errorf("{GC alerts}: nil config watcher, cannot GC alerts")
		return
	}

	// get API client
	apiCl := em.configWatcher.APIClient()
	retries := 0
	for apiCl == nil {
		if em.ctx.Err() != nil {
			em.logger.Infof("{GC alerts}: context cancelled, returning")
			return
		}

		if retries == maxRetries {
			em.logger.Infof("{GC alerts}: API client not available, exhausted retries")
			return
		}
		time.Sleep(1 * time.Second)
		apiCl = em.configWatcher.APIClient()
		retries++
	}

	// fetch all the resolved alerts
	resolvedAlerts := em.memDb.ListObjects("Alert", func(obj memdb.Object) bool {
		alert := obj.(*monitoring.Alert)
		if alert.Spec.State == monitoring.AlertState_RESOLVED.String() && alert.Status.Resolved != nil {
			tm, err := alert.Status.Resolved.Time.Time()
			if err != nil {
				return false
			}
			if time.Since(tm) > retentionPeriod {
				return true
			}
		}
		return false
	})

	for _, alert := range resolvedAlerts {
		if em.ctx.Err() != nil {
			em.logger.Infof("{GC alerts}: context cancelled, returning")
			return
		}

		em.logger.Infof("{GC alerts}: deleting alert {%+v}", alert)
		_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			if _, err := apiCl.MonitoringV1().Alert().Delete(ctx, alert.GetObjectMeta()); err != nil {
				if errStatus, _ := status.FromError(err); errStatus.Code() == codes.Unavailable || errStatus.Code() == codes.NotFound {
					return nil, nil
				}
				return nil, err
			}
			return nil, nil
		}, 2*time.Second, maxRetries)
		if err != nil {
			em.logger.Errorf("{GC alerts}: failed to delete alert {%+v}, err: %v", alert, err)
		}
	}
}

// createEventsElasticTemplate helper function to create index template for events.
func (em *EventsManager) createEventsElasticTemplate(esClient elastic.ESClient) error {
	docType := elastic.GetDocType(globals.Events)
	mapping, err := mapper.ElasticMapper(eventSkeleton,
		docType,
		mapper.WithShardCount(3),
		mapper.WithReplicaCount(2),
		mapper.WithMaxInnerResults(globals.SpyglassMaxResults),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)),
		mapper.WithCharFilter())
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
