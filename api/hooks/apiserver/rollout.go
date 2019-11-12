package impl

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type rolloutHooks struct {
	l   log.Logger
	svc apiserver.Service
}

const (
	createRolloutOp = 1
	modifyRolloutOp = 2
	stopRolloutOp   = 3
)

func updateRolloutObj(ctx context.Context, kv kvstore.Interface, key string, cur *rollout.Rollout, rolloutOp int) error {

	err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		rObj, ok := oldObj.(*rollout.Rollout)
		if !ok {
			return oldObj, errors.New("invalid input type")
		}
		rObj.Spec.Suspend = cur.Spec.Suspend
		if rolloutOp == stopRolloutOp {
			rObj.Spec.Suspend = true
		}
		rObj.Spec.Version = cur.Spec.Version
		rObj.Spec.ScheduledStartTime = cur.Spec.ScheduledStartTime
		rObj.Spec.ScheduledEndTime = cur.Spec.ScheduledEndTime
		rObj.Spec.MaxNICFailuresBeforeAbort = cur.Spec.MaxNICFailuresBeforeAbort
		rObj.Spec.MaxParallel = cur.Spec.MaxParallel
		rObj.Spec.OrderConstraints = cur.Spec.OrderConstraints
		rObj.Spec.DSCMustMatchConstraint = cur.Spec.DSCMustMatchConstraint
		rObj.Spec.DSCsOnly = cur.Spec.DSCsOnly
		rObj.Spec.Strategy = cur.Spec.Strategy
		rObj.Spec.UpgradeType = cur.Spec.UpgradeType
		if rolloutOp == stopRolloutOp || cur.Spec.Suspend {
			rObj.Status.OperationalState = rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String()
		}
		return rObj, nil
	})
	return err
}

func updateRolloutActionObj(rolloutActionObj *rollout.RolloutAction, buf *rollout.Rollout, txn kvstore.Txn, rolloutOp int) (bool, error) {

	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))
	rolloutActionObj.Name = buf.Name
	rolloutActionObj.Spec.Suspend = buf.Spec.Suspend
	if rolloutOp == stopRolloutOp {
		rolloutActionObj.Spec.Suspend = true
	}
	rolloutActionObj.Spec.Version = buf.Spec.Version
	rolloutActionObj.Spec.ScheduledStartTime = buf.Spec.ScheduledStartTime
	rolloutActionObj.Spec.ScheduledEndTime = buf.Spec.ScheduledEndTime
	rolloutActionObj.Spec.MaxNICFailuresBeforeAbort = buf.Spec.MaxNICFailuresBeforeAbort
	rolloutActionObj.Spec.MaxParallel = buf.Spec.MaxParallel
	rolloutActionObj.Spec.OrderConstraints = buf.Spec.OrderConstraints
	rolloutActionObj.Spec.DSCMustMatchConstraint = buf.Spec.DSCMustMatchConstraint
	rolloutActionObj.Spec.DSCsOnly = buf.Spec.DSCsOnly
	rolloutActionObj.Spec.Strategy = buf.Spec.Strategy
	rolloutActionObj.Spec.UpgradeType = buf.Spec.UpgradeType
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return false, errors.New("RolloutAction update operation failed to get timestamp")
	}
	rolloutActionObj.ModTime = api.Timestamp{Timestamp: *ts}
	if buf.Spec.Suspend || rolloutOp == stopRolloutOp {
		rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String()
	}
	if rolloutOp == createRolloutOp {
		rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String()
	}
	err = txn.Update(rolloutActionObjKey, rolloutActionObj)
	if err != nil {
		return false, errors.New("RolloutAction update operation to transaction failed")
	}

	return true, nil
}

func createRolloutActionObj(buf rollout.Rollout, rolloutActionObj *rollout.RolloutAction) error {
	apiSrv := apisrvpkg.MustGetAPIServer()
	rolloutActionObj.Defaults("all")
	rolloutActionObj.APIVersion = apiSrv.GetVersion()
	rolloutActionObj.Name = buf.Name
	rolloutActionObj.Spec = buf.Spec
	rolloutActionObj.SelfLink = rolloutActionObj.MakeURI("configs", buf.APIVersion, "RolloutAction")
	rolloutActionObj.Tenant = buf.Tenant
	rolloutActionObj.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return errors.New("RolloutAction create operation failed to get timestamp")
	}
	rolloutActionObj.CreationTime, rolloutActionObj.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String()
	return nil
}

func checkRolloutInProgress(rolloutActionObj rollout.RolloutAction) bool {

	opState := rolloutActionObj.Status.GetOperationalState()
	if opState == rollout.RolloutStatus_PROGRESSING.String() ||
		opState == rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String() ||
		opState == rollout.RolloutStatus_SCHEDULED.String() ||
		opState == rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String() ||
		opState == rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String() {
		return true
	}
	return false
}

func (h *rolloutHooks) deleteRolloutAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook for deleteRolloutAction  key %s", key)
	var err error

	buf, ok := i.(rollout.Rollout)
	if !ok {
		h.l.Error("Invalid object for rolloutAction")
		return nil, false, errors.New("invalid object")
	}

	if kv == nil || txn == nil {
		h.l.Error("Invalid kvstore and txn objects for rolloutAction")
		return nil, false, errors.New("invalid kvstore and txn objects")
	}

	rolloutObj := rollout.Rollout{}
	rolloutObjKey := buf.MakeKey(string(apiclient.GroupRollout))

	rolloutActionObj := &rollout.RolloutAction{}
	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))

	if err = kv.Get(ctx, rolloutActionObjKey, rolloutActionObj); err != nil {
		return buf, false, errors.New("RolloutAction object not present")
	}
	if rolloutActionObj.Name == buf.Name && checkRolloutInProgress(*rolloutActionObj) {
		return buf, false, errors.New("Rollout is in progress. Cannot delete")
	}

	if err = kv.Get(ctx, rolloutObjKey, &rolloutObj); err != nil {
		return buf, false, errors.New("RolloutObject not present")
	}

	err = txn.Delete(rolloutObjKey)
	if err != nil {
		h.l.InfoLog("msg", "Delete Rollout Failed err(%+v) %v", err, rolloutObjKey)
		return nil, false, err
	}
	h.l.InfoLog("msg", "Delete Rollout Completed %v", rolloutActionObj)
	return buf, false, nil
}

func performRolloutUpdate(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, i interface{}, h *rolloutHooks, rolloutOp int) (interface{}, bool, error) {
	h.l.InfoLog("msg", "performRolloutUpdate  key", key)
	var err error

	buf, ok := i.(rollout.Rollout)
	if !ok {
		h.l.Error("Invalid object for rolloutAction")
		return nil, false, errors.New("invalid object")
	}

	if kv == nil || txn == nil {
		h.l.Error("Invalid kvstore and txn objects for rolloutAction")
		return nil, false, errors.New("invalid kvstore and txn objects")
	}

	if buf.Spec.Version == "" {
		h.l.ErrorLog("Version field is empty in rollout object")
		return nil, false, errors.New("missing version field in rollout object")
	}

	rolloutObj := rollout.Rollout{}
	rolloutObjKey := buf.MakeKey(string(apiclient.GroupRollout))

	rolloutActionObj := &rollout.RolloutAction{}
	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))

	if err = kv.Get(ctx, rolloutActionObjKey, rolloutActionObj); err != nil {
		return buf, false, errors.New("RolloutAction object not present")
	}
	if err = kv.Get(ctx, rolloutObjKey, &rolloutObj); err != nil {
		return buf, false, errors.New("RolloutObject not present")
	}

	h.l.InfoLog("msg", "Updating RolloutAction & Updating Rollout")

	//Update only if it's latest/in-progress
	if buf.Name == rolloutActionObj.Name && checkRolloutInProgress(*rolloutActionObj) {
		if _, err := updateRolloutActionObj(rolloutActionObj, &buf, txn, rolloutOp); err != nil {
			h.l.InfoLog("msg", "Update RolloutActionObject Failed %s", err)
			return buf, false, err
		}
		if err := updateRolloutObj(ctx, kv, key, &buf, rolloutOp); err != nil {
			h.l.Errorf("Error updating rollout: %v", err)
			return nil, false, err
		}
		return buf, false, nil
	}
	h.l.InfoLog("msg", "Specified rollout is not in-progress %v", rolloutActionObj)
	return buf, false, errors.New("Specified rollout is not in-progress")
}

func (h *rolloutHooks) modifyRolloutAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook for UpdateRolloutAction  key %s", key)
	return performRolloutUpdate(ctx, kv, txn, key, i, h, modifyRolloutOp)
}

func (h *rolloutHooks) stopRolloutAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook for StopRolloutAction  key %s", key)
	return performRolloutUpdate(ctx, kv, txn, key, i, h, stopRolloutOp)
}

func (h *rolloutHooks) doRolloutAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook for RolloutAction  key %s", key)
	var err error

	buf, ok := i.(rollout.Rollout)
	if !ok {
		h.l.ErrorLog("Invalid object in pre commit hook for rolloutAction")
		return nil, false, errors.New("invalid object")
	}

	if kv == nil || txn == nil {
		h.l.ErrorLog("Invalid kvstore and txn objects for rolloutAction")
		return nil, false, errors.New("invalid kvstore and txn objects")
	}

	if buf.Spec.Version == "" {
		h.l.ErrorLog("Version field is empty in rollout object")
		return nil, false, errors.New("missing version field in rollout object")
	}

	rolloutObj := rollout.Rollout{}
	rolloutObjKey := buf.MakeKey(string(apiclient.GroupRollout))

	rolloutActionObj := &rollout.RolloutAction{}
	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))

	if err = kv.Get(ctx, rolloutActionObjKey, rolloutActionObj); err != nil {
		//create rolloutActionObj
		h.l.InfoLog("msg", "Creating RolloutAction & Creating Rollout")
		err := createRolloutActionObj(buf, rolloutActionObj)
		if err != nil {
			return buf, false, errors.New("RolloutAction create failed")
		}
		err = txn.Create(rolloutActionObjKey, rolloutActionObj)
		if err != nil {
			return buf, false, errors.New("RolloutAction create operation to transaction failed")
		}
		return buf, true, nil
	}

	txn.AddComparator(kvstore.Compare(kvstore.WithVersion(rolloutActionObjKey), "=", rolloutActionObj.ResourceVersion))

	if err = kv.Get(ctx, rolloutObjKey, &rolloutObj); err != nil {
		//update rolloutAction && create rollout
		h.l.InfoLog("msg", "Updating RolloutAction & Create Rollout")
		if inProgress := checkRolloutInProgress(*rolloutActionObj); inProgress {
			errmsg := fmt.Sprintf("Rollout %v in progress", rolloutActionObj.Name)
			h.l.InfoLog("msg", errmsg)
			return buf, false, errors.New(errmsg)
		}

		if _, err := updateRolloutActionObj(rolloutActionObj, &buf, txn, createRolloutOp); err != nil {
			h.l.InfoLog("msg", "Update RolloutAction Failed %s", err)
			return buf, false, err
		}
		return buf, true, nil
	}
	h.l.InfoLog("msg", "RolloutAction Prehook Completed %v", rolloutActionObj)
	return buf, false, nil
}

func (h *rolloutHooks) getRolloutObject(ctx context.Context, kv kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	h.l.InfoLog("msg", "received getRolloutObject")
	var err error
	buf, ok := in.(rollout.Rollout)
	if !ok {
		h.l.ErrorLog("Invalid object in getRolloutObject")
		return nil, errors.New("Invalid object in getRolloutObject")
	}

	rolloutObj := rollout.Rollout{}
	rolloutObjKey := buf.MakeKey(string(apiclient.GroupRollout))
	if kv != nil {
		err = kv.Get(ctx, rolloutObjKey, &rolloutObj)
		if err != nil {
			h.l.InfoLog("msg", "rollout Object %v is not in kvstore", buf.Name)
			return nil, err
		}
	}
	return rolloutObj, nil
}

func registerRolloutHooks(svc apiserver.Service, logger log.Logger) {
	h := rolloutHooks{}
	h.svc = svc
	h.l = logger
	logger.InfoLog("Service", "RolloutV1", "msg", "registering rolloutAction hook")
	svc.GetMethod("CreateRollout").WithPreCommitHook(h.doRolloutAction)
	svc.GetMethod("StopRollout").WithPreCommitHook(h.stopRolloutAction)
	svc.GetMethod("UpdateRollout").WithPreCommitHook(h.modifyRolloutAction)
	svc.GetMethod("RemoveRollout").WithPreCommitHook(h.deleteRolloutAction)

	svc.GetMethod("CreateRollout").WithResponseWriter(h.getRolloutObject)
	svc.GetMethod("UpdateRollout").WithResponseWriter(h.getRolloutObject)
	svc.GetMethod("StopRollout").WithResponseWriter(h.getRolloutObject)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("rollout.RolloutV1", registerRolloutHooks)
}
