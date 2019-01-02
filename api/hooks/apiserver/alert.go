package impl

import (
	"context"
	"errors"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
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

// 1. update the alert status with user info (resolved by/acknowledged by) when the alert state changes.
// 2. update `total open alerts` or `total acknowledged alerts` count on the associated alert policy.
func (a *alertHooks) updateStatus(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
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

	var incrementAckAlerts int32
	var incrementOpenAlerts int32

	// update alert
	curAlertObj := &monitoring.Alert{}
	if err := kv.ConsistentUpdate(ctx, key, curAlertObj, func(oldObj runtime.Object) (runtime.Object, error) {
		alertObj, ok := oldObj.(*monitoring.Alert)
		if !ok {
			return oldObj, errInternalError
		}

		timeNow, _ := types.TimestampProto(time.Now())
		if alertObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)] {
			// current state: OPEN; possible states: ACKNOWLEDGED, RESOLVED
			switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
			case monitoring.AlertSpec_ACKNOWLEDGED:
				alertObj.Status.Acknowledged = &monitoring.AuditInfo{
					User: userSelfLink,
					Time: &api.Timestamp{Timestamp: *timeNow},
				}
				incrementAckAlerts++
			case monitoring.AlertSpec_RESOLVED:
				alertObj.Status.Resolved = &monitoring.AuditInfo{
					User: userSelfLink,
					Time: &api.Timestamp{
						Timestamp: *timeNow,
					},
				}
				incrementOpenAlerts--
			}
		} else if alertObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_ACKNOWLEDGED)] {
			// current state: ACKNOWLEDGED; possible states: OPEN, RESOLVED
			switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
			case monitoring.AlertSpec_RESOLVED:
				alertObj.Status.Resolved = &monitoring.AuditInfo{
					User: userSelfLink,
					Time: &api.Timestamp{
						Timestamp: *timeNow,
					},
				}
				incrementOpenAlerts--
				incrementAckAlerts--
			case monitoring.AlertSpec_OPEN:
				alertObj.Status.Resolved = nil
				alertObj.Status.Acknowledged = nil
				incrementAckAlerts--
			}
		} else if alertObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_RESOLVED)] {
			// current state: RESOLVED; possible states: OPEN, ACKNOWLEDGED
			switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
			case monitoring.AlertSpec_OPEN:
				alertObj.Status.Resolved = nil
				alertObj.Status.Acknowledged = nil
				incrementOpenAlerts++
			case monitoring.AlertSpec_ACKNOWLEDGED:
				alertObj.Status.Acknowledged = &monitoring.AuditInfo{
					User: userSelfLink,
					Time: &api.Timestamp{Timestamp: *timeNow},
				}
				incrementAckAlerts++
				incrementOpenAlerts++
			}
		}
		alertObj.Spec.State = alert.Spec.State
		return alertObj, nil
	}); err != nil {
		a.logger.Errorf("failed to update alert [%s]: %v", key, err)
		return nil, false, err
	}

	// update alert policy
	if incrementAckAlerts != 0 || incrementOpenAlerts != 0 {
		apKey := (&monitoring.AlertPolicy{
			ObjectMeta: api.ObjectMeta{Name: alert.Status.Reason.GetPolicyID(), Tenant: alert.GetTenant()},
		}).MakeKey("monitoring")

		curAlertPolicyObj := &monitoring.AlertPolicy{}
		if err := kv.ConsistentUpdate(ctx, apKey, curAlertPolicyObj, func(oldObj runtime.Object) (runtime.Object, error) {
			alertPolicyObj, ok := oldObj.(*monitoring.AlertPolicy)
			if !ok {
				return alertPolicyObj, errInternalError
			}

			alertPolicyObj.Status.AcknowledgedAlerts += incrementAckAlerts
			alertPolicyObj.Status.OpenAlerts += incrementOpenAlerts
			return alertPolicyObj, nil
		}); err != nil {
			a.logger.Errorf("failed to update alert policy [%s]: %v", apKey, err)
			return nil, false, err
		}
	}

	return *curAlertObj, false, nil
}

func registerAlertHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	ah := alertHooks{logger: l.WithContext("Service", "Alert")}
	svc.GetCrudService("Alert", apiserver.UpdateOper).WithPreCommitHook(ah.updateStatus)
}
