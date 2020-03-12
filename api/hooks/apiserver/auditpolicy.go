package impl

import (
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/audit/syslog"
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

func registerAuditPolicyHooks(svc apiserver.Service, logger log.Logger) {
	a := &auditPolicyHooks{}
	a.logger = logger.WithContext("Service", "AuditPolicyHooks")
	svc.GetCrudService("AuditPolicy", apiintf.CreateOper).GetRequestType().WithValidate(a.validateAuditPolicy)
}
