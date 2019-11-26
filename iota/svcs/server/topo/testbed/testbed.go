package testbed

import (
	"context"
	"fmt"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"golang.org/x/crypto/ssh"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/iota/svcs/common/runner"
	dataswitch "github.com/pensando/sw/iota/svcs/common/switch"
	vmware "github.com/pensando/sw/iota/svcs/common/vmware"
	"github.com/pensando/sw/venice/utils/log"
)

func (n *TestNode) cleanupEsxNode(cfg *ssh.ClientConfig) error {
	log.Infof("Cleaning up esx node %v", n.Node.EsxConfig.IpAddress)
	host, err := vmware.NewHost(context.Background(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | Clean Esx node, failed to get host handle  %v", err.Error())
		return err
	}

	vms, err := host.GetAllVms()
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | unable to list VMS %v", err.Error())
		return err
	}

	for _, vm := range vms {
		if vm.Name() == constants.EsxControlVMName {
			//Ignore control VM delete
			vm.ReconfigureNetwork(constants.EsxNaplesMgmtNetwork, constants.EsxDefaultNetwork)
			on, _ := host.PoweredOn(vm.Name())
			if !on {
				_, err := host.BootVM(constants.EsxControlVMName)
				if err != nil {
					log.Errorf("TOPO SVC | CleanTestBed | Boot control node failed %v", err.Error())
					return err
				}
			}
			var ip string
			if ip, err = host.GetVMIP(vm.Name()); err != nil {
				log.Errorf("TOPO SVC | CleanTestBed | Get VM IP failed %v", err.Error())
				return err
			}
			runner := runner.NewRunner(cfg)
			addr := fmt.Sprintf("%s:%d", ip, constants.SSHPort)
			log.Errorf("TOPO SVC | CleanUpNode | Clean up control VM node %v  IPAddress: %v ", n.Node.Name, ip)
			for _, cmd := range constants.CleanupCommands {
				err := runner.Run(addr, cmd, constants.RunCommandForeground)
				if err != nil {
					log.Errorf("TOPO SVC | CleanUpNode | Clean up on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
				}
			}
			log.Errorf("TOPO SVC | CleanUpNode | Clean up control VM node %v done, IPAddress: %v", n.Node.Name, ip)
			continue
		}

		vm.Destroy()
		if err != nil {
			log.Errorf("TOPO SVC | CleanTestBed | Destroy vm node failed %v", err.Error())
		}
	}

	/* Delete all invalid VMS too */

	pass := n.Node.EsxConfig.Password
	cfg = &ssh.ClientConfig{
		User: n.Node.EsxConfig.Username,
		Auth: []ssh.AuthMethod{
			ssh.Password(n.Node.EsxConfig.Password),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = pass
				}

				return answers, nil
			}),
		}, HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	log.Infof("Deleting invalid VMs on  esx node %v", n.Node.EsxConfig.IpAddress)
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.EsxConfig.IpAddress, constants.SSHPort)
	cmd := `vim-cmd /vmsvc/getallvm  2>&1 >/dev/null  | cut -d' ' -f 4`
	output, err := runner.RunWithOutput(addr, cmd, constants.RunCommandForeground)
	log.Infof("Invalid VMS out %v", output)
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | Failed to find invalid VMs %v", err.Error())
		return err
	}
	for _, vmID := range strings.Split(output, "\n") {
		cmd = "vim-cmd /vmsvc/unregister " + strings.Replace(vmID, "'", "", -1)
		log.Infof("Running Delete Invalid Command : %v", cmd)
		runner.Run(addr, cmd, constants.RunCommandForeground)
	}

	if nws, err := host.ListNetworks(); err == nil {
		delNws := []vmware.NWSpec{}
		for _, nw := range nws {
			if nw.Name != "VM Network" && nw.Name != constants.EsxDefaultNetwork {
				delNws = append(delNws, vmware.NWSpec{Name: nw.Name})
			}
		}
		host.RemoveNetworks(delNws)
	}

	if vswitches, err := host.ListVswitchs(); err == nil {
		for _, vswitch := range vswitches {
			if vswitch.Name != "vSwitch0" {
				host.RemoveVswitch(vswitch)
			}
		}
	}

	return nil
}

func (n *TestNode) initEsxNode() error {
	var err error
	var vmInfo *vmware.VMInfo

	ctrlVMDir := constants.ControlVMImageDirectory + "/" + constants.EsxControlVMImage

	log.Infof("Initializing ESX node %v", n.Node.EsxConfig.IpAddress)
	host, err := vmware.NewHost(context.Background(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Esx node, failed to get host handle  %v", err.Error())
		return err
	}

	vsname := constants.EsxIotaCtrlSwitch
	vsspec := vmware.VswitchSpec{Name: vsname}
	host.AddVswitch(vsspec)

	nws := []vmware.NWSpec{{Name: constants.EsxDefaultNetwork, Vlan: int32(constants.EsxDefaultNetworkVlan)}, {Name: constants.EsxVMNetwork, Vlan: int32(constants.EsxVMNetworkVlan)}}
	host.AddNetworks(nws, vsspec)

	if !host.VMExists(constants.EsxControlVMName) {
		vmInfo, err = host.DeployVM(constants.EsxControlVMName, constants.EsxControlVMCpus, constants.EsxControlVMMemory, constants.EsxControlVMNetworks, ctrlVMDir)
		if err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Add control node failed %v", err.Error())
			return err
		}
	} else {
		log.Infof("Control VM exists on %v", n.Node.EsxConfig.IpAddress)
		if on, _ := host.PoweredOn(constants.EsxControlVMName); !on {
			log.Infof("Control VM exists already powered on %v", n.Node.EsxConfig.IpAddress)
			vmInfo, err = host.BootVM(constants.EsxControlVMName)
			if err != nil {
				log.Errorf("TOPO SVC | InitTestBed | Boot control node failed %v", err.Error())
				return err
			}
		} else {
			log.Infof("Trying to get IP of Control VM %v", n.Node.EsxConfig.IpAddress)
			ip, _ := host.GetVMIP(constants.EsxControlVMName)
			vmInfo = &vmware.VMInfo{IP: ip, Name: constants.EsxControlVMName}
		}
	}

	log.Infof("Init ESX node complete with IP : %s", vmInfo.IP)
	n.Node.IpAddress = vmInfo.IP

	return n.waitForNodeUp(restartTimeout)
}

// InitNode initializes an iota test node. It copies over IOTA Agent binary and starts it on the remote node
func (n *TestNode) InitNode(reboot bool, c *ssh.ClientConfig, commonArtifacts []string) error {
	var agentBinary string

	if reboot {
		if err := n.RestartNode(); err != nil {
			log.Errorf("Restart node %v failed. Err: %v", n.Node.Name, err)
			return err
		}
	} else if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		if err := n.cleanupEsxNode(c); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Clean up ESX node failed :  %v", err.Error())
			return err
		}
		if err := n.initEsxNode(); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Init ESX node failed :  %v", err.Error())
			return err
		}
	}

	runner := runner.NewRunner(c)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := runner.Run(addr, "sudo mkdir -p /pensando", constants.RunCommandForeground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to create /pensando dir on %v : %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		return fmt.Errorf("Failed to create /pensando dir:. TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
	}

	// Copy Agent Binary to the remote node
	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD {
		agentBinary = constants.IotaAgentBinaryPathFreebsd
	} else {
		agentBinary = constants.IotaAgentBinaryPathLinux
	}

	log.Infof("TOPO SVC | InitTestBed | Running init for TestNode: %v, IPAddress: %v AgentBinary: %v", n.Node.Name, n.Node.IpAddress, agentBinary)
	if err := n.CopyTo(c, constants.DstIotaAgentDir, []string{agentBinary}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy agent binary: %v, to TestNode: %v, at IPAddress: %v", agentBinary, n.Node.Name, n.Node.IpAddress)
		return err
	}

	// Copy Common Artifacts to the remote node
	if err := n.CopyTo(c, constants.ImageArtificatsDirectory, commonArtifacts); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy common artifacts, to TestNode: %v, at IPAddress: %v", n.Node.Name, n.Node.IpAddress)
		return err
	}

	//Copy Nic configuration
	log.Infof("TOPO SVC | InitTestBed | Running init for TestNode: %v, IPAddress: %v Nic Finder Conf: %v", n.Node.Name, n.Node.IpAddress, constants.NicFinderConf)
	if err := n.CopyTo(c, constants.DstIotaAgentDir, []string{constants.NicFinderConf}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to Nic conf file: %v, to TestNode: %v, at IPAddress: %v", constants.NicFinderConf, n.Node.Name, n.Node.IpAddress)
		return err
	}

	log.Infof("TOPO SVC | InitTestBed | Starting IOTA Agent on TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	sudoAgtCmd := fmt.Sprintf("sudo %s", constants.DstIotaAgentBinary)
	if err := n.StartAgent(sudoAgtCmd, c); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to start agent binary: %v, on TestNode: %v, at IPAddress: %v", agentBinary, n.Node.Name, n.Node.IpAddress)
		return err
	}

	return nil
}

// CleanUpNode cleans up the node
func (n *TestNode) CleanUpNode(cfg *ssh.ClientConfig, reboot bool) error {
	log.Infof("TOPO SVC | CleanupNode | Running clean up for TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if reboot {
		log.Infof("TOPO SVC | CleanupNode | Restaring TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
		err := n.RestartNode()
		if err != nil {
			return err
		}
	}
	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		if err := n.cleanupEsxNode(cfg); err != nil {
			log.Errorf("TOPO SVC | CleanUpNode | Clean up  ESX node failed :  %v", err.Error())
			return err
		}
	} else {
		// Dont enforce error handling for clean up path
		done := make(chan bool)
		go func() {
			for _, cmd := range constants.CleanupCommands {
				log.Infof("TOPO SVC | CleanUpNode | Clean up on  %v, IPAddress: %v , cmd: %v", n.Node.Name, n.Node.IpAddress, cmd)
				err := runner.Run(addr, cmd, constants.RunCommandForeground)
				if err != nil {
					log.Errorf("TOPO SVC | CleanUpNode | Clean up on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
				}
			}
			done <- true
		}()

		select {
		case <-done:
			break
		case <-time.After(time.Duration(2) * time.Minute):
			//Reset the node if CIMC info is available
			cmd := fmt.Sprintf("ipmitool -I lanplus -H %s -U %s -P %s power cycle",
				n.CimcIP, n.CimcUserName, n.CimcPassword)

			splitCmd := strings.Split(cmd, " ")
			if stdout, err := exec.Command(splitCmd[0], splitCmd[1:]...).CombinedOutput(); err != nil {
				log.Errorf("TOPO SVC | Reset Node Failed %v", stdout)
			} else {
				log.Errorf("TOPO SVC | Reset Node Success")
			}
		}
	}

	log.Infof("TOPO SVC | CleanupNode | done : %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	return nil
}

// CopyTo copies a file to the node
func (n *TestNode) CopyTo(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	var ip string
	var err error
	var copyHandle *copier.Copier

	if ip, err = n.GetNodeIP(); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed to get node IP")
		return fmt.Errorf("TOPO SVC | CopyTo node failed to get node IP")
	}
	addr := fmt.Sprintf("%s:%d", ip, constants.SSHPort)

	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient, cfg)
	}

	if err := copyHandle.CopyTo(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | CopyTo node %v failed, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
		return fmt.Errorf("CopyTo node failed, TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
	}

	//Update SSH client for future
	if n.SSHClient != nil {
		n.SSHClient = copyHandle.SSHClient
	}
	return nil
}

// CopyFrom copies a file to the node
func (n *TestNode) CopyFrom(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	//copier := copier.NewCopier(cfg)
	var ip string
	var err error
	var copyHandle *copier.Copier
	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient, cfg)
	}

	if ip, err = n.GetNodeIP(); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed to get node IP")
		return fmt.Errorf("TOPO SVC | CopyTo node failed to get node IP")
	}

	addr := fmt.Sprintf("%s:%d", ip, constants.SSHPort)

	if err := copyHandle.CopyFrom(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | CopyFrom node %v failed, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
		return fmt.Errorf("CopyFrom node failed, TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
	}

	//Update SSH client for future
	n.SSHClient = copyHandle.SSHClient

	return nil
}

// StartAgent starts IOTA agent on the remote node
func (n *TestNode) StartAgent(command string, cfg *ssh.ClientConfig) error {
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := runner.Run(addr, command, constants.RunCommandBackground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | StartAgent on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		return fmt.Errorf("StartAgent on node failed. TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
	}
	return nil
}

func clearSwitchPortConfig(dataSwitch dataswitch.Switch, ports []string) error {
	for _, port := range ports {
		dataSwitch.UnsetTrunkMode(port)
	}
	return nil
}

func setSwitchPortConfig(dataSwitch dataswitch.Switch, ports []string,
	nativeVlan int, vlanRange string, igmpDisabled bool, speed dataswitch.PortSpeed,
	mtu uint32, receiveFlowControl, sendFlowControl bool) error {

	for _, port := range ports {
		if err := dataSwitch.SetTrunkMode(port); err != nil {
			return errors.Wrapf(err, "Setting switch trunk mode failed %v", port)
		}
		if err := dataSwitch.SetTrunkVlanRange(port, vlanRange); err != nil {
			return errors.Wrapf(err, "Setting Vlan trunk range failed %v", port)
		}

		if igmpDisabled {
			if err := dataSwitch.DisableIGMP(vlanRange); err != nil {
				return errors.Wrap(err, "Disable IGMP failed")
			}
		}

		if err := dataSwitch.SetSpeed(port, speed); err != nil {
			return errors.Wrap(err, "Setting Port speed failed")
		}

		if err := dataSwitch.SetNativeVlan(port, nativeVlan); err != nil {
			return errors.Wrap(err, "Setting switch trunk mode failed")
		}

		if mtu != 0 {
			if err := dataSwitch.SetMtu(port, mtu); err != nil {
				return errors.Wrap(err, "Setting mtu failed")
			}
		}
		if err := dataSwitch.SetFlowControlReceive(port, receiveFlowControl); err != nil {
			return errors.Wrap(err, "Setting mtu failed")
		}
		if err := dataSwitch.SetFlowControlSend(port, sendFlowControl); err != nil {
			return errors.Wrap(err, "Setting mtu failed")
		}

		//Shut and no shut ports for reliability.
		if err := dataSwitch.LinkOp(port, true); err != nil {
			return errors.Wrap(err, "Setting shut on port failed")
		}

		if err := dataSwitch.LinkOp(port, false); err != nil {
			return errors.Wrap(err, "Setting no shut on port failed")
		}
	}

	return nil
}

func setSwitchQosConfig(dataSwitch dataswitch.Switch, qos *iota.NetworkPolicyQos) error {

	if qos == nil {
		return nil
	}
	log.Print("Doing Switch QOS configuration successful..")
	qosCfg := dataswitch.QosConfig{Name: qos.Name}

	for _, qClass := range qos.GetQosClasses() {
		qosCfg.Classes = append(qosCfg.Classes, dataswitch.QosClass{Mtu: qClass.Mtu,
			Name: qClass.Name, PfsCos: qClass.PausePfcCos})
	}
	err := dataSwitch.DoQosConfig(&qosCfg)
	if err != nil {
		return errors.Wrap(err, "Setting QOS config for switch failed")
	}

	log.Print("Switch QOS configuration successful..")
	return nil
}

func portLinkOp(dataSwitch dataswitch.Switch, ports []string, shutdown bool) error {
	for _, port := range ports {
		if err := dataSwitch.LinkOp(port, shutdown); err != nil {
			return errors.Wrap(err, "Setting switch trunk mode failed")
		}
	}

	return nil
}

func portLinkFlap(ctx context.Context, dataSwitch dataswitch.Switch, ports []string, flapCount, downTime, flapInterval uint32) error {
	log.Infof("Doing port link flap (count %v, downtime %v, interval %v)",
		flapCount, downTime, flapInterval)
	for i := 0; i < int(flapCount); i++ {
		for _, port := range ports {
			if err := dataSwitch.LinkOp(port, true); err != nil {
				return errors.Wrap(err, "Setting switch trunk mode failed")
			}
		}

		time.Sleep(time.Duration(downTime) * time.Second)
		log.Info("Sleeping after link down")

		for _, port := range ports {
			if err := dataSwitch.LinkOp(port, false); err != nil {
				return errors.Wrap(err, "Setting switch trunk mode failed")
			}
		}

		//if cancelled
		select {
		case <-ctx.Done():
			return nil
		default:
			time.Sleep(time.Duration(flapInterval) * time.Second)
		}
	}

	log.Info("port link flap done")
	return nil
}

func checkSwitchConfig(dataSwitch dataswitch.Switch, ports []string, speed dataswitch.PortSpeed) error {
	for _, port := range ports {
		if buf, err := dataSwitch.CheckSwitchConfiguration(port, dataswitch.Trunk, dataswitch.Up, speed); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Switch config check failed %s", err.Error())
			log.Errorf("TOPO SVC | InitTestBed | Switch Output %v", buf)
			for _, m := range strings.Split(buf, "\r\n") {
				log.Printf("%v", m)
			}
			return errors.Wrapf(err, "Checking switch config failed for port %s", port)
		}
	}

	return nil
}

func doVlanConfiguration(dataSwitch dataswitch.Switch, ports []string, vlanConfig *iota.VlanConfig) error {

	for _, port := range ports {
		if vlanConfig.Unset {
			if err := dataSwitch.UnsetTrunkVlanRange(port, vlanConfig.GetVlanRange()); err != nil {
				return errors.Wrap(err, "Setting Vlan trunk range failed")
			}
			if err := dataSwitch.UnsetNativeVlan(port,
				int(vlanConfig.GetNativeVlan())); err != nil {
				return errors.Wrap(err, "Setting Vlan trunk range failed")
			}
		} else {
			if err := dataSwitch.SetTrunkVlanRange(port, vlanConfig.GetVlanRange()); err != nil {
				return errors.Wrap(err, "Setting Vlan trunk range failed")
			}
			if err := dataSwitch.SetNativeVlan(port,
				int(vlanConfig.GetNativeVlan())); err != nil {
				return errors.Wrap(err, "Setting Vlan trunk range failed")
			}
		}

	}

	return nil
}

// DoSwitchOperation allocates vlans based on the switch port ID
func DoSwitchOperation(ctx context.Context, req *iota.SwitchMsg) (err error) {

	for _, ds := range req.DataSwitches {
		n3k := dataswitch.NewSwitch(dataswitch.N3KSwitchType, ds.GetIp(), ds.GetUsername(), ds.GetPassword())
		if n3k == nil {
			log.Errorf("TOPO SVC | InitTestBed | Switch not found %s", dataswitch.N3KSwitchType)
			return errors.New("Switch not found")
		}
		defer n3k.Disconnect()

		switch req.GetOp() {
		case iota.SwitchOp_SHUT_PORTS:
			if err := portLinkOp(n3k, ds.GetPorts(), true); err != nil {
				return errors.Wrap(err, "Port shutdown operation failed")
			}
		case iota.SwitchOp_NO_SHUT_PORTS:
			if err := portLinkOp(n3k, ds.GetPorts(), false); err != nil {
				return errors.Wrap(err, "Port up operation failed")
			}
		case iota.SwitchOp_VLAN_CONFIG:
			if err := doVlanConfiguration(n3k, ds.GetPorts(), req.GetVlanConfig()); err != nil {
				return errors.Wrap(err, "Vlan config operation failed")
			}
		case iota.SwitchOp_FLAP_PORTS:
			if err := portLinkFlap(ctx, n3k, ds.GetPorts(), req.GetFlapInfo().GetCount(),
				req.GetFlapInfo().GetDownTime(),
				req.GetFlapInfo().GetInterval()); err != nil {
				return errors.Wrap(err, "Port up operation failed")
			}

		}
	}

	return nil
}

// SetUpTestbedSwitch allocates vlans based on the switch port ID
func SetUpTestbedSwitch(dsSwitches []*iota.DataSwitch, switchPortID uint32) (vlans []uint32, err error) {

	getSpeed := func(speed iota.DataSwitch_Speed) dataswitch.PortSpeed {
		switch speed {
		case iota.DataSwitch_Speed_10G:
			return dataswitch.Speed10g
		case iota.DataSwitch_Speed_100G:
			return dataswitch.Speed100g
		case iota.DataSwitch_Speed_auto:
			return dataswitch.SpeedAuto
		}
		return dataswitch.SpeedAuto
	}
	//ID 0 means, switch not used for now
	if switchPortID == 0 {
		return nil, nil
	}

	startVlan := (switchPortID * constants.VlansPerTestBed) % constants.MaxVlanRange
	if startVlan == 0 {
		startVlan = 1
	}
	for i := startVlan; i < startVlan+constants.VlansPerTestBed; i++ {
		vlans = append(vlans, i)
	}

	for _, ds := range dsSwitches {
		n3k := dataswitch.NewSwitch(dataswitch.N3KSwitchType, ds.GetIp(), ds.GetUsername(), ds.GetPassword())
		if n3k == nil {
			log.Errorf("TOPO SVC | InitTestBed | Switch not found %s", dataswitch.N3KSwitchType)
			return nil, errors.New("Switch not found")
		}

		clearSwitchPortConfig(n3k, ds.GetPorts())

		speed := getSpeed(ds.GetSpeed())
		vlanRange := strconv.Itoa(int(vlans[0])) + "-" + strconv.Itoa(int(vlans[len(vlans)-1]))
		log.Infof("Reserving vlans %v", vlanRange)
		if err := setSwitchPortConfig(n3k, ds.GetPorts(), int(vlans[0]), vlanRange,
			ds.GetIgmpDisabled(), speed, ds.Mtu, ds.FlowControlReceive, ds.FlowControlSend); err != nil {
			return nil, errors.Wrap(err, "Configuring switch failed")

		}

		if err := setSwitchQosConfig(n3k, ds.GetQos()); err != nil {
			return nil, errors.Wrap(err, "Configuring  QOS  on switch failed")

		}

		if err := checkSwitchConfig(n3k, ds.GetPorts(), speed); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | SwitchPort config check failed  %s", err.Error())
			return nil, err
		}

	}

	return vlans, nil
}

// InitSSHConfig establishes the SSH Config required for remote logging in of the nodes
func InitSSHConfig(user, pass string) *ssh.ClientConfig {
	return &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pass),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = pass
				}
				return answers, nil
			}),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
		Timeout:         time.Second * 30,
	}
}
