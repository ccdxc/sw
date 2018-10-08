package utils

import (
	"bufio"
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/mount"
	"github.com/docker/docker/client"
	"github.com/pkg/errors"
)

var _DockerCtx context.Context
var _DockerClient *client.Client

//Container Stop timeout
var _StopTimeout = (20)

//Interface structure
type Interface struct {
	Name       string
	Vlan       int
	MacAddress string
	IPAddress  string
	PrefixLen  int
	_VlanIntfs map[int]*Interface
}

//NewInterface vlan interface create
func NewInterface(name string, vlan int, macAddress string) *Interface {
	intf := new(Interface)
	intf.Name = name
	intf.Vlan = vlan
	if macAddress != "" {
		intf.MacAddress = macAddress
	}
	intf._VlanIntfs = make(map[int]*Interface)
	return intf
}

//AddVlanInterface add vlan interface
func (intf *Interface) AddVlanInterface(name string, vlan uint32) {
	vlanIntf := NewInterface(name, int(vlan), "")
	intf._VlanIntfs[int(vlan)] = vlanIntf
}

//DeleteVlanInterface delete vlan inteface
func (intf *Interface) DeleteVlanInterface(vlan int) {
	delete(intf._VlanIntfs, vlan)
}

//GetVlanInterface Get vlan inteface
func (intf *Interface) GetVlanInterface(vlan int) *Interface {
	return intf._VlanIntfs[vlan]
}

//PrintInterfaceInformation  Print information
func (intf *Interface) PrintInterfaceInformation() {
	fmt.Println("\t\t Intetrface name     : ", intf.Name)
	fmt.Println("\t\t Mac address         : ", intf.MacAddress)
	fmt.Println("\t\t Encap Vlan          : ", intf.Vlan)
	fmt.Println("\t\t IP Address          : ", intf.IPAddress)
	for intfName := range intf._VlanIntfs {
		intf._VlanIntfs[intfName].PrintInterfaceInformation()
	}

}

//SetIPAddress Set ip address
func (intf *Interface) SetIPAddress(address string) error {
	intf.IPAddress = address
	return nil
}

//AppEngine Interface for an app engine
type AppEngine interface {
	GetName() string
	BringUp()
	RunCommand(cmd []string, timeout int, background bool) (string, string, error)
	StopCommand(CmdHandle)
	AttachInterface(intfName string) error
	AddVlan(intfName string, vlan int)
	DeleteVlan(intfName string, vlan int)
	DetachInterface(intfName string)
	MoveInterface(intfName string, ns int)
	DetachInterfaces()
	GetInterfaces() []*Interface
	SetMacAddress(intfName string, macAddress string, vlan int) error
	SetIPAddress(intfName string, ipAddress string, prefixlen int, vlan int)
	AddArpEntry(ip string, mac string)
	Stop()
	PrintAppInformation()
}

//NS namespace struct
type NS struct {
	Name        string
	_CmdPrefix  []string
	_Interfaces map[string]*Interface
}

//Reset reset the name space
func (ns *NS) Reset() {
	ns.Delete()
}

//AttachInterface attach interface to name space
func (ns *NS) AttachInterface(intfName string) error {
	intf := NewInterface(intfName, 0, "")
	cmdArgs := []string{"ip", "link", "set", "dev", intfName, "netns", ns.Name}
	_, stdout, err := Run(cmdArgs, 0, false, false, nil)
	if err != nil {
		return errors.Wrap(err, stdout)
	}
	ifconfigCmd := []string{"ifconfig", intfName, "up"}
	stdout, err = ns.RunCommand(ifconfigCmd, 0, false)
	if err != nil {
		return errors.Wrap(err, stdout)
	}
	ns._Interfaces[intfName] = intf
	return nil
}

//DetachInterface detach interface
func (ns *NS) DetachInterface(intfName string) {

}

//DetachInterfaces detach interfaces
func (ns *NS) DetachInterfaces() {

}

//MoveInterface move interface to different namespace
func (ns *NS) MoveInterface(intfName string, namespace int) {
	mvIntfCmd := []string{"ip", "link", "set", intfName, "netns", strconv.Itoa(namespace)}
	ns.RunCommand(mvIntfCmd, 0, false)
}

//GetInterfaces get interfaces
func (ns *NS) GetInterfaces() []*Interface {
	var _Interfaces = make([]*Interface, len(ns._Interfaces))
	var i int
	for _, d := range ns._Interfaces {
		_Interfaces[i] = d
		i++
	}
	return _Interfaces
}

//Delete delete namespace
func (ns *NS) Delete() {
	cmdArgs := []string{"ip", "netns", "del", ns.Name}
	Run(cmdArgs, 0, false, false, nil)
}

//Init init namespace
func (ns *NS) Init(skipCreate bool) {

	ns._CmdPrefix = []string{"ip", "netns", "exec", ns.Name}
	if !skipCreate {
		ns.Reset()
		cmdArgs := []string{"ip", "netns", "add", ns.Name}
		Run(cmdArgs, 0, false, false, nil)
	}
	ns._Interfaces = make(map[string]*Interface)

}

//AddVlan add vlan to interface
func (ns *NS) AddVlan(intfName string, vlan uint32) error {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		return errors.New("interface not found")
	}
	vlanIntf := intf.Name + "_" + strconv.FormatUint(uint64(vlan),
		10)
	intf.AddVlanInterface(vlanIntf, vlan)
	addVlanCmd := []string{"ip", "link", "add", "link", intfName, "name", vlanIntf,
		"type", "vlan", "id", strconv.FormatUint(uint64(vlan), 10)}
	_, err := ns.RunCommand(addVlanCmd, 0, false)
	return err
}

//DeleteVlan delete vlan from interface
func (ns *NS) DeleteVlan(intfName string, vlan int) {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		panic("Interface not found!")
	}
	vlanIntf := intf.GetVlanInterface(vlan)
	if vlanIntf == nil {
		panic("Vlan interface not found")
	}
	rmVlanCmd := []string{"ip", "link", "delete", "dev", vlanIntf.Name}
	ns.RunCommand(rmVlanCmd, 0, false)
	intf.DeleteVlanInterface(vlan)
}

//SetMacAddress set mac address of the interface
func (ns *NS) SetMacAddress(intfName string, macAddress string, vlan int) error {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		return errors.New("Interface not found")
	}

	if vlan != 0 {
		intf = intf.GetVlanInterface(vlan)
		if intf == nil {
			return errors.New("Vlan interface not found")
		}
	}
	setMacAddrCmd := []string{"ifconfig", intf.Name, "hw", "ether", macAddress}
	if stdout, err := ns.RunCommand(setMacAddrCmd, 0, false); err != nil {
		return errors.Wrap(err, stdout)
	}
	intf.MacAddress = macAddress
	return nil
}

//SetIPAddress set ip address of the interface
func (ns *NS) SetIPAddress(intfName string, ipAddress string, vlan int) error {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		return errors.New("Primary Interface not found")
	}
	if vlan != 0 {
		intf = intf.GetVlanInterface(vlan)
		if intf == nil {
			return errors.New("Vlan interface not found")
		}
	}
	cmd := []string{"ifconfig", intf.Name, ipAddress}
	if stdout, err := ns.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, stdout)
	}
	return intf.SetIPAddress(ipAddress)
}

//AddArpEntry add arp entry
func (ns *NS) AddArpEntry(ipAddress string, macAddress string) {
	arpCmd := []string{"arp", "-s", ipAddress, macAddress}
	ns.RunCommand(arpCmd, 0, false)
}

//PrintAppInformation print APP information
func (ns *NS) PrintAppInformation() {
	fmt.Println("\t Namespace           : ", ns.Name)
	fmt.Println("\t Interfaces")
	for _, intf := range ns._Interfaces {
		intf.PrintInterfaceInformation()
	}

}

//GetName get name of the interface
func (ns *NS) GetName() string {
	return ns.Name
}

//RunCommand run command in the namespace
func (ns *NS) RunCommand(cmd []string, timeout int, background bool) (string, error) {
	nsCmd := ns._CmdPrefix
	nsCmd = append(nsCmd, cmd...)
	_, out, err := Run(nsCmd, timeout, background, false, nil)
	return out, err
}

//StopCommand stop command
func (ns *NS) StopCommand(cmdHandle CmdHandle) {
	Stop(cmdHandle)
}

//NewNS Create a new namespace instance
func NewNS(name string) *NS {
	ns := new(NS)
	ns.Name = name
	return ns
}

//Container defintion
type Container struct {
	NS
	ContainerName string
	ctx           context.Context
	client        *client.Client
	ctrID         string
	pid           string
}

//CleanUpContainer clean up container
func CleanUpContainer(name string) {

}

//BringUp bring up container
func (ctr *Container) BringUp() {
}

//Stop stop container
func (ctr *Container) Stop() {
	fmt.Println("Stopping container ", ctr.ContainerName)
	ctr.client.ContainerStop(ctr.ctx, ctr.ctrID, nil)
	time.Sleep(3 * time.Second)
	ctr.client.ContainerRemove(ctr.ctx, ctr.ctrID,
		types.ContainerRemoveOptions{Force: true})
}

//PrintAppInformation pring app information
func (ctr *Container) PrintAppInformation() {
	fmt.Println("Container name : ", ctr.ContainerName)
	ctr.NS.PrintAppInformation()
}

//StopCommand stop command
func (ctr *Container) StopCommand(cmdHandle CmdHandle) {
	//TODO
}

//GetName get name of the interface
func (ctr *Container) GetName() string {
	return ctr.ContainerName
}

//RunCommandInNS run command in namespace
func (ctr *Container) RunCommandInNS(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error) {

	_, err := ctr.NS.RunCommand(cmd, int(timeout), background)

	return 0, "", "", err

}

//IsHealthy returns true if container is healty.
func (ctr *Container) IsHealthy() bool {

	inspectData, err := _DockerClient.ContainerInspect(_DockerCtx,
		ctr.ctrID)
	if err != nil || (inspectData.State.Health.Status != "starting" &&
		inspectData.State.Health.Status != "healthy") {
		return false
	}

	return true
}

//CheckProcessRunning Checks whether process running
func (ctr *Container) CheckProcessRunning(process string) bool {

	pidPresent := func(pidData []string, match string) bool {
		for _, data := range pidData {
			if strings.Contains(data, match) {
				return true
			}
		}
		return false
	}

	body, err := ctr.client.ContainerTop(ctr.ctx, ctr.ContainerName, nil)
	if err != nil {
		return false
	}

	for _, p := range body.Processes {
		if pidPresent(p, process) {
			return true
		}
	}

	return false
}

//RunCommand run command on the container
func (ctr *Container) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int32, string, string, error) {

	if shell {
		cmd = []string{"sh", "-c", strings.Join(cmd, " ")}
	}

	resp, err := ctr.client.ContainerExecCreate(ctr.ctx, ctr.ContainerName, types.ExecConfig{
		AttachStdout: true,
		AttachStderr: true,
		Tty:          false,
		Detach:       background,
		Cmd:          cmd,
	})

	var b bytes.Buffer
	printline := func(reader *bufio.Reader) {
		for {
			line, _, ferr := reader.ReadLine()
			if ferr == io.EOF {
				return
			}
			b.WriteString(string(line) + "\n")
		}
	}

	hResp, err := ctr.client.ContainerExecAttach(ctr.ctx, resp.ID,
		types.ExecConfig{})

	if err != nil {
		return -1, "", "", err
	}

	err = ctr.client.ContainerExecStart(ctr.ctx, resp.ID, types.ExecStartCheck{})
	if err != nil {
		return -1, "", "", err
	}

	retCode := 0
	for !background {
		cmdResp, _ := ctr.client.ContainerExecInspect(ctr.ctx, resp.ID)
		printline(hResp.Reader)
		if cmdResp.Running == false {
			retCode = cmdResp.ExitCode
			fmt.Println("Command exit code : ", cmdResp.ExitCode)
			if cmdResp.ExitCode != 0 {
				err = errors.New("Command Failed")
			}
			break
		}
	}

	return ((int32)(retCode)), b.String(), "", err
}

func bringUpAppContainer(name string, registry string) (*string, error) {
	opts := types.ImagePullOptions{}
	out, err := _DockerClient.ImagePull(_DockerCtx,
		registry, opts)
	if err != nil {
		return nil, err
	}
	io.Copy(os.Stdout, out)
	os.Mkdir(os.Getenv("HOME")+"/"+name, 0644)
	resp, err := _DockerClient.ContainerCreate(_DockerCtx, &container.Config{
		Image:           registry,
		NetworkDisabled: true,
		AttachStdin:     true,
		AttachStdout:    true,
		Tty:             true,
		StopSignal:      "SIGKILL",
	}, &container.HostConfig{AutoRemove: true,
		Mounts: []mount.Mount{
			{
				Type:   mount.TypeBind,
				Source: os.Getenv("HOME") + "/" + name,
				Target: "/home/",
			},
		}},
		nil, name)
	if err != nil {
		return nil, err
	}

	fmt.Println("Created container with id : ", resp.ID)

	if err := _DockerClient.ContainerStart(_DockerCtx, resp.ID,
		types.ContainerStartOptions{}); err != nil {
		return nil, err
	}

	return &resp.ID, nil
}

//NewContainer Create a new instance
func NewContainer(name string,
	registry string, containerID string) (*Container, error) {
	_container := new(Container)
	_container.client = _DockerClient
	_container.ctx = _DockerCtx
	var err error

	if containerID != "" {
		_container.ContainerName = containerID
		_container.ctrID = containerID
		_container.NS.Init(false)
	} else {
		id, cErr := bringUpAppContainer(name, registry)
		if cErr != nil {
			return nil, cErr
		}
		_container.ctrID = *id
	}

	//_container.ctrID = resp.ID
	_container.ContainerName = name
	inspectData, err := _DockerClient.ContainerInspect(_DockerCtx,
		_container.ctrID)
	if err != nil {
		return nil, err
	}

	_container.Name = strconv.Itoa(inspectData.State.Pid)
	os.Mkdir("/var/run/netns/", 0755)
	nsCmd := []string{"ln", "-s", "/proc/" + _container.Name + "/ns/net",
		"/var/run/netns/" + _container.Name}
	Run(nsCmd, 0, false, false, nil)
	_container.NS.Init(true)
	return _container, nil
}

//GetContainer handle
var GetContainer = NewContainer

func init() {
	_DockerCtx = context.Background()
	_DockerClient, _ = client.NewEnvClient()
}
