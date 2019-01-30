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

// IPClient helps to set the IP address of the management interfaces
type IPClient struct {
	delphiClient clientAPI.Client
	leaseFile    string
	isDynamic    bool
	watcher      *fsnotify.Watcher
	doneEvent    chan bool
	iface        string
	ipaddress    string
	networkMode  nmd.NetworkMode
	mgmtVlan     uint32
	controllers  []string
	hostname     string
	nmdState     *NMD
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

func (c *IPClient) updateNaplesStatus(controllers []string) error {
	log.Infof("Found Controllers: %v", controllers)
	var naplesMode delphiProto.NaplesStatus_Mode
	var deviceSpec device.SystemSpec
	var appStartSpec []byte

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

	naplesStatus := delphiProto.NaplesStatus{
		Controllers: controllers,
		NaplesMode:  naplesMode,
	}
	c.nmdState.cmdRegURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCUnauthPort)
	c.nmdState.cmdUpdateURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)
	c.nmdState.remoteCertsURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)

	// TODO Check if update cmd and registration need to be triggered again if the naples is already admitted
	if err := c.nmdState.cmd.UpdateCMDClient(controllers); err != nil {
		log.Errorf("Failed to updated cmd client with resolvers: %v. Err: %v", controllers, err)
		return err
	}

	// Start ManagedMode
	if c.nmdState.config.Spec.Mode != nmd.MgmtMode_HOST {
		// Stop classic mode control loop
		// c.nmdState.StopClassicMode(false)
		// Start the admission process
		c.nmdState.Add(1)
		go func() {
			defer c.nmdState.Done()
			c.nmdState.StartManagedMode()
		}()
		//wait for registration TODO Uncomment this section
		//c.waitForRegistration()
	}

	// TODO Remove manual nic admitted state.
	c.nmdState.config.Status.Phase = cluster.SmartNICStatus_ADMITTED

	// Persist bolt db.
	err := c.nmdState.store.Write(&c.nmdState.config)
	if err != nil {
		log.Errorf("Error persisting the naples config in Bolt DB, err: %+v", err)
		return err
	}

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
		if c.nmdState.config.Status.TransitionPhase == nmd.NaplesStatus_REBOOT_PENDING.String() {
			// Previously it was set to reboot pending. We need to clear it
			log.Infof("Clearing out previous reboot pending state.")
			c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()
			naplesStatus.TransitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_DONE

		} else {
			log.Infof("Current Transition Phase is %v", c.nmdState.config.Status.TransitionPhase)
			c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_REBOOT_PENDING.String()
			naplesStatus.TransitionPhase = delphiProto.NaplesStatus_REBOOT_PENDING
		}
	} else {
		c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String()
		naplesStatus.TransitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_SENT
	}

	// Update corresponding delphi object and write to delphi db
	if c.delphiClient != nil && c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
		if err := c.delphiClient.SetObject(&naplesStatus); err != nil {
			log.Errorf("Error writing the naples status object. Err: %v", err)
			return err
		}
	} else {
		log.Error("Delphi c uninitialized")
		return errors.New("watch Lease Events failure. DelphiClient is nil")
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
			controllers, err := readAndParseLease(c.leaseFile)

			if err != nil {
				return
			}

			if controllers != nil && c.mustTriggerUpdate(controllers) {
				err = c.updateNaplesStatus(controllers)
				if err != nil {
					return
				}
			} else {
				// Vendor specified attributes is nil
				c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String()
				log.Infof("Controllers is nil.")
			}

		// watch for errors
		case <-c.watcher.Errors:
			return

		case <-c.doneEvent:
			return
		}
	}
}

func (c *IPClient) startDhclient() error {
	// Update NMD Status
	//c.nmdState.Lock()
	log.Info("Starting DHClient")
	c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_DHCP_SENT.String()
	//c.nmdState.Unlock()

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

	c.doneEvent = make(chan bool)
	go c.watchLeaseEvents()

	// out of the box fsnotify can watch a single file, or a single directory
	if err := c.watcher.Add(c.leaseFile); err != nil {
		return err
	}

	err = runCmd(dynamicIPCommandString)
	if err != nil {
		c.doneEvent <- true
		c.watcher.Close()
		return err
	}

	return nil
}

func (c *IPClient) doStaticIPConfig() error {
	log.Infof("Static IP Config. Controllers: %v", c.controllers)
	if c.controllers == nil {
		return errors.New("no controller IP passed")
	}

	c.isDynamic = false

	err := c.updateNaplesStatus(c.controllers)
	if err != nil {
		return err
	}

	err = runCmd("ip addr flush dev " + c.iface)
	if err != nil {
		return err
	}

	staticIPCommandString := staticIPCmdStr + c.ipaddress + " dev " + c.iface
	return runCmd(staticIPCommandString)
}

func (c *IPClient) doDynamicIPConfig() error {
	log.Info("Starting dynamic ip config")
	killDhclient()
	return c.startDhclient()
}

func (c *IPClient) doOOB() error {
	log.Info("Doing OOB")

	c.iface = "oob_mnic0"
	c.leaseFile = dhcpLeaseFile + c.iface
	log.Infof("Doing OOB " + c.iface)

	if c.ipaddress != "" {
		err := c.doStaticIPConfig()
		if err != nil {
			return err
		}
	} else {
		c.isDynamic = true
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

	if c.ipaddress != "" {
		err := c.doStaticIPConfig()
		if err != nil {
			return err
		}
	} else {
		c.isDynamic = true
		err := c.doDynamicIPConfig()
		if err != nil {
			return err
		}
	}

	return nil
}

// NewIPClient creates a new ipclient to do the IP configuration of Management port on Naples
func NewIPClient(nmdState *NMD, delphiClient clientAPI.Client, networkMode nmd.NetworkMode, ipaddress string, mgmtVlan uint32, hostname string, controllers []string) *IPClient {
	client := &IPClient{
		delphiClient: delphiClient,
		networkMode:  networkMode, // INBAND/OOB
		ipaddress:    ipaddress,
		mgmtVlan:     mgmtVlan,
		hostname:     hostname,
		controllers:  controllers,
		nmdState:     nmdState,
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

	if c.networkMode == nmd.NetworkMode_OOB && c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK {
		// OOB
		log.Info("Got network mode OOB")

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
	if c.isDynamic {
		killDhclient()
		c.doneEvent <- true
		c.watcher.Close()
	}
}

// Update updates the management IPs
func (c *IPClient) Update(networkMode nmd.NetworkMode, ipconfig *cluster.IPConfig, mgmtVlan uint32, hostname string, controllers []string) error {
	log.Info("IP Client Update called")
	c.Stop()

	c.networkMode = networkMode
	if ipconfig != nil {
		c.ipaddress = ipconfig.IPAddress
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
	if len(c.controllers) != len(controllers) {
		mustUpdate = true
		return
	}
	sort.Strings(controllers)
	sort.Strings(c.controllers)
	mustUpdate = !reflect.DeepEqual(controllers, c.controllers)
	return
}
