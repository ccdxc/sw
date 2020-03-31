package impl

import (
	"context"
	"errors"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/hooks/apiserver/utils"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
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
	case apiintf.CreateOper, apiintf.UpdateOper:
		for _, intf := range workload.Spec.Interfaces {
			if len(intf.IpAddresses) > globals.MaxAllowedIPPerInterface {
				msg := fmt.Sprintf("failed to %s workload object: %s, exceeds number of IPs per interface which is %d", oper, workload.Name, globals.MaxAllowedIPPerInterface)
				log.Error(msg)
				return i, false, fmt.Errorf(msg)
			}

			for _, ip := range intf.IpAddresses {
				ipAddress := net.ParseIP(strings.TrimSpace(ip))
				if len(ipAddress) <= 0 {
					msg := fmt.Sprintf("failed to %s workload object: %s, IP addresss %v invalid", oper, workload.Name, globals.MaxAllowedIPPerInterface)
					log.Error(msg)
					return i, false, fmt.Errorf(msg)
				}
			}
		}
		return i, true, nil
	}

	return i, true, nil
}

func (s *workloadHooks) processStartMigration(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	wl, ok := i.(workload.Workload)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting Workload", i)
	}
	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("processStartMigration called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}
	cur := &workload.Workload{}
	if err := kvs.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		wlObj, ok := oldObj.(*workload.Workload)
		if !ok {
			return oldObj, errors.New("invalid object type")
		}
		return utils.ProcessStartMigration(s.logger, wlObj, &wl)
	}); err != nil {
		s.logger.Errorf("Error Stating Migration on workload %s : %v", wl.Name, err)
		return nil, false, err
	}
	return wl, false, nil
}

func (s *workloadHooks) processAbortMigration(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	wl, ok := i.(workload.Workload)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting Workload", i)
	}
	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("processAbortMigration called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}
	// When migration is aborted, update the migration status to indicate abort operation to
	// the dataplane.
	// XXX Wait for DONE status to change the spec so that both new/old information is available to
	// dataplane for ABORT operation ??

	// create empty object
	cur := &workload.Workload{}
	if err := kvs.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		wlObj, ok := oldObj.(*workload.Workload)
		if !ok {
			return oldObj, errors.New("invalid object type")
		}
		return utils.ProcessAbortMigration(s.logger, wlObj, &wl)
	}); err != nil {
		s.logger.Errorf("Error Aborting Migration on workload %s : %v", wl.Name, err)
		return nil, false, err
	}
	return wl, false, nil
}

func (s *workloadHooks) processFinalSyncMigration(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	wl, ok := i.(workload.Workload)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting Workload", i)
	}
	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("processFinishMigration called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}
	// create empty object
	cur := &workload.Workload{}
	if err := kvs.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		wlObj, ok := oldObj.(*workload.Workload)
		if !ok {
			return oldObj, errors.New("invalid object type")
		}
		return utils.ProcessFinalSyncMigration(s.logger, wlObj, &wl)
	}); err != nil {
		s.logger.Errorf("Error Completing Migration on workload %s : %v", wl.Name, err)
		return nil, false, err
	}
	return wl, false, nil
}

func (s *workloadHooks) processFinishMigration(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	wl, ok := i.(workload.Workload)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting Workload", i)
	}
	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("processFinishMigration called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}
	// create empty object
	cur := &workload.Workload{}
	if err := kvs.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		wlObj, ok := oldObj.(*workload.Workload)
		if !ok {
			return oldObj, errors.New("invalid object type")
		}
		return utils.ProcessFinishMigration(s.logger, wlObj, &wl)
	}); err != nil {
		s.logger.Errorf("Error Completing Migration on workload %s : %v", wl.Name, err)
		return nil, false, err
	}
	return wl, false, nil
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

	svc.GetMethod("StartMigration").WithPreCommitHook(r.processStartMigration)
	svc.GetMethod("FinishMigration").WithPreCommitHook(r.processFinishMigration)
	svc.GetMethod("FinalSyncMigration").WithPreCommitHook(r.processFinalSyncMigration)
	svc.GetMethod("AbortMigration").WithPreCommitHook(r.processAbortMigration)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("workload.WorkloadV1", registerWorkloadHooks)
}
