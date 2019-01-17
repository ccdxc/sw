package workload

import (
	"fmt"
	"os"
	"runtime"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	log "github.com/sirupsen/logrus"

	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
)

const (
	//WorkloadTypeContainer container workload
	WorkloadTypeContainer = "container"
	//WorkloadTypeVM vm workload
	WorkloadTypeVM = "vm"
	//WorkloadTypeBareMetal bare-metal workload
	WorkloadTypeBareMetal = "bare-metal"
	//WorkloadTypeRemote remote workload
	WorkloadTypeRemote = "remote"
	bgCmdHandlePrefix  = "bg-cmd"

	//WorkloadTypeESX vm workload
	WorkloadTypeESX = "esx-vm"
)

var (
	//ContainerPrivileged run container in privileged mode
	ContainerPrivileged = true
)

//Workload interface
type Workload interface {
	Name() string
	BringUp(args ...string) error
	Reinit() error
	SetBaseDir(dir string) error
	RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error)
	StopCommand(commandHandle string) (*Cmd.CommandCtx, error)
	AddInterface(name string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error)
	MoveInterface(name string) error
	IsHealthy() bool
	SendArpProbe(ip string, intf string, vlan int) error
	TearDown()
}

type workload interface {
}

type workloadBase struct {
	name       string
	parent     string
	bgCmds     map[string]*Cmd.CommandInfo
	logger     *log.Logger
	bgCmdIndex uint32
	baseDir    string
}

type remoteWorkload struct {
	workloadBase
	ip        string
	port      string
	username  string
	password  string
	sshHandle *ssh.Client
}

type bareMetalWorkload struct {
	workloadBase
}

type vmWorkload struct {
	workloadBase
}

type containerWorkload struct {
	workloadBase
	containerHandle *Utils.Container
}

func vlanIntf(name string, vlan int) string {
	return name + "_" + strconv.Itoa(vlan)
}

func freebsdVlanIntf(name string, vlan int) string {
	return name + "." + strconv.Itoa(vlan)
}

func (app *workloadBase) Name() string {
	return app.name
}

func (app *workloadBase) BringUp(args ...string) error {
	return nil
}

func (app *workloadBase) Reinit() error {
	return nil
}

func (app *workloadBase) SetBaseDir(dir string) error {
	os.Mkdir(dir, 0765)
	os.Chmod(dir, 0777)
	app.baseDir = dir
	return nil
}

func (app *workloadBase) genBgCmdHandle() string {
	handleKey := fmt.Sprintf("%s-%s-%s-%v", app.parent, app.name, bgCmdHandlePrefix, app.bgCmdIndex)
	app.bgCmdIndex++
	return handleKey
}

func (app *workloadBase) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *workloadBase) MoveInterface(name string) error {
	return nil
}

func (app *workloadBase) SendArpProbe(ip string, intf string, vlan int) error {
	return nil
}

func (app *workloadBase) AddInterface(name string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error) {
	return "", nil
}

func (app *workloadBase) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {
	return nil, "", nil
}

func (app *workloadBase) StopCommand(commandHandle string) (*Cmd.CommandCtx, error) {
	return nil, nil
}

func (app *workloadBase) IsHealthy() bool {
	return true
}

func (app *workloadBase) TearDown() {
}

func (app *containerWorkload) BringUp(args ...string) error {
	var err error

	image := args[1]
	name := args[0]
	if image != "" {
		app.containerHandle, err = Utils.NewContainer(name, image, "", app.baseDir, ContainerPrivileged)
	} else {
		/* Workload already spun up */
		app.containerHandle, err = Utils.NewContainer(name, "", name, app.baseDir, ContainerPrivileged)
	}

	if err != nil || app.containerHandle == nil {
		return errors.Errorf("Failed to do bring up of %s", name)
	}

	app.name = name
	app.bgCmds = make(map[string]*Cmd.CommandInfo)
	return err
}

func (app *containerWorkload) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *containerWorkload) MoveInterface(name string) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}

	if err := app.containerHandle.AttachInterface(name); err != nil {
		return errors.Wrap(err, "Interface attach failed")
	}

	return nil

}

//RunArpCmd runs arp command on workload
var RunArpCmd = func(app Workload, ip string, intf string) error {
	arpCmd := []string{"arping", "-c", "5", "-U", ip, "-I", intf}
	cmdResp, _, _ := app.RunCommand(arpCmd, "", 0, false, false)
	if cmdResp.ExitCode != 0 {
		errors.Errorf("Could not send arprobe for  %s (%s) : %s", ip, intf, cmdResp.Stdout)
		return nil
	}
	return nil
}

func (app *containerWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	if vlan != 0 {
		intf = vlanIntf(intf, vlan)
	}

	return RunArpCmd(app, ip, intf)

}

func (app *containerWorkload) AddInterface(name string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error) {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}
	intfToAttach := name

	if vlan != 0 {
		vlanintf := vlanIntf(name, vlan)
		delVlanCmd := []string{"ip", "link", "del", vlanintf}
		//Delete the interface if already exists , ignore the error for now.
		Utils.Run(delVlanCmd, 0, false, false, nil)
		addVlanCmd := []string{"ip", "link", "add", "link", name, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
		if retCode, stdout, _ := Utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", name, vlan, stdout)
		}
		intfToAttach = vlanintf
	}

	if err := app.containerHandle.AttachInterface(intfToAttach); err != nil {
		return "", errors.Wrap(err, "Interface attach failed")
	}

	if macAddress != "" {
		if err := app.containerHandle.SetMacAddress(intfToAttach, macAddress, 0); err != nil {
			return "", errors.Wrapf(err, "Set Mac Address failed")
		}
	}

	if ipaddress != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, ipaddress, 0, false); err != nil {
			return "", errors.Wrapf(err, "Set IP Address failed")
		}
	}

	if ipv6address != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, ipv6address, 0, true); err != nil {
			return "", errors.Wrapf(err, "Set IPv6 Address failed")
		}
	}

	return intfToAttach, nil
}

func (app *containerWorkload) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {

	cmdCtx := &Cmd.CommandCtx{}
	containerCmdHandle, err := app.containerHandle.SetUpCommand(cmd, dir, background, shell)
	if err != nil {
		return nil, "", errors.Wrap(err, "Set up command failed")
	}

	if !background {
		cmdResp, _ := app.containerHandle.RunCommand(containerCmdHandle, timeout)
		cmdCtx.Done = true
		cmdCtx.Stdout = cmdResp.Stdout
		cmdCtx.Stderr = cmdResp.Stderr
		cmdCtx.ExitCode = cmdResp.RetCode
		cmdCtx.TimedOut = cmdResp.Timedout
		return cmdCtx, "", nil
	}

	go func(ctx *Cmd.CommandCtx) {
		cmdResp, _ := app.containerHandle.RunCommand(containerCmdHandle, 0)
		cmdCtx.Done = true
		cmdCtx.ExitCode = cmdResp.RetCode
		cmdCtx.Stdout = cmdResp.Stdout
		cmdCtx.Stderr = cmdResp.Stderr
	}(cmdCtx)

	handleKey := app.genBgCmdHandle()
	cmdInfo := &Cmd.CommandInfo{Ctx: cmdCtx}
	cmdInfo.Handle = (string)(containerCmdHandle)
	app.bgCmds[handleKey] = cmdInfo

	/* Give it couple of seconds to make sure command has started */
	time.Sleep(2 * time.Second)

	return cmdCtx, handleKey, nil
}

func (app *containerWorkload) StopCommand(commandHandle string) (*Cmd.CommandCtx, error) {

	cmdInfo, ok := app.bgCmds[commandHandle]
	if !ok {
		return &Cmd.CommandCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	if !cmdInfo.Ctx.Done {
		app.containerHandle.StopCommand((Utils.CommandHandle)(cmdInfo.Handle.(string)))
	}

	/* Give it 1 second to dump output */
	time.Sleep(1 * time.Second)

	delete(app.bgCmds, commandHandle)
	//For bg command, always return exit code 0
	cmdInfo.Ctx.ExitCode = 0
	return cmdInfo.Ctx, nil
}

func (app *containerWorkload) IsHealthy() bool {
	return app.containerHandle.IsHealthy()
}

func (app *containerWorkload) TearDown() {
	if app.containerHandle != nil {
		app.containerHandle.Stop()
	}
}

func (app *bareMetalWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	if vlan != 0 {
		intf = vlanIntf(intf, vlan)
	}

	return RunArpCmd(app, ip, intf)

}

func (app *bareMetalWorkload) AddInterface(name string, macAddress string, ipaddress string, ipv6address string, vlan int) (string, error) {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}
	intfToAttach := name

	if vlan != 0 {
		vlanintf := ""
		var addVlanCmd []string
		var delVlanCmd []string
		if runtime.GOOS == "freebsd" {
			vlanintf = freebsdVlanIntf(name, vlan)
			delVlanCmd = []string{"ifconfig", vlanintf, "destroy"}
			addVlanCmd = []string{"ifconfig", vlanintf, "create", "inet"}
		} else {
			vlanintf = vlanIntf(name, vlan)
			delVlanCmd = []string{"ip", "link", "del", vlanintf}
			addVlanCmd = []string{"ip", "link", "add", "link", name, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
		}
		Utils.Run(delVlanCmd, 0, false, false, nil)
		if retCode, stdout, _ := Utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", name, vlan, stdout)
		}

		intfToAttach = vlanintf
	}

	if macAddress != "" {
		var setMacAddrCmd []string
		if runtime.GOOS != "freebsd" {
			//setMacAddrCmd = []string{"ifconfig", intfToAttach, "ether", macAddress}
			setMacAddrCmd = []string{"ifconfig", intfToAttach, "hw", "ether", macAddress}
			if retCode, stdout, err := Utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
				return "", errors.Wrap(err, stdout)
			}
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", intfToAttach, ipaddress}
		if retCode, stdout, err := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if ipv6address != "" {
		//unset ipv6 address first
		cmd := []string{"ifconfig", intfToAttach, "inet6", "del", ipv6address}
		Utils.Run(cmd, 0, false, false, nil)
		cmd = []string{"ifconfig", intfToAttach, "inet6", "add", ipv6address}
		if retCode, stdout, err := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	return intfToAttach, nil
}

func (app *bareMetalWorkload) BringUp(args ...string) error {
	//app.name = name
	app.bgCmds = make(map[string]*Cmd.CommandInfo)
	return nil
}

func (app *bareMetalWorkload) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {
	handleKey := ""

	runDir := app.baseDir
	if dir != "" {
		runDir = runDir + "/" + dir
	}

	fmt.Println("base dir ", runDir, dir)

	app.logger.Println("Running cmd ", strings.Join(cmd, " "))
	cmdInfo, _ := Cmd.ExecCmd(cmd, runDir, (int)(timeout), background, shell, nil)

	if background {
		handleKey = app.genBgCmdHandle()
		app.bgCmds[handleKey] = cmdInfo
	}

	return cmdInfo.Ctx, handleKey, nil
}

func (app *bareMetalWorkload) StopCommand(commandHandle string) (*Cmd.CommandCtx, error) {
	cmdInfo, ok := app.bgCmds[commandHandle]
	if !ok {
		return &Cmd.CommandCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	app.logger.Printf("Stopping bare metal Running cmd %v %v\n", cmdInfo.Ctx.Stdout, cmdInfo.Handle)

	Cmd.StopExecCmd(cmdInfo)
	time.Sleep(2 * time.Second)

	return cmdInfo.Ctx, nil
}

func (app *remoteWorkload) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {

	var cmdInfo *Cmd.CommandInfo
	runCmd := strings.Join(cmd, " ")
	//Ignore diretory for remote workload for now
	//Even though mount is suppoted, commenting out as on naples remote
	//we don't to mount yet.
	/*if dir != "" {
		runCmd = "cd " + app.baseDir + "/" + dir + " && " + strings.Join(cmd, " ")
	} else {
		runCmd = "cd " + app.baseDir + " && " + strings.Join(cmd, " ")
	}*/

	if !background {
		for i := 0; i < 2; i++ {
			cmdInfo, _ = Cmd.RunSSHCommand(app.sshHandle, runCmd, timeout, false, false, app.logger)
			if cmdInfo.Ctx.ExitCode == Cmd.SSH_CREATION_FAILED_EXIT_CODE {
				cmdInfo.Ctx.Stderr = "SSH connection failed"
				//Try it again.
				if err := app.Reinit(); err != nil {
					return cmdInfo.Ctx, "", nil
				}
				continue
			}
			//Comand got executed on remote node, break up
			break
		}
		return cmdInfo.Ctx, "", nil
	}

	for i := 0; i < 2; i++ {
		cmdInfo, _ = Cmd.StartSSHBgCommand(app.sshHandle, runCmd, false)
		if cmdInfo.Ctx.ExitCode == Cmd.SSH_CREATION_FAILED_EXIT_CODE {
			cmdInfo.Ctx.Stderr = "SSH connection failed"
			//Try it again.
			if err := app.Reinit(); err != nil {
				return cmdInfo.Ctx, "", nil
			}
			continue
		}
		//Comand got executed on remote node, break up
		break
	}
	handleKey := app.genBgCmdHandle()
	app.bgCmds[handleKey] = cmdInfo

	return cmdInfo.Ctx, handleKey, nil
}

func (app *remoteWorkload) mountDirectory(userName string, password string, srcDir string, dstDir string) error {

	mkdir := []string{"mkdir", "-p", dstDir}
	cmdInfo, _, _ := app.RunCommand(mkdir, "", 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("mkdir command failed " + cmdInfo.Stderr)
	}

	sshKeygen := []string{"ssh-keygen", "-f", "~/.ssh/id_rsa", "-t", "rsa", "-N", "''"}
	cmdInfo, _, _ = app.RunCommand(sshKeygen, "", 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("ssh-keygen command failed " + cmdInfo.Stderr)
	}

	myIP, err := Utils.GetIPAddressOfInterface("eth0")
	if err != nil {
		return err
	}

	sshCopyID := []string{"sshpass", "-v", "-p", password, "ssh-copy-id", "-o", "StrictHostKeyChecking=no", userName + "@" + myIP}
	cmdInfo, _, _ = app.RunCommand(sshCopyID, "", 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("ssh-copy-id command failed " + cmdInfo.Stderr)
	}

	sshFS := []string{"sudo", "nohup", "sshfs", "-o", "allow_other,IdentityFile=/home/" + userName + "/.ssh/id_rsa,StrictHostKeyChecking=no", userName + "@" + myIP + ":" + srcDir, dstDir}
	cmdInfo, _, _ = app.RunCommand(sshFS, "", 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("sshfs command failed " + cmdInfo.Stderr)
	}

	return nil
}

func (app *remoteWorkload) Reinit() error {
	var err error

	config := &ssh.ClientConfig{
		User: app.username,
		Auth: []ssh.AuthMethod{
			ssh.Password(app.password),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = app.password
				}

				return answers, nil
			}),
		}, HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	fmt.Println("App : ", app.ip, app.port, app.username, app.password)
	if app.sshHandle, err = ssh.Dial("tcp", app.ip+":"+app.port, config); err != nil {
		err = errors.Wrapf(err, "SSH connect failed %v:%v@%v", app.username, app.password, app.ip)
		return err
	}

	return nil
}

func (app *remoteWorkload) BringUp(args ...string) error {
	var err error

	app.ip = args[0]
	app.port = args[1]
	app.username = args[2]
	app.password = args[3]

	err = app.Reinit()
	//app.name = name
	app.bgCmds = make(map[string]*Cmd.CommandInfo)
	return err
}

func (app *remoteWorkload) StopCommand(commandHandle string) (*Cmd.CommandCtx, error) {

	cmdInfo, ok := app.bgCmds[commandHandle]
	if !ok {
		return &Cmd.CommandCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	Cmd.StopSSHCmd(cmdInfo)

	time.Sleep(2 * time.Second)

	delete(app.bgCmds, commandHandle)
	//For bg command, always return exit code 0
	cmdInfo.Ctx.ExitCode = 0
	return cmdInfo.Ctx, nil
}

func newContainerWorkload(name string, parent string, logger *log.Logger) Workload {
	return &containerWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}
}

func newBareMetalWorkload(name string, parent string, logger *log.Logger) Workload {
	return &bareMetalWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}
}

func newVMWorkload(name string, parent string, logger *log.Logger) Workload {
	return &vmWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}
}

func newRemoteWorkload(name string, parent string, logger *log.Logger) Workload {
	return &remoteWorkload{workloadBase: workloadBase{name: name, parent: parent, logger: logger}}
}

var iotaWorkloads = map[string]func(name string, parent string, logger *log.Logger) Workload{
	WorkloadTypeContainer: newContainerWorkload,
	WorkloadTypeVM:        newVMWorkload,
	WorkloadTypeESX:       newVMESXWorkload,
	WorkloadTypeBareMetal: newBareMetalWorkload,
	WorkloadTypeRemote:    newRemoteWorkload,
}

//NewWorkload creates a workload
func NewWorkload(workloadType string, name string, parent string, logger *log.Logger) Workload {
	if _, ok := iotaWorkloads[workloadType]; ok {
		return iotaWorkloads[workloadType](name, parent, logger)
	}

	return nil
}
