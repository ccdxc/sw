// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"errors"
	"fmt"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

//StartConsoleLogging start naples logging
func (sm *SysModel) StartConsoleLogging() error {
	return sm.Tb.StartNaplesConsoleLogging()
}

// WorkloadPairs creates full mesh of workload pairs
func (sm *SysModel) WorkloadPairs() *objects.WorkloadPairCollection {
	collection := objects.WorkloadPairCollection{}
	var wrklds []*objects.Workload
	for _, w := range sm.WorkloadsObjs {
		wrklds = append(wrklds, w)
	}
	for i, wf := range wrklds {
		for j := i + 1; j < len(wrklds); j++ {
			ws := wrklds[j]
			pair := objects.WorkloadPair{
				First:  wf,
				Second: ws,
			}
			collection.Pairs = append(collection.Pairs, &pair)
		}
	}

	return &collection
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

	// check venice cluster status
	cl, err := sm.GetCluster()
	if err != nil {
		log.Errorf("Could not get Venice cluster state: %v", err)
		return err
	}

	log.Debugf("Got Cluster: %+v", cl)

	// in mock mode, skip node level checks
	if sm.Tb.IsMockMode() {
		return nil
	}

	if cl.Status.Leader == "" {
		err := fmt.Errorf("Invalid cluster state. No leader")
		log.Errorf("%v", err)
		return err
	}

	foundLeader := false
	for _, vn := range sm.veniceNodes {
		if vn.IP() == cl.Status.Leader {
			foundLeader = true
		}
	}
	if !foundLeader {
		err := fmt.Errorf("Invalid leader node %v", cl.Status.Leader)
		log.Errorf("%v", err)
		return err
	}

	// verify each venice node status
	for _, node := range sm.veniceNodes {
		vn, err := sm.GetVeniceNode(node.IP())
		if err != nil {
			log.Errorf("Error getting venice node %s. Err: %v", node.IP(), err)
			return err
		}

		log.Debugf("Got venice node object: %+v", vn)

		if vn.Status.Phase != cluster.NodeStatus_JOINED.String() {
			err := fmt.Errorf("Invalid node status for %v. Status: %+v", node.Name(), vn.Status)
			log.Errorf("%v", err)
			return err
		}
		if vn.Status.Quorum != true {
			err := fmt.Errorf("Invalid node status for %v. Status: %+v", node.Name(), vn.Status)
			log.Errorf("%v", err)
			return err
		}
	}

	// verify each naples health
	for _, np := range sm.NaplesNodes {
		// check naples status
		err = sm.CheckNaplesHealth(np)
		if err != nil {
			log.Errorf("Naples health check failed. Err: %v", err)
			return err
		}

		// skip further checks in mock mode
		if sm.Tb.IsMockMode() {
			continue
		}

		// check smartnic status in Venice
		snic, err := sm.GetSmartNICByName(np.Name())
		if err != nil {
			err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", np.Name(), err)
			log.Errorf("%v", err)
			return err
		}

		log.Debugf("Got smartnic object: %+v", snic)

		if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", np.Name(), snic.Status)
		}
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

	// verify each naples health
	snics, err := sm.ListSmartNIC()
	if err != nil {
		log.Errorf("Unable to list dsc")
		return err
	}

	for _, snic := range snics {
		if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", snic.Spec.GetID(), snic.Status)
		}
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

	// check venice service status
	_, err = sm.CheckVeniceServiceStatus(cl.Status.Leader)
	if err != nil {
		log.Errorf("Checking venice service status failed : %v", err)
		return err
	}

	// check health of citadel
	err = sm.CheckCitadelServiceStatus()
	if err != nil {
		log.Errorf("Checking venice citadel status failed : %v", err)
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

// GetVeniceServices retrieves the state of the services from the leader node
func (sm *SysModel) GetVeniceServices() (string, error) {
	// check iota cluster health
	err := sm.Tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Invalid Iota cluster state: %v", err)
		return "", err
	}

	// check venice cluster status
	cl, err := sm.GetCluster()
	if err != nil {
		log.Errorf("Could not get Venice cluster state: %v", err)
		return "", err
	}
	return sm.CheckVeniceServiceStatus(cl.Status.Leader)
}

// VerifyPolicyStatus verifies SG policy status
func (sm *SysModel) VerifyPolicyStatus(spc *objects.NetworkSecurityPolicyCollection) error {
	if len(sm.NaplesNodes) == 0 {
		return nil
	}

	for _, pol := range spc.Policies {
		pstat, err := sm.GetNetworkSecurityPolicy(&pol.VenicePolicy.ObjectMeta)
		if err != nil {
			log.Errorf("Error getting SG policy %+v. Err: %v", pol.VenicePolicy.ObjectMeta, err)
			return err
		}

		// verify policy status
		if pstat.Status.PropagationStatus.GenerationID != pstat.ObjectMeta.GenerationID {
			log.Warnf("Propagation generation id did not match: Meta: %+v, PropagationStatus: %+v", pstat.ObjectMeta, pstat.Status.PropagationStatus)
			return fmt.Errorf("Propagation generation id did not match")
		}
		if (pstat.Status.PropagationStatus.Updated != (int32(len(sm.NaplesNodes)) + int32(len(sm.fakeNaples)))) || (pstat.Status.PropagationStatus.Pending != 0) {
			log.Warnf("Propagation status incorrect: Meta: %+v, PropagationStatus: %+v", pstat.ObjectMeta, pstat.Status.PropagationStatus)
			return fmt.Errorf("Propagation status was incorrect")
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

	// verify workload status is good
	if sm.Tb.HasNaplesHW() {
		err = sm.VerifyWorkloadStatus(sm.Workloads())
		if err != nil {
			if collectLogOnErr {
				sm.CollectLogs()
			}
			return err
		}
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
