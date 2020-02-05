package impl

import (
	"context"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type workloadHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

// validateIPAddressHook validates IP address in the workload
func (s *workloadHooks) validateIPAddressHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	workload, ok := i.(workload.Workload)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting Workload", i)
	}

	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("validateIPAddressHook called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}

	switch oper {
	case apiintf.CreateOper:
		fallthrough
	case apiintf.UpdateOper:

		for _, intf := range workload.Spec.Interfaces {
			if len(intf.IpAddresses) > globals.MaxAllowedIPPerInterface {
				msg := fmt.Sprintf("failed to create workload object: %s, exceeds number of IPs per interface which is %d", workload.Name, globals.MaxAllowedIPPerInterface)
				log.Error(msg)
				return i, false, fmt.Errorf(msg)
			}

			for _, ip := range intf.IpAddresses {
				ipAddress := net.ParseIP(strings.TrimSpace(ip))
				if len(ipAddress) <= 0 {
					msg := fmt.Sprintf("failed to create workload object: %s, IP addresss %v invalid", workload.Name, globals.MaxAllowedIPPerInterface)
					log.Error(msg)
					return i, false, fmt.Errorf(msg)
				}
			}
		}
		return i, true, nil
	}

	return i, true, nil
}

func registerWorkloadHooks(svc apiserver.Service, logger log.Logger) {
	r := workloadHooks{
		svc:    svc,
		logger: logger.WithContext("Service", "WorkloadV1"),
	}
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("Workload", apiintf.CreateOper).WithPreCommitHook(r.validateIPAddressHook)
	svc.GetCrudService("Workload", apiintf.UpdateOper).WithPreCommitHook(r.validateIPAddressHook)
	// For workloads created by orchhub
	svc.GetCrudService("Workload", apiintf.UpdateOper).WithPreCommitHook(createOrchCheckHook("Workload"))
	svc.GetCrudService("Workload", apiintf.DeleteOper).WithPreCommitHook(createOrchCheckHook("Workload"))
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("workload.WorkloadV1", registerWorkloadHooks)
}
