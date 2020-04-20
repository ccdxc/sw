package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	handler "github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type eventHooks struct {
	logger log.Logger
}

func (eh *eventHooks) validateSyslogEventPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	evtpolicy, ok := i.(monitoring.EventPolicy)
	if ok != true {
		return i, false, fmt.Errorf("invalid object %T instead of EventPolicy", i)
	}

	if err := handler.ValidateEventPolicySpec(&evtpolicy.Spec); err != nil {
		return i, false, err
	}
	return i, true, nil
}

// If the incoming request has no credentials, we load in credentials from kv store
func (eh *eventHooks) handleCredentialUpdate(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	kind := "eventPolicy"
	hookName := "eventHook"
	methodName := "handleCredentialUpdate"
	new, ok := i.(monitoring.EventPolicy)
	cur := &monitoring.EventPolicy{}
	logger := eh.logger

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
	for _, target := range cur.Spec.Targets {
		key := fmt.Sprintf("%s%s", target.Destination, target.Transport)
		curCredMap[key] = target
	}
	newCredMap := map[string]*monitoring.ExportConfig{}
	for _, target := range new.Spec.Targets {
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

func registerEventHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	eh := eventHooks{logger: l.WithContext("Service", "EventPolicy")}
	svc.GetCrudService("EventPolicy", apiintf.CreateOper).WithPreCommitHook(eh.validateSyslogEventPolicy)
	svc.GetCrudService("EventPolicy", apiintf.UpdateOper).WithPreCommitHook(eh.validateSyslogEventPolicy)
	svc.GetCrudService("EventPolicy", apiintf.UpdateOper).WithPreCommitHook(eh.handleCredentialUpdate)
}
