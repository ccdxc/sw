package utils

import (
	"errors"
	"fmt"
	"sort"
	"strconv"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	// shorthand names for migration stages and status
	stageMigrationNone           = workload.WorkloadMigrationStatus_MIGRATION_NONE.String()
	stageMigrationStart          = workload.WorkloadMigrationStatus_MIGRATION_START.String()
	stageMigrationDone           = workload.WorkloadMigrationStatus_MIGRATION_DONE.String()
	stageMigrationFinalSync      = workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String()
	stageMigrationAbort          = workload.WorkloadMigrationStatus_MIGRATION_ABORT.String()
	stageMigrationFromNonPenHost = workload.WorkloadMigrationStatus_MIGRATION_FROM_NON_PEN_HOST.String()
	// Dataplane Status
	statusNone     = workload.WorkloadMigrationStatus_NONE.String()
	statusStarted  = workload.WorkloadMigrationStatus_STARTED.String()
	statusDone     = workload.WorkloadMigrationStatus_DONE.String()
	statusFailed   = workload.WorkloadMigrationStatus_FAILED.String()
	statusTimedOut = workload.WorkloadMigrationStatus_TIMED_OUT.String()
)

// IsWorkloadMigrating returns whether a workload is in the middle of migrating or not
func IsWorkloadMigrating(wlObj *workload.Workload) bool {
	if wlObj == nil || wlObj.Status.MigrationStatus == nil {
		return false
	}
	// If migration is not started
	// If migration is Done
	// If migration is aborted, and datapath is in terminal state
	if wlObj.Status.MigrationStatus.Stage == stageMigrationNone ||
		wlObj.Status.MigrationStatus.Stage == stageMigrationFromNonPenHost ||
		wlObj.Status.MigrationStatus.Stage == stageMigrationDone ||
		(wlObj.Status.MigrationStatus.Stage == stageMigrationAbort &&
			(wlObj.Status.MigrationStatus.Status == statusDone ||
				wlObj.Status.MigrationStatus.Status == statusFailed)) {
		return false
	}
	return true
}

// ProcessStartMigration is the hook implementation for the action
func ProcessStartMigration(l log.Logger, oldObj, inObj *workload.Workload) (*workload.Workload, error) {
	if IsWorkloadMigrating(oldObj) {
		return oldObj, errors.New("Migration already in progress, cannot start another one now")
	}
	// Check that interfaces do not change as part of migration
	if len(inObj.Spec.Interfaces) != len(oldObj.Spec.Interfaces) {
		return oldObj, errors.New("Number of interface cannot change during migration")
	}
	specIfs := map[string]int{}
	for i, intf := range oldObj.Spec.Interfaces {
		specIfs[intf.MACAddress] = i
	}
	for _, intf := range inObj.Spec.Interfaces {
		if _, ok := specIfs[intf.MACAddress]; !ok {
			return oldObj, errors.New("Interface MAC addr cannot change during migration")
		}
	}
	// copy Spec to Status
	if oldObj.Status.MigrationStatus == nil {
		oldObj.Status.MigrationStatus = &workload.WorkloadMigrationStatus{}
	}
	oldObj.Status.MigrationStatus.Stage = stageMigrationStart
	if oldObj.Status.MigrationStatus.StartedAt == nil {
		oldObj.Status.MigrationStatus.StartedAt = &api.Timestamp{}
	}
	oldObj.Status.MigrationStatus.StartedAt.SetTime(time.Now())
	oldObj.Status.MigrationStatus.CompletedAt = &api.Timestamp{}
	oldObj.Status.MigrationStatus.Status = statusNone
	oldObj.Status.HostName = oldObj.Spec.HostName

	moveSpecInterfacesToStatus(oldObj)

	oldObj.Spec.HostName = inObj.Spec.HostName
	sort.Slice(inObj.Spec.Interfaces, func(i, j int) bool {
		return inObj.Spec.Interfaces[i].MACAddress < inObj.Spec.Interfaces[j].MACAddress
	})
	oldObj.Spec.Interfaces = inObj.Spec.Interfaces

	genID, err := strconv.ParseInt(oldObj.GenerationID, 10, 64)
	if err != nil {
		l.Errorf("error parsing generation ID: %v", err)
		genID = 2
	}
	oldObj.GenerationID = fmt.Sprintf("%d", genID+1)

	return oldObj, nil
}

// ProcessAbortMigration is the hook implementation for the action
func ProcessAbortMigration(l log.Logger, oldObj, inObj *workload.Workload) (*workload.Workload, error) {
	if oldObj.Status.MigrationStatus == nil || oldObj.Status.MigrationStatus.Stage != stageMigrationStart {
		return oldObj, errors.New("Migration not in progress, cannot abort")
	}
	oldObj.Status.MigrationStatus.Stage = stageMigrationAbort
	// copy from Status to Spec
	oldObj.Spec.HostName = oldObj.Status.HostName
	// record existing interfaces
	specIfs := map[string]int{}
	for i, intf := range oldObj.Spec.Interfaces {
		specIfs[intf.MACAddress] = i
	}
	for _, intf := range oldObj.Status.Interfaces {
		if i, ok := specIfs[intf.MACAddress]; ok {
			oldObj.Spec.Interfaces[i].MicroSegVlan = intf.MicroSegVlan
		} else {
			l.Errorf("Interface changes during migration are unexpected")
			oldObj.Spec.Interfaces = append(oldObj.Spec.Interfaces, workload.WorkloadIntfSpec{
				MACAddress:   intf.MACAddress,
				MicroSegVlan: intf.MicroSegVlan,
				ExternalVlan: intf.ExternalVlan,
				IpAddresses:  intf.IpAddresses,
			})
		}
	}
	// sort it to keep the same order between spec and status (for common intfs)
	sort.Slice(oldObj.Spec.Interfaces, func(i, j int) bool {
		return oldObj.Spec.Interfaces[i].MACAddress < oldObj.Spec.Interfaces[j].MACAddress
	})

	genID, err := strconv.ParseInt(oldObj.GenerationID, 10, 64)
	if err != nil {
		l.Errorf("error parsing generation ID: %v", err)
		genID = 2
	}
	oldObj.GenerationID = fmt.Sprintf("%d", genID+1)

	return oldObj, nil
}

// ProcessFinalSyncMigration is the hook implementation for the action
func ProcessFinalSyncMigration(l log.Logger, oldObj, inObj *workload.Workload) (*workload.Workload, error) {
	if oldObj.Status.MigrationStatus == nil ||
		oldObj.Status.MigrationStatus.Stage != stageMigrationStart {
		return oldObj, errors.New("Migration is not in progress, cannot perform final sync")
	}
	oldObj.Status.MigrationStatus.Stage = stageMigrationFinalSync
	return oldObj, nil
}

// ProcessFinishMigration is the hook implementation for the action
func ProcessFinishMigration(l log.Logger, oldObj, inObj *workload.Workload) (*workload.Workload, error) {
	if oldObj.Status.MigrationStatus == nil {
		return oldObj, errors.New("Workload is not migrating, cannot perform finish migration")
	}
	if oldObj.Status.MigrationStatus.Stage != stageMigrationFinalSync &&
		!(oldObj.Status.MigrationStatus.Stage == stageMigrationStart && oldObj.Status.MigrationStatus.Status == statusTimedOut) {
		return oldObj, errors.New("Migration is not in the correct stage, cannot perform finish migration")
	}
	oldObj.Status.MigrationStatus.Stage = stageMigrationDone
	oldObj.Status.HostName = oldObj.Spec.HostName
	oldObj.Status.HostName = oldObj.Spec.HostName
	// Move spec interfaces to status
	moveSpecInterfacesToStatus(oldObj)

	// TODO: Clean up statius
	return oldObj, nil
}

func moveSpecInterfacesToStatus(oldObj *workload.Workload) {
	// record exsting interfaces
	statusIfs := map[string]int{}
	for i, intf := range oldObj.Status.Interfaces {
		if intf.MACAddress == "" {
			continue
		}
		statusIfs[intf.MACAddress] = i
	}
	// Cannot reinit status interfaces[] as it has more information like endpoints etc which is
	// not in the spec, remove any interfaces in status that are no-more in the spec
	newStatusIfs := []workload.WorkloadIntfStatus{}
	for _, intf := range oldObj.Spec.Interfaces {
		if i, ok := statusIfs[intf.MACAddress]; ok {
			statusIntf := oldObj.Status.Interfaces[i]
			statusIntf.MicroSegVlan = intf.MicroSegVlan
			newStatusIfs = append(newStatusIfs, statusIntf)
		} else {
			newStatusIfs = append(newStatusIfs, workload.WorkloadIntfStatus{
				MicroSegVlan: intf.MicroSegVlan,
				ExternalVlan: intf.ExternalVlan,
				MACAddress:   intf.MACAddress,
				IpAddresses:  intf.IpAddresses,
			})
		}
	}
	// sort it to keep the same order between spec and status (for common intfs)
	sort.Slice(newStatusIfs, func(i, j int) bool {
		return newStatusIfs[i].MACAddress < newStatusIfs[j].MACAddress
	})
	oldObj.Status.Interfaces = newStatusIfs
}
