package impl

import (
	"context"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/ctrler/tpm"

	"github.com/pensando/sw/api/generated/monitoring"
	hooksutils "github.com/pensando/sw/api/hooks/apiserver/utils"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type fwlogHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (r *fwlogHooks) validateFwlogPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	policy, ok := i.(monitoring.FwlogPolicy)
	if ok != true {
		return i, false, fmt.Errorf("invalid object %T instead of FwlogPolicy", i)
	}

	if err := hooksutils.ValidateFwLogPolicy(&policy.Spec); err != nil {
		return i, false, err
	}

	switch oper {
	case apiintf.CreateOper:
		pl := monitoring.FwlogPolicyList{}
		policyKey := strings.TrimSuffix(pl.MakeKey(string(apiclient.GroupMonitoring)), "/")
		err := kv.List(ctx, policyKey, &pl)
		if err != nil {
			return nil, false, fmt.Errorf("error retrieving FwlogPolicy: %v", err)
		}

		if len(pl.Items) >= tpm.MaxNumExportPolicy {
			return nil, false, fmt.Errorf("can't configure more than %v FwlogPolicy", tpm.MaxNumExportPolicy)
		}
	}

	return i, true, nil
}

func registerFwlogPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := fwlogHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "fwloghooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("FwlogPolicy", apiintf.CreateOper).WithPreCommitHook(r.validateFwlogPolicy)
	svc.GetCrudService("FwlogPolicy", apiintf.UpdateOper).WithPreCommitHook(r.validateFwlogPolicy)
}
