package utils

import (
	"github.com/pensando/sw/api/generated/cluster"
)

// GetNICCondition returns the condition of the specified type from the status of the supplied DistributedServiceCard object
func GetNICCondition(nic *cluster.DistributedServiceCard, condType cluster.DSCCondition_ConditionType) *cluster.DSCCondition {
	for _, cond := range nic.Status.Conditions {
		if cond.Type == condType.String() {
			return &cond
		}
	}
	return nil
}

// SetNICCondition sets the condition on the status of the supplied SmartNIC object
func SetNICCondition(nic *cluster.DistributedServiceCard, updCond *cluster.DSCCondition) {
	for i, curCond := range nic.Status.Conditions {
		if curCond.Type == updCond.Type {
			nic.Status.Conditions[i] = *updCond
			return
		}
	}
	// if not found, add
	nic.Status.Conditions = append(nic.Status.Conditions, *updCond)
}
