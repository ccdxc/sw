// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"encoding/json"
	"fmt"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	"github.com/pensando/sw/nic/agent/nmd/utils"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/certsproxy"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

const (
	// ConfigURL is URL to configure a nic
	ConfigURL = "/api/v1/naples/"
	// RolloutURL is URL to configure DSCRollout object
	RolloutURL = "/api/v1/naples/rollout/"
	// MonitoringURL is URL to fetch logs and other diags from nic
	MonitoringURL = "/monitoring/v1/naples/"
	// CoresURL is URL to fetch cores from nic
	CoresURL = "/cores/v1/naples/"
	// CmdEXECUrl is URL to fetch output from running executables on Naples
	CmdEXECUrl = "/cmd/v1/naples/"
	// UpdateURL is the URL to help with file upload
	UpdateURL = "/update/"
	// DataURL is the URL to help with copying out any files from /data
	DataURL = "/data/"

	// ProfileURL is the URL to create naples profiles
	ProfileURL = "/api/v1/naples/profiles/"

	// NaplesInfoURL is the URL to GET smartnic object
	NaplesInfoURL = "/api/v1/naples/info/"

	// NaplesVersionURL is the URL to GET all software version info from Naples
	NaplesVersionURL = "/api/v1/naples/version/"

	// Max retry interval in seconds for Registration retries
	// Retry interval is initially exponential and is capped
	// at 3 mins.
	nicRegMaxInterval = 3 * time.Minute
	// agentModeChangeURL is netagent's mode change URL
	agentModeChangeURL = "http://127.0.0.1:9007/api/mode/"
)

// CreateNaplesProfile creates a Naples Profile
func (n *NMD) CreateNaplesProfile(profile nmd.DSCProfile) error {
	log.Infof("Creating Naples Profile : %v", profile)
	if profile.Spec.DefaultPortAdmin != nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String() && profile.Spec.DefaultPortAdmin != nmd.PortAdminState_PORT_ADMIN_STATE_DISABLE.String() {
		log.Infof("Invalid port admin state set. Setting to default of Enabled.")
		profile.Spec.DefaultPortAdmin = nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String()
	}

	n.Lock()
	defer n.Unlock()
	// ensure the profile name is unique.
	for _, p := range n.profiles {
		if profile.Name == p.Name {
			return fmt.Errorf("profile %v already exists", profile.Name)
		}
	}

	c, _ := types.TimestampProto(time.Now())
	profile.CreationTime = api.Timestamp{
		Timestamp: *c,
	}
	profile.ModTime = api.Timestamp{
		Timestamp: *c,
	}

	// persist profile
	if err := n.store.Write(&profile); err != nil {
		log.Errorf("Failed to persist naples profile. Err: %v", err)
		return err
	}
	err := utils.BackupNMDDB()
	if err != nil {
		log.Errorf("Failed to backup nmd.db. Err : %v", err)
	}

	// Update in memory state
	n.profiles = append(n.profiles, &profile)
	return nil
}

func linkDown(intf string) error {
	log.Infof("Bringing link %v down.", intf)

	link, err := netlink.LinkByName(intf)
	if err != nil {
		log.Errorf("Failed to lookup interface %v. Err : %v", intf, err)
		return nil
	}

	return netlink.LinkSetDown(link)
}

func linkUp(intf string) error {
	log.Infof("Bringing link %v down.", intf)

	link, err := netlink.LinkByName(intf)
	if err != nil {
		log.Errorf("Failed to lookup interface %v. Err : %v", intf, err)
		return nil
	}

	return netlink.LinkSetUp(link)
}

func bringInbandUp() error {
	err := linkUp(ipif.NaplesInbandInterface)
	if err != nil {
		log.Errorf("Failed to bring %v up. Err : %v", ipif.NaplesInbandInterface, err)
		return err
	}

	err = linkUp(ipif.NaplesINB0Interface)
	if err != nil {
		log.Errorf("Failed to bring %v up. Err : %v", ipif.NaplesINB0Interface, err)
		return err
	}

	err = linkUp(ipif.NaplesINB1Interface)
	if err != nil {
		log.Errorf("Failed to bring %v up. Err : %v", ipif.NaplesINB1Interface, err)
		return err
	}

	return nil
}

func bringAllLinksDown() error {
	log.Infof("Bringing all links down.")
	err := linkDown(ipif.NaplesInbandInterface)
	if err != nil {
		log.Errorf("Failed to bring down %v. Err : %v", ipif.NaplesInbandInterface, err)
	}

	err = linkDown(ipif.NaplesINB0Interface)
	if err != nil {
		log.Errorf("Failed to bring down %v. Err : %v", ipif.NaplesINB0Interface, err)
	}

	err = linkDown(ipif.NaplesINB1Interface)
	if err != nil {
		log.Errorf("Failed to bring down %v. Err : %v", ipif.NaplesINB1Interface, err)
	}

	return nil
}

// UpdateNaplesConfig updates a local Naples Config object
func (n *NMD) UpdateNaplesConfig(cfg nmd.DistributedServiceCard) error {
	oldCfg, _ := json.Marshal(n.config)
	newCfg, _ := json.Marshal(cfg)
	log.Infof("NAPLES Update: Old: %s", string(oldCfg))
	log.Infof("NAPLES Update: New: %s", string(newCfg))

	// Ensure all links are brought down before performing any mode change operation
	if err := bringAllLinksDown(); err != nil {
		log.Errorf("Failed to bring all links down. Err:%v", err)
	}

	// Perform Mode Validations
	switch cfg.Spec.Mode {
	case nmd.MgmtMode_HOST.String():
		if err := isHostModeValid(cfg.Spec); err != nil {
			return errBadRequest(err)
		}

		// Update Spec
		n.SetNaplesConfig(cfg.Spec)

		if !isDataplaneClassic() {
			log.Info("Setting reboot needed flag")
			n.rebootNeeded = true
		} else {
			n.rebootNeeded = false
		}

		if err := n.handleHostModeTransition(); err != nil {
			return errInternalServer(err)
		}
		isEmulation := false
		if _, err := os.Stat(globals.IotaEmulation); err == nil {
			log.Infof("NMD running in Emulation mode as a real Venice controller is not available. Remove %v file if this was not desired.", globals.IotaEmulation)
			isEmulation = true
		}

		if err := n.PersistState(isEmulation); err != nil {
			return errInternalServer(err)
		}

	case nmd.MgmtMode_NETWORK.String():
		if err := isNetworkModeValid(cfg.Spec); err != nil {
			return errBadRequest(err)
		}

		if n.Pipeline != nil && n.Pipeline.GetPipelineType() == globals.NaplesPipelineApollo {
			// There is no mode change in apulu pipeline
			n.rebootNeeded = false
		} else {
			// Check if reboot is needed after the completion of the mode switch
			// Reboot is required for -
			// 1. When the dataplane is in Classic mode and the intention received from the user is to move to Network Mode
			// 2. When a user initiates a transition to NetworkMode - a user initiated action will have different timestamp than what is currently saved
			//    This is required, as many clients/libraries of other processes have the keys in-memory. A process restart, achieved through Naples Reboot,
			//    is necessary to ensure that these in-memory keys are flushed out.
			if isDataplaneClassic() || types.TimestampString(&n.config.CreationTime.Timestamp) != types.TimestampString(&cfg.CreationTime.Timestamp) {
				log.Info("Setting reboot needed flag")
				n.rebootNeeded = true

				// When moving from HOST->NETWORK, there are no Trust bundles saved
				// When moving from NETWORK->NETWORK, the action can only be issued by a trusted entity
				err := utils.ClearNaplesTrustRoots()
				if err != nil {
					log.Errorf("Failed to clear trust certs. Err : %v", err)
				}
			} else {
				// A reboot is not required only in case when at init time, the saved configs are replayed
				n.rebootNeeded = false
			}
		}

		// Update Spec
		n.SetNaplesConfig(cfg.Spec)

		if err := n.handleNetworkModeTransition(); err != nil {
			return errInternalServer(err)
		}
	default:
		log.Errorf("Invalid mode %v specified.", cfg.Spec.Mode)
		return errBadRequest(fmt.Errorf("invalid mode %v specified", cfg.Spec.Mode))
	}

	return nil
}

// PersistState persists the naples object in nmd.db, appropriate parameters in device.conf and sends notification
// about changes to other proceses by updating the NaplesStatus delphi object.
func (n *NMD) PersistState(updateDelphi bool) (err error) {
	// Persist BoltDB
	log.Info("Persisting state")
	if err = n.store.Write(&n.config); err != nil {
		err = fmt.Errorf("failed to persist naples config. Err: %v ", err)
		return
	}

	// Persist device files
	if n.Pipeline != nil && n.Pipeline.GetPipelineType() == globals.NaplesPipelineApollo {
		// Skip writing device files in case of apollo pipeline
	} else {
		if err = n.writeDeviceFiles(); err != nil {
			err = fmt.Errorf("failed to persist device config. Err: %v ", err)
			return
		}
	}
	n.config.Status.Mode = n.config.Spec.Mode

	var controllers []string
	var mgmtIntf string

	if n.config.Spec.NetworkMode == nmd.NetworkMode_INBAND.String() {
		mgmtIntf = ipif.NaplesInbandInterface
	} else if n.config.Spec.NetworkMode == nmd.NetworkMode_OOB.String() {
		mgmtIntf = ipif.NaplesOOBInterface
	}

	for _, c := range n.config.Status.Controllers {
		controllers = append(controllers, fmt.Sprintf("%s:%s", c, globals.CMDGRPCAuthPort))
	}

	statusObj := agentTypes.DistributedServiceCardStatus{
		DSCMode:     n.config.Status.Mode,
		DSCName:     n.config.Status.DSCName,
		MgmtIntf:    mgmtIntf,
		Controllers: controllers,
	}
	if n.config.Status.IPConfig != nil {
		statusObj.MgmtIP = n.config.Status.IPConfig.IPAddress
	}

	if n.config.Status.IPConfig != nil {
		statusObj.MgmtIP = n.config.Status.IPConfig.IPAddress
	}

	if len(n.DSCInterfaceIPs) != 0 {
		statusObj.DSCInterfaceIPs = n.DSCInterfaceIPs
	}

	if len(n.DSCStaticRoutes) != 0 {
		statusObj.DSCStaticRoutes = n.DSCStaticRoutes
	}

	var resp Response
	ticker := time.NewTicker(time.Minute)
	if err := netutils.HTTPPost(agentModeChangeURL, &statusObj, &resp); err != nil {
		go func() {
			for {
				select {
				case <-ticker.C:
					err := netutils.HTTPPost(agentModeChangeURL, &statusObj, &resp)
					if err != nil {
						log.Errorf("NetAgent failed to respond to mode change. Resp: %v | Err: %v", resp, err)
					} else {
						return
					}
				}
			}
		}()
	}

	if updateDelphi && n.Pipeline != nil {
		if err = n.Pipeline.WriteDelphiObjects(); err != nil {
			return err
		}
	}

	return
}

func (n *NMD) handleNetworkModeTransition() error {
	log.Info("Handling network mode transition")

	spec := n.config.Spec
	n.stateMachine = NewNMDStateMachine()
	if err := n.reconcileIPClient(); err != nil {
		return err
	}

	if spec.IPConfig != nil && len(spec.IPConfig.IPAddress) != 0 {
		// Static IP Config. Populate the status
		n.config.Status.Controllers = n.config.Spec.Controllers
		if err := n.stateMachine.FSM.Event("doStatic", n); err != nil {
			log.Errorf("Static mode transition event failed. Err: %v", err)
			return fmt.Errorf("static mode transition event failed. Err: %v", err)
		}
	} else {
		// Use statically passed controllers if any
		n.config.Status.Controllers = n.config.Spec.Controllers
		if err := n.stateMachine.FSM.Event("doDynamic", n); err != nil {
			log.Errorf("Dynamic mode transition event failed. Err: %v", err)
			if n.IPClient != nil {
				switch n.IPClient.GetDHCPState() {
				case "dhcpTimedout":
					n.config.Status.TransitionPhase = nmd.DistributedServiceCardStatus_DHCP_TIMEDOUT.String()
				case "missingVendorAttributes":
					n.config.Status.TransitionPhase = nmd.DistributedServiceCardStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String()
				}
			}
			n.config.Status.AdmissionPhase = ""
			n.config.Status.TransitionPhase = ""
			n.config.Status.IPConfig = &cmd.IPConfig{}
			return fmt.Errorf("dynamic mode transition event failed. Err: %v", err)
		}
		n.setStaticRoutes()
	}

	go n.runAdmissionControlLoop()

	return nil
}

func (n *NMD) handleHostModeTransition() error {
	log.Info("Handling host mode transition")
	err := n.StopManagedMode()
	if err != nil {
		log.Errorf("Failed to stop network mode control loop. Err: %v", err)
		return err
	}

	log.Info("Clearing  nw mode naples status")
	err = utils.ClearNaplesTrustRoots()
	if err != nil {
		log.Errorf("Error removing trust roots: %v", err)
	}
	// restart rev proxy so that it can go back to HTTP and no client auth

	//Revproxy clean up may take a while
	time.Sleep(1 * time.Second)
	err = n.RestartRevProxyWithRetries()
	if err != nil {
		return err
	}

	if !n.GetRegStatus() {
		n.config.Status.IPConfig = &cmd.IPConfig{}
		n.config.Status.Controllers = []string{}
		n.config.Status.TransitionPhase = ""
		n.config.Status.AdmissionPhase = ""
		n.config.Status.AdmissionPhaseReason = ""

		if n.rebootNeeded {
			n.config.Status.TransitionPhase = nmd.DistributedServiceCardStatus_REBOOT_PENDING.String()
		}

		if err := n.PersistState(true); err != nil {
			log.Errorf("Failed to persist Naples Config. Err : %v", err)
			return err
		}
		return nil
	}
	log.Error("Failed to stop network mode control loop")
	return nil
}

// runAdmissionControlLoop is a wrapper to perform NAPLES admission and persist state
func (n *NMD) runAdmissionControlLoop() {
	log.Info("Started admission control loop and persisting config")
	isEmulation := false
	if _, err := os.Stat(globals.IotaEmulation); err == nil {
		log.Infof("NMD running in Emulation mode as a real Venice controller is not available. Remove %v file if this was not desired.", globals.IotaEmulation)
		isEmulation = true
	}

	if err := n.PersistState(isEmulation); err != nil {
		log.Errorf("Failed to persist config during admission control loop. Err: %v", errInternalServer(err))
	}

	spec := n.config.Spec
	if spec.IPConfig != nil && len(spec.IPConfig.IPAddress) != 0 {
		log.Info("Performing singleton static admission")
		if err := n.triggerAdmissionEvents(); err != nil {
			log.Errorf("Failed to trigger admission events for static IPConfig: %v", err)
		}
		return
	}
	ticker := time.NewTicker(ipif.AdmissionRetryDuration)

	// We have valid controllers. Start Admission
	if len(n.config.Status.Controllers) > 0 {
		n.IPClient.StopDHCPConfig()
		if err := n.triggerAdmissionEvents(); err == nil {
			return
		}
	}

	for {
		select {
		case <-ticker.C:
			if len(n.config.Status.Controllers) > 0 {
				n.IPClient.StopDHCPConfig()
				if err := n.triggerAdmissionEvents(); err != nil {
					log.Errorf("Failed to trigger admission events for DHCP IPConfig: %v", err)
				} else {
					log.Info("Successfully triggered admission events")
					return
				}
			} else {
				log.Info("Waiting for atleast one controller")
			}
		}
	}
}

// triggerAdmissionEvents triggers Admission, NTP and state persistence.
func (n *NMD) triggerAdmissionEvents() error {
	if err := n.stateMachine.FSM.Event("doNTP", n); err != nil {
		log.Errorf("NTP Sync mode transition event failed. Err: %v", err)
		return fmt.Errorf("NTP Sync mode transition event failed. Err: %v", err)
	}

	err := n.stateMachine.FSM.Event("doAdmission", n)
	if err != nil {
		log.Errorf("Naples Admission mode transition event failed. Err: %v", err)
		return fmt.Errorf("NAPLES Admission mode transition event failed. Err: %v", err)
	}
	return nil
}

func (n *NMD) reconcileIPClient() error {
	var mgmtIntf string
	if n.config.Spec.NetworkMode == nmd.NetworkMode_INBAND.String() {
		mgmtIntf = ipif.NaplesInbandInterface

		err := bringInbandUp()
		if err != nil {
			log.Error("Failed to bring INBAND interface up.")
			return err
		}

	} else {
		mgmtIntf = ipif.NaplesOOBInterface

		err := linkUp(ipif.NaplesOOBInterface)
		if err != nil {
			log.Error("Failed to bring OOB interface up.")
			return err
		}
	}

	// Check if we need to reconcile
	if n.IPClient == nil || n.IPClient.GetIPClientIntf() != mgmtIntf {
		pipeline := ""
		if n.Pipeline != nil {
			pipeline = n.Pipeline.GetPipelineType()
		}

		ipClient, err := ipif.NewIPClient(n, mgmtIntf, pipeline)
		if err != nil {
			log.Errorf("Failed to reconcile IPClient. Err: %v", err)
			return fmt.Errorf("failed to reconcile IPClient. Err: %v", err)
		}
		n.IPClient = ipClient
	}
	return nil
}

func (n *NMD) setRegistrationErrorStatus(reason string) {
	nicObj, err := n.GetSmartNIC()
	if err == nil {
		nicObj.Status.AdmissionPhase = cmd.DistributedServiceCardStatus_REGISTERING.String()
		nicObj.Status.AdmissionPhaseReason = reason
		n.SetSmartNIC(nicObj)
	} else {
		log.Errorf("Error getting SmartNIC object: %v", err)
	}
	n.config.Status.AdmissionPhase = cmd.DistributedServiceCardStatus_REGISTERING.String()
	n.config.Status.AdmissionPhaseReason = reason
}

// AdmitNaples performs NAPLES admission
func (n *NMD) AdmitNaples() {
	log.Info("Starting Managed Mode")
	currentVeniceIdx := 0

	n.modeChange.Lock()

	// Set Registration in progress flag
	log.Infof("NIC in managed mode, mac: %v", n.config.Status.Fru.MacStr)
	n.setRegStatus(true)

	// The mode change is completed when we start the registration loop.
	n.modeChange.Unlock()

	for {
		select {

		// Check if registration loop should be stopped
		case <-n.stopNICReg:

			log.Infof("Registration stopped, exiting.")

			// Clear Registration in progress flag
			n.setRegStatus(false)
			return

		// Register NIC
		case <-time.After(n.nicRegInterval):

			// For the NIC in Naples Config, start the registration
			mac := n.config.Status.Fru.MacStr

			n.UpdateNaplesInfoFromConfig()

			nicObj, _ := n.GetSmartNIC()
			// Send NIC register request to CMD
			log.Infof("Registering NIC with CMD : %+v", nicObj)
			msg, err := n.RegisterSmartNICReq(nicObj)

			// Cache it in nicDB
			if msg.AdmissionResponse != nil {
				nicObj.Status.AdmissionPhase = msg.AdmissionResponse.Phase
				nicObj.Status.AdmissionPhaseReason = msg.AdmissionResponse.Reason
				n.config.Status.AdmissionPhase = msg.AdmissionResponse.Phase
				n.config.Status.AdmissionPhaseReason = msg.AdmissionResponse.Reason
			} else {
				if 2*n.nicRegInterval <= 16 {
					n.nicRegInterval = 2 * n.nicRegInterval * time.Second
				} else {
					n.nicRegInterval = globals.NicRegIntvl * time.Second

					if len(n.config.Status.Controllers) == 0 {
						log.Error("Controllers not passed or cleared. Exiting admission.")
						return
					}

					// PS-1770 fix : Try to connect to other Venice node if the current Venice node fails to reply.
					log.Errorf("%v venice IP failed to respond within 16s.", n.config.Status.Controllers[currentVeniceIdx])
					currentVeniceIdx = (currentVeniceIdx + 1) % len(n.config.Status.Controllers)
					log.Infof("Trying the next venice IP, if any, from the list. %v", n.config.Status.Controllers[currentVeniceIdx])
					registrationURL := fmt.Sprintf("%s:%s", n.config.Status.Controllers[currentVeniceIdx], globals.CMDSmartNICRegistrationPort)

					// Re-populate remote certs URLs
					n.remoteCertsURLs = []string{}
					for _, c := range n.config.Status.Controllers {
						n.remoteCertsURLs = append(n.remoteCertsURLs, fmt.Sprintf("%s:%s", c, globals.CMDAuthCertAPIPort))
					}

					cmdAPI, err := cmdif.NewCmdClient(n, registrationURL, n.resolverClient)
					if err != nil {
						log.Errorf("Failed to instantiate CMD Client. Err: %v", err)
						return
					}
					n.cmd = cmdAPI
				}
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
			//    the retry is done at exponential interval and capped at 3min retry.
			// 4. If is the phase is ADMITTED, move on to next stage of sending periodic
			//    NIC updates.
			//
			if err != nil {
				// Rule #1 - continue retry at regular interval
				log.Errorf("Error registering nic, mac: %s err: %+v", mac, err)
				// if there was an error, we should stay in REGISTERING
				n.setRegistrationErrorStatus(err.Error())
				if strings.Contains(err.Error(), "deadline") {
					log.Infof("Setting the transition phase to venice unreachable.")
					n.config.Status.TransitionPhase = nmd.DistributedServiceCardStatus_VENICE_UNREACHABLE.String()
				}
			} else {
				resp := msg.AdmissionResponse
				if resp == nil {
					log.Errorf("Protocol error: no AdmissionResponse in message, mac: %s", mac)
					continue
				}
				log.Infof("Received register response, phase: %+v", resp.Phase)
				switch resp.Phase {

				case cmd.DistributedServiceCardStatus_REJECTED.String():

					// Rule #2 - abort retry, clear registration status flag
					log.Errorf("Invalid NIC, Admission rejected, mac: %s reason: %s", mac, resp.Reason)
					n.setRegStatus(false)
					return

				case cmd.DistributedServiceCardStatus_PENDING.String():

					// Rule #3 - needs slower exponential retry
					// Cap the retry interval at 3 mins
					if 2*n.nicRegInterval <= nicRegMaxInterval {
						n.nicRegInterval = 2 * n.nicRegInterval
					} else {
						n.nicRegInterval = nicRegMaxInterval
					}

				case cmd.DistributedServiceCardStatus_ADMITTED.String():
					metrics := &NMDMetricsMeta{}
					metrics.TypeMeta.Kind = "NMDMetrics"
					metrics.ObjectMeta.Name = "nmd_" + n.GetAgentID()
					metrics.ObjectMeta.Tenant = "default"

					n.metrics = &NMDMetrics{}
					_, err := tsdb.NewVeniceObj(metrics, n.metrics, &tsdb.ObjOpts{})
					if err != nil {
						log.Errorf("Unable to create venice metrics object. %v", err)
					}

					// Venice says all good, but we need to check the credentials we got back
					// to make sure they are valid and come from the expected Venice.
					// If not, we report the error and go back to REGISTERING
					cert, trustChain, trustRoots, cntrls, err := n.parseAdmissionResponse(msg.AdmissionResponse)
					if err != nil {
						log.Errorf("Error parsing cluster credentials: %v", err)
						n.setRegistrationErrorStatus("Cluster trust chain failed validation")
						continue
					} else {

						if len(cntrls) > 0 {
							// Venice may have different list of controllers.
							// Re-populate remote certs URLs
							n.remoteCertsURLs = []string{}
							for _, c := range cntrls {
								n.remoteCertsURLs = append(n.remoteCertsURLs, fmt.Sprintf("%s:%s", c, globals.CMDAuthCertAPIPort))
							}

							nicObj.Spec.Controllers = cntrls
							n.config.Status.Controllers = cntrls
							n.SetSmartNIC(nicObj)
							err = n.UpdateCMDClient(cntrls)
							if err != nil {
								log.Errorf("Error updating CMD Client : %v", err)
							}
						}

						err = n.setClusterCredentials(cert, trustChain, trustRoots)
						if err != nil {
							log.Errorf("Error storing cluster credentials: %v", err)
							// we can remain in ADMITTED state but we need to retry
							continue
						}
					}

					// Rule #4 - registration is success, clear registration status
					// and move on to next stage
					log.Infof("NIC admitted into cluster, mac: %s", mac)
					n.setRegStatus(false)
					n.nicRegInterval = n.nicRegInitInterval

					// Start certificates proxy
					if n.certsListenURL != "" {
						if n.certsProxy != nil {
							log.Info("Previous instance of certs proxy found. Stopping it.")
							n.certsProxy.Stop()
							n.certsProxy = nil

							// We need to give some time for the port to be released and garbage collected by the OS.
							time.Sleep(time.Second)
						}

						certsProxy, err := certsproxy.NewCertsProxy(n.certsListenURL, n.remoteCertsURLs,
							rpckit.WithTLSProvider(n.tlsProvider), rpckit.WithRemoteServerName(globals.Cmd))
						if err != nil {
							log.Errorf("Error starting certificates proxy at %s: %v", n.certsListenURL, err)
							// cannot proceed without certs proxy, retry after nicRegInterval
							continue
						} else {
							log.Infof("Started certificates proxy at %s, forwarding to: %v", n.certsListenURL, n.remoteCertsURLs)
							n.certsProxy = certsProxy
							n.certsProxy.Start()
						}
					}

					_ = stopNtpClient()
					err = startNtpClient(n.config.Status.Controllers)
					if err != nil {
						log.Infof("start NTP client returned %v", err)
					}

					// start watching objects
					go n.cmd.WatchSmartNICUpdates()
					go n.rollout.WatchDSCRolloutUpdates()

					// Start goroutine to send periodic NIC updates
					n.Add(1)
					go func() {
						defer n.Done()
						n.SendNICUpdates()
					}()

					err = n.RestartRevProxyWithRetries()
					if err != nil {
						log.Errorf("Failed to start reverse proxy. Err : %v", err)
					}

					// Registration is complete here. Set the status to Registration done.
					log.Infof("Setting the transition phase to registration done")
					n.config.Status.TransitionPhase = nmd.DistributedServiceCardStatus_VENICE_REGISTRATION_DONE.String()

					nic, _ := n.GetSmartNIC()
					recorder.Event(eventtypes.DSC_ADMITTED, fmt.Sprintf("DSC %s(%s) admitted to the cluster", nic.Spec.ID, nic.Name), nic)
					// Transition to reboot pending only on successful admission only if reboot has not been done.
					if n.rebootNeeded {
						if err := n.stateMachine.FSM.Event("rebootPending", n); err != nil {
							log.Errorf("Reboot pending mode transition event failed. Err: %v", err)
						}
					}
					if err := n.UpdateNaplesInfoFromConfig(); err != nil {
						log.Errorf("Failed to update naples config post updation")
					}

					if err = n.PersistState(true); err != nil {
						log.Errorf("Failed to persist naples state. Err: %v", err)
					}

					return
				case cmd.DistributedServiceCardStatus_UNKNOWN.String():
					// Not an expected response
					log.Errorf("Unknown response, nic: %+v phase: %v", nicObj, resp)

				} // end of switch statement
			} // end of if err != nil statement
		} // end of select statement
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
			if nicObj.Status.AdmissionPhase != cmd.DistributedServiceCardStatus_ADMITTED.String() {
				log.Infof("Skipping health update, phase %v", nicObj.Status.AdmissionPhase)
				continue
			}

			// TODO : Get status from platform and fill nic Status
			nicObj.Status = cmd.DistributedServiceCardStatus{
				AdmissionPhase: cmd.DistributedServiceCardStatus_ADMITTED.String(),
				Conditions:     n.UpdateNaplesHealth(),
			}

			// Send nic status
			log.Infof("Sending NIC health update: %+v", nicObj)
			err := n.UpdateSmartNICReq(nicObj)
			if err != nil {
				log.Errorf("Error updating nic, name:%s  err: %+v",
					nicObj.Name, err)
			}

		}
	}
}

// StopManagedMode stop the ongoing tasks meant for managed mode
func (n *NMD) StopManagedMode() error {
	log.Info("Stopping Managed Mode.")
	n.modeChange.Lock()
	defer n.modeChange.Unlock()

	// stop accepting certificate requests
	n.Lock()
	if n.certsProxy != nil {
		n.certsProxy.Stop()
		n.certsProxy = nil
	}

	if n.resolverClient != nil {
		n.resolverClient.Stop()
	}
	n.Unlock()

	// stop ongoing NIC registration, if any
	if n.GetRegStatus() {
		n.stopNICReg <- true
	}

	// stop ongoing NIC updates, if any
	if n.GetUpdStatus() {
		n.stopNICUpd <- true
	}

	// cancel tsdb connection
	if n.tsdbCancel != nil {
		n.tsdbCancel()
	}

	// Wait for goroutines launched in managed mode
	// to complete
	n.Wait()

	// cmd, rollout and tlsProvider are protected by modeChange lock

	if n.cmd != nil {
		n.cmd.Stop()
		n.cmd = nil
	}

	if n.rollout != nil {
		n.rollout.Stop()
		n.rollout = nil
	}

	// release TLS provider resources
	if n.tlsProvider != nil {
		n.tlsProvider.Close()
		n.tlsProvider = nil
	}

	log.Infof("Stopping managed mode done.")
	return nil
}

// StartNMDRestServer start the REST server
func (n *NMD) StartNMDRestServer() error {
	if !n.GetRestServerStatus() {
		// Start RestServer
		log.Infof("Starting REST server, NIC mac: %v", n.config.Status.Fru.MacStr)
		return n.StartRestServer()
	}
	return nil
}

// StopClassicMode stops the ongoing tasks meant for classic mode
func (n *NMD) StopClassicMode(shutdown bool) error {
	log.Infof("Stopping Classic Mode.")
	// Stop RestServer
	return n.StopRestServer(shutdown)
}

// GetPlatformCertificate returns the certificate containing the NIC identity and public key
func (n *NMD) GetPlatformCertificate(nic *cmd.DistributedServiceCard) ([]byte, error) {
	return n.Platform.GetPlatformCertificate(nic)
}

// GenChallengeResponse returns the response to a challenge issued by CMD to authenticate this NAPLES
func (n *NMD) GenChallengeResponse(nic *cmd.DistributedServiceCard, challenge []byte) ([]byte, []byte, error) {
	signer, err := n.Platform.GetPlatformSigner(nic)
	if err != nil {
		return nil, nil, fmt.Errorf("error getting platform signer: %v", err)
	}
	return certs.GeneratePoPChallengeResponse(signer, challenge)
}

// UpdateNaplesProfile creates a Naples Profile
func (n *NMD) UpdateNaplesProfile(profile nmd.DSCProfile) error {
	log.Infof("Creating Naples Profile : %v", profile)
	if profile.Spec.DefaultPortAdmin != nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String() && profile.Spec.DefaultPortAdmin != nmd.PortAdminState_PORT_ADMIN_STATE_DISABLE.String() {
		log.Infof("Invalid port admin state set. Setting to default of Enabled.")
		profile.Spec.DefaultPortAdmin = nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String()
	}

	n.Lock()
	defer n.Unlock()
	found := false
	// ensure the profile name is unique.
	for _, p := range n.profiles {
		if profile.Name == p.Name {
			found = true
			p.Spec.DefaultPortAdmin = profile.Spec.DefaultPortAdmin

			c, _ := types.TimestampProto(time.Now())
			p.ModTime = api.Timestamp{
				Timestamp: *c,
			}
			break
		}
	}

	if !found {
		log.Errorf("Profile %v not found.", profile.Name)
		return fmt.Errorf("profile %v not found", profile.Name)
	}

	// persist profile
	if err := n.store.Write(&profile); err != nil {
		log.Errorf("Failed to persist naples profile. Err: %v", err)
		return err
	}
	err := utils.BackupNMDDB()
	if err != nil {
		log.Errorf("Failed to backup nmd.db. Err : %v", err)
	}

	return nil
}

// TODO : Use halproto instead of doing the string compare
func isDataplaneClassic() bool {
	cmd := exec.Command("bash", "-c", "halctl show system forwarding-mode")
	out, err := cmd.Output()

	if err != nil || strings.Contains(string(out), "CLASSIC") {
		log.Info("Dataplane is in CLASSIC mode.")
		return true
	}

	log.Info("Dataplane is in HOSTPIN mode")
	return false
}

func (n *NMD) setStaticRoutes() {
	// Configure the static routes
	log.Infof("Setting the classless static routes got from DHCP")
	for _, r := range n.IPClient.GetStaticRoutes() {
		destStr := r.DestAddr.String() + "/" + strconv.Itoa(int(r.DestPrefixLen))
		_, dest, err := net.ParseCIDR(destStr)
		if err != nil {
			log.Errorf("Failed to Parse Destination address %v in static route %v. Err: %v", destStr, r, err)
			continue
		}

		route := &netlink.Route{
			Dst: dest,
			Gw:  r.NextHopAddr,
		}
		if err := netlink.RouteAdd(route); err != nil {
			log.Errorf("Failed to configure static route %v. Err: %v", route, err)
		}
	}
}
