package impl

import (
	"context"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pkg/errors"

	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type rolloutHooks struct {
	l   log.Logger
	svc apiserver.Service
}

func suspendRolloutActionObj(ctx context.Context, kv kvstore.Interface, key string, cur *rollout.Rollout) error {

	err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		rObj, ok := oldObj.(*rollout.Rollout)
		if !ok {
			return oldObj, errors.New("invalid input type")
		}
		rObj.Spec.Suspend = true
		rObj.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
		return rObj, nil
	})
	return err
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
		return err
	}
	rolloutActionObj.CreationTime, rolloutActionObj.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_PROGRESSING)]
	return nil
}
func (h *rolloutHooks) doRolloutAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook for RolloutAction  key %s", key)
	var err error

	buf, ok := i.(rollout.Rollout)
	if !ok {
		h.l.ErrorLog("Invalid object in pre commit hook for rolloutAction")
		return nil, false, errors.New("invalid object")
	}
	rolloutObj := rollout.Rollout{}
	rolloutObjKey := buf.MakeKey(string(apiclient.GroupRollout))
	if kv != nil {
		err = kv.Get(ctx, rolloutObjKey, &rolloutObj)
		// You can suspend a rollout Operation once its scheduled or started
		if err == nil && buf.Spec.Suspend != true {
			h.l.InfoLog("msg", "rollout Object %v already exists", buf.Name)
			return buf, false, err
		}
	}

	rolloutActionObj := &rollout.RolloutAction{}
	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))

	if kv != nil {
		err = kv.Get(ctx, rolloutActionObjKey, rolloutActionObj)
	}
	h.l.InfoLog("msg", "GET rolloutAction %v rolloutActionStatus %v", rolloutActionObj, rolloutActionObj.Status)
	if err != nil {
		//create rolloutActionObj
		h.l.InfoLog("msg", "Creating RolloutAction")
		err := createRolloutActionObj(buf, rolloutActionObj)
		if err != nil {
			return i, false, err
		}
		err = txn.Create(rolloutActionObjKey, rolloutActionObj)
		if err != nil {
			return buf, false, errors.New("RolloutAction create operation to transaction failed")
		}

	} else {
		//update rolloutAction
		h.l.InfoLog("msg", "Updating RolloutAction")
		opState := rolloutActionObj.Status.GetOperationalState()
		cur := &rollout.Rollout{}
		//verify suspend rollout
		if buf.Name == rolloutActionObj.Name &&
			buf.Spec.Suspend == true {
			h.l.InfoLog("msg", "Request to suspend Rollout Operation %s", buf.Name)
			if err := suspendRolloutActionObj(ctx, kv, key, cur); err != nil {
				h.l.Errorf("Error setting suspend flag: %v", err)
				return nil, false, err
			}
			h.l.Infof("Rollout suspend flag is set and locked down")
			rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPENDED)]
			if txn != nil {
				err = txn.Update(rolloutActionObjKey, rolloutActionObj)
				if err != nil {
					return buf, false, errors.New("RolloutAction update operation to transaction failed")
				}
			}
			return buf, false, nil
		}

		if opState == rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_PROGRESSING)] ||
			opState == rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SCHEDULED)] ||
			opState == rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_SUSPEND_IN_PROGRESS)] {
			h.l.InfoLog("msg", "Rollout in progress %#v", rolloutActionObj.Status)
			return buf, false, errors.New("Rollout in progress")
		}
		if txn != nil {
			txn.AddComparator(kvstore.Compare(kvstore.WithVersion(rolloutActionObjKey), "=", rolloutActionObj.ResourceVersion))
		}
		rolloutActionObj.Name = buf.Name
		ts, err := types.TimestampProto(time.Now())
		if err != nil {
			return i, false, err
		}
		rolloutActionObj.ModTime = api.Timestamp{Timestamp: *ts}
		rolloutActionObj.Status.OperationalState = rollout.RolloutStatus_RolloutOperationalState_name[int32(rollout.RolloutStatus_PROGRESSING)]
		if txn != nil {
			err = txn.Update(rolloutActionObjKey, rolloutActionObj)
			if err != nil {
				return buf, false, errors.New("RolloutAction update operation to transaction failed")
			}
		}
	}
	h.l.InfoLog("msg", "RolloutAction Prehook Completed %v", rolloutActionObj)
	return buf, true, nil
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
	svc.GetMethod("DoRollout").WithPreCommitHook(h.doRolloutAction)
	svc.GetMethod("DoRollout").WithResponseWriter(h.getRolloutObject)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("rollout.RolloutV1", registerRolloutHooks)
}
