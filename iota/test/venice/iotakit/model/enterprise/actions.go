// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"context"
	"math/rand"
	"time"
)

// VerifySystemHealth checks all aspects of system, like cluster, workload, policies etc
func (sm *SysModel) VerifySystemHealth(collectLogOnErr bool) error {

	const numRetries = 10
	if err := sm.SysModel.VerifySystemHealth(collectLogOnErr); err != nil {
		return err
	}

	var err error
	// verify policy status is good
	for i := 0; i < numRetries; i++ {
		err = sm.VerifyPolicyStatus(sm.SGPolicies())
		if err == nil {
			break
		}
		time.Sleep(time.Second)
	}
	if err != nil {
		if collectLogOnErr {
			sm.CollectLogs()
		}
		return err
	}

	// verify ping is successful across all workloads
	if sm.Tb.HasNaplesHW() {
		for i := 0; i < numRetries; i++ {
			err = sm.PingPairs(sm.WorkloadPairs().WithinNetwork())
			if err == nil {
				break
			}
		}
		if err != nil {
			if collectLogOnErr {
				sm.CollectLogs()
			}
			return err
		}
	}

	return nil
}

//TriggerDeleteAddConfig triggers link flap
func (sm *SysModel) TriggerDeleteAddConfig(percent int) error {

	err := sm.CleanupAllConfig()
	if err != nil {
		return err
	}

	err = sm.TeardownWorkloads(sm.Workloads())
	if err != nil {
		return err
	}

	return sm.SetupDefaultConfig(context.Background(), sm.Scale, sm.ScaleData)

}

type triggerFunc func(int) error

//RunRandomTrigger runs a random trigger
func (sm *SysModel) RunRandomTrigger(percent int) error {

	triggers := []triggerFunc{
		sm.TriggerDeleteAddConfig,
		sm.TriggerSnapshotRestore,
		sm.TriggerHostReboot,
		sm.TriggerVeniceReboot,
		sm.TriggerVenicePartition,
		sm.TriggerLinkFlap,
		sm.TriggerNaplesUpgrade,
	}

	index := rand.Intn(len(triggers))
	return triggers[index](percent)
}
