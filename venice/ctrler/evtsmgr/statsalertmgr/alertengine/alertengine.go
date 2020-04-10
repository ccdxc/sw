package alertengine

import (
	"context"
	"fmt"
	"math/rand"
	"strconv"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	samtypes "github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

//
// AlertEngine is responsible for running the query response against threshold values from stats alert policy and
// create/update/resolve the alert accordingly.
//
// TODO:
// 1. Add retries for calls to external service e.g. API server.
// 2. Check all the required fields in the alert object is populated (e.g. alert source).
// 3. List open alerts should be served from the local cache instead of fetching the results from API server.
// 4. Hook alert engine with exporter.
// 5. May be the updates to API server should be grouped to avoid sending plenty of requests to API server.
//

// StatsAlertEngine represents the alert engine.
type StatsAlertEngine struct {
	resolverClient resolver.Interface // to connect with API server to create/update alerts
	memDb          *memdb.MemDb       // in-memory db/cache
	apiClients     *apiClients        // list of API clients
	ctx            context.Context    // context to cancel goroutines
	cancelFunc     context.CancelFunc // context to cancel goroutines
	logger         log.Logger         // logger
	wg             sync.WaitGroup
}

// list of API clients to use for CRUD operations on API server.
type apiClients struct {
	sync.RWMutex
	clients []apiclient.Services
}

var (
	numAPIClients     = 5
	requestRetries    = 10
	apiRequestTimeout = 5 * time.Second
)

// NewStatsAlertEngine creates a new alert engine with the given parameters.
func NewStatsAlertEngine(parentCtx context.Context, memDb *memdb.MemDb, resolverClient resolver.Interface,
	logger log.Logger) *StatsAlertEngine {
	// for api client
	rand.Seed(time.Now().UnixNano())

	ctx, cancelFunc := context.WithCancel(parentCtx)
	ae := &StatsAlertEngine{
		resolverClient: resolverClient,
		memDb:          memDb,
		apiClients:     &apiClients{clients: make([]apiclient.Services, 0, numAPIClients)},
		ctx:            ctx,
		cancelFunc:     cancelFunc,
		logger:         logger}

	ae.wg.Add(1)
	go ae.createAPIClients()
	return ae
}

// ProcessQueryResponse runs the query response against policy thresholds and accordingly create/update/resolve
// the alert.
func (a *StatsAlertEngine) ProcessQueryResponse(policyMeta *api.ObjectMeta, qResp *samtypes.QueryResponse) error {
	a.logger.Infof("processing query response, policy: %+v", policyMeta)
	for reporter, resp := range qResp.ByReporterID {
		a.logger.Infof("query resp [%s]: %v", reporter, resp)
	}

	policyID := fmt.Sprintf("%s/%s", policyMeta.GetName(), policyMeta.GetUUID())

	// wait until API client is available
	apCl := a.getAPIClient()
	for apCl == nil {
		a.logger.Errorf("{%s} no API client available to process stats alert policy: %v, retrying..", policyID, policyMeta.GetName())
		if a.ctx.Err() != nil {
			a.logger.Error("context cancelled, returning from processing query response")
			return fmt.Errorf("no API client available to process stats alert policy")
		}
		time.Sleep(time.Second)
		apCl = a.getAPIClient()
	}

	val, err := a.memDb.FindObject("StatsAlertPolicy", policyMeta)
	if err != nil {
		a.logger.Errorf("{%s} failed to get stats alert policy from memdb, err: %v", policyID, err)
		return err
	}
	statsAlertPolicy, ok := val.(*monitoring.StatsAlertPolicy)
	if !ok {
		return fmt.Errorf("{%s} invalid stats alert policy type found in memdb: %v", policyID, val)
	}

	for reporterID, resp := range qResp.ByReporterID { // reporterID = node/DSC ID
		// compute/derive affected object kind based on the metric.group defined in stats alert policy
		objRef := &api.ObjectRef{Kind: a.getObjRefKind(statsAlertPolicy), Name: reporterID}

		// get existing open alerts
		existingAlerts, err := a.getExistingOpenAlert(policyID, apCl, policyMeta, objRef)
		if err != nil {
			return err
		}

		if l := len(existingAlerts); l == 0 { // alert doesn't exists
			if err := a.checkThresholdsAndCreateAlert(policyID, apCl, resp, statsAlertPolicy, objRef); err != nil {
				return err
			}
		} else if l == 1 { // alert already exists; compare it against the threshold values and accordingly resolve/update the alert severity
			if err := a.checkThresholdsAndUpdateAlert(policyID, apCl, existingAlerts[0], resp, statsAlertPolicy, objRef); err != nil {
				return err
			}
		} else { // l > 1; something wrong
			a.logger.Errorf("{%s} more than 1 outstanding alert found for the policy", policyID)
		}
	}

	return nil
}

// Stop stops the alert engine.
func (a *StatsAlertEngine) Stop() {
	a.cancelFunc()
	a.wg.Wait()

	// close all the API clients
	for _, apiCl := range a.apiClients.clients {
		if apiCl != nil {
			apiCl.Close()
		}
	}
	a.apiClients.clients = make([]apiclient.Services, 0, numAPIClients)
}

// returns the list existing open alerts for the given policy and object-ref.
func (a *StatsAlertEngine) getExistingOpenAlert(policyID string, apCl apiclient.Services, policyMeta *api.ObjectMeta, objRef *api.ObjectRef) ([]*monitoring.Alert, error) {
	ctx, cancelFunc := context.WithTimeout(a.ctx, apiRequestTimeout)
	existingAlerts, err := apCl.MonitoringV1().Alert().List(ctx, &api.ListWatchOptions{FieldSelector: fmt.Sprintf(
		"spec.state=%s,status.reason.alert-policy-id=%s/%s,status.object-ref.kind=%s,status.object-ref.name=%s",
		monitoring.AlertState_OPEN.String(), policyMeta.GetName(), policyMeta.GetUUID(), objRef.Kind, objRef.Name)})
	cancelFunc()
	if err != nil {
		a.logger.Errorf("{%s} failed to get list of alerts that belongs to the policy", policyID)
		return nil, err
	}

	return existingAlerts, nil
}

// check the metric value against policy threshold and update/resolve the existing underlying alert.
func (a *StatsAlertEngine) checkThresholdsAndUpdateAlert(policyID string, apCl apiclient.Services, alert *monitoring.Alert,
	metricVale *samtypes.MetricValue, statsAlertPolicy *monitoring.StatsAlertPolicy, objRef *api.ObjectRef) error {
	updateAlert := true
	timeNow, _ := types.TimestampProto(time.Now())

	thresholdMet, err := a.hasThresholdsMet(policyID, metricVale, statsAlertPolicy)
	if err != nil {
		return err
	}

	if !thresholdMet {
		// resolve the alert once threshold values are cleared
		a.logger.Infof("{%s} threshold conditions are cleared, resolving alert", policyID)
		alert.Spec.State = monitoring.AlertState_RESOLVED.String()
		alert.Status.Resolved = &monitoring.AuditInfo{
			User: "system",
			Time: &api.Timestamp{
				Timestamp: *timeNow,
			},
		}
	} else {
		// commute the severity for alert based on the threshold values and current metric value
		newAlertSeverity, newAlertMessage, err := a.computeAlertSeverityAndMessage(
			policyID, metricVale, statsAlertPolicy, objRef.Name)
		if err != nil {
			a.logger.Errorf("{%s} failed to compute alert severity and message, err: %v", policyID, err)
			return err
		}

		if monitoring.AlertSeverity_vvalue[newAlertSeverity] > monitoring.AlertSeverity_vvalue[alert.Status.Severity] {
			a.logger.Infof("{%s} raising the severity of the alert {%v} from %s to %s",
				policyID, alert, alert.Status.Severity, newAlertSeverity)
		} else if monitoring.AlertSeverity_vvalue[newAlertSeverity] < monitoring.AlertSeverity_vvalue[alert.Status.Severity] {
			a.logger.Infof("{%s} lowering the severity of the alert {%v} from %s to %s",
				policyID, alert, alert.Status.Severity, newAlertSeverity)
		} else { // nothing to be done
			updateAlert = false
		}

		alert.Status.Severity = newAlertSeverity
		alert.Status.Message = newAlertMessage
	}

	// update alert if required
	if updateAlert {
		if _, err := a.updateAlertObj(apCl, alert); err != nil {
			if errStatus, ok := status.FromError(err); ok {
				a.logger.Infof("{%s} failed to update alert, err: %v", policyID, errStatus)
			}
			return err
		}
	}

	return nil
}

// check the metric value against policy threshold and create an alert if required.
func (a *StatsAlertEngine) checkThresholdsAndCreateAlert(policyID string, apCl apiclient.Services, metricVale *samtypes.MetricValue,
	statsAlertPolicy *monitoring.StatsAlertPolicy, objRef *api.ObjectRef) error {
	// check if any of the threshold values are met
	thresholdMet, err := a.hasThresholdsMet(policyID, metricVale, statsAlertPolicy)
	if err != nil {
		return err
	}

	if thresholdMet {
		a.logger.Infof("{%s} metric value reached policy threshold, creating alert", policyID)

		// commute the severity for alert based on the threshold values and current metric value
		alertSev, alertMsg, err := a.computeAlertSeverityAndMessage(policyID, metricVale, statsAlertPolicy, objRef.Name)
		if err != nil {
			a.logger.Errorf("{%s} failed to compute alert severity and message, err: %v", policyID, err)
			return err
		}

		// create alert
		if _, err := a.createAlertObj(apCl, statsAlertPolicy, objRef, alertSev, alertMsg); err != nil {
			if errStatus, ok := status.FromError(err); ok {
				a.logger.Infof("{%s} failed to create alert, err: %v", policyID, errStatus)
			}
			return err
		}
	}

	return nil
}

// updateAlertObj helper function to update the given alert using API client.
func (a *StatsAlertEngine) updateAlertObj(apCl apiclient.Services, alert *monitoring.Alert) (bool, error) {
	alertUpdated, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		_, err := apCl.MonitoringV1().Alert().Update(ctx, alert)
		if err == nil {
			return true, nil
		}
		return false, err
	}, 2*time.Second, requestRetries)
	if err != nil {
		return false, err
	}

	return alertUpdated.(bool), err
}

// createAlertObj helper function to construct and create the alert using API client.
func (a *StatsAlertEngine) createAlertObj(apCl apiclient.Services, statsAlertPolicy *monitoring.StatsAlertPolicy,
	objRef *api.ObjectRef, severity, message string) (bool, error) {
	alertUUID := uuid.NewV4().String()
	alertName := alertUUID
	creationTime, _ := types.TimestampProto(time.Now())

	// create alert object
	policyID := fmt.Sprintf("%s/%s", statsAlertPolicy.GetName(), statsAlertPolicy.GetUUID())
	alert := &monitoring.Alert{
		TypeMeta: api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{
			Name:      alertName,
			UUID:      alertUUID,
			Tenant:    statsAlertPolicy.GetTenant(),
			Namespace: statsAlertPolicy.GetNamespace(),
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
			Severity: severity,
			Message:  message,
			Reason: monitoring.AlertReason{
				PolicyID: policyID,
			},
			ObjectRef: objRef,
			TotalHits: 1,
		},
	}
	alert.SelfLink = alert.MakeURI("configs", "v1", "monitoring")

	alertCreated, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		_, err := apCl.MonitoringV1().Alert().Create(ctx, alert)
		if err == nil {
			return true, nil
		}
		return false, err
	}, 2*time.Second, requestRetries)
	if err != nil {
		return false, err
	}

	return alertCreated.(bool), err
}

// helper function that checks whether the given metric value has reached policy threshold.
func (a *StatsAlertEngine) hasThresholdsMet(policyID string, metricValue *samtypes.MetricValue, statsAlertPolicy *monitoring.StatsAlertPolicy) (bool, error) {
	thresholds := statsAlertPolicy.Spec.Thresholds
	// since thresholds.Values are sorted by severity in the API hooks, it is safe to iterate over them in this order:
	// critical -> warn -> info
	for i := len(thresholds.Values) - 1; i >= 0; i-- {
		threshold := thresholds.Values[i]
		raiseVal, err := strconv.ParseFloat(threshold.RaiseValue, 64)
		if err != nil {
			a.logger.Errorf("{%s} failed to convert threshold raise value to float64, err: %v", policyID, err)
			return false, err
		}

		if (thresholds.Operator == monitoring.Operator_GREATER_OR_EQUAL_THAN.String() && metricValue.Val >= raiseVal) ||
			(thresholds.Operator == monitoring.Operator_LESS_OR_EQUAL_THAN.String() && metricValue.Val <= raiseVal) {
			return true, nil
		}
	}
	return false, nil
}

// computes the alert severity and message based on the policy and metric value.
func (a *StatsAlertEngine) computeAlertSeverityAndMessage(policyID string, metricValue *samtypes.MetricValue,
	statsAlertPolicy *monitoring.StatsAlertPolicy, reporterID string) (string, string, error) {
	var alertSev, alertMsg string
	thresholds := statsAlertPolicy.Spec.Thresholds
	for i := len(thresholds.Values) - 1; i >= 0; i-- {
		threshold := thresholds.Values[i]
		raiseVal, err := strconv.ParseFloat(threshold.RaiseValue, 64)
		if err != nil {
			a.logger.Errorf("{%s} failed to convert threshold raise value to float64, err: %v", policyID, err)
			return alertSev, alertMsg, err
		}

		if (thresholds.Operator == monitoring.Operator_GREATER_OR_EQUAL_THAN.String() && metricValue.Val >= raiseVal) ||
			(thresholds.Operator == monitoring.Operator_LESS_OR_EQUAL_THAN.String() && metricValue.Val <= raiseVal) {
			alertSev = threshold.Severity
			if statsAlertPolicy.Spec.MeasurementCriteria != nil {
				alertMsg = fmt.Sprintf("%s(%s) reached (%s) on %s",
					statsAlertPolicy.Spec.MeasurementCriteria.Function, statsAlertPolicy.Spec.Metric.FieldName, threshold.RaiseValue, reporterID)
			} else {
				alertMsg = fmt.Sprintf("%s reached (%s) on %s",
					statsAlertPolicy.Spec.Metric.FieldName, threshold.RaiseValue, reporterID)
			}
			return alertSev, alertMsg, nil
		}
	}
	return alertSev, alertMsg, nil
}

// returns a random API client from the list of clients
func (a *StatsAlertEngine) getAPIClient() apiclient.Services {
	a.apiClients.RLock()
	defer a.apiClients.RUnlock()

	if len(a.apiClients.clients) == 0 {
		return nil
	}

	index := rand.Intn(len(a.apiClients.clients))
	return a.apiClients.clients[index]
}

// helper function to create API clients
func (a *StatsAlertEngine) createAPIClients() {
	defer a.wg.Done()

	numCreated := 0
	for numCreated < numAPIClients {
		if a.ctx.Err() != nil {
			return
		}

		logger := a.logger.WithContext("pkg", fmt.Sprintf("%s-%s-%d", globals.EvtsMgr, "stats-alert-engine-api-client", numCreated))
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

// FIXME! remove this workaround.
// This is a workaround till we get the kind along with reporter ID in metrics query response.
func (a *StatsAlertEngine) getObjRefKind(sap *monitoring.StatsAlertPolicy) string {
	if !utils.IsEmpty(sap.Spec.Metric.Group) {
		return "DistributedServiceCard"
	}

	return sap.Spec.Metric.Kind // has to be either Node or DistributedServiceCard
}
