// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
)

// ActionCtx is the internal state of the actions
type ActionCtx struct {
	err   error
	model *SysModel
}

// Action returns an interface of actions
func (sm *SysModel) Action() *ActionCtx {
	return &ActionCtx{model: sm}
}

// VerifyClusterStatus verifies venice cluster status
func (act *ActionCtx) VerifyClusterStatus() error {
	log.Infof("Verifying cluster health..")

	// check iota cluster health
	err := act.model.tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Invalid Iota cluster state: %v", err)
		return err
	}

	// check venice cluster status
	cl, err := act.model.tb.GetCluster()
	if err != nil {
		log.Errorf("Could not get Venice cluster state: %v", err)
		return err
	}

	log.Debugf("Got Cluster: %+v", cl)

	// in mock mode, skip node level checks
	if act.model.tb.mockMode {
		return nil
	}

	if cl.Status.Leader == "" {
		err := fmt.Errorf("Invalid cluster state. No leader")
		log.Errorf("%v", err)
		return err
	}

	foundLeader := false
	for _, vn := range act.model.veniceNodes {
		if vn.iotaNode.IpAddress == cl.Status.Leader {
			foundLeader = true
		}
	}
	if !foundLeader {
		err := fmt.Errorf("Invalid leader node %v", cl.Status.Leader)
		log.Errorf("%v", err)
		return err
	}

	// verify each venice node status
	for _, node := range act.model.veniceNodes {
		vn, err := act.model.tb.GetVeniceNode(node.iotaNode.IpAddress)
		if err != nil {
			log.Errorf("Error getting venice node %s. Err: %v", node.iotaNode.IpAddress, err)
			return err
		}

		log.Debugf("Got venice node object: %+v", vn)

		if vn.Status.Phase != cluster.NodeStatus_JOINED.String() {
			err := fmt.Errorf("Invalid node status for %v. Status: %+v", node.iotaNode.Name, vn.Status)
			log.Errorf("%v", err)
			return err
		}
		if vn.Status.Quorum != true {
			err := fmt.Errorf("Invalid node status for %v. Status: %+v", node.iotaNode.Name, vn.Status)
			log.Errorf("%v", err)
			return err
		}
	}

	// verify each naples health
	for _, np := range act.model.naples {
		// check naples status
		err = act.model.tb.CheckNaplesHealth(np)
		if err != nil {
			log.Errorf("Naples health check failed. Err: %v", err)
			return err
		}

		// skip further checks in mock mode
		if act.model.tb.mockMode {
			continue
		}

		// check smartnic status in Venice
		snic, err := act.model.tb.GetSmartNICByName(np.iotaNode.Name)
		if err != nil {
			err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", np.iotaNode.Name, err)
			log.Errorf("%v", err)
			return err
		}

		log.Debugf("Got smartnic object: %+v", snic)

		if snic.Status.AdmissionPhase != cluster.SmartNICStatus_ADMITTED.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", np.iotaNode.Name, snic.Status)
		}
		if len(snic.Status.Conditions) < 1 {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("No naples status reported for naples %v", np.iotaNode.Name)
		}
		if snic.Status.Conditions[0].Type != cluster.SmartNICCondition_HEALTHY.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status condition-type %v for naples %v", snic.Status.Conditions[0].Type, np.iotaNode.Name)
		}
		if snic.Status.Conditions[0].Status != cluster.ConditionStatus_TRUE.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status %v for naples %v", snic.Status.Conditions[0].Status, np.iotaNode.Name)
		}
	}

	// check venice service status
	_, err = act.model.tb.CheckVeniceServiceStatus(cl.Status.Leader)
	if err != nil {
		return err
	}

	return nil
}

// GetVeniceServices retrieves the state of the services from the leader node
func (act *ActionCtx) GetVeniceServices() (string, error) {
	// check iota cluster health
	err := act.model.tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Invalid Iota cluster state: %v", err)
		return "", err
	}

	// check venice cluster status
	cl, err := act.model.tb.GetCluster()
	if err != nil {
		log.Errorf("Could not get Venice cluster state: %v", err)
		return "", err
	}
	return act.model.tb.CheckVeniceServiceStatus(cl.Status.Leader)
}

// VerifyPolicyStatus verifies SG policy status
func (act *ActionCtx) VerifyPolicyStatus(spc *SGPolicyCollection) error {
	if spc.err != nil {
		return spc.err
	}
	if len(act.model.naples) == 0 {
		return nil
	}

	for _, pol := range spc.policies {
		pstat, err := act.model.tb.GetSGPolicy(&pol.venicePolicy.ObjectMeta)
		if err != nil {
			log.Errorf("Error getting SG policy %+v. Err: %v", pol.venicePolicy.ObjectMeta, err)
			return err
		}

		// verify policy status
		if pstat.Status.PropagationStatus.GenerationID != pstat.ObjectMeta.GenerationID {
			log.Warnf("Propagation generation id did not match: Meta: %+v, PropagationStatus: %+v", pstat.ObjectMeta, pstat.Status.PropagationStatus)
			return fmt.Errorf("Propagation generation id did not match")
		}
		if (pstat.Status.PropagationStatus.Updated != (int32(len(act.model.naples)) + int32(len(act.model.fakeNaples)))) || (pstat.Status.PropagationStatus.Pending != 0) {
			log.Warnf("Propagation status incorrect: Meta: %+v, PropagationStatus: %+v", pstat.ObjectMeta, pstat.Status.PropagationStatus)
			return fmt.Errorf("Propagation status was incorrect")
		}
	}

	return nil
}

// VerifySystemHealth checks all aspects of system, like cluster, workload, policies etc
func (act *ActionCtx) VerifySystemHealth(collectLogOnErr bool) error {
	const numRetries = 10
	// verify cluster is in good health
	err := act.VerifyClusterStatus()
	if err != nil {
		if collectLogOnErr {
			act.model.tb.CollectLogs()
		}
		return err
	}

	// verify workload status is good
	err = act.VerifyWorkloadStatus(act.model.Workloads())
	if err != nil {
		if collectLogOnErr {
			act.model.tb.CollectLogs()
		}
		return err
	}

	// verify policy status is good
	for i := 0; i < numRetries; i++ {
		err = act.VerifyPolicyStatus(act.model.SGPolicies())
		if err == nil {
			break
		}
		time.Sleep(time.Second)
	}
	if err != nil {
		if collectLogOnErr {
			act.model.tb.CollectLogs()
		}
		return err
	}

	// verify ping is successful across all workloads
	for i := 0; i < numRetries; i++ {
		err = act.PingPairs(act.model.WorkloadPairs().WithinNetwork())
		if err == nil {
			break
		}
	}
	if err != nil {
		if collectLogOnErr {
			act.model.tb.CollectLogs()
		}
		return err
	}

	return nil
}
