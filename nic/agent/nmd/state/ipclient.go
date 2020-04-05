package state

//
//import (
//	"encoding/binary"
//	"errors"
//	"fmt"
//	"io/ioutil"
//	"net"
//	"os"
//	"os/exec"
//	"reflect"
//	"sort"
//	"strconv"
//	"strings"
//	"time"
//
//	"github.com/pensando/netlink"
//
//	"github.com/fsnotify/fsnotify"
//
//	"github.com/pensando/sw/api/generated/cluster"
//	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
//	"github.com/pensando/sw/nic/agent/protos/nmd"
//	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
//	"github.com/pensando/sw/venice/globals"
//	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
//	"github.com/pensando/sw/venice/utils/log"
//)
//
//const dhclientConfStr string = "timeout 3600; request subnet-mask, broadcast-address, time-offset, routers, domain-name, domain-name-servers, host-name, netbios-name-servers, netbios-scope, vendor-encapsulated-options; send vendor-class-identifier \"Pensando\";"
//const dhclientHostnameStr string = " send host-name "
//const dhcpClientCmdStr string = "/sbin/dhclient -1 -nw -q -lf"
//const staticIPCmdStr string = "ip addr add "
//const dhcpLeaseFile string = "/tmp/dhclient.lease"
//const dhclientPidFile string = "/tmp/dhclient.pid"
//const dhclientConfPath string = "/etc/dhcp/dhclient.conf"
//const rebootPendingPath string = "/tmp/.reboot_needed"
//
//// IPClient helps to set the IP address of the management interfaces
//type IPClient struct {
//	delphiClient   clientAPI.Client
//	leaseFile      string
//	isDynamic      bool
//	watcher        *fsnotify.Watcher
//	stopLeaseWatch chan bool
//	iface          string
//	nmdState       *NMD
//	isMock         bool
//}
//
//func createRebootTmpFile() error {
//	f, err := os.Create(rebootPendingPath)
//	defer f.Close()
//
//	log.Infof("Creating reboot temp file.")
//	return err
//}
//
//func checkRebootTmpExist() bool {
//	log.Infof("Check if reboot tmp file exists.")
//	_, err := os.Stat(rebootPendingPath)
//	if err == nil {
//		return true
//	}
//	return false
//}
//
//func getFileSize(fileName string) int64 {
//	fst, err := os.Stat(fileName)
//
//	if err == nil {
//		log.Infof("Get file size [%v]", fst.Size())
//		return fst.Size()
//	}
//
//	return 0
//}
//
//func createDhclientConf(hostname string) error {
//	conf := ""
//
//	var err = os.Remove(dhclientConfPath)
//
//	file, err := os.Create(dhclientConfPath)
//	if err != nil {
//		return err
//	}
//
//	defer file.Close()
//	if hostname != "" {
//		conf = dhclientConfStr + dhclientHostnameStr + "\"" + hostname + "\";"
//	} else {
//		conf = dhclientConfStr
//	}
//
//	_, err = file.WriteString(conf)
//	return err
//}
//
//func prepareVlanCmdStr(iface string, vlan string) string {
//	return "ip link add link " + iface + " name " + iface + "." + vlan + " type vlan id " + vlan + " && ip link set " + iface + "." + vlan + " up"
//}
//
//func getDhclientCommand(ifc string) string {
//	return dhcpClientCmdStr + " " + dhcpLeaseFile + ifc + " -pf " + dhclientPidFile + " " + ifc + " -v"
//}
//

//
//func intfcUp(iface string) error {
//	log.Infof("Bringing interface %v up.", iface)
//	return runCmd("ifconfig " + iface + " up")
//}
//
//func intfcDown(iface string) error {
//	log.Infof("Bringing interface %v down.", iface)
//	return runCmd("ifconfig " + iface + " down")
//}
//
//func clearDhcpLeaseFile(leaseFile string) error {
//	err := runCmd("rm -rf " + leaseFile)
//	if err != nil {
//		return err
//	}
//
//	return runCmd("touch " + leaseFile)
//}
//
//func killDhclient() error {
//	killDhclientCmdStr := "kill -9 $(pidof dhclient)"
//	return runCmd(killDhclientCmdStr)
//}
//
//func parseHexToIP(vendorHex string) ([]string, error) {
//	veniceIP := []string{}
//	var ip string
//	vendorHexSplit := strings.Split(vendorHex, ":")
//
//	for i := 2; i < len(vendorHexSplit); i += 4 {
//		i1, err := strconv.ParseInt("0x"+vendorHexSplit[i], 0, 32)
//		if err != nil {
//			return veniceIP, err
//		}
//
//		i2, err := strconv.ParseInt("0x"+vendorHexSplit[i+1], 0, 32)
//		if err != nil {
//			return veniceIP, err
//		}
//
//		i3, err := strconv.ParseInt("0x"+vendorHexSplit[i+2], 0, 32)
//		if err != nil {
//			return veniceIP, err
//		}
//
//		i4, err := strconv.ParseInt("0x"+vendorHexSplit[i+3], 0, 32)
//		if err != nil {
//			return veniceIP, err
//		}
//
//		ip = fmt.Sprintf("%d.%d.%d.%d", i1, i2, i3, i4)
//		veniceIP = append(veniceIP, ip)
//	}
//
//	return veniceIP, nil
//}
//
//func getIPFromLease(leaseFile string) (string, error) {
//	dat, err := ioutil.ReadFile(leaseFile)
//	if err != nil {
//		return "", err
//	}
//
//	leaseData := string(dat)
//
//	// Remove trailing curly braces for each lease element
//	leaseDataClean := strings.Replace(leaseData, "}", "", -1)
//
//	// Remove the preceding double spaces
//	leaseDataClean = strings.Replace(leaseDataClean, "  ", "", -1)
//
//	// Get all lease elements from the lease file
//	leases := strings.Split(leaseDataClean, "lease {")
//
//	// Get all options from the last lease
//	leaseOption := strings.Split(leases[len(leases)-1], ";")
//
//	for i := range leaseOption {
//		if strings.Contains(leaseOption[i], "fixed-address") {
//			splitOption := strings.Split(leaseOption[i], " ")
//			dhcpIP := splitOption[1]
//			return dhcpIP, nil
//		}
//	}
//
//	return "", nil
//}
//
//func readAndParseLease(leaseFile string) ([]string, error) {
//	dat, err := ioutil.ReadFile(leaseFile)
//	if err != nil {
//		return nil, err
//	}
//
//	leaseData := string(dat)
//
//	log.Infof("Found LEASE FILE: \n%s", leaseData)
//
//	// Remove "option" keyword
//	leaseDataClean := strings.Replace(leaseData, "option ", "", -1)
//
//	// Remove trailing curly braces for each lease element
//	leaseDataClean = strings.Replace(leaseDataClean, "}", "", -1)
//
//	// Remove the preceding double spaces
//	leaseDataClean = strings.Replace(leaseDataClean, "  ", "", -1)
//
//	// Get all lease elements from the lease file
//	leases := strings.Split(leaseDataClean, "lease {")
//
//	// Get all options from the last lease
//	leaseOption := strings.Split(leases[len(leases)-1], ";")
//
//	for i := range leaseOption {
//		if strings.Contains(leaseOption[i], "vendor-encapsulated-options") {
//			vendorHexSplit := strings.Split(leaseOption[i], " ")
//			vendorHex := vendorHexSplit[1]
//			veniceIPs, err := parseHexToIP(vendorHex)
//
//			if err != nil {
//				return nil, err
//			}
//
//			return veniceIPs, nil
//		}
//	}
//
//	return nil, nil
//}
//
//func (c *IPClient) updateDelphiNaplesObject() error {
//	var mgmtIP string
//	// Set up appropriate mode
//	var naplesMode delphiProto.NaplesStatus_Mode
//
//	switch c.nmdState.config.Spec.NetworkMode {
//	case nmd.NetworkMode_INBAND.String():
//		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND
//	case nmd.NetworkMode_OOB.String():
//		naplesMode = delphiProto.NaplesStatus_NETWORK_MANAGED_OOB
//	default:
//		naplesMode = delphiProto.NaplesStatus_HOST_MANAGED
//
//	}
//
//	var transitionPhase delphiProto.NaplesStatus_Transition
//
//	switch c.nmdState.config.Status.TransitionPhase {
//	case nmd.NaplesStatus_DHCP_SENT.String():
//		transitionPhase = delphiProto.NaplesStatus_DHCP_SENT
//	case nmd.NaplesStatus_DHCP_DONE.String():
//		transitionPhase = delphiProto.NaplesStatus_DHCP_DONE
//	case nmd.NaplesStatus_DHCP_TIMEDOUT.String():
//		transitionPhase = delphiProto.NaplesStatus_DHCP_TIMEDOUT
//	case nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String():
//		transitionPhase = delphiProto.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES
//	case nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String():
//		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_SENT
//	case nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String():
//		transitionPhase = delphiProto.NaplesStatus_VENICE_REGISTRATION_DONE
//	case nmd.NaplesStatus_VENICE_UNREACHABLE.String():
//		transitionPhase = delphiProto.NaplesStatus_VENICE_UNREACHABLE
//	case nmd.NaplesStatus_REBOOT_PENDING.String():
//		transitionPhase = delphiProto.NaplesStatus_REBOOT_PENDING
//	default:
//		transitionPhase = 0
//	}
//
//	// For static case write only the IP in mgmt IP and not the subnet
//	if !c.isDynamic {
//		ip, _, err := net.ParseCIDR(c.nmdState.config.Status.IPConfig.IPAddress)
//		if err != nil {
//			log.Errorf("failed to parse addr %v, %v", c.nmdState.config.Status.IPConfig.IPAddress, err)
//		}
//		mgmtIP = ip.String()
//
//	} else {
//		mgmtIP = c.nmdState.config.Status.IPConfig.IPAddress
//	}
//
//	naplesStatus := delphiProto.NaplesStatus{
//		Controllers:     c.nmdState.config.Status.Controllers,
//		NaplesMode:      naplesMode,
//		TransitionPhase: transitionPhase,
//		MgmtIP:          mgmtIP,
//		ID:              c.nmdState.config.Spec.ID,
//		DSCName:    c.nmdState.config.Status.DSCName,
//		Fru: &delphiProto.NaplesFru{
//			ManufacturingDate: c.nmdState.config.Status.Fru.ManufacturingDate,
//			Manufacturer:      c.nmdState.config.Status.Fru.Manufacturer,
//			ProductName:       c.nmdState.config.Status.Fru.ProductName,
//			SerialNum:         c.nmdState.config.Status.Fru.SerialNum,
//			PartNum:           c.nmdState.config.Status.Fru.PartNum,
//			BoardId:           c.nmdState.config.Status.Fru.BoardId,
//			EngChangeLevel:    c.nmdState.config.Status.Fru.EngChangeLevel,
//			NumMacAddr:        c.nmdState.config.Status.Fru.NumMacAddr,
//			MacStr:            c.nmdState.config.Status.Fru.MacStr,
//		},
//	}
//
//	// Update corresponding delphi object and write to delphi db
//	if c.delphiClient != nil && (c.nmdState.config.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() || naplesMode == delphiProto.NaplesStatus_HOST_MANAGED) {
//		if err := c.delphiClient.SetObject(&naplesStatus); err != nil {
//			log.Errorf("Error writing the naples status object. Err: %v", err)
//			return err
//		}
//	} else if !c.isMock {
//		log.Error("Delphi c uninitialized")
//		return errors.New("delphiClient is nil")
//	}
//
//	return nil
//}
//
//func (c *IPClient) updateNaplesStatusIP(ipaddress string, defaultGW string, dnsServers []string) error {
//	if c.nmdState.config.Status.IPConfig == nil {
//		c.nmdState.config.Status.IPConfig = &cluster.IPConfig{
//			IPAddress:  "",
//			DefaultGW:  "",
//			DNSServers: nil,
//		}
//	}
//
//	if ipaddress == "" {
//		return errors.New("ipaddress is empty")
//	}
//
//	c.nmdState.config.Status.IPConfig.IPAddress = ipaddress
//	c.nmdState.config.Status.IPConfig.DefaultGW = defaultGW
//	c.nmdState.config.Status.IPConfig.DNSServers = dnsServers
//
//	return nil
//}
//
//// updateNaplesStatus does the heavy lifting of mode change.
//// This function has many side-effects -
//// 1. Creates the /tmp/.reboot_pending file
//// 2. Updates and triggers Delphi Naples Status object
//// 3. Triggers Naples Admission to Venice cluster using the controller IPs made available
//// 4. Stops/Starts REST server to NMD
//func (c *IPClient) updateNaplesStatus(controllers []string) error {
//	log.Infof("Found Controllers: %v", controllers)
//
//	c.nmdState.config.Status.DSCName = c.nmdState.config.Status.Fru.MacStr
//
//	// TODO : Reenable these lines
//	// Disable any updates to Naples Admission once Naples has been admitted into a Venice Cluster.
//	// if c.nmdState.config.Status.Phase == cluster.SmartNICStatus_ADMITTED {
//	//		log.Infof("Naples already addmitted into Venice Cluster.")
//	//	return errors.New("Cannot change Naples mode as Naples is already admitted into the cluster.")
//	//}
//
//	log.Infof("Updating Naples Status.")
//	log.Infof("Current Spec : %v", c.nmdState.config.Spec)
//	log.Infof("Current Status : %v", c.nmdState.config.Status)
//
//	if c.nmdState.config.Status.Mode == nmd.MgmtMode_NETWORK.String() {
//		log.Info("Currently Naples is Network Managed")
//		if c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK.String() {
//			log.Info("Moving Naples to Network managed mode. Updating CMD Client.")
//			c.nmdState.config.Status.Mode = nmd.MgmtMode_NETWORK.String()
//
//			c.nmdState.cmdRegURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDSmartNICRegistrationPort)
//			c.nmdState.remoteCertsURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDAuthCertAPIPort)
//
//			err := c.nmdState.StopManagedMode()
//			if err != nil {
//				log.Errorf("Error stopping NIC managed mode: %v", err)
//			}
//
//			if err := c.nmdState.UpdateCMDClient(controllers); err != nil {
//				log.Errorf("Failed to updated cmd client with resolvers: %v. Err: %v", controllers, err)
//			}
//
//			// TODO : Remove Auto-admit
//			c.nmdState.config.Status.AdmissionPhase = cluster.SmartNICStatus_ADMITTED.String()
//
//			c.nmdState.Add(1)
//			go func() {
//				defer c.nmdState.Done()
//
//				err := c.nmdState.StartManagedMode()
//				if err != nil {
//					log.Errorf("Error starting NIC managed mode: %v", err)
//				}
//			}()
//
//			// TODO : Reenable these lines
//			//c.waitForRegistration()
//
//			// Reflect reboot pending state only if the nic is admitted.
//			if c.nmdState.config.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() {
//				c.nmdState.config.Status.Controllers = controllers
//				if c.isDynamic && len(c.nmdState.config.Spec.Controllers) != 0 {
//					// In dynamic mode, the spec will be empty and on reboot we need to ensure that it doesn't have stale controllers picked up from boltdb
//					log.Info("Clearing out old controller IPs in spec.")
//					c.nmdState.config.Spec.Controllers = []string{}
//				}
//
//				if !checkRebootTmpExist() {
//					if c.nmdState.config.Status.TransitionPhase == nmd.NaplesStatus_REBOOT_PENDING.String() {
//						// Previously it was set to reboot pending. We need to clear it
//						log.Infof("Clearing out previous reboot pending state.")
//						c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()
//					} else if c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String() {
//						log.Infof("Current Transition Phase is %v", c.nmdState.config.Status.TransitionPhase)
//						c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_REBOOT_PENDING.String()
//						createRebootTmpFile()
//					}
//				}
//			} else {
//				c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String()
//			}
//		} else if c.nmdState.config.Spec.Mode == nmd.MgmtMode_HOST.String() {
//			log.Info("Moving Naples to Host managed mode.")
//			c.nmdState.config.Status.Mode = nmd.MgmtMode_HOST.String()
//
//			err := c.nmdState.StopManagedMode()
//			if err != nil {
//				log.Errorf("Error stopping NIC managed mode: %v", err)
//			}
//
//			err = c.nmdState.StartNMDRestServer()
//			if err != nil {
//				log.Errorf("Classic mode start failed. Err: %v", err)
//			}
//		} else {
//			log.Errorf("Unknown Management Mode in Spec. Bailing from Mode Change")
//			return errors.New("unknown Management Mode in Spec. Bailing from Mode Change")
//		}
//	} else {
//		log.Info("Currently Naples is Host Managed")
//		if c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK.String() {
//			// Stop classic mode control loop
//			// TODO : Uncomment the line below. Currently we do not want to stop REST Server.
//			// c.nmdState.StopClassicMode(false)
//			log.Info("Moving Naples to Network managed mode.")
//			c.nmdState.config.Status.Mode = nmd.MgmtMode_NETWORK.String()
//
//			c.nmdState.cmdRegURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDSmartNICRegistrationPort)
//			c.nmdState.remoteCertsURL = fmt.Sprintf("%s:%s", controllers[0], globals.CMDGRPCAuthPort)
//			if err := c.nmdState.UpdateCMDClient(controllers); err != nil {
//				log.Errorf("Failed to updated cmd client with resolvers: %v. Err: %v", controllers, err)
//			}
//
//			// TODO : Remove Auto-admit
//			c.nmdState.config.Status.AdmissionPhase = cluster.SmartNICStatus_ADMITTED.String()
//
//			c.nmdState.Add(1)
//			go func() {
//				defer c.nmdState.Done()
//
//				err := c.nmdState.StartManagedMode()
//				if err != nil {
//					log.Errorf("Error starting NIC managed mode: %v", err)
//				}
//			}()
//
//			// wait for registration
//			// TODO : Uncomment this section
//			//c.waitForRegistration()
//			if c.nmdState.config.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() {
//				c.nmdState.config.Status.Controllers = controllers
//				if c.isDynamic && len(c.nmdState.config.Spec.Controllers) != 0 {
//					// In dynamic mode, the spec will be empty and on reboot we need to ensure that it doesn't have stale controllers picked up from boltdb
//					log.Info("Clearing out old controller IPs in spec.")
//					c.nmdState.config.Spec.Controllers = []string{}
//				}
//
//				if !checkRebootTmpExist() {
//					if c.nmdState.config.Status.TransitionPhase == nmd.NaplesStatus_REBOOT_PENDING.String() {
//						// Previously it was set to reboot pending. We need to clear it
//						log.Infof("Clearing out previous reboot pending state.")
//						c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()
//					} else if c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String() {
//						log.Infof("Current Transition Phase is %v", c.nmdState.config.Status.TransitionPhase)
//						c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_REBOOT_PENDING.String()
//						createRebootTmpFile()
//					}
//				}
//			} else {
//				c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_REGISTRATION_SENT.String()
//			}
//
//		} else {
//			log.Info("Starting NMD REST server.")
//			c.nmdState.config.Status.Mode = nmd.MgmtMode_HOST.String()
//
//			err := c.nmdState.StartNMDRestServer()
//			if err != nil {
//				log.Errorf("Classic mode start failed. Err: %v", err)
//			}
//		}
//	}
//
//	// Update smart NIc
//	//c.nmdState.UpdateNaplesInfoFromConfig()
//
//	// Persist HAL Configuration
//	if !c.isMock {
//		mgmtIfMAC := parseMgmtIfMAC(c.nmdState.config.Spec.NetworkMode)
//		if err := c.nmdState.PersistHALConfiguration(c.nmdState.config.Spec.NaplesProfile, mgmtIfMAC); err != nil {
//			log.Infof("NIC mode: %v feature profile: %v", c.nmdState.config.Spec.Mode, c.nmdState.config.Spec.NaplesProfile)
//			return err
//		}
//	}
//
//	// Persist bolt db.
//	err := c.nmdState.store.Write(&c.nmdState.config)
//	if err != nil {
//		log.Errorf("Error persisting the naples config in Bolt DB, err: %+v", err)
//		return err
//	}
//
//	// Update Naples Status Delphi Object
//	err = c.updateDelphiNaplesObject()
//	if !c.isMock && err != nil {
//		log.Errorf("Error updating Delphi NaplesStatus object. Err: %v", err)
//		return err
//	}
//
//	return nil
//}
//
//func (c *IPClient) watchLeaseEvents() {
//	for {
//		select {
//		// watch for events
//		case event := <-c.watcher.Events:
//			log.Infof("EVENT! %#v\n", event)
//			var controllers []string
//
//			if event.Op&fsnotify.Write == fsnotify.Write && getFileSize(c.leaseFile) > 0 {
//				ip, _ := getIPFromLease(c.leaseFile)
//				err := c.updateNaplesStatusIP(ip, "", nil)
//				if err != nil {
//					log.Errorf("Lease file does not have IP address in it.")
//					return
//				}
//				// Use the statically passed controllers if available.
//				if len(c.nmdState.config.Spec.Controllers) == 0 {
//					controllers, _ = readAndParseLease(c.leaseFile)
//				} else {
//					controllers = c.nmdState.config.Spec.Controllers
//				}
//
//				if len(controllers) == 0 {
//					// Vendor specified attributes is nil
//					c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String()
//					log.Errorf("Failed to update naples status : %v", err)
//				} else {
//					for _, cIP := range controllers {
//						if vldtor.IPAddr(cIP) != nil {
//							c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_MISSING_VENDOR_SPECIFIED_ATTRIBUTES.String()
//							log.Errorf("Controllers returned is invalid IP")
//							return
//						}
//					}
//
//					c.updateNaplesStatus(controllers)
//				}
//			}
//		// watch for errors
//		case <-c.watcher.Errors:
//			return
//
//		case <-c.stopLeaseWatch:
//			return
//		}
//	}
//}
//
//func (c *IPClient) startDhclient() error {
//	// Update NMD Status
//	//c.nmdState.Lock()
//	log.Infof("Starting DHClient current Transition phase is %v", c.nmdState.config.Status.TransitionPhase)
//	if (c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_REBOOT_PENDING.String()) && (c.nmdState.config.Status.TransitionPhase != nmd.NaplesStatus_VENICE_REGISTRATION_DONE.String()) {
//		c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_DHCP_SENT.String()
//	}
//	//c.nmdState.Unlock()
//	// Kill previous dhclient, if any
//	killDhclient()
//
//	dynamicIPCommandString := getDhclientCommand(c.iface)
//	err := createDhclientConf(c.nmdState.config.Spec.ID)
//	if err != nil {
//		return err
//	}
//
//	err = clearDhcpLeaseFile(c.leaseFile)
//	if err != nil {
//		return err
//	}
//
//	c.watcher, err = fsnotify.NewWatcher()
//	if err != nil {
//		return err
//	}
//
//	c.stopLeaseWatch = make(chan bool)
//	go c.watchLeaseEvents()
//
//	// out of the box fsnotify can watch a single file, or a single directory
//	if err := c.watcher.Add(c.leaseFile); err != nil {
//		return err
//	}
//
//	err = runCmd(dynamicIPCommandString)
//	if err != nil {
//		c.stopLeaseWatch <- true
//		c.watcher.Close()
//		return err
//	}
//
//	return nil
//}
//
//func (c *IPClient) interfaceExists(ifc string) bool {
//	log.Infof("Checking if %v exists", ifc)
//	cmd := fmt.Sprintf("ethtool %v", ifc)
//	err := runCmd(cmd)
//
//	if err != nil {
//		log.Errorf("Issues with interface %v", ifc)
//		return false
//	}
//
//	return true
//}
//
//func (c *IPClient) doStaticIPConfig() error {
//	log.Infof("Static IP Config. Controllers: %v", c.nmdState.config.Spec.Controllers)
//	if c.nmdState.config.Spec.Controllers == nil {
//		return errors.New("no controller IP passed")
//	}
//
//	c.isDynamic = false
//
//	err := runCmd("ip addr flush dev " + c.iface)
//	if err != nil {
//		return err
//	}
//
//	staticIPCommandString := staticIPCmdStr + c.nmdState.config.Spec.IPConfig.IPAddress + "  brd + dev " + c.iface
//	if err := runCmd(staticIPCommandString); err != nil {
//		log.Errorf("Failed to assign static IP.")
//		return err
//	}
//
//	if c.nmdState.config.Spec.IPConfig.DefaultGW != "" {
//		err = runCmd("route add default gw " + c.nmdState.config.Spec.IPConfig.DefaultGW + " " + c.iface)
//		if err != nil {
//			log.Errorf("Failed to add Default GW.")
//			return err
//		}
//	}
//	err = c.updateNaplesStatusIP(c.nmdState.config.Spec.IPConfig.IPAddress, c.nmdState.config.Spec.IPConfig.DefaultGW, c.nmdState.config.Spec.IPConfig.DNSServers)
//	if err != nil {
//		return err
//	}
//
//	// Update Naples Status only once all the Static IP configuration has completed.
//	err = c.updateNaplesStatus(c.nmdState.config.Spec.Controllers)
//	if err != nil {
//		return err
//	}
//
//	return nil
//}
//
//func (c *IPClient) doDynamicIPConfig() error {
//	log.Info("Starting dynamic ip config")
//	c.isDynamic = true
//	return c.startDhclient()
//}
//
//func (c *IPClient) doOOB() error {
//	log.Info("Doing OOB")
//
//	c.iface = "oob_mnic0"
//	c.leaseFile = dhcpLeaseFile + c.iface
//	log.Infof("Doing OOB " + c.iface)
//	// Ensure the interface is up. Ignore errors here. It will fail downstream anyway.
//	intfcUp(c.iface)
//
//	if c.nmdState.config.Spec.IPConfig.IPAddress != "" {
//		err := c.doStaticIPConfig()
//		if err != nil {
//			return err
//		}
//	} else {
//
//		err := c.doDynamicIPConfig()
//		if err != nil {
//			return err
//		}
//	}
//
//	return nil
//}
//
//func (c *IPClient) doInband() error {
//	log.Infof("Doing Inband. Client IP Addresses: %v. Mgmt VLAN: %v", c.nmdState.config.Spec.IPConfig.IPAddress, c.nmdState.config.Spec.MgmtVlan)
//
//	c.iface = "bond0"
//	c.leaseFile = dhcpLeaseFile + c.iface
//	// Ensure the interface is up.
//	intfcUp(c.iface)
//
//	if c.nmdState.config.Spec.MgmtVlan != 0 {
//		setVlanCmdStr := prepareVlanCmdStr(c.iface, fmt.Sprint(c.nmdState.config.Spec.MgmtVlan))
//		err := runCmd(setVlanCmdStr)
//		if err != nil {
//			return err
//		}
//
//		c.iface = "bond0." + fmt.Sprint(c.nmdState.config.Spec.MgmtVlan)
//	}
//
//	if c.nmdState.config.Spec.IPConfig.IPAddress != "" {
//		err := c.doStaticIPConfig()
//		if err != nil {
//			return err
//		}
//	} else {
//		err := c.doDynamicIPConfig()
//		if err != nil {
//			return err
//		}
//	}
//
//	return nil
//}
//
////TODO : Remove the long list of parameters passed here, and use nmdState instead
//
//// NewIPClient creates a new ipif to do the IP configuration of Management port on Naples
//func NewIPClient(isMock bool, nmdState *NMD, delphiClient clientAPI.Client) *IPClient {
//	client := &IPClient{
//		delphiClient: delphiClient,
//		nmdState:     nmdState,
//		isMock:       isMock,
//	}
//
//	return client
//}
//
//// Start function starts the IPClient
//func (c *IPClient) Start() error {
//	log.Info("IP Client Start called")
//	// Start REST Server. StartNMDRestServer is confusing. TODO Rename this method
//	if err := c.nmdState.StartNMDRestServer(); err != nil {
//		log.Errorf("Failed to start rest server. Err: %v", err)
//		return err
//	}
//
//	// TODO : Find a better way to express this.
//	if !c.isMock && !c.interfaceExists("bond0") && !c.interfaceExists("oob_mnic0") {
//		log.Errorf("Interfaces don't exist. Automatically setting IPClient to run in Mock mode.")
//		c.isMock = true
//	}
//
//	if c.isMock && len(c.nmdState.config.Spec.Controllers) > 0 {
//		// Update Naples Status will error out if either controllers or ipaddress passed in Spec is nil.
//		// This would mean, no DHCP configuration mode will be supported right now.
//		// DHCP configuration mode can be added to mock mode in future.
//		log.Info("IPClient in MOCK Mode. Calling Update Naples Status directly.")
//		// Hardcoding "1.1.1.1" as it is Mock mode and it will be ignored anyway
//		if c.nmdState.config.Spec.IPConfig != nil && c.nmdState.config.Spec.IPConfig.IPAddress != "" {
//			c.updateNaplesStatusIP(c.nmdState.config.Spec.IPConfig.IPAddress, "", nil)
//		} else {
//			c.updateNaplesStatusIP("1.1.1.1", "", nil)
//		}
//		return c.updateNaplesStatus(c.nmdState.config.Spec.Controllers)
//	}
//
//	if c.nmdState.config.Spec.NetworkMode == nmd.NetworkMode_OOB.String() && c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK.String() {
//		// OOB
//		return c.doOOB()
//		//if err != nil {
//		//	return err
//		//} // Admission of Napl
//		//
//	} else if c.nmdState.config.Spec.NetworkMode == nmd.NetworkMode_INBAND.String() && c.nmdState.config.Spec.Mode == nmd.MgmtMode_NETWORK.String() {
//		// INBAND
//		log.Info("Got network mode inband")
//
//		return c.doInband()
//		//if err != nil {
//		//	return err
//		//}
//		// Admission of Naples
//	} else { // Moving to HOST mode
//		c.updateNaplesStatusIP("", "", nil)
//		c.nmdState.config.Spec.NetworkMode = ""
//		return c.updateNaplesStatus(nil)
//	}
//}
//
//// Stop function stops IPClient's goroutines
//func (c *IPClient) Stop() {
//	c.nmdState.config.Status.TransitionPhase = ""
//	// Add a de-admission cycle here?
//	c.nmdState.config.Status.AdmissionPhase = ""
//	c.nmdState.config.Status.Controllers = []string{}
//	c.nmdState.config.Status.IPConfig = &cluster.IPConfig{
//		IPAddress:  "",
//		DefaultGW:  "",
//		DNSServers: make([]string, 0),
//	}
//	// Bring the currently active management interface down.
//	intfcDown(c.iface)
//
//	if _, err := os.Stat(rebootPendingPath); err == nil {
//		os.Remove(rebootPendingPath)
//	}
//
//	// Should we de-admit at this point?
//	if c.isDynamic {
//		killDhclient()
//		c.stopLeaseWatch <- true
//		c.watcher.Close()
//		c.isDynamic = false
//	}
//}
//
//// Update updates the management IPs
//func (c *IPClient) Update() error {
//	log.Info("IP Client Update called")
//	c.Stop()
//	return c.Start()
//}
//
//// waitForRegistration waits for a specific timeout to check if the nic has been admitted.
//// On a timeout, it doesn't stop nmd's control loop, but only updates the transition phase.
//func (c *IPClient) waitForRegistration() {
//	registrationDone := make(chan bool, 1)
//	ticker := time.NewTicker(time.Second * 10)
//	timeout := time.After(nicRegistrationWaitTime)
//
//	for {
//		select {
//		case <-ticker.C:
//			log.Info("Checking if the nic has been admitted")
//			if c.nmdState.config.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() {
//				registrationDone <- true
//			}
//		case <-registrationDone:
//			log.Info("Nic Registration completed")
//			return
//		case <-timeout:
//			log.Errorf("Failed to complete nic registration. Updating NaplesStatus")
//			c.nmdState.config.Status.TransitionPhase = nmd.NaplesStatus_VENICE_UNREACHABLE.String()
//			return
//		}
//	}
//}
//
//func (c *IPClient) isAfterReboot() (mustUpdate bool) {
//	if !checkRebootTmpExist() && c.nmdState.config.Status.TransitionPhase == nmd.NaplesStatus_REBOOT_PENDING.String() {
//		return true
//	}
//
//	return false
//}
//
//// hasControllerChanged checks if the controllers information has changed.
//func (c *IPClient) hasControllerChanged(controllers []string) (mustUpdate bool) {
//	if len(c.nmdState.config.Status.Controllers) != len(controllers) {
//		mustUpdate = true
//		return
//	}
//
//	sort.Strings(controllers)
//	sort.Strings(c.nmdState.config.Status.Controllers)
//	mustUpdate = !reflect.DeepEqual(controllers, c.nmdState.config.Status.Controllers)
//	return
//}
//
//func parseMgmtIfMAC(mgmtNetwork string) (mgmtIfMAC uint64) {
//	switch mgmtNetwork {
//	case nmd.NetworkMode_INBAND.String():
//		mgmtLink, err := netlink.LinkByName("bond0")
//		if err != nil {
//			log.Errorf("Could not find system mac on interface bond0. Err: %v", err)
//			return
//		}
//		mgmtIfMAC = macToUint64(mgmtLink.Attrs().HardwareAddr)
//		return
//	case nmd.NetworkMode_OOB.String():
//		mgmtLink, err := netlink.LinkByName("oob_mnic0")
//		if err != nil {
//			log.Errorf("Could not find system mac on interface oob_mnic0. Err: %v", err)
//			return
//		}
//		mgmtIfMAC = macToUint64(mgmtLink.Attrs().HardwareAddr)
//		return
//	default:
//		return
//	}
//}
//
//func macToUint64(macAddr net.HardwareAddr) (mac uint64) {
//	b := make([]byte, 8)
//	// oui-48 format
//	if len(macAddr) == 6 {
//		// fill 0 lsb
//		copy(b[2:], macAddr)
//	}
//	mac = binary.BigEndian.Uint64(b)
//	return
//}
