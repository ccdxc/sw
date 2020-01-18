package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/orchestration"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

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
