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

	"github.com/pkg/errors"
	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/iota/svcs/common/vmware"
)

const (
	sshPort = 22

	restartTimeout = 300 //300 seconds for node restart

	arpTimeout = 3000 //3 seconds
)

type vmESXWorkload struct {
	remoteWorkload
	vmName       string
	host         *vmware.Host
	vm           *vmware.VM
	hostIP       string
	hostUsername string
	hostPassword string
	agentClient  iota.IotaAgentApiClient
}

func newVMESXWorkload(name string, parent string, logger *log.Logger) Workload {
	return &vmESXWorkload{remoteWorkload: remoteWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}}
}

func (vm *vmESXWorkload) waitForVMUp(timeout time.Duration) error {
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

func (vm *vmESXWorkload) startVMAgent(args ...string) error {

	//VM workloads are linux today
	agentBinary := constants.IotaAgentBinaryPathLinux

	//Directory is mounted copy there
	cpCmd := []string{"cp", constants.DstIotaAgentBinary, vm.baseDir}

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

func (vm *vmESXWorkload) BringUp(args ...string) error {
	var dataVMDir string
	var vmInfo *vmware.VMInfo

	vm.vmName = args[0]
	dataVMDir = args[1]
	vm.hostIP = args[2]
	vm.hostUsername = args[3]
	vm.hostPassword = args[4]
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

	host, err := vmware.NewHost(context.Background(), vm.hostIP, vm.hostUsername, vm.hostPassword)
	if err != nil {
		return errors.Wrap(err, "Cannot connect to ESX Host")
	}

	//Power on is not consistent.
	host.DestoryVM(vm.vmName)
	if !host.VMExists(vm.vmName) {
		vm.logger.Info("Deploying VM...")
		vmInfo, err = host.DeployVM(vm.vmName, cpu, memory,
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
	vm.host = host
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

func (vm *vmESXWorkload) AddInterface(name string, workloadInterface string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error) {

	vsname := constants.EsxIotaDataSwitch
	vsspec := vmware.VswitchSpec{Name: vsname}

	nwName := constants.EsxDataNWPrefix + strconv.Itoa(vlan)

	nws := []vmware.NWSpec{{Name: nwName, Vlan: int32(vlan)}}
	if _, err := vm.host.AddNetworks(nws, vsspec); err != nil {
		//return "", errors.Wrap(err, "Error in creating network")
	}

	if err := vm.vm.ReconfigureNetwork(constants.EsxDefaultNetwork, nwName); err != nil {
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
	if macAddress != "" {
		var setMacAddrCmd []string
		//setMacAddrCmd = []string{"ifconfig", intfToAttach, "ether", macAddress}
		setMacAddrCmd = []string{"ifconfig", constants.EsxDataVMInterface, "hw", "ether", macAddress}
		if ctx, _, err := vm.RunCommand(setMacAddrCmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, ipaddress}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, "inet6", "add", ipv6address}
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

func (vm *vmESXWorkload) MoveInterface(name string) error {
	/* Move interface is not supported for ESX workloads */
	return errors.New("move interface not supported for esx workloads")
}

func (vm *vmESXWorkload) RunCommand(cmd []string, dir string, retries, timeout uint32, background bool, shell bool) (*Cmd.CmdCtx, string, error) {

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

func (vm *vmESXWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	vm.logger.Println("Sending Arp probe on intf : " + intf)
	return RunArpCmd(vm, ip, intf)

}

//GetWorkloadAgent handle.
func (vm *vmESXWorkload) GetWorkloadAgent() interface{} {
	return vm.agentClient
}

func (vm *vmESXWorkload) MgmtIP() string {
	return vm.ip
}

//TearDown VM workload, just power off.
func (vm *vmESXWorkload) TearDown() {

	//Stop all bg cmds first
	vm.workloadBase.TearDown()
	//Power off VM in ESX, Deploy is expensive
	vm.vm.PowerOff()

	vm.vm.Destroy()
}
