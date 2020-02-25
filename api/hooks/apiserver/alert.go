package impl

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/apiserver"
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
	errTransactionFailed = errors.New("transaction execution error")
	errInternalError     = errors.New("internal error")
	errNoUserInContext   = errors.New("no user found in context")
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

		temp := strings.Split(alert.Status.Reason.GetPolicyID(), "/")
		if len(temp) != 2 {
			return nil, false, errInvalidInputType
		}
		policyName, policyUUID := temp[0], temp[1]

		// these two variables are used in the closures passed to consistent update requirement.
		flags := alertUpdateFlags{
			alert:           alert,
			userSelfLink:    userSelfLink,
			alertPolicyUUID: policyUUID,
		}

		// update alert
		curAlertObj := &monitoring.Alert{}
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
		return fmt.Errorf("invalid format %v", ad.SyslogExport.Format)
	}

	if len(ad.SyslogExport.Targets) == 0 {
		return fmt.Errorf("no targets configured")
	}

	adTargets := map[string]bool{}
	for _, c := range ad.SyslogExport.Targets {
		if key, err := json.Marshal(c); err == nil {
			ks := string(key)
			if _, ok := adTargets[ks]; ok {
				return fmt.Errorf("found duplicate target %v %v", c.Destination, c.Transport)
			}
			adTargets[ks] = true

		}

		if c.Destination == "" {
			return fmt.Errorf("cannot configure empty destination")
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
			return fmt.Errorf("transport should be in protocol/port format")
		}

		if _, ok := map[string]bool{
			"tcp": true,
			"udp": true,
		}[strings.ToLower(tr[0])]; !ok {
			return fmt.Errorf("invalid protocol %v\n Accepted protocols: TCP, UDP", tr[0])
		}

		adPort, err := strconv.Atoi(tr[1])
		if err != nil {
			return fmt.Errorf("invalid port %v", tr[1])
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
		return i, false, fmt.Errorf("invalid object %T instead of AlertDestination", i)
	}
	if err := validateAlertDestinationSpec(&alertdest.Spec); err != nil {
		return i, false, err
	}
	return i, true, nil
}

func registerAlertHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	ah := alertHooks{logger: l.WithContext("Service", "Alert")}
	adh := alertHooks{logger: l.WithContext("Service", "AlertDestination")}
	svc.GetCrudService("Alert", apiintf.UpdateOper).WithPreCommitHook(ah.updateStatus)
	svc.GetCrudService("AlertDestination", apiintf.CreateOper).WithPreCommitHook(adh.validateAlertDestination)
	svc.GetCrudService("AlertDestination", apiintf.UpdateOper).WithPreCommitHook(adh.validateAlertDestination)
}
