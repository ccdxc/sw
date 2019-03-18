// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
)

// RegisterSmartNICReq registers a NIC with CMD
func (n *NMD) RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error) {

	if n.cmd == nil {
		log.Errorf("Failed to register NIC, mac: %s cmd not ready", nic.ObjectMeta.Name)
		return grpc.RegisterNICResponse{}, errors.New("cmd not ready yet")
	}
	resp, err := n.cmd.RegisterSmartNICReq(nic)
	if err != nil {
		log.Errorf("Failed to register NIC, mac: %s err: %v", nic.ObjectMeta.Name, err)
		return resp, err
	}

	log.Infof("Register NIC response mac: %s response: %v", nic.ObjectMeta.Name, resp)
	recorder.Event(cmd.NICAdmitted, evtsapi.SeverityLevel_INFO, fmt.Sprintf("Smart NIC %s admitted to the cluster", nic.GetName()), nic)
	return resp, nil
}

// UpdateSmartNICReq registers a NIC with CMD/Venice cluster
func (n *NMD) UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error) {

	if n.cmd == nil {
		log.Errorf("Failed to update NIC, mac: %s cmd not ready", nic.ObjectMeta.Name)
		return nil, errors.New("cmd not ready yet")
	}
	nicObj, err := n.cmd.UpdateSmartNICReq(nic)
	if err != nil || nic == nil {
		log.Errorf("Failed to update NIC, mac: %s err: %v", nic.ObjectMeta.Name, err)
		return nil, err
	}

	log.Infof("Update NIC response mac: %s nic: %+v", nic.ObjectMeta.Name, nicObj)
	return nicObj, nil
}

// CreateSmartNIC creates a local smartNIC object
func (n *NMD) CreateSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC create, mac: %s", nic.ObjectMeta.Name)

	// When we re-connect to CMD we may get "create" notifications for the NIC.
	// In that case  we need to treat the notification as an update, because
	// (instead of just overriding what we have) because otherwise we may miss
	// a transition.
	if n.nic == nil {
		// add the nic to database
		n.SetSmartNIC(nic)
		err := n.store.Write(nic)
		return err
	}
	return n.UpdateSmartNIC(nic)
}

// UpdateSmartNIC updates the local smartNIC object
// Only meant to be called when receiving events from SmartNIC watcher
func (n *NMD) UpdateSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC update, mac: %s, new phase: %s", nic.ObjectMeta.Name, nic.Status.AdmissionPhase)

	// get current state from db
	oldNic, err := n.GetSmartNIC()
	if err != nil {
		log.Errorf("Error retrieving state for nic %+v: %v", nic, err)
	}

	// update nic in the DB
	n.SetSmartNIC(nic)
	err = n.store.Write(nic)
	if err != nil {
		log.Errorf("Error updating NMD state %+v: %v", nic, err)
	}

	// We need to check the old phase because we may receive multiple notifications
	// from CMD with phase = pending before we actually shut down the updates channel
	// and we don't want to do a stop/start for each of them.
	if oldNic != nil && oldNic.Status.AdmissionPhase == cmd.SmartNICStatus_ADMITTED.String() &&
		nic.Status.AdmissionPhase == cmd.SmartNICStatus_PENDING.String() {
		log.Infof("SmartNIC %s has been de-admitted from cluster", nic.ObjectMeta.Name)
		// NIC has been de-admitted by user. Stop and restart managed mode.
		// This will stop health updates and start registration attempts.
		go func() {
			// this will spawn a goroutine that will wait for cleanup to finish and then restart managaed mode
			// it has to be done in a separate goroutine because this code is executing in the context of the
			// watcher and the watcher has to terminate for the cleanup to be complete
			err = n.StopManagedMode()
			if err != nil {
				log.Errorf("Error stopping NIC managed mode: %v", err)
			}
			n.Add(1)
			defer n.Done()
			err = n.StartManagedMode()
			if err != nil {
				log.Errorf("Error starting NIC managed mode: %v", err)
			}
		}()
	}

	return err
}

// DeleteSmartNIC deletes the local smartNIC object
func (n *NMD) DeleteSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC delete, mac: %s", nic.ObjectMeta.Name)

	// remove nic from DB
	n.SetSmartNIC(nil)
	err := n.store.Delete(nic)

	return err
}
