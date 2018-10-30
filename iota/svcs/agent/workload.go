package agent

import (
	"bytes"
	"fmt"
	"io"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
	log "github.com/sirupsen/logrus"
)

const (
	workloadTypeContainer = "container"
	workloadTypeVM        = "vm"
	workloadTypeBareMetal = "bare-metal"
	workloadTypeRemote    = "remote"
	bgCmdHandlePrefix     = "bg-cmd"
)

var (
	workloadDir = Common.DstIotaWorkloadsDir
)

type workload interface {
	entity
}

type workloadBase struct {
	name       string
	bgCmds     map[string]*commandHandle
	logger     *log.Logger
	bgCmdIndex uint32
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

func (app *workloadBase) Name() string {
	return app.name
}

func (app *workloadBase) BringUp(args ...string) error {
	return nil
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

func (app *workloadBase) AddInterface(name string, macAddress string, ipaddress string, vlan int) error {
	return nil
}

func (app *workloadBase) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (*commandCtx, error) {
	return nil, nil
}

func (app *workloadBase) StopCommand(commandHandle string) (*commandCtx, error) {
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
		app.containerHandle, err = Utils.NewContainer(name, image, "", workloadDir)
	} else {
		/* Workload already spun up */
		app.containerHandle, err = Utils.NewContainer(name, "", name, workloadDir)
	}

	if err != nil || app.containerHandle == nil {
		return errors.Errorf("Failed to do bring up of %s", name)
	}

	app.name = name
	app.bgCmds = make(map[string]*commandHandle)
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

var runArpCmd = func(app workload, ip string, intf string) error {
	arpCmd := []string{"arping", "-c", "5", "-U", ip, "-I", intf}
	cmdResp, _ := app.RunCommand(arpCmd, 0, false, false)
	if cmdResp.exitCode != 0 {
		errors.Errorf("Could not send arprobe for  %s (%s) : %s", ip, intf, cmdResp.stdout)
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

	return runArpCmd(app, ip, intf)

}

func (app *containerWorkload) AddInterface(name string, macAddress string, ipaddress string, vlan int) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}
	intfToAttach := name

	if vlan != 0 {
		vlanintf := vlanIntf(name, vlan)
		addVlanCmd := []string{"ip", "link", "add", "link", name, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
		if retCode, stdout, _ := Utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", name, vlan, stdout)
		}
		intfToAttach = vlanintf
	}

	if err := app.containerHandle.AttachInterface(intfToAttach); err != nil {
		return errors.Wrap(err, "Interface attach failed")
	}

	if macAddress != "" {
		if err := app.containerHandle.SetMacAddress(intfToAttach, macAddress, 0); err != nil {
			return errors.Wrapf(err, "Set Mac Address failed")
		}
	}

	if ipaddress != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, ipaddress, 0); err != nil {
			return errors.Wrapf(err, "Set IP Address failed")
		}
	}

	return nil
}

func (app *containerWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (*commandCtx, error) {

	cmdCtx := commandCtx{}
	containerCmdHandle, err := app.containerHandle.SetUpCommand(cmd, timeout, background, shell)
	if err != nil {
		return nil, errors.Wrap(err, "Set up command failed")
	}

	if !background {
		cmdResp, _ := app.containerHandle.RunCommand(containerCmdHandle)
		cmdCtx.done = true
		cmdCtx.exitCode = cmdResp.RetCode
		cmdCtx.stdout = cmdResp.Stdout
		cmdCtx.stderr = cmdResp.Stderr
		return &cmdCtx, nil
	}

	cmdHandle := commandHandle{ctx: &cmdCtx}
	go func(ctx *commandCtx) {
		cmdResp, _ := app.containerHandle.RunCommand(containerCmdHandle)
		cmdCtx.done = true
		ctx.exitCode = cmdResp.RetCode
		ctx.stdout = cmdResp.Stdout
		ctx.stderr = cmdResp.Stderr
	}(&cmdCtx)

	cmdCtx.handleKey = fmt.Sprintf("%s-%v", bgCmdHandlePrefix, app.bgCmdIndex)
	app.bgCmdIndex++
	cmdHandle.handle = (string)(containerCmdHandle)
	app.bgCmds[cmdCtx.handleKey] = &cmdHandle

	/* Give it couple of seconds to make sure command has started */
	time.Sleep(2 * time.Second)

	return &cmdCtx, nil
}

func (app *containerWorkload) StopCommand(commandHandle string) (*commandCtx, error) {

	cmdCtx, ok := app.bgCmds[commandHandle]
	if !ok {
		return &commandCtx{exitCode: -1, stdout: "", stderr: ""}, nil
	}

	if !cmdCtx.ctx.done {
		app.containerHandle.StopBgCommand((Utils.CommandHandle)(cmdCtx.handle.(string)))
	}

	/* Give it 1 second to dump output */
	time.Sleep(1 * time.Second)

	delete(app.bgCmds, commandHandle)
	return cmdCtx.ctx, nil
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

	return runArpCmd(app, ip, intf)

}

func (app *bareMetalWorkload) AddInterface(name string, macAddress string, ipaddress string, vlan int) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}
	intfToAttach := name

	if vlan != 0 {
		vlanintf := vlanIntf(name, vlan)
		addVlanCmd := []string{"ip", "link", "add", "link", name, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
		if retCode, stdout, _ := Utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", name, vlan, stdout)
		}
		intfToAttach = vlanintf
	}

	if macAddress != "" {
		setMacAddrCmd := []string{"ifconfig", intfToAttach, "hw", "ether", macAddress}
		if retCode, stdout, err := Utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", intfToAttach, ipaddress}
		if retCode, stdout, err := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}

	return nil
}

func (app *bareMetalWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (*commandCtx, error) {
	cmdCtx := commandCtx{}
	retCode, stdout, _ := Utils.Run(cmd, (int)(timeout), background, shell, nil)
	cmdCtx.exitCode = (int32)(retCode)
	cmdCtx.stdout = stdout
	cmdCtx.done = true
	return &cmdCtx, nil
}

func (app *bareMetalWorkload) StopCommand(commandHandle string) (*commandCtx, error) {
	return nil, nil
}

func (app *remoteWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (*commandCtx, error) {
	var stdoutBuf, stderrBuf bytes.Buffer
	cmdCtx := commandCtx{}
	if !background {
		var retCode int
		retCode, cmdCtx.stdout, cmdCtx.stderr = Utils.RunSSHCommand(app.sshHandle, strings.Join(cmd, " "), false, false, app.logger)
		cmdCtx.done = true
		cmdCtx.exitCode = (int32)(retCode)
		return &cmdCtx, nil
	}

	sshSession, sshOut, sshErr, err := Utils.CreateSSHSession(app.sshHandle)
	if err != nil {
		return nil, errors.Wrap(err, "Error in sss session creation")
	}

	cmdHandle := commandHandle{ctx: &cmdCtx}
	shout := io.MultiWriter(&stdoutBuf)
	ssherr := io.MultiWriter(&stderrBuf)

	go func(ctx *commandCtx) {
		go func() {
			io.Copy(shout, sshOut)
		}()
		go func() {

			io.Copy(ssherr, sshErr)
		}()

		fullCmd := "sh -c \"" + strings.Join(cmd, " ") + "\""

		app.logger.Println("Starting background cmd ", fullCmd)
		err := sshSession.Start(fullCmd)
		if err == nil {
			err := sshSession.Wait()
			defer sshSession.Close()
			switch v := err.(type) {
			case *ssh.ExitError:
				ctx.exitCode = (int32)(v.Waitmsg.ExitStatus())
			default:
				ctx.exitCode = -1
			}
			ctx.stderr = stderrBuf.String()
			ctx.stdout = stdoutBuf.String()
		} else {
			ctx.exitCode = 1
			ctx.stderr = "Failed to start cmd : " + strings.Join(cmd, " ")
		}

		ctx.done = true
		app.logger.Println("Command done", strings.Join(cmd, " "), stderrBuf.String(), stdoutBuf.String())
	}(&cmdCtx)

	cmdHandle.handle = sshSession
	cmdCtx.handleKey = fmt.Sprintf("%s-%v", bgCmdHandlePrefix, app.bgCmdIndex)
	app.bgCmdIndex++
	app.bgCmds[cmdCtx.handleKey] = &cmdHandle

	/* Give it couple of seconds to make sure command has started */
	time.Sleep(2 * time.Second)
	return &cmdCtx, nil
}

func (app *remoteWorkload) BringUp(args ...string) error {
	var err error

	app.ip = args[0]
	app.port = args[1]
	app.username = args[2]
	app.password = args[3]

	config := &ssh.ClientConfig{
		User: app.username,
		Auth: []ssh.AuthMethod{
			ssh.Password(app.password),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	fmt.Println("App : ", app.ip, app.port, app.username, app.password)
	if app.sshHandle, err = ssh.Dial("tcp", app.ip+":"+app.port, config); err != nil {
		err = errors.Wrapf(err, "SSH connect failed")
		return err
	}

	//app.name = name
	app.bgCmds = make(map[string]*commandHandle)
	return err
}

func (app *remoteWorkload) StopCommand(commandHandle string) (*commandCtx, error) {

	cmdCtx, ok := app.bgCmds[commandHandle]
	if !ok {
		return &commandCtx{exitCode: -1, stdout: "", stderr: ""}, nil
	}

	if !cmdCtx.ctx.done {
		app.logger.Println("Stopping cmd with handle : ", commandHandle)
		sshSession := cmdCtx.handle.(*ssh.Session)
		sshSession.Close()
		/* Give it 1 second to dump output */
		time.Sleep(1 * time.Second)
	} else {
		app.logger.Println("Command already done : ", commandHandle)
	}

	delete(app.bgCmds, commandHandle)
	return cmdCtx.ctx, nil
}

func newContainerWorkload(logger *log.Logger) workload {
	return &containerWorkload{workloadBase: workloadBase{logger: logger}}
}

func newBareMetalWorkload(logger *log.Logger) workload {
	return &bareMetalWorkload{workloadBase: workloadBase{logger: logger}}
}

func newVMWorkload(logger *log.Logger) workload {
	return &vmWorkload{workloadBase: workloadBase{logger: logger}}
}

func newRemoteWorkload(logger *log.Logger) workload {
	return &remoteWorkload{workloadBase: workloadBase{logger: logger}}
}

var iotaWorkloads = map[string]func(logger *log.Logger) workload{
	workloadTypeContainer: newContainerWorkload,
	workloadTypeVM:        newVMWorkload,
	workloadTypeBareMetal: newBareMetalWorkload,
	workloadTypeRemote:    newRemoteWorkload,
}

func newWorkload(workloadType string, logger *log.Logger) workload {
	if _, ok := iotaWorkloads[workloadType]; ok {
		return iotaWorkloads[workloadType](logger)
	}

	return nil
}
