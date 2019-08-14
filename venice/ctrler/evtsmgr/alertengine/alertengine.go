package alertengine

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine/exporter"
	eapiclient "github.com/pensando/sw/venice/ctrler/evtsmgr/apiclient"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/utils"
	aeutils "github.com/pensando/sw/venice/utils/alertengine"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	pkgName  = "evts-alerts-engine"
	maxRetry = 15
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
	logger          log.Logger                // logger
	resolverClient  resolver.Interface        // to connect with apiserver to fetch alert policies; and send alerts
	memDb           *memdb.MemDb              // in-memory db/cache
	configWatcher   *eapiclient.ConfigWatcher // API server client
	exporter        *exporter.AlertExporter   // exporter to export alerts to different destinations
	ctx             context.Context           // context to cancel goroutines
	cancelFunc      context.CancelFunc        // context to cancel goroutines
	wg              sync.WaitGroup            // for version watcher routine
	maintenanceMode *bool                     // indicates if the maintenance mode is on or not
}

// NewAlertEngine creates the new events alert engine.
func NewAlertEngine(parentCtx context.Context, memDb *memdb.MemDb, configWatcher *eapiclient.ConfigWatcher,
	logger log.Logger, resolverClient resolver.Interface) (Interface, error) {
	if nil == logger || nil == resolverClient || nil == configWatcher {
		return nil, errors.New("all parameters are required")
	}

	ctx, cancelFunc := context.WithCancel(parentCtx)
	ae := &alertEngineImpl{
		logger:         logger,
		resolverClient: resolverClient,
		configWatcher:  configWatcher,
		memDb:          memDb,
		exporter:       exporter.NewAlertExporter(memDb, configWatcher, logger.WithContext("submodule", "alert_exporter")),
		ctx:            ctx,
		cancelFunc:     cancelFunc,
	}

	ae.wg.Add(1)
	go ae.watchVersionObject()

	return ae, nil
}

// ProcessEvents will be called from the events manager whenever the events are received.
// And, it creates an alert whenever the event matches any policy.
func (a *alertEngineImpl) ProcessEvents(reqID string, eventList *evtsapi.EventList) {
	a.logger.Infof("debug: {req: %s} processing events at alert engine", reqID)
	defer a.logger.Infof("debug: {req: %s} finished processing events at alert engine", reqID)

	maintenanceMode := false
	a.RLock()
	if a.configWatcher.APIClient() == nil || a.maintenanceMode == nil {
		a.logger.Errorf("alert engine not ready to process events, waiting for API client and maintenance mode updates: %p, %v",
			a.configWatcher.APIClient(), a.maintenanceMode)
		a.RUnlock()
		return
	}
	if *a.maintenanceMode {
		maintenanceMode = true
	}
	a.RUnlock()

	for _, evt := range eventList.GetItems() {
		if maintenanceMode && evt.GetCategory() != eventattrs.Category_Rollout.String() {
			// skip processing all events that don't belong to rollout category
			// as a result, only rollout alerts will be triggered in the maintenance mode
			continue
		}

		// fetch alert policies belonging to evt.Tenant
		alertPolicies := a.memDb.GetAlertPolicies(
			memdb.WithTenantFilter(evt.GetTenant()),
			memdb.WithResourceFilter("Event"),
			memdb.WithEnabledFilter(true))

		// run over each alert policy
		for _, ap := range alertPolicies {
			var reqs []*fields.Requirement
			for _, t := range ap.Spec.GetRequirements() {
				r := *t
				reqs = append(reqs, &r)
			}
			ap.Spec.Requirements = reqs

			if err := a.runPolicy(ap, evt); err != nil {
				a.logger.Errorf("failed to run policy: %v on event: %v, err: %v", ap.GetName(), evt.GetMessage(), err)
			}
		}
	}
}

// Stop stops the alert engine by closing all the workers.
func (a *alertEngineImpl) Stop() {
	a.Lock()
	defer a.Unlock()

	a.cancelFunc()
	a.wg.Wait()

	a.exporter.Stop() // this will stop any exports that're in line
}

// watches version objects and sets the maintenance mode accordingly
func (a *alertEngineImpl) watchVersionObject() {
	defer a.wg.Done()

	watchCh := a.memDb.WatchVersion()
	defer a.memDb.StopWatchVersion(watchCh)

	for {
		select {
		case <-a.ctx.Done():
			return
		case evt, ok := <-watchCh:
			if !ok {
				a.logger.Errorf("error reading version from the channel, closing")
				return
			}
			versionObj := evt.Obj.(*cluster.Version)
			switch evt.EventType {
			case memdb.CreateEvent, memdb.UpdateEvent:
				if !utils.IsEmpty(versionObj.Status.RolloutBuildVersion) {
					a.setMaintenanceMode(true)
				}
				a.setMaintenanceMode(false)
			case memdb.DeleteEvent:
				a.setMaintenanceMode(false)
			}
		}
	}
}

// setMaintenanceMode sets the maintenance mode flag
func (a *alertEngineImpl) setMaintenanceMode(flag bool) {
	a.Lock()
	defer a.Unlock()
	if flag {
		if a.maintenanceMode == nil || (a.maintenanceMode != nil && !*a.maintenanceMode) {
			a.logger.Infof("entering maintenance mode, only upgrade events will be processed")
		}
	} else {
		if a.maintenanceMode != nil && *a.maintenanceMode {
			a.logger.Infof("leaving maintenance mode")
		}
	}
	a.maintenanceMode = &flag
}

// runPolicy helper function to run the given policy against event. Also, it updates
// totalHits and openAlerts on the alertPolicy.
func (a *alertEngineImpl) runPolicy(ap *monitoring.AlertPolicy, evt *evtsapi.Event) error {
	match, reqWithObservedVal := aeutils.Match(ap.Spec.GetRequirements(), evt)
	if match {
		a.logger.Debugf("event {%s: %s} matched the alert policy {%s} with requirements:[%+v]", evt.GetName(), evt.GetMessage(), ap.GetName(), ap.Spec.GetRequirements())
		created, err := a.createAlert(ap, evt, reqWithObservedVal)
		if err != nil {
			errStatus, ok := status.FromError(err)
			a.logger.Errorf("failed to create alert for event: %v, err: %v, %v", evt.GetUUID(), err, errStatus)
			// TODO: @Yuva, figure out if there are better ways to handle this case
			if ok && errStatus.Code() == codes.InvalidArgument && errStatus.Message() == "Request validation failed" {
				return nil
			}
		}

		if created {
			a.logger.Debugf("alert created from event {%s:%s}", evt.GetName(), evt.GetMessage())
			err = a.updateAlertPolicy(ap.GetObjectMeta(), 1, 1) // update total hits and open alerts count
		} else {
			a.logger.Debugf("existing open alert found for event {%s:%s}", evt.GetName(), evt.GetMessage())
			err = a.updateAlertPolicy(ap.GetObjectMeta(), 1, 0) //update only hits, alert exists already,
			// (source.nodeName, event.Type, event.Severity)
		}

		return err
	}

	return nil
}

// createAlert helper function to construct and create the alert using API client.
func (a *alertEngineImpl) createAlert(alertPolicy *monitoring.AlertPolicy, evt *evtsapi.Event,
	matchedRequirements []*monitoring.MatchedRequirement) (bool, error) {
	var err error
	alertUUID := uuid.NewV4().String()
	alertName := alertUUID
	creationTime, _ := types.TimestampProto(time.Now())

	// create alert object
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
				PolicyID:            alertPolicy.GetName(),
				MatchedRequirements: matchedRequirements,
			},
			Source: &monitoring.AlertSource{
				Component: evt.GetSource().GetComponent(),
				NodeName:  evt.GetSource().GetNodeName(),
			},
			EventURI:  evt.GetSelfLink(),
			ObjectRef: evt.GetObjectRef(),
		},
	}
	alert.SelfLink = alert.MakeURI("configs", "v1", "monitoring")

	alertCreated, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		if a.configWatcher.APIClient() == nil {
			return false, fmt.Errorf("empty API client")
		}

		// check there is an existing alert from the same event
		outstandingAlerts := a.memDb.GetAlerts(
			memdb.WithTenantFilter(alertPolicy.GetTenant()),
			memdb.WithAlertStateFilter([]monitoring.AlertState{monitoring.AlertState_OPEN}),
			memdb.WithAlertPolicyIDFilter(alertPolicy.GetName()),
			memdb.WithEventURIFilter(evt.GetSelfLink()))
		if len(outstandingAlerts) > 0 {
			a.logger.Debug("outstanding alert found that matches the event URI and policy")
			return false, nil
		}

		// if there is no alert from the same event, check if there is an alert from some other event.
		if evt.GetObjectRef() != nil {
			filters := []memdb.FilterFn{
				memdb.WithTenantFilter(alertPolicy.GetTenant()),
				memdb.WithAlertStateFilter([]monitoring.AlertState{monitoring.AlertState_OPEN, monitoring.AlertState_ACKNOWLEDGED}),
				memdb.WithAlertPolicyIDFilter(alertPolicy.GetName()),
				memdb.WithObjectRefFilter(evt.GetObjectRef()),
				memdb.WithEventMessageFilter(evt.GetMessage()),
			}

			outstandingAlerts = a.memDb.GetAlerts(filters...)
			if len(outstandingAlerts) >= 1 { // there should be exactly one outstanding alert; not more than that
				a.logger.Debug("1 or more outstanding alert found that matches the object ref. and policy")
				return false, nil
			}
		}

		// evt.GetObjectRef() == nil; cannot find outstanding alert if any.
		// create an alert
		_, err = a.configWatcher.APIClient().MonitoringV1().Alert().Create(ctx, alert)
		if err == nil {
			return true, nil
		}
		return false, err
	}, 100*time.Millisecond, maxRetry)

	if err != nil {
		return false, err
	}

	// TODO: run field selector on it (AlertDestination.Spec.Selector)
	if alertCreated.(bool) { // export alert
		if err := a.exporter.Export(alertPolicy.Spec.GetDestinations(), alert); err != nil {
			log.Errorf("failed to export alert %v to destinations %v, err: %v", alert.GetObjectMeta(), alertPolicy.Spec.GetDestinations(), err)
		}
	}

	return alertCreated.(bool), err
}

// updateAlertPolicy helper function to update total hits and open alerts count on the alert policy.
func (a *alertEngineImpl) updateAlertPolicy(meta *api.ObjectMeta, incrementTotalHitsBy,
	incrementOpenAlertsBy int) error {
	_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		if a.configWatcher.APIClient() == nil {
			return nil, fmt.Errorf("empty API client")
		}

		a.Lock() // to avoid racing updates
		defer a.Unlock()

		ap, err := a.configWatcher.APIClient().MonitoringV1().AlertPolicy().Get(ctx,
			&api.ObjectMeta{Name: meta.GetName(), Tenant: meta.GetTenant(), Namespace: meta.GetNamespace(), ResourceVersion: meta.GetResourceVersion(), UUID: meta.GetUUID()}) // get the alert policy
		if err != nil {
			return nil, err
		}

		ap.Status.OpenAlerts += int32(incrementOpenAlertsBy)
		ap.Status.TotalHits += int32(incrementTotalHitsBy)

		ap, err = a.configWatcher.APIClient().MonitoringV1().AlertPolicy().UpdateStatus(ctx, ap) // update the policy
		if err != nil {
			return nil, err
		}

		return nil, nil
	}, time.Second, maxRetry)

	return err
}
