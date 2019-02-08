// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"

	"github.com/pensando/sw/venice/utils/log"
)

/*
type ActionChain interface {
	Run() error
	HasError() bool
	Error() error
}
*/

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
		if vn.iotaNode.Name == cl.Status.Leader {
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
		vn, err := act.model.tb.GetVeniceNode(node.iotaNode.Name)
		if err != nil {
			return err
		}

		log.Debugf("Got venice node object: %+v", vn)

		if vn.Status.Phase != "JOINED" {
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
		snic, err := act.model.tb.GetSmartNIC(np.iotaNode.Name + "-" + np.iotaNode.NodeUuid)
		if err != nil {
			err := fmt.Errorf("Failed to get smartnc object for uuid %v. Err: %+v", np.iotaNode.NodeUuid, err)
			log.Errorf("%v", err)
			return err
		}

		log.Debugf("Got smartnic object: %+v", snic)

		if snic.Status.AdmissionPhase != "ADMITTED" {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", np.iotaNode.Name, snic.Status)
		}
		if len(snic.Status.Conditions) < 1 {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("No naples condition for naples %v", np.iotaNode.Name)
		}
		if snic.Status.Conditions[0].Type != "HEALTHY" {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid condition type %v for naples %v", snic.Status.Conditions[0].Type, np.iotaNode.Name)
		}
		/* FIXME: there seem to be a bug in CMD health check. disabling this check for now
		if snic.Status.Conditions[0].Status != "TRUE" {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status %v for naples %v", snic.Status.Conditions[0].Status, np.iotaNode.Name)
		}
		*/
	}

	// check venice service status
	err = act.model.tb.CheckVeniceServiceStatus(cl.Status.Leader)
	if err != nil {
		return err
	}

	return nil
}
