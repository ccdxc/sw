package state

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"reflect"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/fsnotify/fsnotify"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/nmd/protos/halproto"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const dhclientConfStr string = "timeout 3600; request subnet-mask, broadcast-address, time-offset, routers, domain-name, domain-name-servers, host-name, netbios-name-servers, netbios-scope, vendor-encapsulated-options; send vendor-class-identifier \"Pensando\";"
const dhclientHostnameStr string = " send host-name "
const dhcpClientCmdStr string = "/sbin/dhclient -1 -nw -q -lf"
const staticIPCmdStr string = "ip addr add "
const dhcpLeaseFile string = "/tmp/dhclient.lease"
const dhclientPidFile string = "/tmp/dhclient.pid"
const dhclientConfPath string = "/etc/dhcp/dhclient.conf"
const rebootPendingPath string = "/tmp/.reboot_needed"

// IPClient helps to set the IP address of the management interfaces
type IPClient struct {
	delphiClient   clientAPI.Client
	leaseFile      string
	isDynamic      bool
	watcher        *fsnotify.Watcher
	stopLeaseWatch chan bool
	iface          string
	ipaddress      string
	gateway        string
	networkMode    nmd.NetworkMode
	mgmtVlan       uint32
	controllers    []string
	hostname       string
	nmdState       *NMD
	isMock         bool
}

func createRebootTmpFile() error {
	f, err := os.Create(rebootPendingPath)
	defer f.Close()

	log.Infof("Creating reboot temp file.")
	return err
}

func checkRebootTmpExist() bool {
	log.Infof("Check if reboot tmp file exists.")
	_, err := os.Stat(rebootPendingPath)
	if err == nil {
		return true
	}
	return false
}

func getFileSize(fileName string) int64 {
	fst, err := os.Stat(fileName)

	if err == nil {
		log.Infof("Get file size [%v]", fst.Size())
		return fst.Size()
	}

	return 0
}

func createDhclientConf(hostname string) error {
	conf := ""

	var err = os.Remove(dhclientConfPath)

	file, err := os.Create(dhclientConfPath)
	if err != nil {
		return err
	}

	defer file.Close()
	if hostname != "" {
		conf = dhclientConfStr + dhclientHostnameStr + "\"" + hostname + "\";"
	} else {
		conf = dhclientConfStr
	}

	_, err = file.WriteString(conf)
	return err
}

func prepareVlanCmdStr(iface string, vlan string) string {
	return "ip link add link " + iface + " name " + iface + "." + vlan + " type vlan id " + vlan + " && ip link set " + iface + "." + vlan + " up"
}

func getDhclientCommand(ifc string) string {
	return dhcpClientCmdStr + " " + dhcpLeaseFile + ifc + " -pf " + dhclientPidFile + " " + ifc + " -v"
}

func runCmd(cmdStr string) error {
	log.Infof("Running : " + cmdStr)
	cmd := exec.Command("bash", "-c", cmdStr)
	_, err := cmd.Output()

	if err != nil {
		log.Errorf("Failed Running : " + cmdStr)
	}

	return err
}

func clearDhcpLeaseFile(leaseFile string) error {
	err := runCmd("rm -rf " + leaseFile)
	if err != nil {
		return err
	}

	return runCmd("touch " + leaseFile)
}

func killDhclient() error {
	killDhclientCmdStr := "cat " + dhclientPidFile + " | xargs kill -9"
	return runCmd(killDhclientCmdStr)
}

func parseHexToIP(vendorHex string) ([]string, error) {
	veniceIP := []string{}
	var ip string
	vendorHexSplit := strings.Split(vendorHex, ":")

	for i := 2; i < len(vendorHexSplit); i += 4 {
		i1, err := strconv.ParseInt("0x"+vendorHexSplit[i], 0, 32)
		if err != nil {
			return veniceIP, err
		}

		i2, err := strconv.ParseInt("0x"+vendorHexSplit[i+1], 0, 32)
		if err != nil {
			return veniceIP, err
		}

		i3, err := strconv.ParseInt("0x"+vendorHexSplit[i+2], 0, 32)
		if err != nil {
			return veniceIP, err
		}

		i4, err := strconv.ParseInt("0x"+vendorHexSplit[i+3], 0, 32)
		if err != nil {
			return veniceIP, err
		}

		ip = fmt.Sprintf("%d.%d.%d.%d", i1, i2, i3, i4)
		veniceIP = append(veniceIP, ip)
	}

	return veniceIP, nil
}

func getIPFromLease(leaseFile string) (string, error) {
	dat, err := ioutil.ReadFile(leaseFile)
	if err != nil {
		return "", err
	}

	leaseData := string(dat)

	// Remove trailing curly braces for each lease element
	leaseDataClean := strings.Replace(leaseData, "}", "", -1)

	// Remove the preceding double spaces
	leaseDataClean = strings.Replace(leaseDataClean, "  ", "", -1)

	// Get all lease elements from the lease file
	leases := strings.Split(leaseDataClean, "lease {")

	// Get all options from the last lease
	leaseOption := strings.Split(leases[len(leases)-1], ";")

	for i := range leaseOption {
		if strings.Contains(leaseOption[i], "fixed-address") {
			splitOption := strings.Split(leaseOption[i], " ")
			dhcpIP := splitOption[1]
			return dhcpIP, nil
		}
	}

	return "", nil
}

func readAndParseLease(leaseFile string) ([]string, error) {
	dat, err := ioutil.ReadFile(leaseFile)
	if err != nil {
		return nil, err
	}

	leaseData := string(dat)

	log.Infof("Found LEASE FILE: \n%s", leaseData)

	// Remove "option" keyword
	leaseDataClean := strings.Replace(leaseData, "option ", "", -1)

	// Remove trailing curly braces for each lease element
	leaseDataClean = strings.Replace(leaseDataClean, "}", "", -1)

	// Remove the preceding double spaces
	leaseDataClean = strings.Replace(leaseDataClean, "  ", "", -1)

	// Get all lease elements from the lease file
	leases := strings.Split(leaseDataClean, "lease {")

	// Get all options from the last lease
	leaseOption := strings.Split(leases[len(leases)-1], ";")

	for i := range leaseOption {
		if strings.Contains(leaseOption[i], "vendor-encapsulated-options") {
			vendorHexSplit := strings.Split(leaseOption[i], " ")
			vendorHex := vendorHexSplit[1]
			veniceIPs, err := parseHexToIP(vendorHex)

			if err != nil {
				return nil, err
			}

			return veniceIPs, nil
		}
	}

	return nil, nil
}

func (c *IPClient) updateDelphiNaplesObject(naplesMode delphiProto.NaplesStatus_Mode) error {
	var transitionPhase delphiProto.NaplesStatus_Transition

	switch c.nmdState.config.Status.TransitionPhase {
	case nmd.NaplesStatus_DHCP_SENT.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_SENT
	case nmd.NaplesStatus_DHCP_DONE.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_DONE
	case nmd.NaplesStatus_DHCP_TIMEDOUT.String():
		transitionPhase = delphiProto.NaplesStatus_DHCP_TIMEDOUT
	case nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String():
		transitionPhase = delphiProto.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES
	case nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_SENT
	case nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_DONE
	case nmd.NaplesStatus_VENICE_UNREACHABLE.String():
		transitionPhase = delphiProto.NaplesStatus_VENICE_UNREACHABLE
	case nmd.NaplesStatus_REBOOT_PENDING.String():
		transitionPhase = delphiProto.NaplesStatus_REBOOT_PENDING
	default:
		transitionPhase = 0
	}

	naplesStatus := delphiProto.NaplesStatus{
		Controllers:     c.nmdState.config.Status.Controllers,
		NaplesMode:      naplesMode,
		TransitionPhase: transitionPhase,
	}

	// Update corresponding delphi object and write to delphi db
	if c.delphiClient != nil && c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
		if err := c.delphiClient.SetObject(&naplesStatus); err != nil {
			log.Errorf("Error writing the naples status object. Err: %v", err)
			return err
		}
	} else {
		log.Error("Delphi c uninitialized")
		return errors.New("delphiClient is nil")
	}

	return nil
}

// updateNaplesStatus does the heavy lifting of mode change.
// This function has many side-effects -
// 1. Creates the /tmp/.reboot_pending file
// 2. Updates and triggers Delphi Naples Status object
// 3. Triggers Naples Admission to Venice cluster using the controller IPs made available
// 4. Stops/Starts REST server to NMD
func (c *IPClient) updateNaplesStatus(controllers []string, ipaddress string) error {
	log.Infof("Found Controllers: %v", controllers)
	var naplesMode delphiProto.NaplesStatus_Mode
	var deviceSpec device.SystemSpec
	var appStartSpec []byte

	if c.nmdState.config.Status.IPConfig == nil {
		c.nmdState.config.Status.IPConfig = &cluster.IPConfig{
			IPAddress:  "",
			DefaultGW:  "",
			DNSServers: nil,
		}
	}

	c.nmdState.config.Status.IPConfig.IPAddress = ipaddress

	// For Static configuration case, we assume that the user knows what they are doing.
	if c.isDynamic && !c.mustTriggerUpdate(controllers) {
		log.Errorf("Cannot trigger update as controllers break security gurantees. Bailing from Naples Status Update.")
		return nil
	}

	// TODO : Reenable these lines
	// Disable any updates to Naples Admission once Naples has been admitted into a Venice Cluster.
	// if c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
	//		log.Infof("Naples already addmitted into Venice Cluster.")
	//	return errors.New("Cannot change Naples mode as Naples is already admitted into the cluster.")
	//}

	log.Infof("Updating Naples Status. Current %v", c.nmdState.config.Status)

	// Set up appropriate mode
	switch c.networkMode {
	case nmd.NetworkMode_INBAND:
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND
		deviceSpec.FwdMode = device.ForwardingMode_FORWARDING_MODE_HOSTPIN
		appStartSpec = []byte("hostpin")
	case nmd.NetworkMode_OOB:
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_OOB
		deviceSpec.FwdMode = device.ForwardingMode_FORWARDING_MODE_HOSTPIN
		appStartSpec = []byte("hostpin")
	default:
		naplesMode = delphiProto.NaplesStatus_HOST_MANAGED
		deviceSpec.FwdMode = device.ForwardingMode_FORWARDING_MODE_CLASSIC
		appStartSpec = []byte("classic")
	}

	if c.nmdState.config.Status.Mode == nmd.MgmtMode_NETWORK.String() {
		log.Info("Currently Naples is Network Managed")
		if c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK {
			log.Info("Moving Naples to Network managed mode. Updating CMD Client.")
			c.nmdState.cmdRegURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCUnauthPort)
			c.nmdState.cmdUpdURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)
			c.nmdState.remoteCertsURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)

			go func() {
				err := c.nmdState.StopManagedMode()
				if err != nil {
					log.Errorf("Error stopping NIC managed mode: %v", err)
				}
				c.nmdState.Add(1)
				defer c.nmdState.Done()

				if err := c.nmdState.UpdateCMDClient(controllers); err != nil {
					log.Errorf("Failed to updated cmd client with resolvers: %v. Err: %v", controllers, err)
				}

				err = c.nmdState.StartManagedMode()
				if err != nil {
					log.Errorf("Error starting NIC managed mode: %v", err)
				}
			}()
		} else if c.nmdState.config.Spec.Mode == nmd.MgmtMode_HOST {
			log.Info("Moving Naples to Host managed mode.")
			go func() {
				err := c.nmdState.StopManagedMode()
				if err != nil {
					log.Errorf("Error stopping NIC managed mode: %v", err)
				}

				err = c.nmdState.StartClassicMode()
				if err != nil {
					log.Errorf("Classic mode start failed. Err: %v", err)
				}
			}()
		} else {
			log.Errorf("Unknown Management Mode in Spec. Bailing from Mode Change")
			return errors.New("unknown Management Mode in Spec. Bailing from Mode Change")
		}
	} else {
		log.Info("Currently Naples is Host Managed")
		if c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK {
			// Stop classic mode control loop
			// TODO : Uncomment the line below. Currently we do not want to stop REST Server.
			// c.nmdState.StopClassicMode(false)
			log.Info("Moving Naples to Network managed mode.")
			c.nmdState.cmdRegURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCUnauthPort)
			c.nmdState.cmdUpdURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)
			c.nmdState.remoteCertsURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)

			if err := c.nmdState.UpdateCMDClient(controllers); err != nil {
				log.Errorf("Failed to updated cmd client with resolvers: %v. Err: %v", controllers, err)
				return err
			}

			// Start the admission process
			c.nmdState.Add(1)
			go func() {
				defer c.nmdState.Done()
				c.nmdState.StartManagedMode()
			}()

			// wait for registration
			// TODO : Uncomment this section
			// c.waitForRegistration()
		} else {
			log.Info("Starting NMD REST server.")

			err := c.nmdState.StartClassicMode()
			if err != nil {
				log.Errorf("Classic mode start failed. Err: %v", err)
			}
		}
	}

	// TODO Remove manual nic admitted state.
	c.nmdState.config.Status.Phase = cluster.SmartNICStatus_ADMITTED

	// Persist bolt db.
	// err := c.nmdState.store.Write(&c.nmdState.config)
	// if err != nil {
	//	log.Errorf("Error persisting the naples config in Bolt DB, err: %+v", err)
	//	return err
	//}

	// Write HAL CFG FILE
	// Create the /sysconfig/config0 if it doesn't exist. Needed for non naples nmd test environments
	if _, err := os.Stat(globals.NaplesModeConfigFile); os.IsNotExist(err) {
		os.MkdirAll(path.Dir(globals.NaplesModeConfigFile), 0664)
	}
	data, err := json.MarshalIndent(deviceSpec, "", "  ")
	if err != nil {
		log.Errorf("Failed to marshal device spec. Err: %v", err)
		return err
	}
	if err = ioutil.WriteFile(globals.NaplesModeConfigFile, data, 0444); err != nil {
		log.Errorf("Failed to write the device spec to %s. Err: %v", globals.NaplesModeConfigFile, err)
	}

	// Update app-start.conf file. TODO Remove this workaround when all the processes are migrated to read from device.conf
	appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	if err = ioutil.WriteFile(appStartConfFilePath, appStartSpec, 0755); err != nil {
		log.Errorf("Failed to write app start conf. Err: %v", err)
	}

	// Reflect reboot pending state only if the nic is admitted.
	if c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
		c.nmdState.config.Status.Controllers = controllers
		if c.isDynamic && len(c.nmdState.config.Spec.Controllers) != 0 {
			// In dynamic mode, the spec will be empty and on reboot we need to ensure that it doesn't have stale controllers picked up from boltdb
			log.Info("Clearing out old controller IPs in spec.")
			c.nmdState.config.Spec.Controllers = []string{}
		}

		if !checkRebootTmpExist() {
			if c.nmdState.config.Status.TransitionPhase == nmd.NaplesStatus_REBOOT_PENDING.String() {
				// Previously it was set to reboot pending. We need to clear it
				log.Infof("Clearing out previous reboot pending state.")
				c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()
			} else if c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String() {
				log.Infof("Current Transition Phase is %v", c.nmdState.config.Status.TransitionPhase)
				c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_REBOOT_PENDING.String()
				createRebootTmpFile()
			}
		}
	} else {
		c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String()
	}

	// Update Naples Status Delphi Object
	err = c.updateDelphiNaplesObject(naplesMode)
	if !c.isMock && err != nil {
		log.Errorf("Error updating Delphi NaplesStatus object. Err: %v", err)
		return err
	}

	// Persist bolt db.
	err = c.nmdState.store.Write(&c.nmdState.config)
	if err != nil {
		log.Errorf("Error persisting the naples config in Bolt DB, err: %+v", err)
		return err
	}

	return nil
}

func (c *IPClient) watchLeaseEvents() {
	for {
		select {
		// watch for events
		case event := <-c.watcher.Events:
			log.Infof("EVENT! %#v\n", event)

			if event.Op&fsnotify.Write == fsnotify.Write && getFileSize(c.leaseFile) > 0 {
				controllers, err := readAndParseLease(c.leaseFile)

				if err != nil {
					return
				}

				if controllers == nil {
					// Vendor specified attributes is nil
					c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String()
					log.Errorf("Controllers is nil.")
				}

				ip, err := getIPFromLease(c.leaseFile)
				if err != nil || ip == "" {
					return
				}

				err = c.updateNaplesStatus(controllers, ip)
				if err != nil {
					return
				}
			}
		// watch for errors
		case <-c.watcher.Errors:
			return

		case <-c.stopLeaseWatch:
			return
		}
	}
}

func (c *IPClient) startDhclient() error {
	// Update NMD Status
	//c.nmdState.Lock()
	log.Infof("Starting DHClient current Transition phase is %v", c.nmdState.config.Status.TransitionPhase)
	if (c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_REBOOT_PENDING.String()) && (c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()) {
		c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_DHCP_SENT.String()
	}
	//c.nmdState.Unlock()
	// Kill previous dhclient, if any
	killDhclient()

	dynamicIPCommandString := getDhclientCommand(c.iface)
	err := createDhclientConf(c.hostname)
	if err != nil {
		return err
	}

	err = clearDhcpLeaseFile(c.leaseFile)
	if err != nil {
		return err
	}

	c.watcher, err = fsnotify.NewWatcher()
	if err != nil {
		return err
	}

	c.stopLeaseWatch = make(chan bool)
	go c.watchLeaseEvents()

	// out of the box fsnotify can watch a single file, or a single directory
	if err := c.watcher.Add(c.leaseFile); err != nil {
		return err
	}

	err = runCmd(dynamicIPCommandString)
	if err != nil {
		c.stopLeaseWatch <- true
		c.watcher.Close()
		return err
	}

	return nil
}

func (c *IPClient) interfaceExists(ifc string) bool {
	log.Infof("Checking if %v exists", ifc)
	cmd := fmt.Sprintf("ethtool %v", ifc)
	err := runCmd(cmd)

	if err != nil {
		log.Errorf("Issues with interface %v", ifc)
		return false
	}

	return true
}

func (c *IPClient) doStaticIPConfig() error {
	log.Infof("Static IP Config. Controllers: %v", c.controllers)
	if c.controllers == nil {
		return errors.New("no controller IP passed")
	}

	c.isDynamic = false

	err := runCmd("ip addr flush dev " + c.iface)
	if err != nil {
		return err
	}

	staticIPCommandString := staticIPCmdStr + c.ipaddress + " dev " + c.iface
	if err := runCmd(staticIPCommandString); err != nil {
		return err
	}

	if c.gateway != "" {
		err = runCmd("ip route add default via " + c.gateway)
		if err != nil {
			return err
		}
	}

	// Update Naples Status only once all the Static IP configuration has completed.
	err = c.updateNaplesStatus(c.controllers, c.ipaddress)
	if err != nil {
		return err
	}

	//runCmd("usr/sbin/rdate 10.7.100.2")
	return nil
}

func (c *IPClient) doDynamicIPConfig() error {
	log.Info("Starting dynamic ip config")
	c.isDynamic = true
	return c.startDhclient()
}

func (c *IPClient) doOOB() error {
	log.Info("Doing OOB")

	c.iface = "oob_mnic0"
	c.leaseFile = dhcpLeaseFile + c.iface
	log.Infof("Doing OOB " + c.iface)
	c.nmdState.config.Status.NetworkMode = nmd.NetworkMode_OOB.String()

	if c.ipaddress != "" {
		err := c.doStaticIPConfig()
		if err != nil {
			return err
		}
	} else {

		err := c.doDynamicIPConfig()
		if err != nil {
			return err
		}
	}

	return nil
}

func (c *IPClient) doInband() error {
	log.Infof("Doing Inband. Client IP Addresses: %v. Mgmt VLAN: %v", c.ipaddress, c.mgmtVlan)

	c.iface = "bond0"
	c.leaseFile = dhcpLeaseFile + c.iface

	if c.mgmtVlan != 0 {
		setVlanCmdStr := prepareVlanCmdStr(c.iface, fmt.Sprint(c.mgmtVlan))
		err := runCmd(setVlanCmdStr)
		if err != nil {
			return err
		}

		c.iface = "bond0." + fmt.Sprint(c.mgmtVlan)
	}
	c.nmdState.config.Status.NetworkMode = nmd.NetworkMode_INBAND.String()

	if c.ipaddress != "" {
		err := c.doStaticIPConfig()
		if err != nil {
			return err
		}
	} else {
		err := c.doDynamicIPConfig()
		if err != nil {
			return err
		}
	}

	return nil
}

//TODO : Remove the long list of parameters passed here, and use nmdState instead

// NewIPClient creates a new ipclient to do the IP configuration of Management port on Naples
func NewIPClient(isMock bool, nmdState *NMD, delphiClient clientAPI.Client, networkMode nmd.NetworkMode, ipaddress string, mgmtVlan uint32, hostname string, controllers []string) *IPClient {
	client := &IPClient{
		delphiClient: delphiClient,
		networkMode:  networkMode, // INBAND/OOB
		ipaddress:    ipaddress,
		mgmtVlan:     mgmtVlan,
		hostname:     hostname,
		controllers:  controllers,
		nmdState:     nmdState,
		isMock:       isMock,
	}

	return client
}

// Start function starts the IPClient
func (c *IPClient) Start() error {
	log.Info("IP Client Start called")
	// Start REST Server. StartClassicMode is confusing. TODO Rename this method
	if err := c.nmdState.StartClassicMode(); err != nil {
		log.Errorf("Failed to start rest server. Err: %v", err)
		return err
	}

	// TODO : Find a better way to express this.
	if !c.isMock && !c.interfaceExists("bond0") && !c.interfaceExists("oob_mnic0") {
		log.Errorf("Interfaces don't exist. Automatically setting IPClient to run in Mock mode.")
		c.isMock = true
	}

	if c.isMock {
		// Update Naples Status will error out if either controllers or ipaddress passed in Spec is nil.
		// This would mean, no DHCP configuration mode will be supported right now.
		// DHCP configuration mode can be added to mock mode in future.
		log.Info("IPClient in MOCK Mode. Calling Update Naples Status directly.")
		return c.updateNaplesStatus(c.controllers, c.ipaddress)
	}

	if c.networkMode == nmd.NetworkMode_OOB && c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK {
		// OOB
		return c.doOOB()
		//if err != nil {
		//	return err
		//} // Admission of Napl
		//
	} else if c.networkMode == nmd.NetworkMode_INBAND && c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK {
		// INBAND
		log.Info("Got network mode inband")

		return c.doInband()
		//if err != nil {
		//	return err
		//}
		// Admission of Naples
	}
	return nil
}

// Stop function stops IPClient's goroutines
func (c *IPClient) Stop() {
	if _, err := os.Stat(rebootPendingPath); err == nil {
		// We will hit this codepath if we are reconfiguring Mode before effecting rebooting Naples
		os.Remove(rebootPendingPath)
		c.nmdState.config.Status.TransitionPhase = ""
		c.nmdState.config.Status.Controllers = []string{}
		c.nmdState.config.Status.IPConfig = nil

		// Should we de-admit at this point?
	}

	if c.isDynamic {
		killDhclient()
		c.stopLeaseWatch <- true
		c.watcher.Close()
		c.isDynamic = false
	}
}

// Update updates the management IPs
func (c *IPClient) Update(networkMode nmd.NetworkMode, ipconfig *cluster.IPConfig, mgmtVlan uint32, hostname string, controllers []string) error {
	log.Info("IP Client Update called")
	c.Stop()

	c.networkMode = networkMode
	if ipconfig != nil {
		c.ipaddress = ipconfig.IPAddress
		c.gateway = ipconfig.DefaultGW
	}
	c.mgmtVlan = mgmtVlan
	c.hostname = hostname
	c.controllers = controllers
	return c.Start()
}

// waitForRegistration waits for a specific timeout to check if the nic has been admitted.
// On a timeout, it doesn't stop nmd's control loop, but only updates the transition phase.
func (c *IPClient) waitForRegistration() {
	registrationDone := make(chan bool, 1)
	ticker := time.NewTicker(time.Second * 10)
	timeout := time.After(nicRegistrationWaitTime)

	for {
		select {
		case <-ticker.C:
			log.Info("Checking if the nic has been admitted")
			if c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
				registrationDone <- true
			}
		case <-registrationDone:
			log.Info("Nic Registration completed")
			return
		case <-timeout:
			log.Errorf("Failed to complete nic registration. Updating NaplesStatus")
			c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_UNREACHABLE.String()
			return
		}
	}
}

// mustTriggerUpdate checks if the controllers information has changed.
func (c *IPClient) mustTriggerUpdate(controllers []string) (mustUpdate bool) {
	if len(c.nmdState.config.Status.Controllers) != len(controllers) {
		mustUpdate = true
		return
	}
	sort.Strings(controllers)
	sort.Strings(c.nmdState.config.Status.Controllers)
	mustUpdate = !reflect.DeepEqual(controllers, c.nmdState.config.Status.Controllers)
	return
}
