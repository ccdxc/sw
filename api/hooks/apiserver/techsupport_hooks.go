package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type techSupportRequestHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (r *techSupportRequestHooks) validateTechSupportRequest(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	tsr, ok := i.(monitoring.TechSupportRequest)
	if !ok {
		return i, false, fmt.Errorf("Invalid input type")
	}

	if tsr.Spec.NodeSelector != nil && (len(tsr.Spec.NodeSelector.Names) != 0 || (tsr.Spec.NodeSelector.Labels != nil && len(tsr.Spec.NodeSelector.Labels.Requirements) > 0)) {
		return i, true, nil
	}

	return i, false, fmt.Errorf("no nodes or dscs are selected for collecting techsupport")
}

func registerTechSupportRequestHooks(svc apiserver.Service, logger log.Logger) {
	r := techSupportRequestHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "TechSupportRequest")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("TechSupportRequest", apiintf.CreateOper).WithPreCommitHook(r.validateTechSupportRequest)
	svc.GetCrudService("TechSupportRequest", apiintf.UpdateOper).WithPreCommitHook(r.validateTechSupportRequest)
}
