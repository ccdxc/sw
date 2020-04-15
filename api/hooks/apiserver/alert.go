package impl

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/metrics/genfields"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type alertHooks struct {
	logger log.Logger
}

// Pre-commit hook to update the resolved/acknowledge alert status when the alert state changes to RESOLVED/ACKNOWLEDGED/OPEN.

var (
	// errTransactionFailed is returned when a transaction is failed
	errTransactionFailed = errors.New("transaction execution error")
	// errInternalError is returned when an internal error is encountered (e.g. marshal/unmarshal, type casting issues)
	errInternalError = errors.New("internal error")
	// errInvalidMetricGroup is returned when no user is found in the context
	errNoUserInContext = errors.New("no user found in context")
	// errInvalidSyslogExportFormat is returned for invalid syslog export in the alert destination
	errInvalidSyslogExportFormat = errors.New("invalid syslog export format")
	// errNoSyslogTargetsFound is returned when no syslog targets were found in the alert destination
	errNoSyslogTargetsFound = errors.New("no targets configured")
	// errDuplicateTargetsFound is returned when there are duplicate targets in the alert destination
	errDuplicateTargetsFound = errors.New("found duplicate target in the policy")
	// errEmptyDestination is returned when the target carries an empty destination in the alert destination
	errEmptyDestination = errors.New("cannot configure empty destination")
	// errInvalidTransportFormat is returned when the transport format is invalid
	errInvalidTransportFormat = errors.New("transport should be in protocol/port format")
	// errInvalidProtocol is returned when invalid protocol is given in the alert destination
	errInvalidProtocol = errors.New("invalid protocol; Accepted protocol: UDP")
	// errInvalidPort is returned when invalid port is given in the alert destination
	errInvalidPort = errors.New("invalid port")
	// errInvalidMetricGroup is returned when incorrect metric group is given in the stats alert policy
	errInvalidMetricGroup = errors.New("invalid metric group")
	// errInvalidMetricKind is returned when incorrect metric kind is given in the stats alert policy
	errInvalidMetricKind = errors.New("invalid metric kind")
	// errInvalidMetricFieldName is returned when incorrect metric field-name is given in the stats alert policy
	errInvalidMetricFieldName = errors.New("invalid metric field-name")
	// errEmptyThresholdValues is returned when the threshold list on stats alert policy is empty
	errEmptyThresholdValues = errors.New("empty list of threshold values")
	// errInvalidMeasurementWindow is returned when invalid measurement-criteria window is given in the stats alert policy
	errInvalidMeasurementWindow = errors.New("invalid measurement-criteria window, allowed values: 5m, 10m, 30m, 1h")
	// errDuplicateSeveritiesInThreshold is returned when severities are repeated in the threshold values
	errDuplicateSeveritiesInThreshold = errors.New("duplicate severities found in the threshold values")
	// errInvalidThresholdValues is returned when threshold raise-values are invalid or not in order
	errInvalidThresholdValues = errors.New("invalid threshold raise-value or the values are not in order")
	// errInvalidAlertPolicyID is returned when incorrect alert policy id is found on the alert object
	errInvalidAlertPolicyID = errors.New("invalid alert policy ID")
)

var (
	allowedMeasurementWindowList = regexp.MustCompile(`^5m$|^10m$|^30m$|^1h$`)
)

type alertUpdateFlags struct {
	incrementAckAlerts  int32
	incrementOpenAlerts int32
	userSelfLink        string
	alert               monitoring.Alert
	alertPolicyUUID     string
}

func (a *alertHooks) getAlertUpdFunc(flags *alertUpdateFlags) kvstore.UpdateFunc {
	return func(oldObj runtime.Object) (runtime.Object, error) {
		// zero out to avoid inconsistent counters during consistent_update retries
		flags.incrementOpenAlerts = 0
		flags.incrementAckAlerts = 0

		alertObj, ok := oldObj.(*monitoring.Alert)
		if !ok {
			return oldObj, errInternalError
		}

		timeNow, _ := types.TimestampProto(time.Now())
		if alertObj.Spec.State == monitoring.AlertState_OPEN.String() {
			// current state: OPEN; possible states: ACKNOWLEDGED, RESOLVED
			switch monitoring.AlertState(monitoring.AlertState_vvalue[flags.alert.Spec.State]) {
			case monitoring.AlertState_ACKNOWLEDGED:
				alertObj.Status.Acknowledged = &monitoring.AuditInfo{
					User: flags.userSelfLink,
					Time: &api.Timestamp{Timestamp: *timeNow},
				}
				flags.incrementAckAlerts++
			case monitoring.AlertState_RESOLVED:
				alertObj.Status.Resolved = &monitoring.AuditInfo{
					User: flags.userSelfLink,
					Time: &api.Timestamp{
						Timestamp: *timeNow,
					},
				}
				flags.incrementOpenAlerts--
			}
		} else if alertObj.Spec.State == monitoring.AlertState_ACKNOWLEDGED.String() {
			// current state: ACKNOWLEDGED; possible states: OPEN, RESOLVED
			switch monitoring.AlertState(monitoring.AlertState_vvalue[flags.alert.Spec.State]) {
			case monitoring.AlertState_RESOLVED:
				alertObj.Status.Resolved = &monitoring.AuditInfo{
					User: flags.userSelfLink,
					Time: &api.Timestamp{
						Timestamp: *timeNow,
					},
				}
				flags.incrementOpenAlerts--
				flags.incrementAckAlerts--
			case monitoring.AlertState_OPEN:
				alertObj.Status.Resolved = nil
				alertObj.Status.Acknowledged = nil
				flags.incrementAckAlerts--
			}
		} else if alertObj.Spec.State == monitoring.AlertState_RESOLVED.String() {
			// current state: RESOLVED; possible states: OPEN, ACKNOWLEDGED
			switch monitoring.AlertState(monitoring.AlertState_vvalue[flags.alert.Spec.State]) {
			case monitoring.AlertState_OPEN:
				alertObj.Status.Resolved = nil
				alertObj.Status.Acknowledged = nil
				flags.incrementOpenAlerts++
			case monitoring.AlertState_ACKNOWLEDGED:
				alertObj.Status.Acknowledged = &monitoring.AuditInfo{
					User: flags.userSelfLink,
					Time: &api.Timestamp{Timestamp: *timeNow},
				}
				flags.incrementAckAlerts++
				flags.incrementOpenAlerts++
			}
		}
		alertObj.ObjectMeta.ModTime.Timestamp = *timeNow
		alertObj.Spec.State = flags.alert.Spec.State
		return alertObj, nil
	}
}

func (a *alertHooks) getAlertPolUpdFunc(flags *alertUpdateFlags) kvstore.UpdateFunc {
	return func(oldObj runtime.Object) (runtime.Object, error) {
		alertPolicyObj, ok := oldObj.(*monitoring.AlertPolicy)
		if !ok {
			return alertPolicyObj, errInternalError
		}

		if alertPolicyObj.GetUUID() != flags.alertPolicyUUID { // nothing to be updated
			a.logger.Infof("alert policy {%s} not the same as the one that created the alert, skipping update", alertPolicyObj.GetName())
			return oldObj, nil
		}

		if flags.incrementAckAlerts != 0 || flags.incrementOpenAlerts != 0 {
			alertPolicyObj.Status.AcknowledgedAlerts += flags.incrementAckAlerts
			alertPolicyObj.Status.OpenAlerts += flags.incrementOpenAlerts
			return alertPolicyObj, nil
		}
		return oldObj, nil
	}
}

// 1. update the alert status with user info (resolved by/acknowledged by) when the alert state changes.
// 2. update `total open alerts` or `total acknowledged alerts` count on the associated alert policy.
func (a *alertHooks) updateStatus(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	a.logger.DebugLog("msg", "updateStatus hook called")
	alert, ok := i.(monitoring.Alert)
	if !ok {
		return nil, false, errInvalidInputType
	}

	// As of today, system does not ack/resolve the alert. Those requests come only from the user. So, avoid
	// going through the below code if the request comes from API Server.
	if apiutils.IsUserRequestCtx(ctx) {
		// get user meta from the context
		userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
		if !ok || userMeta == nil {
			a.logger.Errorf("no user present in context passed to alert update operation in alert hook")
			return nil, false, errNoUserInContext
		}
		userSelfLink := (&auth.User{ObjectMeta: *userMeta}).MakeURI("configs", "v1", "auth")

		curAlertObj := &monitoring.Alert{}
		aKey := (&monitoring.Alert{ObjectMeta: *alert.GetObjectMeta()}).MakeKey("monitoring")
		if err := kv.Get(ctx, aKey, curAlertObj); err != nil {
			a.logger.Errorf("failed to get alert object, err: %v", err)
			return nil, false, errInvalidInputType
		}
		temp := strings.Split(curAlertObj.Status.Reason.GetPolicyID(), "/")
		if len(temp) != 2 {
			return nil, false, errInvalidAlertPolicyID
		}
		policyName, policyUUID := temp[0], temp[1]

		// these two variables are used in the closures passed to consistent update requirement.
		flags := alertUpdateFlags{
			alert:           alert,
			userSelfLink:    userSelfLink,
			alertPolicyUUID: policyUUID,
		}

		// update alert
		alertUpdFn := a.getAlertUpdFunc(&flags)
		reqs := []apiintf.ConstUpdateItem{
			{Key: key, Func: alertUpdFn, Into: curAlertObj},
		}

		apKey := (&monitoring.AlertPolicy{
			ObjectMeta: api.ObjectMeta{Name: policyName, Tenant: alert.GetTenant()},
		}).MakeKey("monitoring")
		if err := kv.Get(ctx, apKey, &monitoring.AlertPolicy{}); err == nil {
			curAlertPolicyObj := &monitoring.AlertPolicy{}
			polUpdateFn := a.getAlertPolUpdFunc(&flags)
			reqs = append(reqs, apiintf.ConstUpdateItem{Key: apKey, Func: polUpdateFn, Into: curAlertPolicyObj})
		}
		rq, err := apiutils.GetRequirements(ctx)
		if err != nil {
			return *curAlertObj, true, err
		}
		rq.(apiintf.RequirementSet).NewConsUpdateRequirement(reqs)

		return *curAlertObj, false, nil
	}

	return alert, true, nil
}

func validateAlertDestinationSpec(ad *monitoring.AlertDestinationSpec) error {

	if _, ok := monitoring.MonitoringExportFormat_vvalue[ad.SyslogExport.Format]; !ok {
		return errInvalidSyslogExportFormat
	}

	if len(ad.SyslogExport.Targets) == 0 {
		return errNoSyslogTargetsFound
	}

	adTargets := map[string]bool{}
	for _, c := range ad.SyslogExport.Targets {
		if key, err := json.Marshal(c); err == nil {
			ks := string(key)
			if _, ok := adTargets[ks]; ok {
				return errDuplicateTargetsFound
			}
			adTargets[ks] = true

		}

		if c.Destination == "" {
			return errEmptyDestination
		}

		adNetIP, _, err := net.ParseCIDR(c.Destination)
		if err != nil {
			adNetIP = net.ParseIP(c.Destination)
		}

		if adNetIP == nil {
			// treat it as hostname and resolve
			if _, err := net.LookupHost(c.Destination); err != nil {
				return fmt.Errorf("failed to resolve name %s, error: %v", c.Destination, err)
			}
		}

		tr := strings.Split(c.Transport, "/")
		if len(tr) != 2 {
			return errInvalidTransportFormat
		}

		if _, ok := map[string]bool{
			"tcp": true,
			"udp": true,
		}[strings.ToLower(tr[0])]; !ok {
			return errInvalidProtocol
		}

		adPort, err := strconv.Atoi(tr[1])
		if err != nil {
			return errInvalidPort
		}

		if uint(adPort) > uint(^uint16(0)) {
			return fmt.Errorf("invalid port %v (> %d)", adPort, ^uint16(0))
		}
	}
	return nil
}

func (a *alertHooks) validateAlertDestination(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	a.logger.DebugLog("msg", "validateAlertDestination hook called")
	alertdest, ok := i.(monitoring.AlertDestination)
	if !ok {
		return i, false, errInvalidInputType
	}
	if err := validateAlertDestinationSpec(&alertdest.Spec); err != nil {
		return i, false, err
	}
	return i, true, nil
}

func (a *alertHooks) validateStatsAlertPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	a.logger.DebugLog("msg", "validateStatsAlertPolicy hook called")
	statsAlertPolicy, ok := i.(monitoring.StatsAlertPolicy)
	if !ok {
		return i, false, errInvalidInputType
	}

	// validate group->kind->field-name mappings
	metric := statsAlertPolicy.Spec.Metric
	if !utils.IsEmpty(metric.Group) {
		if !genfields.IsGroupValid(metric.Group) {
			return i, false, errInvalidMetricGroup
		}
		if !genfields.IsKindValid(metric.Group, metric.Kind) {
			return i, false, errInvalidMetricKind
		}
		if !genfields.IsFieldNameValid(metric.Group, metric.Kind, metric.FieldName) {
			return i, false, errInvalidMetricFieldName
		}
	}

	// validate measurement window
	if statsAlertPolicy.Spec.MeasurementCriteria != nil && !utils.IsEmpty(statsAlertPolicy.Spec.MeasurementCriteria.Window) {
		mWindow := statsAlertPolicy.Spec.MeasurementCriteria.Window
		if _, err := time.ParseDuration(mWindow); err != nil {
			return i, false, errInvalidMeasurementWindow
		}
		if !allowedMeasurementWindowList.MatchString(mWindow) {
			return i, false, errInvalidMeasurementWindow
		}
	}

	// validate threshold values; cannot be empty
	if len(statsAlertPolicy.Spec.Thresholds.Values) == 0 {
		return i, false, errEmptyThresholdValues
	}

	// validate threshold values; severities cannot be repeated
	severities := map[string]struct{}{}
	for _, thVal := range statsAlertPolicy.Spec.Thresholds.Values {
		if _, found := severities[thVal.Severity]; found { // duplicate severity found
			return i, false, errDuplicateSeveritiesInThreshold
		}
		severities[thVal.Severity] = struct{}{}
	}

	// Sort threshold values in the order of increasing severity
	sort.SliceStable(statsAlertPolicy.Spec.Thresholds.Values, func(i, j int) bool {
		val1 := monitoring.AlertSeverity(monitoring.AlertSeverity_vvalue[statsAlertPolicy.Spec.Thresholds.Values[i].Severity])
		val2 := monitoring.AlertSeverity(monitoring.AlertSeverity_vvalue[statsAlertPolicy.Spec.Thresholds.Values[j].Severity])
		return val1 < val2
	})

	// validate threshold values of different severity based on the operator
	switch statsAlertPolicy.Spec.Thresholds.Operator {
	case monitoring.Operator_GREATER_OR_EQUAL_THAN.String():
		// threshold raise-values should be in increasing order from lowest to highest severity
		if sorted := sort.SliceIsSorted(statsAlertPolicy.Spec.Thresholds.Values, func(i, j int) bool {
			val1, err := strconv.ParseFloat(statsAlertPolicy.Spec.Thresholds.Values[i].RaiseValue, 64)
			if err != nil {
				return false
			}
			val2, err := strconv.ParseFloat(statsAlertPolicy.Spec.Thresholds.Values[j].RaiseValue, 64)
			if err != nil {
				return false
			}
			return val1 < val2
		}); !sorted {
			return i, false, errInvalidThresholdValues
		}
	case monitoring.Operator_LESS_OR_EQUAL_THAN.String():
		// threshold raise-values should be in decreasing order from lowest to highest severity
		if sorted := sort.SliceIsSorted(statsAlertPolicy.Spec.Thresholds.Values, func(i, j int) bool {
			val1, err := strconv.ParseFloat(statsAlertPolicy.Spec.Thresholds.Values[i].RaiseValue, 64)
			if err != nil {
				return false
			}
			val2, err := strconv.ParseFloat(statsAlertPolicy.Spec.Thresholds.Values[j].RaiseValue, 64)
			if err != nil {
				return false
			}
			return val1 > val2
		}); !sorted {
			return i, false, errInvalidThresholdValues
		}
	}

	return i, true, nil
}

func registerAlertHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	ah := alertHooks{logger: l.WithContext("Service", "Alert")}
	adh := alertHooks{logger: l.WithContext("Service", "AlertDestination")}
	sap := alertHooks{logger: l.WithContext("Service", "StatsAlertPolicy")}
	svc.GetCrudService("Alert", apiintf.UpdateOper).WithPreCommitHook(ah.updateStatus)
	svc.GetCrudService("AlertDestination", apiintf.CreateOper).WithPreCommitHook(adh.validateAlertDestination)
	svc.GetCrudService("AlertDestination", apiintf.UpdateOper).WithPreCommitHook(adh.validateAlertDestination)
	svc.GetCrudService("StatsAlertPolicy", apiintf.CreateOper).WithPreCommitHook(sap.validateStatsAlertPolicy)
	svc.GetCrudService("StatsAlertPolicy", apiintf.UpdateOper).WithPreCommitHook(sap.validateStatsAlertPolicy)
}
