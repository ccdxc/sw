package alertengine

import (
	"context"
	"errors"
	"fmt"
	"math/rand"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine/exporter"
	eapiclient "github.com/pensando/sw/venice/ctrler/evtsmgr/apiclient"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	aeutils "github.com/pensando/sw/venice/utils/alertengine"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	pkgName                   = "evts-alerts-engine"
	maxRetry                  = 15
	numAPIClients             = 10
	alertPolicyUpdateInterval = 10 * time.Second
	alertUpdateInterval       = 10 * time.Second
)

//
// Module responsible for converting events to alerts.
// 1. Runs inline with events manager.
//
// 2. Any error that occurs while processing alert policies against each event
//    will be ignored. Alert will be generated in the next run when the same event is received (again, best effort).
//
// 3. Overlapping alert policies will trigger multiple alerts for the same event.
//
// 4. Event based alerts are not auto resolvable. So, this module will only create, no further
//    monitoring is done to auto-resolve.
//
// 5. There can be only one outstanding alert for an objRef (tenant, namespace, kind and name) by single alert policy.
//
// 6. If evt.objRef == nil, we could end up with multiple alerts for the same event (if it keeps happening) as there is no way
//    detect duplicate alert.
//
// 7. Exports the generated alert to the list of destinations from the given alert policy.
//
// Example syslog message:
// BSD format - msg: `alert.message - json(alert attributes map)`
//  <10>2018-09-26T12:14:56-07:00 host1 pen-events[24208]: DUMMYEVENT-3-CRITICAL - {\"meta\":{\"creation-time\":\"2018-09-26 19:14:55.968942 +0000 UTC\",
//  \"mod-time\":\"2018-09-26 19:14:55.968942 +0000 UTC\",\"name\":\"93d40499-196d-49fd-9154-ef1d5e0bf52b\",\"namespace\":\"default\",\"tenant\":\"default\",\"uuid\":\"93d40499-196d-49fd-9154-ef1d5e0bf52b\"},
//  \"spec\":{\"state\":\"OPEN\"},\"status\":{\"event-uri\":\"/events/v1/events/c4bac3ed-7216-4771-97bb-27b16fbd72f4\",\"message\":\"DUMMYEVENT-3-CRITICAL\",\"severity\":\"CRITICAL\"},
//  \"status.object-ref\":{\"kind\":\"Node\",\"name\":\"qaJaB\",\"namespace\":\"default\",\"tenant\":\"default\",\"uri\":\"\"},\"status.reason\":{\"policy-id\":\"71f016e3-51bc-430d-9aea-7e5886a0ec96\"},
//  \"status.source\":{\"component\":\"5f51d9b1-3e50-400b-b0fe-6e19ce528f2a\",\"node-name\":\"test-node\"},\"type\":{\"kind\":\"Alert\"}}"
//
// RFC5424 format - msg: alert.message, msgID: alert.UUID, structured data: stringify(alert attributes map)
//  <10>1 2018-09-26T12:14:56-07:00 host1 pen-events 24208 93d40499-196d-49fd-9154-ef1d5e0bf52b [status message=\"DUMMYEVENT-3-CRITICAL\" event-uri=\"/events/v1/events/c4bac3ed-7216-4771-97bb-27b16fbd72f4\"
//  severity=\"CRITICAL\"][status.reason policy-id=\"71f016e3-51bc-430d-9aea-7e5886a0ec96\"][status.source component=\"5f51d9b1-3e50-400b-b0fe-6e19ce528f2a\" node-name=\"test-node\"][status.object-ref namespace=\"default\"
//  kind=\"Node\" name=\"qaJaB\" uri=\"\" tenant=\"default\"][type kind=\"Alert\"][meta mod-time=\"2018-09-26 19:14:55.968942 +0000 UTC\" name=\"93d40499-196d-49fd-9154-ef1d5e0bf52b\" uuid=\"93d40499-196d-49fd-9154-ef1d5e0bf52b\"
//  tenant=\"default\" namespace=\"default\" creation-time=\"2018-09-26 19:14:55.968942 +0000 UTC\"][spec state=\"OPEN\"] DUMMYEVENT-3-CRITICAL"
//

// AlertEngine represents the events alerts engine which is responsible fo converting
// events to alerts based on the event based alert policies.
type alertEngineImpl struct {
	sync.RWMutex
	logger                    log.Logger                // logger
	resolverClient            resolver.Interface        // to connect with apiserver to fetch alert policies; and send alerts
	memDb                     *memdb.MemDb              // in-memory db/cache
	configWatcher             *eapiclient.ConfigWatcher // API server client
	exporter                  *exporter.AlertExporter   // exporter to export alerts to different destinations
	ctx                       context.Context           // context to cancel goroutines
	cancelFunc                context.CancelFunc        // context to cancel goroutines
	wg                        sync.WaitGroup            // for version watcher routine
	eventsQueue               chan *evtsapi.Event       // queue holding events that were not processed due to apiserver connection failure
	apiClients                *apiClients
	alertPolicyStatusCounters *alertPolicyStatusCounters
	alertHitCounter           *alertHitCounter
}

// alertPolicyStatusCounters represents the alert policy status that needs to be updated with the API server
type alertPolicyStatusCounters struct {
	sync.Mutex
	counters map[string]*apStatusCounters
}

// alertHitCounter represents the alert total-hit counter that needs to be updated with API server
type alertHitCounter struct {
	sync.Mutex
	counter map[string]int32
}

// apStatusCounters captures the status counters
type apStatusCounters struct {
	meta                  *api.ObjectMeta
	incrementOpenAlertsBy int32
	incrementTotalHitsBy  int32
}

type apiClients struct {
	sync.RWMutex
	clients []apiclient.Services
}

// NewAlertEngine creates the new events alert engine.
func NewAlertEngine(parentCtx context.Context, memDb *memdb.MemDb, configWatcher *eapiclient.ConfigWatcher,
	logger log.Logger, resolverClient resolver.Interface) (Interface, error) {
	if nil == logger || nil == resolverClient || nil == configWatcher {
		return nil, errors.New("all parameters are required")
	}

	rand.Seed(time.Now().UnixNano())

	ctx, cancelFunc := context.WithCancel(parentCtx)
	ae := &alertEngineImpl{
		logger:                    logger,
		resolverClient:            resolverClient,
		configWatcher:             configWatcher,
		memDb:                     memDb,
		exporter:                  exporter.NewAlertExporter(memDb, configWatcher, logger.WithContext("submodule", "alert_exporter")),
		ctx:                       ctx,
		cancelFunc:                cancelFunc,
		eventsQueue:               make(chan *evtsapi.Event, 500),
		apiClients:                &apiClients{clients: make([]apiclient.Services, 0, numAPIClients)},
		alertPolicyStatusCounters: &alertPolicyStatusCounters{counters: make(map[string]*apStatusCounters)},
		alertHitCounter:           &alertHitCounter{counter: make(map[string]int32)},
	}

	ae.wg.Add(4)
	go ae.createAPIClients(numAPIClients)
	go ae.processEventsFromQueue()
	go ae.updateAlertPolicies()
	go ae.updateAlerts()

	return ae, nil
}

// ProcessEvents will be called from the events manager whenever the events are received.
// And, it creates an alert whenever the event matches any policy.
func (a *alertEngineImpl) ProcessEvents(reqID string, eventList *evtsapi.EventList) {
	start := time.Now()
	a.logger.Infof("{req: %s} processing events at alert engine, len: %d", reqID, len(eventList.Items))

	a.RLock()
	if len(a.apiClients.clients) == 0 {
		a.logger.Errorf("{req: %s} waiting for API client(s): %v, events will be queued for later processing", reqID, len(a.apiClients.clients))
		a.RUnlock()
		a.addToEventsQueueForLaterProcessing(reqID, eventList)
		return
	}
	a.RUnlock()

	for _, evt := range eventList.GetItems() {
		// get api client to process this event
		apCl := a.getAPIClient()
		a.processEvent(reqID, apCl, evt)
	}
	a.logger.Infof("{req: %s} finished processing events at alert engine, took: %v", reqID, time.Since(start))
}

// Stop stops the alert engine by closing all the workers.
func (a *alertEngineImpl) Stop() {
	a.Lock()
	defer a.Unlock()

	a.cancelFunc()
	a.wg.Wait()

	close(a.eventsQueue)

	a.exporter.Stop() // this will stop any exports that're in line

	a.logger.Infof("final update on the pending alert policies")
	a.updateAlertPoliciesHelper() // try updating any pending alert policies

	// close all the clients
	a.apiClients.Lock()
	for _, client := range a.apiClients.clients {
		client.Close()
	}
	a.apiClients.Unlock()
}

// adds the given list of events to queue for later processing
func (a *alertEngineImpl) addToEventsQueueForLaterProcessing(reqID string, eventList *evtsapi.EventList) {
	for _, evt := range eventList.GetItems() {
		select {
		case <-a.ctx.Done():
			a.logger.Infof("{req: %s} context cancelled, failed to add event(s) for later processing ", reqID)
			return
		case a.eventsQueue <- evt:
			break
		default:
			a.logger.Infof("{req: %s} queue is full, failed to add event(s) for later processing", reqID)
			return
		}
	}
}

// helper function to process the given event against the alert policies.
func (a *alertEngineImpl) processEvent(reqID string, apCl apiclient.Services, evt *evtsapi.Event) {
	// fetch alert policies belonging to evt.Tenant
	alertPolicies := a.memDb.GetAlertPolicies(
		memdb.WithTenantFilter(evt.GetTenant()),
		memdb.WithResourceFilter("Event"),
		memdb.WithEnabledFilter(true))

	if len(alertPolicies) == 0 {
		return
	}

	// run over each alert policy
	for _, ap := range alertPolicies {
		var reqs []*fields.Requirement
		for _, t := range ap.Spec.GetRequirements() {
			r := *t
			reqs = append(reqs, &r)
		}
		ap.Spec.Requirements = reqs

		if err := a.runPolicy(reqID, apCl, ap, evt); err != nil {
			a.logger.Errorf("{req: %s} failed to run policy: %v on event: %v, err: %v", reqID, ap.GetName(), evt.GetMessage(), err)
		}
	}
}

// start processing events from the queue once apiclients are created
func (a *alertEngineImpl) processEventsFromQueue() {
	defer a.wg.Done()

	a.logger.Info("starting to process events from queue")

	reqID := "events-from-queue"
	for {
		select {
		case <-a.ctx.Done():
			a.logger.Info("context cancelled, returning from processing events queue")
			return
		case evt, ok := <-a.eventsQueue:
			if !ok {
				a.logger.Info("events queue closed, returning")
			}

			a.logger.Infof("processing event from the queue: {%s}", evt.Message)

			// wait until API client is available
			apiCl := a.getAPIClient()
			for apiCl == nil {
				if a.ctx.Err() != nil {
					a.logger.Info("context cancelled, returning from processing events queue")
					return
				}
				time.Sleep(time.Second)
				apiCl = a.getAPIClient()
			}

			// wait until sometime for the policies to become available if they're not yet already.
			_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				alertPolicies := a.memDb.GetAlertPolicies()
				if len(alertPolicies) == 0 {
					a.logger.Info("found 0 policies, retrying...")
					return nil, fmt.Errorf("found 0 policies")
				}
				return alertPolicies, nil
			}, 100*time.Millisecond, maxRetry)
			if err != nil {
				a.logger.Error(err)
			}

			a.processEvent(reqID, apiCl, evt)
		}
	}
}

// helper function to create API clients
func (a *alertEngineImpl) createAPIClients(numClients int) {
	defer a.wg.Done()

	numCreated := 0
	for numCreated < numClients {
		if a.ctx.Err() != nil {
			return
		}

		logger := a.logger.WithContext("pkg", fmt.Sprintf("%s-%s-%d", globals.EvtsMgr, "alert-engine-api-client", numCreated))
		cl, err := apiclient.NewGrpcAPIClient(globals.EvtsMgr, globals.APIServer, logger,
			rpckit.WithBalancer(balancer.New(a.resolverClient)), rpckit.WithLogger(logger))
		if err != nil {
			a.logger.Errorf("failed to create API client {API server URLs from resolver: %v}, err: %v", a.resolverClient.GetURLs(globals.APIServer), err)
			continue
		}

		if a.ctx.Err() != nil {
			return
		}

		a.logger.Infof("created client {%d}", numCreated)
		a.apiClients.Lock()
		a.apiClients.clients = append(a.apiClients.clients, cl)
		a.apiClients.Unlock()
		numCreated++
	}
}

// returns a random API client from the list of clients
func (a *alertEngineImpl) getAPIClient() apiclient.Services {
	a.apiClients.RLock()
	defer a.apiClients.RUnlock()

	if len(a.apiClients.clients) == 0 {
		return nil
	}

	index := rand.Intn(len(a.apiClients.clients))
	return a.apiClients.clients[index]
}

// runPolicy helper function to run the given policy against event. Also, it updates
// totalHits and openAlerts on the alertPolicy.
func (a *alertEngineImpl) runPolicy(reqID string, apCl apiclient.Services, ap *monitoring.AlertPolicy, evt *evtsapi.Event) error {
	match, reqWithObservedVal := aeutils.Match(ap.Spec.GetRequirements(), evt)
	if match {
		a.logger.Debugf("{req: %s} event {%s: %s} matched the alert policy {%s} with requirements:[%+v]",
			reqID, evt.GetName(), evt.GetMessage(), ap.GetName(), ap.Spec.GetRequirements())
		created, err := a.createAlert(reqID, apCl, ap, evt, reqWithObservedVal)
		if err != nil {
			errStatus, ok := status.FromError(err)
			a.logger.Errorf("{req: %s} failed to create alert for event: %v, err: %v, %v", reqID, evt.GetUUID(), err, errStatus)
			// TODO: @Yuva, figure out if there are better ways to handle this case
			if ok && errStatus.Code() == codes.InvalidArgument && errStatus.Message() == "Request validation failed" {
				return nil
			}
			return err
		}

		if created {
			a.logger.Infof("{req: %s} alert created from event {%s: %v, %s, %s, %v} for the policy {%s/%s}", reqID,
				evt.GetName(), evt.GetCount(), evt.GetMessage(), evt.GetSelfLink(), evt.GetObjectRef(), ap.GetName(), ap.GetUUID())
			a.updateAlertPolicyStatusCounters(ap.GetObjectMeta(), 1, 1)
		} else {
			a.logger.Infof("{req: %s} existing open alert found for event {%s: %v, %s, %s, %v}  for the policy {%s/%s}",
				reqID, evt.GetName(), evt.GetCount(), evt.GetMessage(), evt.GetSelfLink(), evt.GetObjectRef(), ap.GetName(), ap.GetUUID())
			a.updateAlertPolicyStatusCounters(ap.GetObjectMeta(), 1, 0)
		}
	}

	return nil
}

// createAlert helper function to construct and create the alert using API client.
func (a *alertEngineImpl) createAlert(reqID string, apCl apiclient.Services, alertPolicy *monitoring.AlertPolicy,
	evt *evtsapi.Event, matchedRequirements []*monitoring.MatchedRequirement) (bool, error) {
	alertUUID := uuid.NewV4().String()
	alertName := alertUUID
	creationTime, _ := types.TimestampProto(time.Now())

	// create alert object
	policyID := fmt.Sprintf("%s/%s", alertPolicy.GetName(), alertPolicy.GetUUID())
	alert := &monitoring.Alert{
		TypeMeta: api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{
			Name:      alertName,
			UUID:      alertUUID,
			Tenant:    evt.GetTenant(),
			Namespace: evt.GetNamespace(),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
		},
		Spec: monitoring.AlertSpec{
			State: monitoring.AlertState_OPEN.String(),
		},
		Status: monitoring.AlertStatus{
			Severity: alertPolicy.Spec.GetSeverity(),
			Message:  evt.GetMessage(),
			Reason: monitoring.AlertReason{
				PolicyID:            policyID,
				MatchedRequirements: matchedRequirements,
			},
			Source: &monitoring.AlertSource{
				Component: evt.GetSource().GetComponent(),
				NodeName:  evt.GetSource().GetNodeName(),
			},
			EventURI:  evt.GetSelfLink(),
			ObjectRef: evt.GetObjectRef(),
			TotalHits: 1,
		},
	}
	alert.SelfLink = alert.MakeURI("configs", "v1", "monitoring")

	alertCreated, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		// check there is an existing alert from the same event
		if alertName, found := a.memDb.AnyOutstandingAlertsByURI(alertPolicy.GetTenant(), policyID, evt.GetSelfLink()); found {
			a.logger.Infof("{req: %s} outstanding alert found that matches the event URI", reqID)
			a.updateAlertHitCounter(alertPolicy.GetTenant(), alertName) // update total-hits on the alert
			return false, nil
		}

		// if there is no alert from the same event, check if there is an alert from some other event.
		if evt.GetObjectRef() != nil {
			// for resource events, use partial string match to check if an alert is already there
			message := evt.GetMessage()
			if evt.GetCategory() == eventattrs.Category_Resource.String() {
				if eventtypes.EventType(eventtypes.EventType_value[evt.GetType()]) == eventtypes.DISK_THRESHOLD_EXCEEDED {
					message = globals.DiskHighThresholdMessage
				}
			}

			if alertName, found := a.memDb.AnyOutstandingAlertsByMessageAndRef(alertPolicy.GetTenant(), policyID,
				message, evt.GetObjectRef()); found {
				a.logger.Infof("{req: %s} outstanding alert found that matches the message and object ref. ", reqID)
				a.updateAlertHitCounter(alertPolicy.GetTenant(), alertName) // update total-hits on the alert
				return false, nil
			}
		}

		// evt.GetObjectRef() == nil; cannot find outstanding alert if any.
		// create an alert
		_, err := apCl.MonitoringV1().Alert().Create(ctx, alert)
		if err == nil {
			return true, nil
		}
		return false, err
	}, 2*time.Second, maxRetry)
	if err != nil {
		return false, err
	}

	// TODO: run field selector on it (AlertDestination.Spec.Selector)
	if alertCreated.(bool) { // export alert
		if err := a.exporter.Export(alertPolicy.Spec.GetDestinations(), alert); err != nil {
			log.Errorf("{req: %s} failed to export alert %v to destinations %v, err: %v", reqID, alert.GetObjectMeta(), alertPolicy.Spec.GetDestinations(), err)
		}
	}

	return alertCreated.(bool), err
}

// updateAlertHitCounter updates the local cache with alert hit counter; and these will be eventually
// propagated to API server.
func (a *alertEngineImpl) updateAlertHitCounter(tenant, alertName string) {
	a.alertHitCounter.Lock()
	key := fmt.Sprintf("%s:%s", tenant, alertName)
	if _, found := a.alertHitCounter.counter[key]; !found {
		a.alertHitCounter.counter[key] = 1
	} else {
		a.alertHitCounter.counter[key]++
	}
	a.alertHitCounter.Unlock()
}

// updateAlerts helper function to update total hits on the alert object in the intervals of 10s
// NOTE: evtsmgr will lose the local cache during restarts, as a result alerts will not updated.
func (a *alertEngineImpl) updateAlerts() {
	defer a.wg.Done()

	ticker := time.NewTicker(alertUpdateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-a.ctx.Done():
			a.logger.Info("context cancelled, returning from update alert policies")
			return
		case <-ticker.C:
			a.updateAlertsHelper()
		}
	}
}

// helper function to update alerts hit counter
func (a *alertEngineImpl) updateAlertsHelper() {
	apCl := a.getAPIClient()
	if apCl == nil {
		a.logger.Infof("no API client available, skipping alert policy update")
		return
	}

	a.alertHitCounter.Lock()
	for key, value := range a.alertHitCounter.counter {
		tmp := strings.Split(key, ":")
		tenant, alertName := tmp[0], tmp[1]
		_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			alert, err := apCl.MonitoringV1().Alert().Get(ctx,
				&api.ObjectMeta{
					Name:   alertName,
					Tenant: tenant,
				}) // get the alert
			if err != nil {
				errStatus, _ := status.FromError(err)
				if errStatus.Code() == codes.NotFound {
					a.logger.Errorf("local cache reported that the alert found, but it's not found in API server")
					return nil, nil
				} else if errStatus.Code() == codes.Unavailable { // API server unavailable, retry in the next interval
					return nil, nil
				}
				return nil, err
			}

			alert.Status.TotalHits += value

			alert, err = apCl.MonitoringV1().Alert().UpdateStatus(ctx, alert) // update the alert with new counter
			if err != nil {
				errStatus, _ := status.FromError(err)
				a.logger.Debugf("failed to update alert {%v}, increment total-hits by 1, err: %v, %v", alertName, err, errStatus)
				return nil, err
			}

			// delete the entry after successful update to avoid backlog of old entries
			delete(a.alertHitCounter.counter, key)

			return nil, nil
		}, 2*time.Second, maxRetry)

		if err != nil {
			a.logger.Errorf("failed to update alert {%v}, increment total-hits by: 1, err: %v", alertName, err)
		}
	}
	a.alertHitCounter.Unlock()
}

// updateAlertPolicyStatusCounters during alert creation, the respective alert policy needs to be updated
// total number of hits and total open alerts on that policy. This function updates the local map which will be
// propagated to the API server in intervals.
func (a *alertEngineImpl) updateAlertPolicyStatusCounters(meta *api.ObjectMeta, incrementTotalHitsBy, incrementOpenAlertsBy int32) {
	a.alertPolicyStatusCounters.Lock()
	counters, found := a.alertPolicyStatusCounters.counters[meta.GetKey()]
	if !found {
		counters = &apStatusCounters{meta: meta}
	}
	counters.incrementTotalHitsBy += incrementTotalHitsBy
	counters.incrementOpenAlertsBy += incrementOpenAlertsBy
	a.alertPolicyStatusCounters.counters[meta.GetKey()] = counters
	a.alertPolicyStatusCounters.Unlock()
}

// updateAlertPolicies helper function to update total hits and open alerts count on the alert policy in the
// intervals of 10s.
// NOTE: evtsmgr will lose the local cache during restarts, as a result alert policy will not updated.
func (a *alertEngineImpl) updateAlertPolicies() {
	defer a.wg.Done()

	ticker := time.NewTicker(alertPolicyUpdateInterval)
	defer ticker.Stop()

	for {
		select {
		case <-a.ctx.Done():
			a.logger.Info("context cancelled, returning from update alert policies")
			return
		case <-ticker.C:
			a.updateAlertPoliciesHelper()
		}
	}
}

// helper function to update alert policies
func (a *alertEngineImpl) updateAlertPoliciesHelper() {
	apCl := a.getAPIClient()
	if apCl == nil {
		a.logger.Infof("no API client available, skipping alert policy update")
		return
	}

	a.alertPolicyStatusCounters.Lock()
	for key, counters := range a.alertPolicyStatusCounters.counters {
		if counters.incrementOpenAlertsBy > 0 || counters.incrementTotalHitsBy > 0 {
			a.logger.Infof("updating alert policy with counters {%v}, total-hits: %v, open-alerts: %v",
				counters.meta.Name, counters.incrementTotalHitsBy, counters.incrementOpenAlertsBy)
			_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				ap, err := apCl.MonitoringV1().AlertPolicy().Get(ctx,
					&api.ObjectMeta{
						Name:      counters.meta.GetName(),
						Tenant:    counters.meta.GetTenant(),
						Namespace: counters.meta.GetNamespace(),
						UUID:      counters.meta.GetUUID(),
					}) // get the alert policy
				if err != nil {
					errStatus, _ := status.FromError(err)
					if errStatus.Code() == codes.NotFound {
						delete(a.alertPolicyStatusCounters.counters, key)
						return nil, nil
					} else if errStatus.Code() == codes.Unavailable { // retry in the next interval
						return nil, nil
					}
					return nil, err
				}

				ap.Status.OpenAlerts += counters.incrementOpenAlertsBy
				ap.Status.TotalHits += counters.incrementTotalHitsBy

				ap, err = apCl.MonitoringV1().AlertPolicy().UpdateStatus(ctx, ap) // update the policy
				if err != nil {
					errStatus, _ := status.FromError(err)
					a.logger.Debugf("failed to update alert policy {%v}, increment total-hits by: %v, open-alerts by: %v, err: %v, %v",
						counters.meta.Name, counters.incrementTotalHitsBy, counters.incrementOpenAlertsBy, err, errStatus)
					return nil, err
				}

				// delete the entry after successful update to avoid backlog of old entries
				delete(a.alertPolicyStatusCounters.counters, key)

				return nil, nil
			}, 2*time.Second, maxRetry)

			if err != nil {
				a.logger.Errorf("failed to update alert policy {%v}, increment total-hits by: %v, open-alerts by: %v, err: %v",
					counters.meta.Name, counters.incrementTotalHitsBy, counters.incrementOpenAlertsBy, err)
			}
		}
	}
	a.alertPolicyStatusCounters.Unlock()
}
