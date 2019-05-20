package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/nic/agent/tpa/state"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type flowExpHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (r *flowExpHooks) validateFlowExportPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	policy, ok := i.(monitoring.FlowExportPolicy)
	if ok != true {
		return i, false, fmt.Errorf("invalid object %T instead of monitoring.FlowExportPolicy", i)
	}

	if err := state.ValidateFlowExportPolicy(&policy); err != nil {
		return i, false, err
	}
	return i, true, nil
}

func registerFlowExpPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := flowExpHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "flowexphooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("FlowExportPolicy", apiintf.CreateOper).WithPreCommitHook(r.validateFlowExportPolicy)
	svc.GetCrudService("FlowExportPolicy", apiintf.UpdateOper).WithPreCommitHook(r.validateFlowExportPolicy)
}
