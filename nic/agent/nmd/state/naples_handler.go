// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/certsproxy"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	// ConfigURL is URL to configure a nic in classic mode
	ConfigURL = "/api/v1/naples/"
	// MonitoringURL is URL to fetch logs and other diags from nic in classic mode
	MonitoringURL = "/monitoring/v1/naples/"
	// CoresURL is URL to fetch cores from nic in classic mode
	CoresURL = "/cores/v1/naples/"
	// CmdEXECUrl is URL to fetch output from running executables on Naples in classic mode
	CmdEXECUrl = "/cmd/v1/naples/"
	// UpdateURL is the URL to help with file upload
	UpdateURL = "/update/"

	// Max retry interval in seconds for Registration retries
	// Retry interval is initially exponential and is capped
	// at 30min.
	nicRegMaxInterval = (30 * 60 * time.Second)
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

		case nmd.MgmtMode_HOST:
			n.StopManagedMode()
			n.StartClassicMode()

		case nmd.MgmtMode_NETWORK:

			// TODO : We need to stop rest server only
			// after NIC registration succeeds. (either
			// ADMITTED or PENDING for manual approval)
			// Also some tests needs to be refactored
			// before stopping the classic mode with
			// flag=true to stop rest server, since the
			// tests use the REST api to change mode from
			// managed -> classic. Managed to classic mode
			// change should be done by update SmartNIC object
			// on Venice and NMD should react to it via watcher
			// update.
			n.StopClassicMode(false)

			n.Add(1)
			go func() {
				defer n.Done()
				n.StartManagedMode()
			}()

		}
	}
	log.Infof("NIC mode: %v change completed err: %v", n.config.Spec.Mode, err)
	n.UpdateMgmtIP()

	return err
}

// StartManagedMode starts the tasks required for managed mode
func (n *NMD) StartManagedMode() error {
	// Set Registration in progress flag
	log.Infof("NIC in managed mode, mac: %v", n.config.Spec.PrimaryMAC)
	n.setRegStatus(true)

	err := n.initTLSProvider()
	if err != nil {
		return fmt.Errorf("Error initializing TLS provider: %v", err)
	}

	for {
		select {

		// Check if registration loop should be stopped
		case <-n.stopNICReg:

			log.Infof("Registration stopped, exiting.")

			// Clear Registration in progress flag
			n.setRegStatus(false)
			return nil

		// Register NIC
		case <-time.After(n.nicRegInterval):

			// For the NIC in Naples Config, start the registration
			mac := n.config.Spec.PrimaryMAC
			name := mac

			nicObj, _ := n.GetSmartNIC()

			if nicObj != nil {

				// Update smartNIC object
				nicObj.TypeMeta.Kind = "SmartNIC"
				nicObj.ObjectMeta.Name = name
				nicObj.Spec.IPConfig = n.config.Spec.IPConfig
				nicObj.Spec.Hostname = n.config.Spec.Hostname
				nicObj.Spec.MgmtMode = cmd.SmartNICSpec_NETWORK.String()
				nicObj.Spec.MgmtVlan = n.config.Spec.MgmtVlan
				nicObj.Spec.Controllers = n.config.Spec.Controllers
				nicObj.Status.PrimaryMAC = mac
				nicObj.Status.AdmissionPhase = cmd.SmartNICStatus_REGISTERING.String()
			} else {

				// Construct new smartNIC object
				nicObj = &cmd.SmartNIC{
					TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
					ObjectMeta: api.ObjectMeta{
						Name: name,
					},
					Spec: cmd.SmartNICSpec{
						Hostname:    n.config.Spec.Hostname,
						IPConfig:    n.config.Spec.IPConfig,
						MgmtMode:    cmd.SmartNICSpec_NETWORK.String(),
						NetworkMode: cmd.SmartNICSpec_OOB.String(),
						MgmtVlan:    n.config.Spec.MgmtVlan,
						Controllers: n.config.Spec.Controllers,
					},
					// TODO: get these from platform
					Status: cmd.SmartNICStatus{
						AdmissionPhase: cmd.SmartNICStatus_REGISTERING.String(),
						SerialNum:      "0x0123456789ABCDEFghijk",
						PrimaryMAC:     mac,
					},
				}
			}

			// Send NIC register request to CMD
			log.Infof("Registering NIC with CMD : %+v", nicObj)
			msg, err := n.RegisterSmartNICReq(nicObj)

			// Cache it in nicDB
			if msg.AdmissionResponse != nil {
				nicObj.Status.AdmissionPhase = msg.AdmissionResponse.Phase
			}
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
				resp := msg.AdmissionResponse
				if resp == nil {
					log.Errorf("Protocol error: no AdmissionResponse in message, mac: %s", mac)
				}
				log.Infof("Received register response: %+v", resp)
				switch resp.Phase {

				case cmd.SmartNICStatus_REJECTED.String():

					// Rule #2 - abort retry, clear registration status flag
					log.Errorf("Invalid NIC, Admission rejected, mac: %s reason: %s", mac, resp.Reason)
					n.setRegStatus(false)
					return err

				case cmd.SmartNICStatus_PENDING.String():

					// Rule #3 - needs slower exponential retry
					// Cap the retry interval at 30mins
					if 2*n.nicRegInterval <= nicRegMaxInterval {
						n.nicRegInterval = 2 * n.nicRegInterval
					} else {
						n.nicRegInterval = nicRegMaxInterval
					}

					log.Infof("NIC waiting for manual approval of admission into cluster, mac: %s reason: %s",
						mac, resp.Reason)

				case cmd.SmartNICStatus_ADMITTED.String():

					// Rule #4 - registration is success, clear registration status
					// and move on to next stage
					log.Infof("NIC admitted into cluster, mac: %s", mac)
					n.setRegStatus(false)

					err = n.setClusterCredentials(resp)
					if err != nil {
						log.Errorf("Error processing cluster credentials: %v", err)
					}

					// start watching objects
					go n.cmd.WatchSmartNICUpdates()
					go n.rollout.WatchSmartNICRolloutUpdates()

					// Start certificates proxy
					if n.certsListenURL != "" {
						certsProxy, err := certsproxy.NewCertsProxy(n.certsListenURL, n.remoteCertsURL,
							rpckit.WithTLSProvider(n.tlsProvider), rpckit.WithRemoteServerName(globals.Cmd))
						if err != nil {
							log.Errorf("Error starting certificates proxy at %s: %v", n.certsListenURL, err)
							// still try to proceed
						} else {
							log.Infof("Started certificates proxy at %s, forwarding to: %s", n.certsListenURL, n.remoteCertsURL)
						}
						n.certsProxy = certsProxy
						n.certsProxy.Start()
					}

					// Start goroutine to send periodic NIC updates
					n.Add(1)
					go func() {
						defer n.Done()
						n.SendNICUpdates()
					}()

					return nil

				case cmd.SmartNICStatus_UNKNOWN.String():

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
		case <-time.After(n.nicUpdInterval):
			nicObj := n.nic

			// Skip until NIC is admitted
			if nicObj.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
				log.Infof("Skipping health update, phase %v", nicObj.Status.AdmissionPhase)
				continue
			}

			// TODO : Get status from platform and fill nic Status
			nicObj.Status = cmd.SmartNICStatus{
				AdmissionPhase: cmd.SmartNICStatus_ADMITTED.String(),
				Conditions: []cmd.SmartNICCondition{
					{
						Type:               cmd.SmartNICCondition_HEALTHY.String(),
						Status:             cmd.ConditionStatus_TRUE.String(),
						LastTransitionTime: time.Now().Format(time.RFC3339),
					},
				},
			}

			// Send nic status
			log.Infof("Sending NIC health update: %+v", nicObj)
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
	// stop accepting certificate requests
	if n.certsProxy != nil {
		n.certsProxy.Stop()
		n.certsProxy = nil
	}
	// stop ongoing NIC registration, if any
	close(n.stopNICReg)
	// stop ongoing NIC updates, if any
	close(n.stopNICUpd)

	// Wait for goroutines launched in managed mode
	// to complete
	n.Wait()

	// release TLS provider resources
	if n.tlsProvider != nil {
		n.tlsProvider.Close()
		n.tlsProvider = nil
	}

	return nil
}

// StartClassicMode start the tasks required for classic mode
func (n *NMD) StartClassicMode() error {

	// Start RestServer
	log.Infof("NIC in classic mode, mac: %v", n.config.Spec.PrimaryMAC)

	return n.StartRestServer()
}

// StopClassicMode stops the ongoing tasks meant for classic mode
func (n *NMD) StopClassicMode(shutdown bool) error {

	// Stop RestServer
	return n.StopRestServer(shutdown)
}

// GetPlatformCertificate returns the certificate containing the NIC identity and public key
func (n *NMD) GetPlatformCertificate(nic *cmd.SmartNIC) ([]byte, error) {
	return n.platform.GetPlatformCertificate(nic)
}

// GenChallengeResponse returns the response to a challenge issued by CMD to authenticate this NAPLES
func (n *NMD) GenChallengeResponse(nic *cmd.SmartNIC, challenge []byte) ([]byte, []byte, error) {
	signer, err := n.platform.GetPlatformSigner(nic)
	if err != nil {
		return nil, nil, fmt.Errorf("Error getting platform signer: %v", err)
	}
	return certs.GeneratePoPChallengeResponse(signer, challenge)
}
