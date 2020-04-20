package impl

import (
	"context"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/apiclient"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/ctrler/tpm"

	"github.com/pensando/sw/api/generated/monitoring"
	hooksutils "github.com/pensando/sw/api/hooks/apiserver/utils"
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

// If the incoming request has no credentials, we load in credentials from kv store
func (r *fwlogHooks) handleCredentialUpdate(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	kind := "fwlogPolicy"
	hookName := "fwlogHooks"
	methodName := "handleCredentialUpdate"
	new, ok := i.(monitoring.FwlogPolicy)
	cur := &monitoring.FwlogPolicy{}
	logger := r.logger

	logger.DebugLog("msg", "%s %s called", hookName, methodName)
	if !ok {
		logger.ErrorLog("method", methodName, "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, fmt.Errorf("Invalid input type")
	}

	if oper != apiintf.UpdateOper {
		logger.ErrorLog("method", methodName, "msg", fmt.Sprintf("called for invalid api operation %s", oper))
		return i, true, fmt.Errorf("Invalid input type")
	}

	if err := kv.Get(ctx, key, cur); err != nil {
		logger.ErrorLog("method", methodName,
			"msg", fmt.Sprintf("error getting object with key [%s] in API server pre-commit hook for update %s", kind, key),
			"error", err)
		return new, true, err
	}

	// Check and merge credentials
	// Map config by destination,target to index
	curCredMap := map[string]int{}
	for i, target := range cur.Spec.Targets {
		key := fmt.Sprintf("%s%s", target.Destination, target.Transport)
		curCredMap[key] = i
	}
	newCredMap := map[string]int{}
	for i, target := range new.Spec.Targets {
		key := fmt.Sprintf("%s%s", target.Destination, target.Transport)
		if _, ok := newCredMap[key]; ok {
			// Duplicate targets found in new object
			return new, true, fmt.Errorf("duplicate targets are not allowed")
		}
		newCredMap[key] = i
	}

	// decrypt credentials as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty key in that case
	if err := cur.ApplyStorageTransformer(context.Background(), false); err != nil {
		logger.ErrorLog("method", methodName, "msg", "error decrypting credentials field", "error", err)
		return new, true, err
	}

	for key, i := range newCredMap {
		if new.Spec.Targets[i].Credentials != nil {
			continue
		}
		if j, ok := curCredMap[key]; ok {
			curCredentials := cur.Spec.Targets[j]
			new.Spec.Targets[i] = curCredentials
		}
	}

	if !dryRun {
		logger.DebugLog("method", methodName, "msg", fmt.Sprintf("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion))
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
	}

	return new, true, nil
}

func registerFwlogPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := fwlogHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "fwloghooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("FwlogPolicy", apiintf.CreateOper).WithPreCommitHook(r.validateFwlogPolicy)
	svc.GetCrudService("FwlogPolicy", apiintf.UpdateOper).WithPreCommitHook(r.validateFwlogPolicy)
	svc.GetCrudService("FwlogPolicy", apiintf.UpdateOper).WithPreCommitHook(r.handleCredentialUpdate)
}
