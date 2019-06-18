package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
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

func registerEventHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	eh := eventHooks{logger: l.WithContext("Service", "EventPolicy")}
	svc.GetCrudService("EventPolicy", apiintf.CreateOper).WithPreCommitHook(eh.validateSyslogEventPolicy)
	svc.GetCrudService("EventPolicy", apiintf.UpdateOper).WithPreCommitHook(eh.validateSyslogEventPolicy)
}
