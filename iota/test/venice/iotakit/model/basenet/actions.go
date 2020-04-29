package basenet

import (
	"context"
	"errors"
	"fmt"
	"math/rand"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
)

//VerifyNaplesStatus verify naples status
func (sm *SysModel) VerifyNaplesStatus() error {

	//TODO
	// verify each naples health
	for _, np := range sm.NaplesNodes {
		// check naples status

		err := sm.CheckNaplesHealth(np)
		if err != nil {
			log.Errorf("Naples health check failed. Err: %v", err)
			return err
		}

		// check smartnic status in Venice
		for _, inst := range np.Instances {
			dsc := inst.Dsc
			snic, err := sm.GetSmartNICByName(dsc.Status.PrimaryMAC)
			if err != nil {
				err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", dsc.Status.PrimaryMAC, err)
				log.Errorf("%v", err)
				return err
			}

			log.Debugf("Got smartnic object: %+v", snic)

			//if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
			//log.Errorf("Invalid Naples status: %+v", snic)
			//return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", np.Name(), snic.Status)
			//}
			if len(snic.Status.Conditions) < 1 {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("No naples status reported for naples %v", np.Name())
			}
			if snic.Status.Conditions[0].Type != cluster.DSCCondition_HEALTHY.String() {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("Invalid status condition-type %v for naples %v", snic.Status.Conditions[0].Type, np.Name())
			}
			if snic.Status.Conditions[0].Status != cluster.ConditionStatus_TRUE.String() {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("Invalid status %v for naples %v", snic.Status.Conditions[0].Status, np.Name())
			}
		}
	}

	// verify each naples health
	snics, err := sm.ListSmartNIC()
	if err != nil {
		log.Errorf("Unable to list dsc")
		return err
	}

	for _, snic := range snics {
		//	if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
		//	log.Errorf("Invalid Naples status: %+v", snic)
		//	return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", snic.Spec.GetID(), snic.Status)
		//	}
		if len(snic.Status.Conditions) < 1 {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("No naples status reported for naples %v", snic.Spec.GetID())
		}
		if snic.Status.Conditions[0].Type != cluster.DSCCondition_HEALTHY.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status condition-type %v for naples %v", snic.Spec.GetID(), snic.Status.Conditions[0].Type)
		}
		if snic.Status.Conditions[0].Status != cluster.ConditionStatus_TRUE.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status %v for naples %v", snic.Spec.GetID(), snic.Status.Conditions[0].Status)
		}
	}

	return nil

}

// VerifySystemHealth checks all aspects of system, like cluster, workload, policies etc
func (sm *SysModel) VerifySystemHealth(collectLogOnErr bool) error {
	const numRetries = 10
	// verify cluster is in good health
	err := sm.VerifyClusterStatus()
	if err != nil {
		if collectLogOnErr {
			sm.CollectLogs()
		}
		return err
	}

	//Verify Config is in sync
	for i := 0; i < numRetries; i++ {
		var done bool
		done, err = sm.IsConfigPushComplete()
		if done && err == nil {
			break
		}
		time.Sleep(5 * time.Second)

		err = errors.New("Config push incomplete")
	}
	if err != nil {
		return err
	}

	return nil
}

// VerifyClusterStatus verifies venice cluster status
func (sm *SysModel) VerifyClusterStatus() error {
	log.Infof("Verifying cluster health..")

	// check iota cluster health
	err := sm.Tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Invalid Iota cluster state: %v", err)
		return err
	}

	if err := sm.VerifyVeniceStatus(); err != nil {
		return err
	}

	if err := sm.VerifyNaplesStatus(); err != nil {
		return err
	}

	//Make sure config push in complete.
	if ok, err := sm.IsConfigPushComplete(); !ok || err != nil {
		log.Errorf("Config push incomplete")
		return errors.New("Config not in sync")
	}

	// verify ping is successful across all workloads
	if sm.Tb.HasNaplesHW() {
		for i := 0; i < 10; i++ {
			err = sm.PingPairs(sm.WorkloadPairs().WithinNetwork())
			if err == nil {
				break
			}
		}
		if err != nil {
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
