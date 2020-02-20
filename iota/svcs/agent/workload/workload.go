package workload

import (
	"fmt"
	"io/ioutil"
	"os"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
	"unicode/utf8"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	log "github.com/sirupsen/logrus"

	cmd "github.com/pensando/sw/iota/svcs/agent/command"
	utils "github.com/pensando/sw/iota/svcs/agent/utils"
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

	//WorkloadTypeVcenter vcenter type worklaod
	WorkloadTypeVcenter = "vcenter-vm"

	//WorkloadTypeMacVlan  macvlan workload
	WorkloadTypeMacVlan = "mac-vlan"

	//WorkloadTypeMacVlanEncap  macvlan Encap workload
	WorkloadTypeMacVlanEncap = "mac-vlan-encap"

	//WorkloadTypeContainerMacVlan  macvlan workload
	WorkloadTypeContainerMacVlan = "container-mac-vlan"
)

var (
	//ContainerPrivileged run container in privileged mode
	ContainerPrivileged  = true
	macVlanIntfPrefixCnt uint64
)

//InterfaceSpec def
type InterfaceSpec struct {
	IntfType      string
	Parent        string
	Name          string
	Mac           string
	IPV4Address   string
	IPV6Address   string
	Switch        string
	NetworkName   string
	PrimaryVlan   int
	SecondaryVlan int
}

//Workload interface
type Workload interface {
	Name() string
	BringUp(args ...string) error
	Reinit() error
	SetBaseDir(dir string) error
	RunCommand(cmd []string, dir string, retries uint32, timeout uint32,
		background bool, shell bool) (*cmd.CmdCtx, string, error)
	StopCommand(commandHandle string) (*cmd.CmdCtx, error)
	WaitCommand(commandHandle string) (*cmd.CmdCtx, error)
	AddInterface(spec InterfaceSpec) (string, error)
	AddSecondaryIpv4Addresses(intf string, ipaddresses []string) error
	AddSecondaryIpv6Addresses(intf string, ipaddresses []string) error
	MoveInterface(name string) error
	IsHealthy() bool
	SendArpProbe(ip string, intf string, vlan int) error
	MgmtIP() string
	SetMgmtIP(string)
	GetWorkloadAgent() interface{}
	SetWorkloadAgent(interface{})
	TearDown()
	SetConnector(cluster, hostname string, conn interface{})
	Host() string
	Type() string
	SetSwitch(name string)
	Switch() string
}

func isFreeBsd() bool {
	return runtime.GOOS == "freebsd"
}

type workload interface {
}

type workloadBase struct {
	name   string
	parent string
	bgCmds *sync.Map

	logger     *log.Logger
	bgCmdIndex uint64
	baseDir    string
	wlType     string
	switchName string
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
	subIF string
}

type bareMetalMacVlanWorkload struct {
	bareMetalWorkload
}

type bareMetalMacVlanEncapWorkload struct {
	bareMetalWorkload
}

type vmWorkload struct {
	workloadBase
}

type containerWorkload struct {
	workloadBase
	containerHandle *utils.Container
	subIF           string
}

type containerMacVlanWorkload struct {
	containerWorkload
}

func vlanIntf(name string, vlan int) string {
	return name + "_" + strconv.Itoa(vlan)
}

func generateMacVlanIntfName(vlan int) string {
	cnt := atomic.AddUint64(&macVlanIntfPrefixCnt, 1)
	return "mv_" + strconv.Itoa(vlan) + "_" + strconv.FormatUint(cnt, 10)
}

func freebsdVlanIntf(name string, vlan int) string {
	return name + "." + strconv.Itoa(vlan)
}

func macVlanIntf(name string, vlan int) string {
	return name + "_" + "m" + strconv.Itoa(vlan)
}

func (app *workloadBase) SetSwitch(name string) {
	app.switchName = name
}

func (app *workloadBase) Switch() string {
	return app.switchName
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

func (app *workloadBase) SetConnector(cluster, hostname string, conn interface{}) {
}

func (app *workloadBase) Type() string {
	return app.wlType
}

func (app *workloadBase) Host() string {
	return "N/A"
}

func (app *workloadBase) SetBaseDir(dir string) error {
	os.Mkdir(dir, 0765)
	os.Chmod(dir, 0777)
	app.baseDir = dir
	return nil
}

func (app *workloadBase) genBgCmdHandle() string {
	handleKey := fmt.Sprintf("%s-%s-%s-%v", app.parent, app.name, bgCmdHandlePrefix, app.bgCmdIndex)
	atomic.AddUint64(&app.bgCmdIndex, 1)
	return handleKey
}

func (app *workloadBase) MgmtIP() string {
	return ""
}

func (app *workloadBase) SetMgmtIP(string) {
	return
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

func (app *workloadBase) AddInterface(spec InterfaceSpec) (string, error) {
	return "", nil
}

func (app *workloadBase) AddSecondaryIpv4Addresses(intf string, ipaddresses []string) error {
	return nil
}

func (app *workloadBase) AddSecondaryIpv6Addresses(intf string, ipaddresses []string) error {
	return nil
}

func (app *workloadBase) RunCommand(cmd []string, dir string, timeout uint32,
	retries uint32, background bool, shell bool) (*cmd.CmdCtx, string, error) {
	return nil, "", nil
}

func (app *workloadBase) WaitCommand(commandHandle string) (*cmd.CmdCtx, error) {
	return nil, nil
}

func (app *workloadBase) StopCommand(commandHandle string) (*cmd.CmdCtx, error) {
	return nil, nil
}

//GetWorkloadAgent handle.
func (app *workloadBase) GetWorkloadAgent() interface{} {
	return nil
}

//SetWorkloadAgent sets workload agent
func (app *workloadBase) SetWorkloadAgent(interface{}) {
}

func (app *workloadBase) IsHealthy() bool {
	return true
}

func (app *workloadBase) TearDown() {

	if app.bgCmds != nil {
		app.bgCmds.Range(func(key interface{}, value interface{}) bool {
			app.StopCommand(key.(string))
			return true
		})
	}

	app.bgCmds = new(sync.Map)

}

func (app *containerWorkload) BringUp(args ...string) error {
	var err error

	image := args[1]
	name := args[0]

	ports := ""
	if len(args) > 2 {
		ports = args[2]
	}

	pBindings := []utils.PortBinding{}
	if ports != "" {
		portMaps := strings.Split(ports, ",")
		for _, portMap := range portMaps {
			portProto := strings.Split(portMap, ":")
			if len(portProto) == 2 {
				bind := utils.PortBinding{Port: portProto[0], Proto: portProto[1]}
				pBindings = append(pBindings, bind)
			}

		}
	}
	if image != "" {
		app.containerHandle, err = utils.NewContainer(name, image, "", app.baseDir, ContainerPrivileged, pBindings)
	} else {
		/* Workload already spun up */
		app.containerHandle, err = utils.NewContainer(name, "", name, app.baseDir, ContainerPrivileged, pBindings)
	}

	if err != nil || app.containerHandle == nil {
		return errors.Errorf("Failed to do bring up of %s", name)
	}

	app.name = name
	app.bgCmds = new(sync.Map)

	return err
}

func (app *containerWorkload) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *containerWorkload) MoveInterface(name string) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
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
	cmdResp, _, _ := app.RunCommand(arpCmd, "", 0, 0, false, false)
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

func (app *containerWorkload) AddInterface(spec InterfaceSpec) (string, error) {
	ifconfigCmd := []string{"ifconfig", spec.Parent, "up"}
	if retCode, stdout, _ := utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", spec.Parent, stdout)
	}
	intfToAttach := spec.Parent

	if spec.PrimaryVlan != 0 {
		vlanintf := vlanIntf(spec.Parent, spec.PrimaryVlan)
		delVlanCmd := []string{"ip", "link", "del", vlanintf}
		//Delete the interface if already exists , ignore the error for now.
		utils.Run(delVlanCmd, 0, false, false, nil)
		addVlanCmd := []string{"ip", "link", "add", "link", spec.Parent, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(spec.PrimaryVlan)}
		if retCode, stdout, _ := utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", spec.Parent, spec.PrimaryVlan, stdout)
		}
		intfToAttach = vlanintf
	}

	if err := app.containerHandle.AttachInterface(intfToAttach); err != nil {
		return "", errors.Wrap(err, "Interface attach failed")
	}

	if spec.Mac != "" {
		if err := app.containerHandle.SetMacAddress(intfToAttach, spec.Mac, 0); err != nil {
			return "", errors.Wrapf(err, "Set Mac Address failed")
		}
	}

	if spec.IPV4Address != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, spec.IPV4Address, 0, false); err != nil {
			return "", errors.Wrapf(err, "Set IP Address failed")
		}
	}

	if spec.IPV6Address != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, spec.IPV6Address, 0, true); err != nil {
			return "", errors.Wrapf(err, "Set IPv6 Address failed")
		}
	}

	app.subIF = intfToAttach
	return intfToAttach, nil
}

func (app *containerMacVlanWorkload) BringUp(args ...string) error {
	return app.containerWorkload.BringUp(args...)
}

func (app *containerMacVlanWorkload) AddInterface(spec InterfaceSpec) (string, error) {

	stdout, err := addMacVlanInterface(spec.Parent, spec.Name, spec.IPV4Address, spec.IPV6Address, spec.Mac)
	if err != nil {
		return stdout, err
	}

	return app.containerWorkload.AddInterface(spec)
}

func (app *containerWorkload) AddSecondaryIpv4Addresses(intf string, ipaddresses []string) error {
	return nil
}

func (app *containerWorkload) AddSecondaryIpv6Addresses(intf string, ipaddresses []string) error {
	return nil
}

func (app *containerWorkload) RunCommand(cmds []string, dir string, retries uint32,
	timeout uint32, background bool, shell bool) (*cmd.CmdCtx, string, error) {

	fixUtf := func(r rune) rune {
		if r == utf8.RuneError {
			return -1
		}
		return r
	}
	cmdCtx := &cmd.CmdCtx{}
	containerCmdHandle, err := app.containerHandle.SetUpCommand(cmds, dir, background, shell)
	if err != nil {
		return nil, "", errors.Wrap(err, "Set up command failed")
	}

	if !background {
		var cmdResp utils.CommandResp
		for i := (uint32)(0); i <= retries; i++ {
			cmdResp, _ = app.containerHandle.RunCommand(containerCmdHandle, timeout)
			if cmdResp.RetCode == 0 {
				break
			}
			app.logger.Info("Command failed, retrying")
		}
		cmdCtx.Done = true
		cmdCtx.Stdout = strings.Map(fixUtf, cmdResp.Stdout)
		cmdCtx.Stderr = strings.Map(fixUtf, cmdResp.Stderr)
		cmdCtx.ExitCode = cmdResp.RetCode
		cmdCtx.TimedOut = cmdResp.Timedout
		return cmdCtx, "", nil
	}

	go func(ctx *cmd.CmdCtx) {
		cmdResp, _ := app.containerHandle.RunCommand(containerCmdHandle, 0)
		cmdCtx.Done = true
		cmdCtx.ExitCode = cmdResp.RetCode
		cmdCtx.Stdout = strings.Map(fixUtf, cmdResp.Stdout)
		cmdCtx.Stderr = strings.Map(fixUtf, cmdResp.Stderr)
		cmdCtx.Complete(nil)
	}(cmdCtx)

	handleKey := app.genBgCmdHandle()
	cmdInfo := &cmd.CmdInfo{Ctx: cmdCtx}
	cmdInfo.Handle = (string)(containerCmdHandle)
	app.bgCmds.Store(handleKey, cmdInfo)

	/* Give it couple of seconds to make sure command has started */
	time.Sleep(2 * time.Second)

	return cmdCtx, handleKey, nil
}

func (app *containerWorkload) StopCommand(commandHandle string) (*cmd.CmdCtx, error) {

	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)

	if !cmdInfo.Ctx.Done {
		app.containerHandle.StopCommand((utils.CommandHandle)(cmdInfo.Handle.(string)))
	}

	/* Give it 1 second to dump output */
	time.Sleep(1 * time.Second)

	app.bgCmds.Delete(commandHandle)
	//For bg command, always return exit code 0
	cmdInfo.Ctx.ExitCode = 0
	return cmdInfo.Ctx, nil
}

func (app *containerWorkload) WaitCommand(commandHandle string) (*cmd.CmdCtx, error) {

	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)
	cmdInfo.Ctx.Wait()
	/* Give it 1 second to dump output */
	time.Sleep(1 * time.Second)

	app.bgCmds.Delete(commandHandle)
	//For bg command, always return exit code 0
	cmdInfo.Ctx.ExitCode = 0
	return cmdInfo.Ctx, nil
}

func (app *containerWorkload) IsHealthy() bool {
	return app.containerHandle.IsHealthy()
}

func (app *containerWorkload) TearDown() {
	if app.containerHandle != nil {
		app.logger.Println("Deleting sub interface : " + app.subIF)
		delIntfCmd := []string{"ip", "link", "del", app.subIF}
		app.RunCommand(delIntfCmd, "", 0, 0, false, false)
		app.logger.Println("Stopping container....")
		app.containerHandle.Stop()
		app.logger.Println("Stopped container....")
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

func (app *bareMetalWorkload) AddInterface(spec InterfaceSpec) (string, error) {

	ifconfigCmd := []string{"ifconfig", spec.Parent, "up"}
	if retCode, stdout, _ := utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", spec.Parent, stdout)
	}
	intfToAttach := spec.Parent

	if spec.PrimaryVlan != 0 {
		vlanintf := ""
		var addVlanCmd []string
		var delVlanCmd []string
		if isFreeBsd() {
			vlanintf = freebsdVlanIntf(spec.Parent, spec.PrimaryVlan)
			delVlanCmd = []string{"ifconfig", vlanintf, "destroy"}
			addVlanCmd = []string{"ifconfig", vlanintf, "create", "inet"}
		} else {
			vlanintf = vlanIntf(spec.Parent, spec.PrimaryVlan)
			delVlanCmd = []string{"ip", "link", "del", vlanintf}
			addVlanCmd = []string{"ip", "link", "add", "link", spec.Parent, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(spec.PrimaryVlan)}
		}
		utils.Run(delVlanCmd, 0, false, false, nil)
		if retCode, stdout, _ := utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", spec.Parent, spec.PrimaryVlan, stdout)
		}

		intfToAttach = vlanintf
	}

	if spec.Mac != "" {
		var setMacAddrCmd []string
		if !isFreeBsd() {
			//Mac address change only works on linux
			setMacAddrCmd = []string{"ifconfig", intfToAttach, "hw", "ether", spec.Mac}
			if retCode, stdout, err := utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
				return "", errors.Wrap(err, stdout)
			}
		}
	}

	if spec.IPV4Address != "" {
		cmd := []string{"ifconfig", intfToAttach, spec.IPV4Address}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if spec.IPV6Address != "" {
		//unset ipv6 address first
		cmd := []string{"ifconfig", intfToAttach, "inet6", "del", spec.IPV6Address}
		utils.Run(cmd, 0, false, false, nil)
		cmd = []string{"ifconfig", intfToAttach, "inet6", "add", spec.IPV6Address}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	app.subIF = intfToAttach
	return intfToAttach, nil
}

func (app *bareMetalWorkload) AddSecondaryIpv4Addresses(intf string, ipaddresses []string) error {
	return nil
}

func (app *bareMetalWorkload) AddSecondaryIpv6Addresses(intf string, ipaddresses []string) error {
	return nil
}

func addMacVlanInterface(parentInterface, workloadInterface, ipaddress, ipv6address, macAddress string) (string, error) {
	ifconfigCmd := []string{"ifconfig", parentInterface, "up"}
	if retCode, stdout, _ := utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", parentInterface, stdout)
	}

	var addVlanCmd []string
	var delVlanCmd []string
	if isFreeBsd() {
		return "", errors.New("Mac vlan Not supported on freebsd")
	}
	delVlanCmd = []string{"ip", "link", "del", workloadInterface}
	addVlanCmd = []string{"ip", "link", "add", "link", parentInterface, "name", workloadInterface, "type", "macvlan"}
	utils.Run(delVlanCmd, 0, false, false, nil)
	if retCode, stdout, _ := utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("IP link failed to create mac vlan failed %s, err :%s", workloadInterface, stdout)
	}

	if macAddress != "" {
		var setMacAddrCmd []string
		setMacAddrCmd = []string{"ifconfig", workloadInterface, "hw", "ether", macAddress}
		if retCode, stdout, err := utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ip", "addr", "add", ipaddress, "dev", workloadInterface}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if ipv6address != "" {
		//unset ipv6 address first
		cmd := []string{"ip", "addr", "del", ipaddress, "dev", workloadInterface}
		utils.Run(cmd, 0, false, false, nil)
		cmd = []string{"ip", "addr", "add", ipaddress, "dev", workloadInterface}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	cmd := []string{"ip", "link", "set", "dev", workloadInterface, "up"}
	if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Wrap(err, stdout)
	}

	return "", nil
}

func (app *bareMetalMacVlanWorkload) AddInterface(spec InterfaceSpec) (string, error) {

	stdout, err := addMacVlanInterface(spec.Parent, spec.Name, spec.IPV4Address, spec.IPV6Address, spec.Mac)
	if err != nil {
		return stdout, err
	}

	app.subIF = spec.Name

	return spec.Name, nil
}

func (app *bareMetalMacVlanWorkload) AddSecondaryIpv4Addresses(intf string, ipaddresses []string) error {
	for _, ipaddress := range ipaddresses {
		cmd := []string{"ip", "addr", "add", ipaddress, "dev", intf}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}
	return nil
}

func (app *bareMetalMacVlanWorkload) AddSecondaryIpv6Addresses(intf string, ipaddresses []string) error {
	for _, ipaddress := range ipaddresses {
		cmd := []string{"ip", "addr", "add", ipaddress, "dev", intf}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}
	return nil
}

func (app *bareMetalMacVlanEncapWorkload) AddInterface(spec InterfaceSpec) (string, error) {

	var addVlanCmd []string
	var delVlanCmd []string

	if isFreeBsd() {
		return "", errors.New("Mac vlan Not supported on freebsd")
	}

	ifconfigCmd := []string{"ifconfig", spec.Parent, "up"}
	utils.Run(ifconfigCmd, 0, false, false, nil)

	if spec.PrimaryVlan == 0 {
		return "", errors.Errorf("Need vlan for Mac vlan Encap workload on :%s", spec.Parent)
	}

	vlanintf := vlanIntf(spec.Parent, spec.PrimaryVlan)
	addVlanCmd = []string{"ip", "link", "add", "link", spec.Parent, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(spec.PrimaryVlan)}
	//Ignore error as sub if could be created earlier
	utils.Run(addVlanCmd, 0, false, false, nil)

	parentInterface := vlanintf
	workloadInterface := generateMacVlanIntfName(spec.PrimaryVlan)

	ifconfigCmd = []string{"ifconfig", parentInterface, "up"}
	if retCode, stdout, _ := utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", parentInterface, stdout)
	}

	delVlanCmd = []string{"ip", "link", "del", workloadInterface}
	addVlanCmd = []string{"ip", "link", "add", "link", parentInterface, "name", workloadInterface, "type", "macvlan"}
	utils.Run(delVlanCmd, 0, false, false, nil)
	if retCode, stdout, _ := utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("IP link failed to create mac vlan failed %s, err :%s", workloadInterface, stdout)
	}

	if spec.Mac != "" {
		var setMacAddrCmd []string
		setMacAddrCmd = []string{"ifconfig", workloadInterface, "hw", "ether", spec.Mac}
		if retCode, stdout, err := utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if spec.IPV4Address != "" {
		cmd := []string{"ifconfig", workloadInterface, spec.IPV4Address}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	if spec.IPV6Address != "" {
		//unset ipv6 address first
		cmd := []string{"ifconfig", workloadInterface, "inet6", "del", spec.IPV6Address}
		utils.Run(cmd, 0, false, false, nil)
		cmd = []string{"ifconfig", workloadInterface, "inet6", "add", spec.IPV6Address}
		if retCode, stdout, err := utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return "", errors.Wrap(err, stdout)
		}
	}

	//Disbale rp filtering
	cmd := []string{"sysctl", "-w", "net.ipv4.conf." + workloadInterface + ".rp_filter=2"}
	utils.Run(cmd, 0, false, false, nil)

	app.subIF = workloadInterface
	return workloadInterface, nil
}

func (app *bareMetalWorkload) TearDown() {
	var delVlanCmd []string

	//Stop all bg cmds first
	app.workloadBase.TearDown()
	if app.subIF != "" {
		if isFreeBsd() {
			delVlanCmd = []string{"ifconfig", app.subIF, "destroy"}
		} else {
			delVlanCmd = []string{"ip", "link", "del", app.subIF}
		}
		app.logger.Infof("Deleting subif %v\n", app.subIF)
		utils.Run(delVlanCmd, 0, false, false, nil)
	} else {
		app.logger.Info("No subif to delete")
	}

	app.subIF = ""
}

func (app *bareMetalWorkload) BringUp(args ...string) error {
	//app.name = name
	app.bgCmds = new(sync.Map)
	return nil
}

func (app *bareMetalWorkload) RunCommand(cmds []string, dir string, retries uint32,
	timeout uint32, background bool, shell bool) (*cmd.CmdCtx, string, error) {
	handleKey := ""

	runDir := app.baseDir
	if dir != "" {
		runDir = runDir + "/" + dir
	}

	fmt.Println("base dir ", runDir, dir)

	app.logger.Println("Running cmd ", strings.Join(cmds, " "))

	var cmdInfo *cmd.CmdInfo
	for i := (uint32)(0); i <= retries; i++ {
		cmdInfo, _ = cmd.ExecCmd(cmds, runDir, (int)(timeout), background, shell, nil)
		if cmdInfo.Ctx.ExitCode == 0 {
			break
		}
		app.logger.Info("Command failed, retrying")
	}

	if background && cmdInfo.Ctx.ExitCode == 0 {
		handleKey = app.genBgCmdHandle()
		app.bgCmds.Store(handleKey, cmdInfo)
	}

	return cmdInfo.Ctx, handleKey, nil
}

func (app *bareMetalWorkload) StopCommand(commandHandle string) (*cmd.CmdCtx, error) {
	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)
	app.logger.Printf("Stopping bare metal Running cmd %v %v\n", cmdInfo.Ctx.Stdout, cmdInfo.Handle)

	cmd.StopExecCmd(cmdInfo)
	time.Sleep(2 * time.Second)
	app.bgCmds.Delete(commandHandle)

	return cmdInfo.Ctx, nil
}

func (app *bareMetalWorkload) WaitCommand(commandHandle string) (*cmd.CmdCtx, error) {
	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)
	app.logger.Printf("Waiting for bare metal Running cmd %v %v\n", cmdInfo.Ctx.Stdout, cmdInfo.Handle)

	cmd.WaitForExecCmd(cmdInfo)
	time.Sleep(2 * time.Second)
	app.bgCmds.Delete(commandHandle)

	return cmdInfo.Ctx, nil
}

func (app *remoteWorkload) RunCommand(cmds []string, dir string,
	retries uint32, timeout uint32, background bool, shell bool) (*cmd.CmdCtx, string, error) {
	var cmdInfo *cmd.CmdInfo
	runCmd := strings.Join(cmds, " ")
	if app.sshHandle == nil {
		if err := app.Reinit(); err != nil {
			cmdInfo = &cmd.CmdInfo{Ctx: &cmd.CmdCtx{
				ExitCode: 1,
				Stderr:   "SSH connection failed",
			}}
			return cmdInfo.Ctx, "", nil
		}
	}

	if !background {
	Loop:
		for i := (uint32)(0); i <= retries; i++ {
			for i := 0; i < 2; i++ {
				cmdInfo, _ = cmd.RunSSHCommand(app.sshHandle, runCmd, timeout, false, false, app.logger)
				if cmdInfo.Ctx.ExitCode == cmd.SSHCreationFailedExitCode {
					cmdInfo.Ctx.Stderr = "SSH connection failed"
					//Try it again.
					if err := app.Reinit(); err != nil {
						//Make sure we don't crash next time
						app.sshHandle = nil
						return cmdInfo.Ctx, "", nil
					}
					continue
				}
				//Comand got executed on remote node, break up
				if cmdInfo.Ctx.ExitCode == 0 {
					break Loop
				}
				app.logger.Info("Command failed, retrying")
			}
		}
		return cmdInfo.Ctx, "", nil
	}

	for i := 0; i < 2; i++ {
		cmdInfo, _ = cmd.StartSSHBgCommand(app.sshHandle, runCmd, false)
		if cmdInfo.Ctx.ExitCode == cmd.SSHCreationFailedExitCode {
			cmdInfo.Ctx.Stderr = "SSH connection failed"
			//Try it again.
			if err := app.Reinit(); err != nil {
				//Make sure we don't crash next time
				app.sshHandle = nil
				return cmdInfo.Ctx, "", nil
			}
			continue
		}
		//Comand got executed on remote node, break up
		break
	}
	handleKey := app.genBgCmdHandle()
	app.bgCmds.Store(handleKey, cmdInfo)

	return cmdInfo.Ctx, handleKey, nil
}

func (app *remoteWorkload) mountDirectory(userName string, password string, srcDir string, dstDir string) error {

	mkdir := []string{"mkdir", "-p", dstDir}
	cmdInfo, _, _ := app.RunCommand(mkdir, "", 0, 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("mkdir command failed " + cmdInfo.Stderr)
	}

	sshKeygen := []string{"ssh-keygen", "-f", "~/.ssh/id_rsa", "-t", "rsa", "-N", "''"}
	cmdInfo, _, _ = app.RunCommand(sshKeygen, "", 0, 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("ssh-keygen command failed " + cmdInfo.Stderr)
	}

	myIP, err := utils.GetIPAddressOfInterface("eth0")
	if err != nil {
		return err
	}

	sshCopyID := []string{"sshpass", "-v", "-p", password, "ssh-copy-id", "-o", "StrictHostKeyChecking=no", userName + "@" + myIP}
	cmdInfo, _, _ = app.RunCommand(sshCopyID, "", 0, 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("ssh-copy-id command failed " + cmdInfo.Stderr)
	}

	sshFS := []string{"sudo", "nohup", "sshfs", "-o", "allow_other,IdentityFile=/home/" + userName + "/.ssh/id_rsa,StrictHostKeyChecking=no", userName + "@" + myIP + ":" + srcDir, dstDir}
	cmdInfo, _, _ = app.RunCommand(sshFS, "", 0, 0, false, false)
	if cmdInfo.ExitCode != 0 {
		return errors.New("sshfs command failed " + cmdInfo.Stderr)
	}

	return nil
}

func getKeyFile() (key ssh.Signer, err error) {
	sshDir := os.Getenv("HOME") + "/.ssh"
	file := sshDir + "/id_rsa"
	buf, err := ioutil.ReadFile(file)
	if err != nil {
		return
	}
	key, err = ssh.ParsePrivateKey(buf)
	if err != nil {
		return
	}
	return
}

func (app *remoteWorkload) Reinit() error {
	var err error

	connectWithPassword := func() error {
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

	connectWithPrivateKey := func() error {
		// Now in the main function DO:
		var key ssh.Signer
		var err error
		if key, err = getKeyFile(); err != nil {
			return err
		}

		config := &ssh.ClientConfig{
			User: app.username,
			Auth: []ssh.AuthMethod{
				ssh.PublicKeys(key),
			},
			HostKeyCallback: ssh.InsecureIgnoreHostKey(),
		}
		fmt.Println("(Private key : App : ", app.ip, app.port, app.username)
		if app.sshHandle, err = ssh.Dial("tcp", app.ip+":"+app.port, config); err != nil {
			err = errors.Wrapf(err, "SSH connect failed %v:%v@%v", app.username, app.password, app.ip)
			return err
		}
		return nil
	}

	if err := connectWithPassword(); err == nil {
		return nil
	}

	app.logger.Errorf("Failed conneting with password, trying with private key")
	return connectWithPrivateKey()
}

func (app *remoteWorkload) BringUp(args ...string) error {
	var err error

	app.ip = args[0]
	app.port = args[1]
	app.username = args[2]
	app.password = args[3]

	err = app.Reinit()
	//app.name = name
	app.bgCmds = new(sync.Map)
	return err
}

func (app *remoteWorkload) StopCommand(commandHandle string) (*cmd.CmdCtx, error) {

	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)

	cmd.StopSSHCmd(cmdInfo)

	time.Sleep(2 * time.Second)

	app.bgCmds.Delete(commandHandle)
	//For bg command, always return exit code 0
	cmdInfo.Ctx.ExitCode = 0
	return cmdInfo.Ctx, nil
}

func (app *remoteWorkload) WaitCommand(commandHandle string) (*cmd.CmdCtx, error) {

	item, ok := app.bgCmds.Load(commandHandle)
	if !ok {
		return &cmd.CmdCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	cmdInfo := item.(*cmd.CmdInfo)

	cmd.WaitSSHCmd(cmdInfo)
	time.Sleep(1 * time.Second)
	app.bgCmds.Delete(commandHandle)
	return cmdInfo.Ctx, nil
}

func newContainerWorkload(name string, parent string, logger *log.Logger) Workload {
	return &containerWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeContainer, name: name, parent: parent, logger: logger}}
}

func newBareMetalWorkload(name string, parent string, logger *log.Logger) Workload {
	return &bareMetalWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeBareMetal, name: name, parent: parent, logger: logger}}
}

func newBareMetalMacVlanWorkload(name string, parent string, logger *log.Logger) Workload {
	return &bareMetalMacVlanWorkload{bareMetalWorkload: bareMetalWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeMacVlan, name: name, parent: parent, logger: logger}}}
}

func newContainerMacVlanWorkload(name string, parent string, logger *log.Logger) Workload {
	return &containerMacVlanWorkload{containerWorkload: containerWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeContainerMacVlan, name: name, parent: parent, logger: logger}}}
}

func newBareMetalMacVlanEncapWorkload(name string, parent string, logger *log.Logger) Workload {
	return &bareMetalMacVlanEncapWorkload{bareMetalWorkload: bareMetalWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeMacVlanEncap, name: name, parent: parent, logger: logger}}}
}

func newVMWorkload(name string, parent string, logger *log.Logger) Workload {
	return &vmWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeVM, name: name, parent: parent, logger: logger}}
}

func newRemoteWorkload(name string, parent string, logger *log.Logger) Workload {
	return &remoteWorkload{workloadBase: workloadBase{
		wlType: WorkloadTypeRemote, name: name, parent: parent, logger: logger}}
}

var iotaWorkloads = map[string]func(name string, parent string, logger *log.Logger) Workload{
	WorkloadTypeContainer:        newContainerWorkload,
	WorkloadTypeVM:               newVMWorkload,
	WorkloadTypeESX:              newVMESXWorkload,
	WorkloadTypeVcenter:          newVMVcenterWorkload,
	WorkloadTypeBareMetal:        newBareMetalWorkload,
	WorkloadTypeRemote:           newRemoteWorkload,
	WorkloadTypeMacVlan:          newBareMetalMacVlanWorkload,
	WorkloadTypeContainerMacVlan: newContainerMacVlanWorkload,
	WorkloadTypeMacVlanEncap:     newBareMetalMacVlanEncapWorkload,
}

//NewWorkload creates a workload
func NewWorkload(workloadType string, name string, parent string, logger *log.Logger) Workload {
	if _, ok := iotaWorkloads[workloadType]; ok {
		return iotaWorkloads[workloadType](name, parent, logger)
	}

	return nil
}
