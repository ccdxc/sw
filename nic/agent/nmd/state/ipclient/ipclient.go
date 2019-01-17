package platform

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strconv"
	"strings"

	"github.com/fsnotify/fsnotify"

	"github.com/pensando/sw/nic/agent/nmd/protos"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/log"
)

const dhclientConfStr string = "request subnet-mask, broadcast-address, time-offset, routers, domain-name, domain-name-servers, host-name, netbios-name-servers, netbios-scope, vendor-encapsulated-options; send vendor-class-identifier \"Pensando\";"
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

	// Set up appropriate mode
	switch c.networkMode {
	case nmd.NetworkMode_INBAND:
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND
	case nmd.NetworkMode_OOB:
		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_OOB
	default:
		naplesMode = delphiProto.NaplesStatus_HOST_MANAGED
	}

	naplesStatus := delphiProto.NaplesStatus{
		Controllers: controllers,
		NaplesMode:  naplesMode,
	}

	if c.delphiClient != nil {
		if err := c.delphiClient.SetObject(&naplesStatus); err != nil {
			log.Errorf("Error writing the naples status object. Err: %v", err)
			return err
		}
	} else {
		log.Error("Delphi c uninitialized")
		return errors.New("watch Lease Events failure. DelphiClient is nil")
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

			if controllers != nil {
				err = c.updateNaplesStatus(controllers)
				if err != nil {
					return
				}
			} else {
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
	if c.controllers == nil {
		return errors.New("No controller IP passed")
	}

	err := c.updateNaplesStatus(c.controllers)
	if err != nil {
		return err
	}
	c.isDynamic = false

	err = runCmd("ip addr flush dev " + c.iface)
	if err != nil {
		return err
	}

	staticIPCommandString := staticIPCmdStr + c.ipaddress + " dev " + c.iface
	return runCmd(staticIPCommandString)
}

func (c *IPClient) doDynamicIPConfig() error {
	killDhclient()
	return c.startDhclient()
}

func (c *IPClient) doOOB() error {
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
func NewIPClient(delphiClient clientAPI.Client, networkMode nmd.NetworkMode, ipaddress string, mgmtVlan uint32, hostname string, controllers []string) *IPClient {
	client := &IPClient{
		delphiClient: delphiClient,
		networkMode:  networkMode, // INBAND/OOB
		ipaddress:    ipaddress,
		mgmtVlan:     mgmtVlan,
		hostname:     hostname,
		controllers:  controllers,
	}

	return client
}

// Start function starts the IPClient
func (c *IPClient) Start() error {
	if c.networkMode == nmd.NetworkMode_OOB {
		// OOB
		err := c.doOOB()
		if err != nil {
			return err
		}
	} else if c.networkMode == nmd.NetworkMode_INBAND {
		// INBAND
		err := c.doInband()
		if err != nil {
			return err
		}
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
func (c *IPClient) Update(networkMode nmd.NetworkMode, ipaddress string, mgmtVlan uint32, hostname string, controllers []string) error {
	c.Stop()

	c.networkMode = networkMode
	c.ipaddress = ipaddress
	c.mgmtVlan = mgmtVlan
	c.hostname = hostname
	c.controllers = controllers

	return c.Start()
}
