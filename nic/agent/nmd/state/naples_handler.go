// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"time"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// ConfigURL in URL to configure a nic in classic mode
	ConfigURL = "/api/v1/naples/"

	// Max retry interval in seconds for Registration retries
	// Retry interval is initially exponential and is capped
	// at 30min.
	nicRegMaxInterval = (30 * 60)
)

// UpdateNaplesConfig updates a local Naples Config object
func (n *NMD) UpdateNaplesConfig(cfg nmd.Naples) error {

	log.Infof("NIC mode desired: %v", cfg.Spec.Mode)

	// Detect if there is a mode change
	var modeChanged bool
	config := n.GetNaplesConfig()
	if config.Spec.Mode != cfg.Spec.Mode {
		modeChanged = true
	}

	log.Infof("Modechange: %v old-mode: %v", modeChanged, config.Spec.Mode)

	// Update nic config in the DB
	n.setNaplesConfig(cfg)
	err := n.store.Write(&cfg)

	// Handle mode change
	if modeChanged {

		// Handle the new mode
		switch cfg.Spec.Mode {

		case nmd.NaplesMode_CLASSIC_MODE:
			n.StopManagedMode()
			n.StartClassicMode()

		case nmd.NaplesMode_MANAGED_MODE:
			n.StopClassicMode(false)

			n.Add(1)
			go func() {
				defer n.Done()
				n.StartManagedMode()
			}()

		}
	}
	log.Infof("NIC mode: %v change completed err: %v", n.config.Spec.Mode, err)

	return err
}

// StartManagedMode starts the tasks required for managed mode
func (n *NMD) StartManagedMode() error {

	// Set Registration in progress flag
	log.Infof("NIC in managed mode, mac: %v", n.config.Spec.PrimaryMac)
	n.setRegStatus(true)
	for {
		select {

		// Check if registration loop should be stopped
		case <-n.stopNICReg:

			log.Infof("Registration stopped, exiting.")

			// Clear Registration in progress flag
			n.setRegStatus(false)
			return nil

		// Register NIC
		case <-time.After(n.nicRegInterval * time.Second):

			// For the NIC in Naples Config, start the registration
			mac := n.config.Spec.PrimaryMac

			nicObj, _ := n.GetSmartNIC()

			if nicObj != nil {

				// Update smartNIC object
				nicObj.TypeMeta.Kind = "SmartNIC"
				nicObj.ObjectMeta.Name = mac
				nicObj.Spec.Phase = cmd.SmartNICSpec_REGISTERING.String()
				nicObj.Spec.MgmtIp = n.config.Spec.MgmtIp
				nicObj.Spec.NodeName = n.config.Spec.NodeName

			} else {

				// Construct new smartNIC object
				nicObj = &cmd.SmartNIC{
					TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
					ObjectMeta: api.ObjectMeta{Name: mac},
					Spec: cmd.SmartNICSpec{
						Phase:  cmd.SmartNICSpec_REGISTERING.String(),
						MgmtIp: n.config.Spec.MgmtIp,
						// TODO: get mgmt ip from platform
						NodeName: n.config.Spec.NodeName,
					},
				}
			}
			if nicObj.Spec.MgmtIp == "" {
				nicObj.Spec.MgmtIp = "0.0.0.0"
			}

			// Send NIC register request to CMD
			log.Infof("++++ Registering NIC with CMD : %+v", nicObj)
			resp, err := n.RegisterSmartNICReq(nicObj)

			// Cache it in nicDB
			nicObj.Spec.Phase = resp.Phase
			n.SetSmartNIC(nicObj)

			// Error and Phase response is handled according to the following rules.
			//
			// 1. If there are RPC errors (in connecting to CMD) we should retry at
			//    regular interval.
			// 2. If the factory cert is invalid, phase is REJECTED and reason indicates
			//    why it is rejected. In this case, there is no retry done.
			// 3. If the phase is PENDING, it indicates that the certificate is valid, but it
			//    is either not auto-admitted or not manually approved. In such cases
			//    the retry is done at exponential interval and capped at 30min retry.
			// 4. If is the phase is ADMITTED, move on to next stage of sending periodic
			//    NIC updates.
			//
			if err != nil {

				// Rule #1 - continue retry at regular interval
				log.Errorf("Error registering nic, mac: %s err: %+v", mac, err)
			} else {

				log.Infof("Received register response: %+v", resp)
				switch resp.Phase {

				case cmd.SmartNICSpec_REJECTED.String():

					// Rule #2 - abort retry, clear registration status flag
					log.Errorf("Invalid NIC, Admission rejected, mac: %s reason: %s", mac, resp.Reason)
					n.setRegStatus(false)
					return err

				case cmd.SmartNICSpec_PENDING.String():

					// Rule #3 - needs slower exponential retry
					// Cap the retry interval at 30mins
					if 2*n.nicRegInterval <= nicRegMaxInterval {
						n.nicRegInterval = 2 * n.nicRegInterval
					} else {
						n.nicRegInterval = nicRegMaxInterval
					}

					log.Infof("NIC waiting for manual approval of admission into cluster, mac: %s reason: %s",
						mac, resp.Reason)

				case cmd.SmartNICSpec_ADMITTED.String():

					// Rule #4 - registration is success, clear registration status
					// and move on to next stage
					log.Infof("NIC admitted into cluster, mac: %s", mac)
					n.setRegStatus(false)

					// Start goroutine to send periodic NIC updates
					n.Add(1)
					go func() {
						defer n.Done()
						n.SendNICUpdates()
					}()

					return nil

				case cmd.SmartNICSpec_UNKNOWN.String():

					// Not an expected response
					log.Fatalf("Unknown response, nic: %+v phase: %v", nicObj, resp)
				}
			}
		}
	}
}

// SendNICUpdates sends periodic updates about NIC to CMD
func (n *NMD) SendNICUpdates() error {

	n.setUpdStatus(true)
	for {
		select {

		// Check if NICUpdate loop should be stopped
		case <-n.stopNICUpd:

			log.Infof("NICUpdate stopped, exiting.")
			n.setUpdStatus(false)
			return nil

		// NIC update timer callback
		case <-time.After(n.nicUpdInterval * time.Second):

			nicObj := n.nic

			// Skip until NIC is admitted
			if nicObj.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
				continue
			}

			// TODO : Get status from platform and fill nic Status
			nicObj.Status = cmd.SmartNICStatus{
				Conditions: []*cmd.SmartNICCondition{
					{
						Type:               cmd.SmartNICCondition_HEALTHY.String(),
						Status:             cmd.ConditionStatus_TRUE.String(),
						LastTransitionTime: time.Now().Format(time.RFC3339),
					},
				},
			}

			// Send nic status
			log.Info("++++ Sending NIC health update: %+v", nicObj)
			_, err := n.UpdateSmartNICReq(nicObj)
			if err != nil {
				log.Errorf("Error updating nic, name:%s  err: %+v",
					nicObj.Name, err)
			}

		}
	}
}

// StopManagedMode stop the ongoing tasks meant for managed mode
func (n *NMD) StopManagedMode() error {

	// stop ongoing NIC registration, if any
	close(n.stopNICReg)
	// stop ongoing NIC updates, if any
	close(n.stopNICUpd)

	// Wait for goroutines launched in managed mode
	// to complete
	n.Wait()

	return nil
}

// StartClassicMode start the tasks required for classic mode
func (n *NMD) StartClassicMode() error {

	// Start RestServer
	log.Infof("NIC in classic mode, mac: %v", n.config.Spec.PrimaryMac)

	return n.StartRestServer()
}

// StopClassicMode stops the ongoing tasks meant for classic mode
func (n *NMD) StopClassicMode(shutdown bool) error {

	// Stop RestServer
	return n.StopRestServer(shutdown)
}
