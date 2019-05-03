package impl

import (
	"context"
	"errors"
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
}

func (a *alertHooks) getAlertUpdFunc(flags *alertUpdateFlags) kvstore.UpdateFunc {
	return func(oldObj runtime.Object) (runtime.Object, error) {
		alertObj, ok := oldObj.(*monitoring.Alert)
		if !ok {
			return oldObj, errInternalError
		}

		timeNow, _ := types.TimestampProto(time.Now())
		if alertObj.Spec.State == monitoring.AlertState_OPEN.String() {
			// current state: OPEN; possible states: ACKNOWLEDGED, RESOLVED
			switch monitoring.AlertState(monitoring.AlertState_value[flags.alert.Spec.State]) {
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
			switch monitoring.AlertState(monitoring.AlertState_value[flags.alert.Spec.State]) {
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
			switch monitoring.AlertState(monitoring.AlertState_value[flags.alert.Spec.State]) {
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
		alertObj.Spec.State = flags.alert.Spec.State
		return alertObj, nil
	}
}

func (a *alertHooks) getAlertPolUpdFunc(flags *alertUpdateFlags) kvstore.UpdateFunc {
	return func(oldObj runtime.Object) (runtime.Object, error) {
		if flags.incrementAckAlerts != 0 || flags.incrementOpenAlerts != 0 {
			alertPolicyObj, ok := oldObj.(*monitoring.AlertPolicy)
			if !ok {
				return alertPolicyObj, errInternalError
			}

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

	// get user meta from the context
	userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
	if !ok || userMeta == nil {
		a.logger.Errorf("no user present in context passed to alert update operation in alert hook")
		return nil, false, errNoUserInContext
	}
	userSelfLink := (&auth.User{ObjectMeta: *userMeta}).MakeURI("configs", "v1", "auth")

	// these two variables are used in the closures passed to consistent update requirement.
	flags := alertUpdateFlags{
		alert:        alert,
		userSelfLink: userSelfLink,
	}

	// update alert
	curAlertObj := &monitoring.Alert{}
	alertUpdFn := a.getAlertUpdFunc(&flags)

	apKey := (&monitoring.AlertPolicy{
		ObjectMeta: api.ObjectMeta{Name: alert.Status.Reason.GetPolicyID(), Tenant: alert.GetTenant()},
	}).MakeKey("monitoring")
	curAlertPolicyObj := &monitoring.AlertPolicy{}

	polUpdateFn := a.getAlertPolUpdFunc(&flags)
	reqs := []apiintf.ConstUpdateItem{
		{Key: key, Func: alertUpdFn, Into: curAlertObj},
		{Key: apKey, Func: polUpdateFn, Into: curAlertPolicyObj},
	}
	rq, err := apiutils.GetRequirements(ctx)
	if err != nil {
		return *curAlertObj, true, err
	}
	rq.(apiintf.RequirementSet).NewConsUpdateRequirement(reqs)

	return *curAlertObj, false, nil
}

func registerAlertHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	ah := alertHooks{logger: l.WithContext("Service", "Alert")}
	svc.GetCrudService("Alert", apiintf.UpdateOper).WithPreCommitHook(ah.updateStatus)
}
