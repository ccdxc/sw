package utils

import (
	"reflect"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetSetNICCondition(t *testing.T) {
	var nic cluster.SmartNIC
	// get non-existant
	getCond := GetNICCondition(&nic, cluster.SmartNICCondition_HEALTHY)
	Assert(t, getCond == nil, "Cond should have been nil")
	// set
	setCond := &cluster.SmartNICCondition{
		Type:               cluster.SmartNICCondition_HEALTHY.String(),
		Status:             cluster.ConditionStatus_TRUE.String(),
		LastTransitionTime: time.Now().UTC().Format(time.RFC3339),
	}
	SetNICCondition(&nic, setCond)
	getCond = GetNICCondition(&nic, cluster.SmartNICCondition_HEALTHY)
	Assert(t, reflect.DeepEqual(getCond, setCond), "Set and get mismatch. Set: %+v, get: %+c", setCond, getCond)
	// update
	setCond.Status = cluster.ConditionStatus_FALSE.String()
	SetNICCondition(&nic, setCond)
	getCond = GetNICCondition(&nic, cluster.SmartNICCondition_HEALTHY)
	Assert(t, reflect.DeepEqual(getCond, setCond), "Set and get mismatch. Set: %+v, get: %+c", setCond, getCond)
}
