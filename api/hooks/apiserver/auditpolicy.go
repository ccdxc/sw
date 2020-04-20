package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/audit/syslog"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type auditPolicyHooks struct {
	logger log.Logger
}

func (a *auditPolicyHooks) validateAuditPolicy(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	if ignoreSpec {
		// Only spec fields are validated
		return nil
	}
	policy, ok := i.(monitoring.AuditPolicy)
	if !ok {
		return []error{fmt.Errorf("invalid object %T instead of AuditPolicy", i)}
	}
	return syslog.ValidateSyslogAuditorConfig(policy.Spec.Syslog)
}

// If the incoming request has no credentials, we load in credentials from kv store
func (a *auditPolicyHooks) handleCredentialUpdate(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	kind := "audit-policy"
	hookName := "auditPolicyHook"
	methodName := "handleCredentialUpdate"
	new, ok := i.(monitoring.AuditPolicy)
	cur := &monitoring.AuditPolicy{}
	logger := a.logger

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
	// Map config by destination,target
	curCredMap := map[string]*monitoring.ExportConfig{}
	for _, target := range cur.Spec.Syslog.Targets {
		key := fmt.Sprintf("%s%s", target.Destination, target.Transport)
		curCredMap[key] = target
	}
	newCredMap := map[string]*monitoring.ExportConfig{}
	for _, target := range new.Spec.Syslog.Targets {
		key := fmt.Sprintf("%s%s", target.Destination, target.Transport)
		if _, ok := newCredMap[key]; ok {
			// Duplicate targets found in new object
			return new, true, fmt.Errorf("duplicate targets are not allowed")
		}
		newCredMap[key] = target
	}

	// decrypt credentials as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty key in that case
	if err := cur.ApplyStorageTransformer(context.Background(), false); err != nil {
		logger.ErrorLog("method", methodName, "msg", "error decrypting credentials field", "error", err)
		return new, true, err
	}

	for key, newTarget := range newCredMap {
		if newTarget.Credentials != nil {
			continue
		}
		if curTarget, ok := curCredMap[key]; ok {
			newTarget.Credentials = curTarget.Credentials
		}
	}

	if !dryRun {
		logger.DebugLog("method", methodName, "msg", fmt.Sprintf("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion))
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
	}

	return new, true, nil
}

func registerAuditPolicyHooks(svc apiserver.Service, logger log.Logger) {
	a := &auditPolicyHooks{}
	a.logger = logger.WithContext("Service", "AuditPolicyHooks")
	svc.GetCrudService("AuditPolicy", apiintf.CreateOper).GetRequestType().WithValidate(a.validateAuditPolicy)
	svc.GetCrudService("AuditPolicy", apiintf.UpdateOper).WithPreCommitHook(a.handleCredentialUpdate)
}
