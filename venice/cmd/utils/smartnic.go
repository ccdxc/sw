package utils

import (
	"github.com/pensando/sw/api/generated/cluster"
)

// GetNICCondition returns the condition of the specified type from the status of the supplied SmartNIC object
func GetNICCondition(nic *cluster.SmartNIC, condType cluster.SmartNICCondition_ConditionType) *cluster.SmartNICCondition {
	for _, cond := range nic.Status.Conditions {
		if cond.Type == condType.String() {
			return &cond
		}
	}
	return nil
}

// SetNICCondition sets the condition on the status of the supplied SmartNIC object
func SetNICCondition(nic *cluster.SmartNIC, updCond *cluster.SmartNICCondition) {
	for i, curCond := range nic.Status.Conditions {
		if curCond.Type == updCond.Type {
			nic.Status.Conditions[i] = *updCond
			return
		}
	}
	// if not found, add
	nic.Status.Conditions = append(nic.Status.Conditions, *updCond)
}
