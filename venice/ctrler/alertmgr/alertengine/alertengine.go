// Package alertengine opens, resolves, purges alerts based on the incoming policy engine results.
package alertengine

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	apiservice "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/ctrler/alertmgr/policyengine"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// Wait time between API server retries.
	apiSrvWaitIntvl = time.Second

	// Maximum number of API server retries.
	maxAPISrvRetries = 200

	// Wait time before resolving an alert, to allow for flaps.
	debounceTime = 2 * time.Second

	// Debounce ticker interval.
	debounceTimerInterval = 500 * time.Millisecond

	// Minimum time a resolved alert should exist before getting garbage collected.
	gcRetentionTime = 24 * time.Hour

	// Garbage collector ticker interval.
	gcTimerInterval = 30 * time.Minute
)

// Interface for alertengine
type Interface interface {
	// Run alert engine in a separate go routine until explicitly stopped.
	// KV operations run concurrently in separate go routines; KV operations for the same alert are serialized.
	Run(context.Context, apiservice.Services, <-chan *policyengine.PEOutput) (<-chan *monitoring.Alert, <-chan error, error)

	// Stop alert engine.
	Stop()

	// GetRunningStatus of alert engine.
	GetRunningStatus() bool
}

type alertEngine struct {
	// AlertMgr API client.
	apiClient apiservice.Services

	// AlertMgr logger.
	logger log.Logger

	// AlertMgr API server resolver.
	rslvr resolver.Interface

	// Alertengine run context.
	ctx    context.Context
	cancel context.CancelFunc

	// AlertMgr object db.
	objdb objectdb.Interface

	// Incoming channel from policy engine.
	inCh chan *policyengine.PEOutput

	// Output channel.
	// Output channel is closed when alert engine is explicitly stopped.
	outCh chan *monitoring.Alert

	// Error channel.
	// AlertMgr must monitor the error channel and shutdown on error, allowing itself to be restarted.
	// Error channel is never closed (it will be garbage collected).
	errCh chan error

	// Alert cache.
	// Reflects the current status of alerts; KV store updates are done in background in separate goroutines.
	// No mutex required, as all cache operations are done by the main alertengine goroutine only.
	cache alertCache

	// KV store updates tracker.
	// KV store updates may take time, so they are run in parallel.
	// However, updates for the same alert are serialized.
	kvOps kvOpTracker

	// Debounce ticker to mitigate flaps.
	debounceTicker *time.Ticker

	// Garbage collector ticker.
	gcTicker *time.Ticker

	// Wait group for goroutines spawned.
	wg sync.WaitGroup

	// Running status of alert engine.
	// No mutex required as this is set and read only by the alertmgr run goroutine.
	running bool
}

// Alert Cache.
type alertCache struct {
	alertsState              map[*monitoring.Alert]alertState
	alertsByObjectAndPolicy  map[string]map[string]*monitoring.Alert
	alertsByPolicy           map[string][]*monitoring.Alert
	alertsInDebounceInterval []*debounceInfo
	alertsInGCInterval       []*monitoring.Alert
}

// Alert States
type alertState int

const (
	alertStateOpenAcked alertState = iota
	alertStateOpenUnacked
	alertStateOpenDebounceTimerRunning
	alertStateResolved
)

// AlertOp alert operations
type AlertOp int

const (
	// AlertOpCreate Alert create operation
	AlertOpCreate AlertOp = iota
	// AlertOpDelete Alert delete operation
	AlertOpDelete
	// AlertOpResolve Alert resolve operation
	AlertOpResolve
	// AlertOpReopen Alert reopen operation
	AlertOpReopen
	// AlertOpUpdate Alert update operation
	AlertOpUpdate
	// AlertOpNone nil operation
	AlertOpNone
)

// Put alert in debounce list.
func (ae *alertEngine) debounceAlertInCache(alert *monitoring.Alert) {
	list := ae.cache.alertsInDebounceInterval
	list = append(list, &debounceInfo{alert: alert, numTicks: int(debounceTime / debounceTimerInterval)})
	ae.cache.alertsInDebounceInterval = list
	ae.cache.alertsState[alert] = alertStateOpenDebounceTimerRunning
}

// Remove alert from debounce list.
func (ae *alertEngine) bounceAlertInCache(alert *monitoring.Alert) {
	list := ae.cache.alertsInDebounceInterval
	for pos, dbncInfo := range list {
		if dbncInfo.alert == alert {
			list = append(list[:pos], list[pos+1:]...)
			ae.cache.alertsInDebounceInterval = list
			ae.cache.alertsState[alert] = alertStateOpenUnacked
		}
	}
}

// Remove alert from garbage collect list.
func (ae *alertEngine) reopenAlertInCache(alert *monitoring.Alert) {
	list := ae.cache.alertsInGCInterval
	for pos, gcAlert := range list {
		if gcAlert == alert {
			list = append(list[:pos], list[pos+1:]...)
			ae.cache.alertsInGCInterval = list
			ae.cache.alertsState[alert] = alertStateOpenUnacked
		}
	}
}

func (ae *alertEngine) createAlertInCache(alert *monitoring.Alert) {
	objRef := alert.Status.ObjectRef.String()
	polID := alert.Status.Reason.PolicyID

	polMap, found := ae.cache.alertsByObjectAndPolicy[objRef]
	if !found {
		polMap = make(map[string]*monitoring.Alert)
	}

	polMap[polID] = alert
	ae.cache.alertsByObjectAndPolicy[objRef] = polMap
	ae.cache.alertsByPolicy[polID] = append(ae.cache.alertsByPolicy[polID], alert)
	ae.cache.alertsState[alert] = alertStateOpenUnacked
	ae.logger.Debugf("alertcache dump %v", ae.cache.alertsByObjectAndPolicy)
}

func (ae *alertEngine) resolveAlertInCache(alert *monitoring.Alert, pos int) {
	list := ae.cache.alertsInDebounceInterval
	list = append(list[:pos], list[pos+1:]...)
	ae.cache.alertsInDebounceInterval = list
	ae.cache.alertsInGCInterval = append(ae.cache.alertsInGCInterval, alert)
	ae.cache.alertsState[alert] = alertStateResolved
	ae.logger.Debugf("resolved alert %v", alert)
}

func (ae *alertEngine) deleteAlertInCache(alert *monitoring.Alert) {
	objRef := alert.Status.ObjectRef.String()
	polID := alert.Status.Reason.PolicyID

	if state, found := ae.cache.alertsState[alert]; found {
		switch state {
		case alertStateOpenDebounceTimerRunning:
			list := ae.cache.alertsInDebounceInterval
			for pos, dbncInfo := range list {
				if dbncInfo.alert == alert {
					list = append(list[:pos], list[pos+1:]...)
					ae.cache.alertsInDebounceInterval = list
				}
			}
		case alertStateResolved:
			list := ae.cache.alertsInGCInterval
			for pos, gcAlert := range list {
				if gcAlert == alert {
					list = append(list[:pos], list[pos+1:]...)
					ae.cache.alertsInGCInterval = list
				}
			}
		}
		delete(ae.cache.alertsByObjectAndPolicy[objRef], polID)
		delete(ae.cache.alertsState, alert)
	}

	ae.logger.Debugf("alertcache dump %v", ae.cache.alertsByObjectAndPolicy)
}

func (ae *alertEngine) deleteAlertsInCacheByObject(objRef string) []*monitoring.Alert {
	var alertsToDelete []*monitoring.Alert
	if polMap, found := ae.cache.alertsByObjectAndPolicy[objRef]; found {
		for _, alert := range polMap {
			ae.deleteAlertInCache(alert)
			alertsToDelete = append(alertsToDelete, alert)
		}
		delete(ae.cache.alertsByObjectAndPolicy, objRef)
	}

	return alertsToDelete
}

func (ae *alertEngine) deleteAlertsInCacheByPolicy(polID string) []*monitoring.Alert {
	var alertsToDelete []*monitoring.Alert
	for _, polMap := range ae.cache.alertsByObjectAndPolicy {
		if alert, found := polMap[polID]; found {
			ae.deleteAlertInCache(alert)
			alertsToDelete = append(alertsToDelete, alert)
		}
	}

	return alertsToDelete
}

// A map of KV operations in flight and pending operations (for the same alerts), if any.
type kvOpTracker struct {
	sync.Mutex
	opMap map[string]*waitQ
}

type waitQ struct {
	inProcess bool
	q         []kvOp
}

type kvOp struct {
	action AlertOp
	alert  *monitoring.Alert
}

// Check if the given KV operation can be done now or must be deferred.
func (ae *alertEngine) doKVOpNowOrLater(key string, op kvOp) string {
	ae.kvOps.Lock()
	defer ae.kvOps.Unlock()

	if wq, found := ae.kvOps.opMap[key]; found {
		if wq.inProcess {
			// A KV operation for this alert is already in progress.
			wq.q = append(wq.q, op)
			return "later"
		}

		if len(wq.q) > 0 {
			// Some KV operation for this alert are already pending.
			wq.q = append(wq.q, op)
			return "later"
		}

		wq.inProcess = true
		return "now"
	}

	ae.kvOps.opMap[key] = &waitQ{inProcess: true}
	return "now"
}

// Once a KV operation is done, fetch a pending operation for the same alert.
func (ae *alertEngine) nextKVOp(key string) (alert *monitoring.Alert, op AlertOp) {
	ae.kvOps.Lock()
	defer ae.kvOps.Unlock()

	if wq, found := ae.kvOps.opMap[key]; found {
		if wq.inProcess {
			if len(wq.q) == 0 {
				delete(ae.kvOps.opMap, key)
				return nil, AlertOpNone
			}

			qHead := wq.q[0]
			ae.kvOps.opMap[key].q = wq.q[1:]
			return qHead.alert, qHead.action
		}
	}
	ae.logger.Errorf("KV q not active, %v", key)
	return nil, AlertOpNone
}

type debounceInfo struct {
	alert    *monitoring.Alert
	numTicks int
}

// New alert engine.
func New(logger log.Logger, rslvr resolver.Interface, objdb objectdb.Interface) (Interface, error) {
	// Create cache maps.
	alertsByObjectAndPolicy := make(map[string]map[string]*monitoring.Alert)
	alertsByPolicy := make(map[string][]*monitoring.Alert)
	alertsState := make(map[*monitoring.Alert]alertState)

	// Fetch alerts that are already there.
	var resolvedAlerts []*monitoring.Alert
	for _, a := range objdb.List("Alert") {
		alert := a.(*monitoring.Alert)
		refPol := alert.Status.Reason.PolicyID
		refObj := alert.Status.ObjectRef.String()

		var state alertState
		if alert.Status.Resolved != nil {
			state = alertStateResolved
			resolvedAlerts = append(resolvedAlerts, alert)
		} else {
			state = alertStateOpenUnacked
		}
		alertsState[alert] = state
		alertsByPolicy[refPol] = append(alertsByPolicy[refPol], alert)
		polMap, ok := alertsByObjectAndPolicy[refObj]
		if !ok {
			polMap = make(map[string]*monitoring.Alert)
		}
		polMap[refObj] = alert
		alertsByObjectAndPolicy[refObj] = polMap
	}

	cache := alertCache{alertsByObjectAndPolicy: alertsByObjectAndPolicy, alertsState: alertsState, alertsByPolicy: alertsByPolicy, alertsInGCInterval: resolvedAlerts}

	ae := &alertEngine{
		logger: logger,
		rslvr:  rslvr,
		objdb:  objdb,
		cache:  cache,
	}

	// Create KV operations map.
	opMap := make(map[string]*waitQ)
	ae.kvOps = kvOpTracker{opMap: opMap}

	logger.Infof("Created new alert engine")
	return ae, nil
}

func (ae *alertEngine) Run(ctx context.Context, apiClient apiservice.Services, inCh <-chan *policyengine.PEOutput) (<-chan *monitoring.Alert, <-chan error, error) {
	if ae.running {
		return nil, nil, fmt.Errorf("alert engine already running")
	}

	ae.ctx, ae.cancel = context.WithCancel(ctx)
	ae.outCh = make(chan *monitoring.Alert)
	ae.errCh = make(chan error, 1)
	ae.apiClient = apiClient

	go func() {
		defer ae.cleanup()

		// Start debounce and garbage collector tickers.
		ae.debounceTicker = time.NewTicker(debounceTimerInterval)
		ae.gcTicker = time.NewTicker(gcTimerInterval)

		for {
			select {
			case <-ae.debounceTicker.C:
				ae.processDebounceTick()
			case <-ae.gcTicker.C:
				ae.garbageCollect()
			case peResult, ok := <-inCh:
				if ok {
					err := ae.processInput(peResult)
					if err != nil {
						if err != ae.ctx.Err() {
							ae.errCh <- err
						}
						return
					}
				}
			case <-ae.ctx.Done():
				ae.logger.Errorf("Context cancelled, exiting")
				return
			}
		}
	}()

	ae.running = true
	ae.logger.Infof("Running alert engine")
	return ae.outCh, ae.errCh, nil
}

func (ae *alertEngine) Stop() {
	ae.cancel()
}

func (ae *alertEngine) GetRunningStatus() bool {
	return ae.running
}

func (ae *alertEngine) processInput(peResult *policyengine.PEOutput) error {
	ae.logger.Debugf("Received input %v", peResult)

	obj := peResult.Object.(runtime.Object)
	switch obj.(type) {
	case *monitoring.Alert:
		return ae.processAlert(peResult)
	case *monitoring.AlertPolicy:
		return ae.processAlertPolicy(peResult)
	//	TODO case *monitoring.AlertDestination:
	default:
		return ae.processObject(peResult)
	}
}

func (ae *alertEngine) processObject(peOutput *policyengine.PEOutput) error {
	if peOutput.Op != kvstore.Deleted && !peOutput.WasPolicyApplied {
		return nil
	}

	ometa, err := runtime.GetObjectMeta(peOutput.Object)
	if err != nil {
		return err
	}

	ae.logger.Debugf("Processing object, meta: %v, op: ", ometa, peOutput.Op)

	objectRef := &api.ObjectRef{
		//Tenant:    ometa.Tenant,
		//Namespace: ometa.Namespace,
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Kind:      peOutput.Object.(runtime.Object).GetObjectKind(),
		Name:      ometa.Name,
		URI:       ometa.SelfLink,
	}
	objRef := objectRef.String()

	var alertsToCreate []*monitoring.Alert
	var alertsToUpdate []*monitoring.Alert
	var alertsToDelete []*monitoring.Alert

	switch peOutput.Op {
	case kvstore.Created, kvstore.Updated:
		polMap, found := ae.cache.alertsByObjectAndPolicy[objRef]
		if !found {
			// This object did not match any policy previously.
			for _, mp := range peOutput.MatchingPolicies {
				alert := ae.createAlert(peOutput.Object.(runtime.Object), mp)
				ae.createAlertInCache(alert)
				alertsToCreate = append(alertsToCreate, alert)
			}
		} else {
			// This object matched at least one policy previously.
			// Check which policies it still matches and accordingly resolve/reopen alerts.
			for polID, alert := range polMap {
				stillMatches := func() bool {
					for _, mp := range peOutput.MatchingPolicies {
						mPolID := fmt.Sprintf("%s/%s", mp.Policy.GetName(), mp.Policy.GetUUID())
						if polID == mPolID {
							return true
						}
					}
					return false
				}()

				now, _ := types.TimestampProto(time.Now())
				if stillMatches {
					switch ae.cache.alertsState[alert] {
					case alertStateOpenUnacked:
					case alertStateOpenAcked:
					case alertStateOpenDebounceTimerRunning:
						ae.bounceAlertInCache(alert)
						alert.ModTime = api.Timestamp{Timestamp: *now}
						alertsToUpdate = append(alertsToUpdate, alert)
					case alertStateResolved:
						alert.Spec.State = monitoring.AlertState_OPEN.String()
						alert.Status.Resolved = nil
						alert.ModTime = api.Timestamp{Timestamp: *now}
						ae.reopenAlertInCache(alert)
						alertsToUpdate = append(alertsToUpdate, alert)
					}
				} else {
					switch ae.cache.alertsState[alert] {
					case alertStateOpenUnacked:
						alert.ModTime = api.Timestamp{Timestamp: *now}
						ae.debounceAlertInCache(alert)
					case alertStateOpenAcked:
						// TODO
					case alertStateOpenDebounceTimerRunning:
					case alertStateResolved:
					}
				}
			}

			// Create alerts for new matches.
			for _, mp := range peOutput.MatchingPolicies {
				polID := fmt.Sprintf("%s/%s", mp.Policy.GetName(), mp.Policy.GetUUID())
				if _, found := ae.cache.alertsByObjectAndPolicy[objRef][polID]; !found {
					alert := ae.createAlert(peOutput.Object.(runtime.Object), mp)
					ae.createAlertInCache(alert)
					alertsToCreate = append(alertsToCreate, alert)
				}
			}
		}

	case kvstore.Deleted:
		alertsToDelete = ae.deleteAlertsInCacheByObject(objRef)
	}

	export := func(alert *monitoring.Alert) error {
		select {
		case <-ae.ctx.Done():
			ae.logger.Errorf("Context cancelled, exiting")
			return ae.ctx.Err()
		case ae.outCh <- alert:
			return nil
		}
	}

	// TODO handle errors from dispatchAlert
	dispatchAlerts := func() {
		for _, alert := range alertsToCreate {
			//export(alert)
			ae.dispatchAlert(alert, AlertOpCreate)
		}
		for _, alert := range alertsToUpdate {
			export(alert)
			ae.dispatchAlert(alert, AlertOpUpdate)
		}
		for _, alert := range alertsToDelete {
			ae.dispatchAlert(alert, AlertOpDelete)
		}
	}

	dispatchAlerts()
	return nil
}

func (ae *alertEngine) processAlert(peOutput *policyengine.PEOutput) error {
	return nil
}

func (ae *alertEngine) processAlertPolicy(peOutput *policyengine.PEOutput) error {
	pol := peOutput.Object.(*monitoring.AlertPolicy)

	ae.logger.Debugf("Processing alert policy, %v", pol)

	switch peOutput.Op {
	case kvstore.Created:
		// Policy has already been run on objects.
		// Create alert if there is a matching object.
		if peOutput.WasPolicyApplied {
			obj := peOutput.MatchingObj.Obj.(runtime.Object)
			pol := policyengine.MatchingPolicy{Policy: peOutput.Object.(*monitoring.AlertPolicy), Reqs: peOutput.MatchingObj.Reqs}
			alert := ae.createAlert(obj, pol)
			ae.createAlertInCache(alert)
			ae.dispatchAlert(alert, AlertOpCreate)
		}

	case kvstore.Updated:
		// Unsupported

	case kvstore.Deleted:
		// Delete all alerts corresponding to this policy.
		polID := fmt.Sprintf("%s/%s", pol.GetName(), pol.GetUUID())
		alertsToDelete := ae.deleteAlertsInCacheByPolicy(polID)
		for _, alert := range alertsToDelete {
			ae.dispatchAlert(alert, AlertOpDelete)
		}
	}

	return nil
}

func (ae *alertEngine) cleanup() {
	if ae.running {
		ae.wg.Wait()
		ae.running = false
		ae.debounceTicker.Stop()
		ae.gcTicker.Stop()
		close(ae.errCh)
		close(ae.outCh)
	}
}

func (ae *alertEngine) processDebounceTick() {
	var alertsToResolve []*monitoring.Alert

	updateCache := func() {
		for pos, dbncInfo := range ae.cache.alertsInDebounceInterval {
			alert := dbncInfo.alert
			dbncInfo.numTicks = dbncInfo.numTicks - 1
			if dbncInfo.numTicks <= 0 {
				ae.resolveAlertInCache(dbncInfo.alert, pos)
				alertsToResolve = append(alertsToResolve, alert)
			}
		}
	}

	updateCache()
	for _, alert := range alertsToResolve {
		ae.ResolveAlert(alert)
		ae.dispatchAlert(alert, AlertOpUpdate)
	}
}

func (ae *alertEngine) garbageCollect() {
	var alertsToDelete []*monitoring.Alert

	updateCache := func() error {
		for _, alert := range ae.cache.alertsInGCInterval {
			rtime, err := alert.Status.Resolved.Time.Time()
			if err != nil {
				ae.logger.Errorf("Failed to get resolved time for alert %v, err %v", alert, err)
				continue
			}
			if time.Since(rtime) > gcRetentionTime {
				ae.deleteAlertInCache(alert)
				alertsToDelete = append(alertsToDelete, alert)
			}
		}
		return nil
	}

	updateCache()
	for _, alert := range alertsToDelete {
		ae.dispatchAlert(alert, AlertOpDelete)
	}
}

func (ae *alertEngine) createAlert(obj runtime.Object, mp policyengine.MatchingPolicy) *monitoring.Alert {
	m, _ := runtime.GetObjectMeta(obj)

	pol := mp.Policy
	polID := fmt.Sprintf("%s/%s", pol.GetName(), pol.GetUUID())
	alertUUID := uuid.NewV4().String()
	alertName := alertUUID
	creationTime, _ := types.TimestampProto(time.Now())

	// create alert object
	alert := &monitoring.Alert{
		TypeMeta: api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{
			Name: alertName,
			UUID: alertUUID,
			//Tenant:       m.Tenant,
			//Namespace:    m.Namespace,
			Tenant:       globals.DefaultTenant,
			Namespace:    globals.DefaultNamespace,
			CreationTime: api.Timestamp{Timestamp: *creationTime},
			ModTime:      api.Timestamp{Timestamp: *creationTime},
		},
		Spec: monitoring.AlertSpec{
			State: monitoring.AlertState_OPEN.String(),
		},
		Status: monitoring.AlertStatus{
			Severity: pol.Spec.GetSeverity(),
			Message:  pol.Spec.GetMessage(),
			Reason: monitoring.AlertReason{
				PolicyID:            polID,
				MatchedRequirements: mp.Reqs,
			},
			//Source: &monitoring.AlertSource{Component: evt.GetSource().GetComponent(), NodeName:  evt.GetSource().GetNodeName(),},
			//EventURI: evt.GetSelfLink(),
			ObjectRef: &api.ObjectRef{
				//Tenant:    m.GetTenant(),
				//Namespace: m.GetNamespace(),
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Kind:      obj.GetObjectKind(),
				Name:      m.GetName(),
				URI:       m.GetSelfLink(),
			},
		},
	}

	alert.SelfLink = alert.MakeURI("configs", "v1", "monitoring")
	ae.logger.Infof("Created alert, %v", alert)
	return alert
}

func (ae *alertEngine) ResolveAlert(alert *monitoring.Alert) {
	alert.Spec.State = monitoring.AlertState_RESOLVED.String()
	now, _ := types.TimestampProto(time.Now())
	alert.Status.Resolved = &monitoring.AuditInfo{User: "System", Time: &api.Timestamp{Timestamp: *now}}
	alert.ModTime = api.Timestamp{Timestamp: *now}
	ae.logger.Infof("Resolved alert, %v", alert)
}

// TODO error-handling
func (ae *alertEngine) dispatchAlert(alert *monitoring.Alert, op AlertOp) {
	key := alert.Status.ObjectRef.String() + alert.Status.Reason.PolicyID
	kvOper := func(alert *monitoring.Alert, op AlertOp) {
		var err error
		switch op {
		case AlertOpCreate:
			_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				_, err := ae.apiClient.MonitoringV1().Alert().Create(ctx, alert)
				if err != nil {
					return false, err
				}
				return true, nil
			}, apiSrvWaitIntvl, maxAPISrvRetries)

		case AlertOpUpdate:
			_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				_, err := ae.apiClient.MonitoringV1().Alert().Update(ctx, alert)
				if err != nil {
					return false, err
				}
				return true, nil
			}, apiSrvWaitIntvl, maxAPISrvRetries)

		case AlertOpDelete:
			_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				_, err := ae.apiClient.MonitoringV1().Alert().Delete(ctx, alert.GetObjectMeta())
				if err != nil {
					return false, err
				}
				return true, nil
			}, apiSrvWaitIntvl, maxAPISrvRetries)
		}

		if err != nil {
			ae.logger.Errorf("KV operation %v error %v", op, err)
		}
	}

	if ae.doKVOpNowOrLater(key, kvOp{action: op, alert: alert}) == "now" {
		ae.wg.Add(1)
		go func() {
			defer ae.wg.Done()
			for {
				kvOper(alert, op)
				ae.logger.Infof("Dispatched alert, %v", alert)
				alert, op = ae.nextKVOp(key)
				if alert == nil {
					break
				}
			}
		}()
	}
}
