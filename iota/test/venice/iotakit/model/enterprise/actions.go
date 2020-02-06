// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
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
