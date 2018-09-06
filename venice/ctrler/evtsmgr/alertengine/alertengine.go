package alertengine

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
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
	pkgName  = "evts-alerts-engine"
	maxRetry = 5
)

// Module responsible for converting events to alerts.
// 1. Runs inline with events manager.
// 2. Any error that occurs while processing alert policies against each event
//    will be ignored. Alert will be generated in the next run when the same event is received (again, best effort).
// 3. Overlapping alert policies will trigger multiple alerts for the same event.
// 4. Event based alerts are not auto resolvable. So, this module will only create, no further
//    monitoring is done to auto-resolve.
// 5. There can be only one outstanding alert for an objRef (tenant, namespace, kind and name) by single alert policy.
// 6. If evt.objRef == nil, we could end up with multiple alerts for the same event (if it keeps happening) as there is no way
//    detect duplicate alert.

// TODO:
// 1. handle alert delivery (Alert Destination)

// AlertEngine represents the events alerts engine which is responsible fo converting
// events to alerts based on the event based alert policies.
type alertEngineImpl struct {
	sync.Mutex
	logger         log.Logger         // logger
	resolverClient resolver.Interface // to connect with apiserver to fetch alert policies; and send alerts
	memDb          *memdb.MemDb       // in-memory db/cache
	apiClient      apiclient.Services // API server client
}

// NewAlertEngine creates the new events alert engine.
func NewAlertEngine(memDb *memdb.MemDb, logger log.Logger, resolverClient resolver.Interface) (Interface, error) {
	var err error
	if nil == logger || nil == resolverClient {
		return nil, errors.New("all parameters are required")
	}

	ae := &alertEngineImpl{
		logger:         logger,
		resolverClient: resolverClient,
		memDb:          memDb,
	}

	// create API client
	if ae.apiClient, err = ae.createAPIClient(); err != nil {
		ae.logger.Errorf("failed to create API client, err: %v", err)
		return nil, err
	}

	return ae, nil
}

// ProcessEvents will be called from the events manager whenever the events are received.
// And, it creates an alert whenever the event matches any policy.
func (a *alertEngineImpl) ProcessEvents(eventList *evtsapi.EventList) {
	for _, evt := range eventList.GetItems() {
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
				a.logger.Errorf("failed to run policy: %v on event: %v, err: %v", ap.GetUUID(), evt.GetUUID(), err)
			}
		}
	}
}

// Stop stops the alert engine by closing all the workers.
func (a *alertEngineImpl) Stop() {
	a.Lock()
	defer a.Unlock()

	if a.apiClient != nil {
		if err := a.apiClient.Close(); err != nil {
			a.logger.Errorf("failed to close API client, err: %v", err)
		}
	}
}

// runPolicy helper function to run the given policy against event. Also, it updates
// totalHits and openAlerts on the alertPolicy.
func (a *alertEngineImpl) runPolicy(ap *monitoring.AlertPolicy, evt *evtsapi.Event) error {
	match, reqWithObservedVal := aeutils.Match(ap.Spec.GetRequirements(), evt)
	if match {
		created, err := a.createAlert(ap, evt, reqWithObservedVal)
		if err != nil {
			a.logger.Errorf("failed to create alert for event: %v, err: %v", evt.GetUUID(), err)
		}

		if created { // update total hits and open alerts count
			err = a.updateAlertPolicy(ap.GetObjectMeta(), 1, 1)
		} else { // update only hits, alert exists already, (source.nodeName, event.Type, event.Severity)
			err = a.updateAlertPolicy(ap.GetObjectMeta(), 1, 0)
		}

		return err
	}

	return nil
}

// createAlert helper function to construct and create the alert using API client.
func (a *alertEngineImpl) createAlert(alertPolicy *monitoring.AlertPolicy, evt *evtsapi.Event, matchedRequirements []*monitoring.MatchedRequirement) (bool, error) {
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
			State: monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)],
		},
		Status: monitoring.AlertStatus{
			Severity: alertPolicy.Spec.GetSeverity(),
			Message:  evt.GetMessage(),
			Reason: monitoring.AlertReason{
				PolicyID:            alertPolicy.GetUUID(),
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

	alertCreated, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		if evt.GetObjectRef() != nil {
			outstandingAlerts := a.memDb.GetAlerts(
				memdb.WithTenantFilter(alertPolicy.GetTenant()),
				memdb.WithAlertStateFilter(monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)]),
				memdb.WithAlertPolicyIDFilter(alertPolicy.GetUUID()),
				memdb.WithObjectRefFilter(evt.GetObjectRef()))
			if len(outstandingAlerts) >= 1 { // there should be exactly one outstanding alert; not more than that
				a.logger.Debug("alert exists already or more than 1 outstanding alert found")
				return false, nil
			}
		}

		// evt.GetObjectRef() == nil; cannot find outstanding alert if any.
		// create an alert
		_, err = a.apiClient.MonitoringV1().Alert().Create(context.Background(), alert)
		if err == nil {
			return true, nil
		}
		return false, err
	}, 60*time.Millisecond, maxRetry)

	return alertCreated.(bool), err
}

// updateAlertPolicy helper function to update total hits and open alerts count on the alert policy.
func (a *alertEngineImpl) updateAlertPolicy(meta *api.ObjectMeta, incrementTotalHitsBy, incrementOpenAlertsBy int) error {
	fmt.Println("updating policy", meta.Name)
	_, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		a.Lock() // to avoid racing updates
		defer a.Unlock()

		ap, err := a.apiClient.MonitoringV1().AlertPolicy().Get(context.Background(),
			&api.ObjectMeta{Name: meta.GetName(), Tenant: meta.GetTenant(), Namespace: meta.GetNamespace(), ResourceVersion: meta.GetResourceVersion(), UUID: meta.GetUUID()}) // get the alert policy
		if err != nil {
			return nil, err
		}

		ap.Status.OpenAlerts += int32(incrementOpenAlertsBy)
		ap.Status.TotalHits += int32(incrementTotalHitsBy)

		ap, err = a.apiClient.MonitoringV1().AlertPolicy().Update(context.Background(), ap) // update the policy
		if err != nil {
			return nil, err
		}

		return nil, nil
	}, 60*time.Millisecond, maxRetry)

	return err
}

// createAPIClient helper function to create API server client.
func (a *alertEngineImpl) createAPIClient() (apiclient.Services, error) {
	client, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return apiclient.NewGrpcAPIClient(globals.EvtsMgr, globals.APIServer, a.logger.WithContext("pkg", "evtsmgr-alert-engine"),
			rpckit.WithBalancer(balancer.New(a.resolverClient)))
	}, 2*time.Second, maxRetry)
	if err != nil {
		a.logger.Errorf("failed to create API client, err: %v", err)
		return nil, err
	}

	return client.(apiclient.Services), err
}
