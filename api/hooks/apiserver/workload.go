package impl

import (
	"context"
	"errors"
	"fmt"
	"net"
	"sort"
	"strings"
	"time"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	// shorthand names for migration stages and status
	stageMigrationNone  = workload.WorkloadMigrationStatus_MIGRATION_NONE.String()
	stageMigrationStart = workload.WorkloadMigrationStatus_MIGRATION_START.String()
	stageMigrationDone  = workload.WorkloadMigrationStatus_MIGRATION_DONE.String()
	stageMigrationAbort = workload.WorkloadMigrationStatus_MIGRATION_ABORT.String()
	// Dataplane Status
	statusNone     = workload.WorkloadMigrationStatus_NONE.String()
	statusStarted  = workload.WorkloadMigrationStatus_STARTED.String()
	statusDone     = workload.WorkloadMigrationStatus_DONE.String()
	statusFailed   = workload.WorkloadMigrationStatus_FAILED.String()
	statusTimedOut = workload.WorkloadMigrationStatus_TIMED_OUT.String()
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

func isMigrationInProgress(wlObj *workload.Workload) bool {
	if wlObj.Status.MigrationStatus != nil && wlObj.Status.MigrationStatus.Stage != stageMigrationNone &&
		wlObj.Status.MigrationStatus.Status != statusDone && wlObj.Status.MigrationStatus.Status != statusTimedOut &&
		wlObj.Status.MigrationStatus.Status != statusFailed {
		return true
	}
	return false
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
		if isMigrationInProgress(wlObj) {
			return oldObj, errors.New("Migration already in progress, cannot start another one now")
		}
		// copy Spec to Status
		if wlObj.Status.MigrationStatus == nil {
			wlObj.Status.MigrationStatus = &workload.WorkloadMigrationStatus{}
		}
		wlObj.Status.MigrationStatus.Stage = stageMigrationStart
		if wlObj.Status.MigrationStatus.StartedAt == nil {
			wlObj.Status.MigrationStatus.StartedAt = &api.Timestamp{}
		}
		wlObj.Status.MigrationStatus.StartedAt.SetTime(time.Now())
		wlObj.Status.MigrationStatus.Status = statusNone
		wlObj.Status.HostName = wlObj.Spec.HostName
		// record exsting interfaces
		statusIfs := map[string]int{}
		for i, intf := range wlObj.Status.Interfaces {
			if intf.MACAddress == "" {
				continue
			}
			statusIfs[intf.MACAddress] = i
		}
		// Cannot reinit status interfaces[] as it has more information like endpoints etc which is
		// not in the spec
		for _, intf := range wlObj.Spec.Interfaces {
			if i, ok := statusIfs[intf.MACAddress]; ok {
				wlObj.Status.Interfaces[i].MicroSegVlan = intf.MicroSegVlan
			} else {
				wlObj.Status.Interfaces = append(wlObj.Status.Interfaces, workload.WorkloadIntfStatus{
					MicroSegVlan: intf.MicroSegVlan,
					ExternalVlan: intf.ExternalVlan,
					MACAddress:   intf.MACAddress,
					IpAddresses:  intf.IpAddresses,
				})
			}
		}
		// sort it to keep the same order between spec and status (for common intfs)
		sort.Slice(wlObj.Status.Interfaces, func(i, j int) bool {
			return wlObj.Status.Interfaces[i].MACAddress < wlObj.Status.Interfaces[j].MACAddress
		})
		wlObj.Spec.HostName = wl.Spec.HostName
		wlObj.Spec.Interfaces = wl.Spec.Interfaces
		return oldObj, nil
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
		if wlObj.Status.MigrationStatus == nil || wlObj.Status.MigrationStatus.Stage != stageMigrationStart {
			return oldObj, errors.New("Migration not in progress, cannot abort")
		}
		wlObj.Status.MigrationStatus.Stage = stageMigrationAbort
		// copy from Status to Spec
		wlObj.Spec.HostName = wlObj.Status.HostName
		// record existing interfaces
		specIfs := map[string]int{}
		for i, intf := range wlObj.Spec.Interfaces {
			specIfs[intf.MACAddress] = i
		}
		for _, intf := range wlObj.Status.Interfaces {
			if i, ok := specIfs[intf.MACAddress]; ok {
				wlObj.Spec.Interfaces[i].MicroSegVlan = intf.MicroSegVlan
			} else {
				s.logger.Errorf("Interface changes during migration are unexpected")
				wlObj.Spec.Interfaces = append(wlObj.Spec.Interfaces, workload.WorkloadIntfSpec{
					MACAddress:   intf.MACAddress,
					MicroSegVlan: intf.MicroSegVlan,
					ExternalVlan: intf.ExternalVlan,
					IpAddresses:  intf.IpAddresses,
				})
			}
		}
		// sort it to keep the same order between spec and status (for common intfs)
		sort.Slice(wlObj.Spec.Interfaces, func(i, j int) bool {
			return wlObj.Spec.Interfaces[i].MACAddress < wlObj.Spec.Interfaces[j].MACAddress
		})
		return oldObj, nil
	}); err != nil {
		s.logger.Errorf("Error Aborting Migration on workload %s : %v", wl.Name, err)
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
		if wlObj.Status.MigrationStatus == nil ||
			wlObj.Status.MigrationStatus.Stage != stageMigrationStart {
			return oldObj, errors.New("Migration is not in progress, cannot be completed")
		}
		wlObj.Status.MigrationStatus.Stage = stageMigrationDone
		return oldObj, nil
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
	svc.GetMethod("AbortMigration").WithPreCommitHook(r.processAbortMigration)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("workload.WorkloadV1", registerWorkloadHooks)
}
