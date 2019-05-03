// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
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
	if n.config.Status.AdmissionPhase == cmd.SmartNICStatus_ADMITTED.String() {
		recorder.Event(eventtypes.NIC_ADMITTED, fmt.Sprintf("Smart NIC %s admitted to the cluster", nic.GetName()), nic)
	}
	return resp, nil
}

// UpdateSmartNICReq registers a NIC with CMD/Venice cluster
func (n *NMD) UpdateSmartNICReq(nic *cmd.SmartNIC) error {

	if n.cmd == nil {
		log.Errorf("Failed to update NIC, mac: %s cmd not ready", nic.ObjectMeta.Name)
		return errors.New("cmd not ready yet")
	}
	err := n.cmd.UpdateSmartNICReq(nic)
	if err != nil || nic == nil {
		log.Errorf("Failed to update NIC, mac: %s err: %v", nic.ObjectMeta.Name, err)
		return err
	}

	log.Infof("Update NIC response mac: %s", nic.ObjectMeta.Name)
	return nil
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

	log.Infof("SmartNIC update, mac: %s, phase: %s, mgmt mode: %s", nic.ObjectMeta.Name, nic.Status.AdmissionPhase, nic.Spec.MgmtMode)

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

	// Handle de-admission and decommission

	// We need to check the old spec and status values because we may receive multiple
	// notifications from CMD before we actually shut down the updates channel and we
	// don't want to do react to each of them.
	if oldNic != nil {
		decommission := oldNic.Spec.MgmtMode == cmd.SmartNICSpec_NETWORK.String() &&
			nic.Spec.MgmtMode == cmd.SmartNICSpec_HOST.String()

		deAdmission := oldNic.Status.AdmissionPhase == cmd.SmartNICStatus_ADMITTED.String() &&
			nic.Status.AdmissionPhase == cmd.SmartNICStatus_PENDING.String()

		if decommission || deAdmission {
			// Spawn a goroutine that will wait for cleanup to finish and then restart managed or classic mode.
			// This has to be done in a separate goroutine because this function is executing in the context of
			// the watcher and the watcher has to terminate for the cleanup to be complete
			go func() {
				err = n.StopManagedMode()
				if err != nil {
					log.Errorf("Error stopping NIC managed mode: %v", err)
				}
				n.Add(1)
				defer n.Done()

				if decommission {
					// NIC has been decommissioned by user. Go back to classic mode.
					log.Infof("SmartNIC %s has been decommissioned, triggering change to HOST managed mode", nic.ObjectMeta.Name)
					// Update config status to reflect the mode change
					n.config.Spec.Mode = nmd.MgmtMode_HOST.String()
					n.config.Status.Mode = nmd.MgmtMode_HOST.String()
					n.config.Status.AdmissionPhase = cmd.SmartNICStatus_DECOMMISSIONED.String()
					n.config.Status.AdmissionPhaseReason = "SmartNIC management mode changed to HOST"
					err = n.StartClassicMode()
					if err != nil {
						log.Errorf("Error starting NIC managed mode: %v", err)
					}
				} else {
					// NIC has been de-admitted by user. Cleanup and restart managed mode.
					// This will stop health updates and start registration attempts.
					log.Infof("SmartNIC %s has been de-admitted from cluster", nic.ObjectMeta.Name)
					err = n.StartManagedMode()
					if err != nil {
						log.Errorf("Error starting NIC managed mode: %v", err)
					}
				}
			}()
		}
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
