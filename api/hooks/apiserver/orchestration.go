package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/orchestration"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// ErrOrchManaged is the error message returned when a user tries to modify an object created by Orchhub
var ErrOrchManaged = fmt.Errorf("object is managed by Venice and can only be deleted by Admin")

type orchHooks struct {
	logger log.Logger
}

func (o *orchHooks) validateOrchestrator(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {

	orch, ok := i.(orchestration.Orchestrator)
	if !ok {
		o.logger.ErrorLog("method", "validateOrchestrator", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, fmt.Errorf("Invalid input type")
	}

	var orchs orchestration.OrchestratorList
	kindKey := orchs.MakeKey(string(apiclient.GroupOrchestration))
	err := kv.List(ctx, kindKey, &orchs)
	if err != nil {
		return nil, true, fmt.Errorf("Error retrieving orchestrators: %v", err)
	}

	for _, otherOrch := range orchs.Items {
		if otherOrch.Spec.URI == orch.Spec.URI && otherOrch.Name != orch.Name {
			return i, true, fmt.Errorf("URI is already used by Orchestrator %s", otherOrch.Name)
		}
	}
	return i, true, nil
}

func createOrchCheckHook(kind string) apiserver.PreCommitFunc {
	return func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {

		existingObj, err := runtime.GetDefaultScheme().New(kind)
		err = kvs.Get(ctx, key, existingObj)
		if err != nil {
			// Update/delete call will return 404
			return i, true, nil
		}

		existingObjMeta, err := runtime.GetObjectMeta(existingObj)
		if err != nil {
			return i, true, fmt.Errorf("Failed to extract object meta: %s", err)
		}

		if ctxutils.GetPeerID(ctx) != globals.APIGw {
			// No checks required if coming from inside venice
			return i, true, nil
		}

		// If the request is a delete, and the user is super admin, no need to check
		if oper == apiintf.DeleteOper {
			isAdmin, ok := authzgrpcctx.UserIsAdminFromIncomingContext(ctx)
			if !ok {
				return i, true, fmt.Errorf("Failed to get user role from context")
			}
			if isAdmin {
				// Admin is allowed to delete all objects
				return i, true, nil
			}
		}
		for k := range existingObjMeta.Labels {
			// if it has orch-name label then the object cannot be deleted unless the user is admin
			if k == utils.OrchNameKey {
				return i, true, ErrOrchManaged
			}
		}
		return i, true, nil
	}
}

func registerOrchestrationHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	oh := orchHooks{logger: l.WithContext("Service", "Orchestrator")}
	svc.GetCrudService("Orchestrator", apiintf.CreateOper).WithPreCommitHook(oh.validateOrchestrator)
	svc.GetCrudService("Orchestrator", apiintf.UpdateOper).WithPreCommitHook(oh.validateOrchestrator)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("orchestration.OrchestrationV1", registerOrchestrationHooks)
}
