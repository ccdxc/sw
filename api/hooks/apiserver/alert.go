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
)

type alertHooks struct {
	logger log.Logger
}

// Pre-commit hook to update the resolved/acknowledge alert status when the alert state changes to RESOLVED/ACKNOWLEDGED/OPEN.

// 1. update the alert status with user info (resolved by/acknowledged by) when the alert state changes.
// 2. update `total open alerts` or `total acknowledged alerts` count on the associated alert policy.
func (a *alertHooks) updateStatus(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	a.logger.DebugLog("msg", "updateStatus hook called")
	alert, ok := i.(monitoring.Alert)
	if !ok {
		return i, true, errors.New("invalid input type")
	}

	timeNow, _ := types.TimestampProto(time.Now())
	alert.ObjectMeta.ModTime = api.Timestamp{Timestamp: *timeNow}

	//get user meta from the context
	userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
	if !ok || userMeta == nil {
		a.logger.Errorf("no user present in context passed to alert update operation in alert hook")
		return i, true, errors.New("no user data found in the context")
	}
	userSelfLink := (&auth.User{ObjectMeta: *userMeta}).MakeURI("configs", "v1", "auth")

	// get the existing alert object from KV store
	existingObj := &monitoring.Alert{}
	if err := kv.Get(ctx, key, existingObj); err != nil {
		a.logger.Errorf("error getting alert object with key [%s] in alert update pre-commit hook", key)
		return i, true, err
	}

	// get the associated alert policy from KV store
	apKey := (&monitoring.AlertPolicy{
		ObjectMeta: api.ObjectMeta{Name: alert.Status.Reason.GetPolicyID(), Tenant: alert.GetTenant()},
	}).MakeKey("monitoring")
	alertPolicy := &monitoring.AlertPolicy{}
	if err := kv.Get(ctx, apKey, alertPolicy); err != nil {
		a.logger.Errorf("error getting alert policy object with key [%s] in alert update pre-commit hook", apKey)
		return i, true, err
	}

	// no state changes; nothing to be done
	if existingObj.Spec.State == alert.Spec.State {
		return i, true, nil
	}

	if existingObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)] {
		// current state: OPEN; possible states: ACKNOWLEDGED, RESOLVED
		switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
		case monitoring.AlertSpec_ACKNOWLEDGED:
			alert.Status.Acknowledged = &monitoring.AuditInfo{
				User: userSelfLink,
				Time: &api.Timestamp{Timestamp: *timeNow},
			}
			alertPolicy.Status.AcknowledgedAlerts++
		case monitoring.AlertSpec_RESOLVED:
			alert.Status.Resolved = &monitoring.AuditInfo{
				User: userSelfLink,
				Time: &api.Timestamp{
					Timestamp: *timeNow,
				},
			}
			alertPolicy.Status.OpenAlerts--
		}
	} else if existingObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_ACKNOWLEDGED)] {
		// current state: ACKNOWLEDGED; possible states: OPEN, RESOLVED
		switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
		case monitoring.AlertSpec_RESOLVED:
			alert.Status.Resolved = &monitoring.AuditInfo{
				User: userSelfLink,
				Time: &api.Timestamp{
					Timestamp: *timeNow,
				},
			}
			alertPolicy.Status.OpenAlerts--
			alertPolicy.Status.AcknowledgedAlerts--
		case monitoring.AlertSpec_OPEN:
			alert.Status.Resolved = nil
			alert.Status.Acknowledged = nil
			alertPolicy.Status.AcknowledgedAlerts--
		}
	} else if existingObj.Spec.State == monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_RESOLVED)] {
		// current state: RESOLVED; possible states: OPEN, ACKNOWLEDGED
		switch monitoring.AlertSpec_AlertState(monitoring.AlertSpec_AlertState_value[alert.Spec.State]) {
		case monitoring.AlertSpec_OPEN:
			alert.Status.Resolved = nil
			alert.Status.Acknowledged = nil
			alertPolicy.Status.OpenAlerts++
		case monitoring.AlertSpec_ACKNOWLEDGED:
			alert.Status.Acknowledged = &monitoring.AuditInfo{
				User: userSelfLink,
				Time: &api.Timestamp{Timestamp: *timeNow},
			}
			alertPolicy.Status.AcknowledgedAlerts++
			alertPolicy.Status.OpenAlerts++
		}
	}

	if err := txn.Update(key, &alert, kvstore.Compare(kvstore.WithVersion(key), "=", alert.ResourceVersion)); err != nil {
		a.logger.Errorf("failed to update alert [%s] in a txn, err: %v", key, err)
		return i, true, err
	}

	if err := txn.Update(apKey, alertPolicy, kvstore.Compare(kvstore.WithVersion(apKey), "=", alertPolicy.ResourceVersion)); err != nil {
		a.logger.Errorf("failed to update alert policy [%s] in a txn, err: %v", apKey, err)
		return i, true, err
	}

	if _, err := txn.Commit(ctx); err != nil {
		a.logger.Errorf("failed to commit a txn with multiple updates, err: %v", err)
		return i, true, err
	}

	return alert, false, nil
}

func registerAlertHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	ah := alertHooks{logger: l.WithContext("Service", "Alert")}
	svc.GetCrudService("Alert", apiserver.UpdateOper).WithPreCommitHook(ah.updateStatus)
}
