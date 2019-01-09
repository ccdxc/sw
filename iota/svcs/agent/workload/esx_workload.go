package workload

import (
	"context"
	"fmt"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	constants "github.com/pensando/sw/iota/svcs/common"

	"github.com/pensando/sw/iota/svcs/common/vmware"
	"github.com/pkg/errors"
	log "github.com/sirupsen/logrus"
)

const (
	sshPort = 22

	restartTimeout = 300 //300 seconds for node restart
)

type vmESXWorkload struct {
	remoteWorkload
	vmName       string
	host         *vmware.Host
	vm           *vmware.VM
	hostIP       string
	hostUsername string
	hostPassword string
}

func (vm *vmESXWorkload) downloadDataVMImage(image string) (string, error) {

	dataVMDir := constants.DataVMImageDirectory + "/" + image
	imageName := image + ".ova"
	cwd, _ := os.Getwd()
	mkdir := []string{"mkdir", "-p", dataVMDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	os.Chdir(dataVMDir)
	defer os.Chdir(cwd)

	buildIt := []string{"/usr/local/bin/buildit", "-t", constants.BuildItURL, "image", "pull", "-o", imageName, image}
	if stdout, err := exec.Command(buildIt[0], buildIt[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	vm.logger.Info("Download complete for VM image")
	tarCmd := []string{"tar", "-xvf", imageName}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	return dataVMDir, nil
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

func (vm *vmESXWorkload) BringUp(args ...string) error {
	var dataVMDir string
	var err error

	vm.vmName = args[0]
	image := args[1]
	vm.hostIP = args[2]
	vm.hostUsername = args[3]
	vm.hostPassword = args[4]

	if dataVMDir, err = vm.downloadDataVMImage(image); err != nil {
		return errors.Wrap(err, "Download control image failed")
	}

	host, err := vmware.NewHost(context.Background(), vm.hostIP, vm.hostUsername, vm.hostPassword)
	if err != nil {
		return errors.Wrap(err, "Cannot connect to ESX Host")
	}

	host.DestoryVM(vm.vmName)

	vm.logger.Info("Deploying VM...")
	vmInfo, err := host.DeployVM(vm.vmName, constants.EsxControlVMCpus, constants.EsxControlVMMemory, constants.EsxDataVMNetworks, dataVMDir)
	if err != nil {
		return errors.Wrap(err, "Deploy VM failed")
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
	err = vm.remoteWorkload.BringUp(vm.ip, strconv.Itoa(sshPort), constants.EsxDataVMUsername, constants.EsxDataVMPassword)
	if err != nil {
		return errors.Wrap(err, "Remote workload bring up failed")
	}

	return vm.remoteWorkload.mountDirectory(constants.EsxDataVMUsername, constants.EsxDataVMPassword, vm.baseDir, vm.baseDir)
}

func (vm *vmESXWorkload) AddInterface(name string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error) {

	vsname := constants.EsxIotaDataSwitch
	vsspec := vmware.VswitchSpec{Name: vsname}

	nwName := constants.EsxDataNWPrefix + strconv.Itoa(vlan)

	nws := []vmware.NWSpec{{Name: nwName, Vlan: int32(vlan)}}
	if _, err := vm.host.AddNetworks(nws, vsspec); err != nil {
		//return "", errors.Wrap(err, "Error in creating network")
	}

	/*
		if err := vm.host.PowerOffVM(vm.vmName); err != nil {
			return "", errors.Wrap(err, "Failed to power off VM")
		}
	*/
	if err := vm.vm.ReconfigureNetwork(constants.EsxVMNetwork, nwName); err != nil {
		return "", errors.Wrap(err, "Error in Reconfiguring VM network")
	}

	/*
		if _, err := vm.host.BootVM(vm.vmName); err != nil {
			return "", errors.Wrap(err, "Failed to power off VM")
		}

		if err := vm.waitForVMUp(restartTimeout); err != nil {
			return "", errors.Wrap(err, "VM did not come up after reconfiguration.")
		}

		//CAll remote workload bring up to set up SSH handles
		err := vm.remoteWorkload.BringUp(vm.ip, strconv.Itoa(sshPort), constants.EsxDataVMUsername, constants.EsxDataVMPassword)
		if err != nil {
			return "", errors.Wrap(err, "Remote workload bring up failed after reconfiguration")
		}

	*/
	if err := Utils.DisableDhcpOnInterfaceRemote(vm.sshHandle, constants.EsxDataVMInterface); err != nil {
		return "", errors.Wrap(err, "Disabling DHCP on interface failed")
	}

	if macAddress != "" {
		var setMacAddrCmd []string
		//setMacAddrCmd = []string{"ifconfig", intfToAttach, "ether", macAddress}
		setMacAddrCmd = []string{"ifconfig", constants.EsxDataVMInterface, "hw", "ether", macAddress}
		if ctx, _, err := vm.RunCommand(setMacAddrCmd, "", 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, ipaddress}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", constants.EsxDataVMInterface, "inet6", "add", ipv6address}
		if ctx, _, err := vm.RunCommand(cmd, "", 0, false, true); ctx.ExitCode != 0 {
			return "", errors.Wrap(err, ctx.Stdout)
		}
	}

	return constants.EsxDataVMInterface, nil
}

func (vm *vmESXWorkload) MoveInterface(name string) error {
	/* Move interface is not supported for ESX workloads */
	return errors.New("move interface not supported for esx workloads")
}

func (vm *vmESXWorkload) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {

	var runCmd string
	if dir != "" {
		runCmd = "cd " + vm.baseDir + "/" + dir + " && " + strings.Join(cmd, " ")
	} else {
		runCmd = "cd " + vm.baseDir + " && " + strings.Join(cmd, " ")
	}

	if !background {
		cmdInfo, _ := Cmd.RunSSHCommand(vm.sshHandle, runCmd, timeout, true, false, vm.logger)
		return cmdInfo.Ctx, "", nil
	}

	cmdInfo, _ := Cmd.StartSSHBgCommand(vm.sshHandle, runCmd, true)
	handleKey := vm.genBgCmdHandle()
	vm.bgCmds[handleKey] = cmdInfo

	return cmdInfo.Ctx, handleKey, nil
}

func (vm *vmESXWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	vm.logger.Println("Sending Arp probe on intf : " + intf)
	return RunArpCmd(vm, ip, intf)

}
