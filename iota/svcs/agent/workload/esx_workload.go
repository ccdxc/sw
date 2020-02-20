package workload

import (
	"context"
	"fmt"
	"net"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"golang.org/x/crypto/ssh"

	"github.com/pkg/errors"
	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/iota/svcs/common/vmware"
)

const (
	sshPort = 22

	restartTimeout = 300 //300 seconds for node restart

	arpTimeout = 3000 //3 seconds
)

type vmESXWorkloadBase struct {
	remoteWorkload
	vmName          string
	hostName        string
	cluster         string
	vm              *vmware.VM
	agentClient     iota.IotaAgentApiClient
	agentBinaryPath string
}

type vmESXWorkload struct {
	vmESXWorkloadBase
	host *vmware.Host
}

type vmVcenterWorkload struct {
	vmESXWorkloadBase
	vhost vmware.EntityIntf
}

func newVMESXWorkload(name string, parent string, logger *log.Logger) Workload {
	return &vmESXWorkload{vmESXWorkloadBase: vmESXWorkloadBase{
		remoteWorkload: remoteWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}}}
}

func newVMVcenterWorkload(name string, parent string, logger *log.Logger) Workload {
	return &vmVcenterWorkload{
		vmESXWorkloadBase: vmESXWorkloadBase{remoteWorkload: remoteWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}}}
}

func (vm *remoteWorkload) waitForVMUp(timeout time.Duration) error {
	cTimeout := time.After(time.Second * time.Duration(timeout))
	for {
		conn, _ := net.DialTimeout("tcp", net.JoinHostPort(vm.ip, "22"), 2*time.Second)
		if conn != nil {
			conn.Close()
			break
		}
		select {
		case <-cTimeout:
			msg := fmt.Sprintf("Timeout system to be up %s ", vm.ip)
			return errors.New(msg)
		default:
			time.Sleep(100 * time.Millisecond)
		}
	}
	return nil
}

func (vm *vmESXWorkloadBase) startVMAgent(args ...string) error {

	//VM workloads are linux today
	agentBinary := constants.IotaAgentBinaryPathLinux

	//Directory is mounted copy there
	cpCmd := []string{"cp", vm.agentBinaryPath, vm.baseDir}

	if retCode, _, stderr := Utils.Run(cpCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Copy agent to workload VM failed %s : %s", vm.Name(), stderr)
	}

	vm.logger.Printf("Starting VM agent on node %s", vm.Name())

	sudoAgtCmd := []string{vm.baseDir + "/" + filepath.Base(agentBinary)}

	if ctx, _, err := vm.RunCommand(sudoAgtCmd, "", 0, 0, true, true); ctx.ExitCode != 0 {
		vm.logger.Errorf("Starting VM agent on node %s failed", vm.Name())
		return errors.Wrap(err, ctx.Stdout)
	}

	agentURL := fmt.Sprintf("%s:%d", vm.ip, constants.IotaAgentPort)
	c, err := constants.CreateNewGRPCClient(vm.Name(), agentURL, constants.GrpcMaxMsgSize)
	if err != nil {
		errorMsg := fmt.Sprintf("Could not create GRPC Connection to IOTA VM Agent. Err: %v", err)
		return errors.Wrap(err, errorMsg)
	}

	vm.agentClient = iota.NewIotaAgentApiClient(c.Client)

	req := &iota.Node{Name: vm.Name(), IpAddress: "",
		Type: iota.PersonalityType_PERSONALITY_COMMAND_NODE,
	}

	resp, err := vm.agentClient.AddNode(context.Background(), req)
	if err != nil {
		vm.logger.Errorf("Failed add not grpc messeage ")
		return err
	}

	if resp.NodeStatus.GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		vm.logger.Errorf("Failed to add naples esx personality %v", resp.NodeStatus.GetErrorMsg())
		return err
	}

	return nil
}

func (vm *vmESXWorkload) SetConnector(cluster, host string, conn interface{}) {
	vm.host = conn.(*vmware.Host)
}

func (vm *vmVcenterWorkload) SetConnector(cluster, host string, conn interface{}) {
	vm.vhost = conn.(vmware.EntityIntf)
	vm.hostName = host
	vm.cluster = cluster
	log.Infof("Setting vcenter connector with host %v %v", host, vm.vhost)
}

func (vm *vmESXWorkload) BringUp(args ...string) error {
	var dataVMDir string
	var vmInfo *vmware.VMInfo
	var err error

	vm.vmName = args[0]
	dataVMDir = args[1]
	cpu := uint(0)
	memory := uint(0)

	if val, err := strconv.Atoi(args[2]); err == nil && val != 0 {
		cpu = (uint)(val)
	}

	if val, err := strconv.Atoi(args[3]); err == nil && val != 0 {
		memory = (uint)(val)
	}

	vm.agentBinaryPath = args[4]
	//Power on is not consistent.
	host := vm.host
	host.DestoryVM(vm.vmName)
	if !host.VMExists(vm.vmName) {
		vm.logger.Info("Deploying VM...")
		vmInfo, err = host.DeployVM("", "", vm.vmName, cpu, memory,
			constants.EsxDataVMNetworks, dataVMDir)
		if err != nil {
			return errors.Wrap(err, "Deploy VM failed")
		}
	} else {
		vmInfo, err = host.BootVM(vm.vmName)
		if err != nil {
			return errors.Wrap(err, "Booting Deployed VM failed")
		}
	}

	vm.logger.Info("Deploying VM Complete...")
	vm.ip = vmInfo.IP
	if err = vm.waitForVMUp(restartTimeout); err != nil {
		return errors.Wrap(err, "SSH connection failed")
	}

	vm.logger.Info("VM is up now..")
	vm.vm, err = host.NewVM(vm.vmName)
	if err != nil {
		return errors.Wrap(err, "Could not find deployed VM")
	}

	//CAll remote workload bring up to set up SSH handles
	vm.logger.Infof("Connecting to  VM %v at %v:%v", vm.vmName, vm.ip, sshPort)
	err = vm.remoteWorkload.BringUp(vm.ip, strconv.Itoa(sshPort), constants.EsxDataVMUsername, constants.EsxDataVMPassword)
	if err != nil {
		return errors.Wrap(err, "Remote workload bring up failed")
	}

	err = vm.remoteWorkload.mountDirectory(constants.EsxDataVMUsername, constants.EsxDataVMPassword, vm.baseDir, vm.baseDir)

	if err != nil {
		vm.logger.Errorf("Mounting VM directory failed")
	}

	cmd := []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh1=1024"}
	vm.RunCommand(cmd, "", 0, 0, false, true)
	cmd = []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh2=2048"}
	vm.RunCommand(cmd, "", 0, 0, false, true)
	cmd = []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh3=4096"}
	vm.RunCommand(cmd, "", 0, 0, false, true)

	return vm.startVMAgent()
}

func (vm *vmESXWorkload) AddInterface(spec InterfaceSpec) (string, error) {

	vsspec := vmware.VswitchSpec{Name: vm.Switch()}

	nwName := constants.EsxDataNWPrefix + strconv.Itoa(spec.PrimaryVlan)

	nws := []vmware.NWSpec{{Name: nwName, Vlan: int32(spec.PrimaryVlan)}}
	if err := vm.host.AddNetworks(nws, vsspec); err != nil {
		//return "", errors.Wrap(err, "Error in creating network")
	}

	if err := vm.vm.ReconfigureNetwork(constants.EsxDefaultNetwork, nwName, 0); err != nil {
		return "", errors.Wrap(err, "Error in Reconfiguring Def network")
	}

	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterface); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on interface failed")
	}

	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterfaceExtra1); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on extra interface failed")
	}
	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterfaceExtra2); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on extra interface failed")
	}
	if spec.Mac != "" {
		var setMacAddrCmd []string
		//setMacAddrCmd = []string{"ifconfig", intfToAttach, "ether", macAddress}
		setMacAddrCmd = []string{"ifconfig", constants.EsxDataVMInterface, "hw", "ether", spec.Mac}
		if ctx, _, err := vm.RunCommand(setMacAddrCmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if spec.IPV4Address != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, spec.IPV4Address}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if spec.IPV6Address != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, "inet6", "add", spec.IPV6Address}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	cmd := []string{"sudo", "sysctl", "-w", "net.ipv4.neigh." + constants.EsxDataVMInterface + ".retrans_time_ms=" + strconv.Itoa(arpTimeout)}
	if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
		return "", errors.Wrap(err, "ARP retrans timeout set failed")
	}

	return constants.EsxDataVMInterface, nil
}

func (vm *vmESXWorkloadBase) MoveInterface(name string) error {
	/* Move interface is not supported for ESX workloads */
	return errors.New("move interface not supported for esx workloads")
}

func (vm *vmESXWorkloadBase) RunCommand(cmd []string, dir string, retries, timeout uint32, background bool, shell bool) (*Cmd.CmdCtx, string, error) {

	var runCmd string
	if dir != "" {
		runCmd = "cd " + vm.baseDir + "/" + dir + " && " + strings.Join(cmd, " ")
	} else {
		runCmd = "cd " + vm.baseDir + " && " + strings.Join(cmd, " ")
	}

	if !background {
		var cmdInfo *Cmd.CmdInfo
		for i := (uint32)(0); i <= retries; i++ {
			cmdInfo, _ = Cmd.RunSSHCommand(vm.sshHandle, runCmd, timeout, true, false, vm.logger)
			//Comand got executed on remote node, break up
			if cmdInfo.Ctx.ExitCode == 0 {
				break
			}
			vm.logger.Info("Command failed, retrying")
		}
		return cmdInfo.Ctx, "", nil
	}

	cmdInfo, err := Cmd.StartSSHBgCommand(vm.sshHandle, runCmd, true)
	if err != nil {
		return &Cmd.CmdCtx{ExitCode: 1, Stderr: err.Error()}, "", nil
	}
	handleKey := vm.genBgCmdHandle()
	vm.bgCmds.Store(handleKey, cmdInfo)

	return cmdInfo.Ctx, handleKey, nil
}

func (vm *vmESXWorkloadBase) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	vm.logger.Println("Sending Arp probe on intf : " + intf)
	return RunArpCmd(vm, ip, intf)

}

//GetWorkloadAgent handle.
func (vm *vmESXWorkloadBase) GetWorkloadAgent() interface{} {
	return vm.agentClient
}

//SetsWorkloadAgent sets workload agent
func (vm *vmESXWorkloadBase) SetWorkloadAgent(agent interface{}) {
	vm.agentClient = agent.(iota.IotaAgentApiClient)
}

func (vm *vmESXWorkloadBase) MgmtIP() string {
	return vm.ip
}

func (vm *vmESXWorkloadBase) SetMgmtIP(ip string) {
	vm.ip = ip
}

func (vm *vmESXWorkloadBase) Host() string {
	return vm.hostName
}

//TearDown VM workload, just power off.
func (vm *vmESXWorkloadBase) TearDown() {

	//Stop all bg cmds first
	vm.workloadBase.TearDown()
	if vm.vm != nil {
		log.Errorf("Tearing down vm %v", vm.Name())
		//Power off VM in ESX, Deploy is expensive
		vm.vm.PowerOff()
		vm.vm.Destroy()
	} else {
		log.Errorf("VM reference nil, cannot teardown %v %p %p", vm.Name(), vm, vm.vm)
	}
}

func (vm *vmVcenterWorkload) BringUp(args ...string) error {
	var dataVMDir string
	var clusterName string
	var hostName string
	var vmInfo *vmware.VMInfo
	var err error

	vm.vmName = args[0]
	dataVMDir = args[1]
	clusterName = args[2]
	hostName = args[3]
	vm.agentBinaryPath = args[4]
	cpu := uint(constants.EsxDataVMCpus)
	memory := uint(constants.EsxDataVMMemory)

	if len(args) > 5 {
		if val, err := strconv.Atoi(args[5]); err == nil && val != 0 {
			cpu = (uint)(val)
		}
	}

	if len(args) > 6 {
		if val, err := strconv.Atoi(args[6]); err == nil && val != 0 {
			memory = (uint)(val)
		}
	}

	//Power on is not consistent.
	host := vm.vhost
	log.Infof("Bringing up workload on vcenter handle %v", host)
	host.DestoryVM(vm.vmName)
	if !host.VMExists(vm.vmName) {
		vm.logger.Infof("Deploying VM on cluster %v, host %v", clusterName, hostName)
		vmInfo, err = host.DeployVM(clusterName, hostName, vm.vmName, cpu, memory,
			constants.EsxDataVMNetworks, dataVMDir)
		if err != nil {
			return errors.Wrap(err, "Deploy VM failed")
		}
	} else {
		vmInfo, err = host.BootVM(vm.vmName)
		if err != nil {
			return errors.Wrap(err, "Booting Deployed VM failed")
		}
	}

	vm.logger.Info("Deploying VM Complete...")
	vm.ip = vmInfo.IP
	if err = vm.waitForVMUp(restartTimeout); err != nil {
		return errors.Wrap(err, "SSH connection failed")
	}

	vm.logger.Info("VM is up now..")
	vm.vm, err = host.NewVM(vm.vmName)
	if err != nil {
		return errors.Wrap(err, "Could not find deployed VM")
	}

	//CAll remote workload bring up to set up SSH handles
	vm.logger.Infof("Connecting to  VM %v at %v:%v", vm.vmName, vm.ip, sshPort)
	err = vm.remoteWorkload.BringUp(vm.ip, strconv.Itoa(sshPort), constants.EsxDataVMUsername, constants.EsxDataVMPassword)
	if err != nil {
		return errors.Wrap(err, "Remote workload bring up failed")
	}

	err = vm.remoteWorkload.mountDirectory(constants.EsxDataVMUsername, constants.EsxDataVMPassword, vm.baseDir, vm.baseDir)

	if err != nil {
		vm.logger.Errorf("Mounting VM directory failed")
	}

	cmd := []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh1=1024"}
	vm.RunCommand(cmd, "", 0, 0, false, true)
	cmd = []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh2=2048"}
	vm.RunCommand(cmd, "", 0, 0, false, true)
	cmd = []string{"sysctl", "-w", "net.ipv4.neigh.default.gc_thresh3=4096"}
	vm.RunCommand(cmd, "", 0, 0, false, true)

	return vm.startVMAgent()
}

func (vm *vmVcenterWorkload) AddInterface(spec InterfaceSpec) (string, error) {
	var pgName string
	var err error

	//vsname := constants.VcenterDCDvs
	vsname := spec.Switch

	log.Errorf("Add interface %v %p %p", vm.Name(), vm, vm.vm)
	relaxSecurity := false
	if spec.IntfType == iota.InterfaceType_INTERFACE_TYPE_DVS_PVLAN.String() {

		private := false
		pgName = constants.EsxDataNWPrefix + strconv.Itoa(spec.PrimaryVlan)
		if spec.SecondaryVlan != 0 {
			pgName = constants.EsxDataNWPrefix + strconv.Itoa(spec.SecondaryVlan)
			private = true
		}
		//Create the port group
		err = vm.vhost.AddPortGroupToDvs(vsname,
			[]vmware.DvsPortGroup{vmware.DvsPortGroup{Name: pgName,
				VlanOverride: true,
				Private:      private,
				Ports:        32, Type: "earlyBinding",
				Vlan: int32(spec.PrimaryVlan)}})

		if err != nil && !strings.Contains(err.Error(), "already exists") {
			return "", errors.Wrap(err, "Failed to add portgroup to dvs")
		}

	} else {
		if spec.NetworkName != "" {
			//Use network name specified.
			pgName = spec.NetworkName
			relaxSecurity = true

		} else {
			pgName, err = vm.vhost.FindDvsPortGroup(vsname, vmware.DvsPGMatchCriteria{Type: vmware.DvsVlanID, VlanID: int32(spec.PrimaryVlan)})
			if err != nil {
				pgName = constants.EsxDataNWPrefix + strconv.Itoa(spec.PrimaryVlan)
				//Create the port group
				err = vm.vhost.AddPortGroupToDvs(vsname,
					[]vmware.DvsPortGroup{vmware.DvsPortGroup{Name: pgName,
						VlanOverride: true,
						Ports:        32, Type: "earlyBinding",
						Vlan: int32(spec.PrimaryVlan)}})
				if err != nil {
					return "", errors.Wrap(err, "Failed to add portgroup to dvs")
				}
			}
		}
	}

	if err := vm.vm.ReconfigureNetwork(constants.EsxDefaultNetwork, pgName, 1); err != nil {
		return "", errors.Wrapf(err, "Error in Reconfiguring Def network to %v", pgName)
	}

	if relaxSecurity {
		err := vm.vhost.RelaxSecurityOnPg(vsname, pgName)
		if err != nil {
			return "", errors.Wrap(err, "Failed to relax security on pg")
		}
	}

	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterface); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on interface failed")
	}

	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterfaceExtra1); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on extra interface failed")
	}
	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterfaceExtra2); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on extra interface failed")
	}
	if spec.Mac != "" {
		var setMacAddrCmd []string
		//setMacAddrCmd = []string{"ifconfig", intfToAttach, "ether", macAddress}
		setMacAddrCmd = []string{"ifconfig", constants.EsxDataVMInterface, "hw", "ether", spec.Mac}
		if ctx, _, err := vm.RunCommand(setMacAddrCmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if spec.IPV4Address != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, spec.IPV4Address}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if spec.IPV6Address != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, "inet6", "add", spec.IPV6Address}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	cmd := []string{"sudo", "sysctl", "-w", "net.ipv4.neigh." + constants.EsxDataVMInterface + ".retrans_time_ms=" + strconv.Itoa(arpTimeout)}
	if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
		return "", errors.Wrap(err, "ARP retrans timeout set failed")
	}

	return constants.EsxDataVMInterface, nil
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

//GetWorkloadAgent handle.
func (vm *vmVcenterWorkload) GetWorkloadAgent() interface{} {
	return vm.agentClient
}

func (vm *vmVcenterWorkload) startVMAgent(args ...string) error {

	//VM workloads are linux today
	agentBinary := constants.IotaAgentBinaryPathLinux

	addr := fmt.Sprintf("%s:%d", vm.ip, constants.SSHPort)

	cfg := InitSSHConfig(constants.EsxDataVMUsername, constants.EsxDataVMPassword)
	copyHandle := copier.NewCopier(cfg)

	if err := copyHandle.CopyTo(addr, vm.baseDir, []string{vm.agentBinaryPath}); err != nil {
		vm.logger.Errorf("Error copying  VM agent on node %s failed", vm.Name())
		return fmt.Errorf("CopyTo node failed, TestNode: %v, IPAddress: %v , Err: %v", vm.Name(), addr, err)
	}

	vm.logger.Printf("Starting VM agent on node %s", vm.Name())

	sudoAgtCmd := []string{vm.baseDir + "/" + filepath.Base(agentBinary)}

	if ctx, _, err := vm.RunCommand(sudoAgtCmd, "", 0, 0, true, true); ctx.ExitCode != 0 {
		vm.logger.Errorf("Starting VM agent on node %s failed", vm.Name())
		return errors.Wrap(err, ctx.Stdout)
	}

	agentURL := fmt.Sprintf("%s:%d", vm.ip, constants.IotaAgentPort)
	c, err := constants.CreateNewGRPCClient(vm.Name(), agentURL, constants.GrpcMaxMsgSize)
	if err != nil {
		errorMsg := fmt.Sprintf("Could not create GRPC Connection to IOTA VM Agent. Err: %v", err)
		return errors.Wrap(err, errorMsg)
	}

	vm.agentClient = iota.NewIotaAgentApiClient(c.Client)

	req := &iota.Node{Name: vm.Name(), IpAddress: "",
		Type: iota.PersonalityType_PERSONALITY_COMMAND_NODE,
	}

	resp, err := vm.agentClient.AddNode(context.Background(), req)
	if err != nil {
		vm.logger.Errorf("Failed add not grpc messeage ")
		return err
	}

	if resp.NodeStatus.GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		vm.logger.Errorf("Failed to add naples esx personality %v", resp.NodeStatus.GetErrorMsg())
		return err
	}

	return nil
}
