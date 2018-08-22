package infra

import (
	"bufio"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strconv"
	"time"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"

	Common "github.com/pensando/sw/nic/e2etests/go/common"
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
func (intf *Interface) AddVlanInterface(name string, vlan int) {
	vlanIntf := NewInterface(name, vlan, "")
	intf._VlanIntfs[vlan] = vlanIntf
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
func (intf *Interface) SetIPAddress(address string, prefixlen int) {
	intf.IPAddress = address
	intf.PrefixLen = prefixlen
}

//AppEngine Interface for an app engine
type AppEngine interface {
	GetName() string
	BringUp()
	RunCommand(cmd []string, timeout int, background bool) (Common.CmdHandle, error)
	StopCommand(Common.CmdHandle)
	AttachInterface(intfName string)
	AddVlan(intfName string, vlan int)
	DeleteVlan(intfName string, vlan int)
	DetachInterface(intfName string)
	MoveInterface(intfName string, ns int)
	DetachInterfaces()
	RouteAdd(dstIp string, nexHop string, nextHopMac string) error
	RouteDelete(dstIp string, nexHop string) error
	GetInterfaces() []*Interface
	SetMacAddress(intfName string, macAddress string, vlan int)
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
func (ns *NS) AttachInterface(intfName string) {
	intf := NewInterface(intfName, 0, "")
	cmdArgs := []string{"ip", "link", "set", "dev", intfName, "netns", ns.Name}
	Common.Run(cmdArgs, 0, false)
	ifconfigCmd := []string{"ifconfig", intfName, "up"}
	ns.RunCommand(ifconfigCmd, 0, false)
	ns._Interfaces[intfName] = intf
}

//DetachInterface detach interface
func (ns *NS) DetachInterface(intfName string) {

}

//DetachInterfaces detach interfaces
func (ns *NS) DetachInterfaces() {

}

func (ns *NS) RouteAdd(dstIp string, nexHop string, nextHopMac string) error {
	cmd := []string{"route", "add", dstIp, "gw", nexHop}
	if _, err := ns.RunCommand(cmd, 0, false); err != nil {
		return err
	}
	cmd = []string{"arp", "-s", nexHop, nextHopMac}
	_, err := ns.RunCommand(cmd, 0, false)
	return err
}

func (ns *NS) RouteDelete(dstIp string, nexHop string) error {

	cmd := []string{"route", "delete", dstIp}
	if _, err := ns.RunCommand(cmd, 0, false); err != nil {
		return err
	}

	cmd = []string{"arp", "-d", nexHop}
	_, err := ns.RunCommand(cmd, 0, false)
	return err
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
	for _, intf := range ns._Interfaces {
		for _, subIntf := range intf._VlanIntfs {
			delVlanCmd := []string{"ip", "link", "delete", subIntf.Name}
			ns.RunCommand(delVlanCmd, 0, false)
		}
		ns.MoveInterface(intf.Name, 1)
	}
	cmdArgs := []string{"ip", "netns", "del", ns.Name}
	Common.Run(cmdArgs, 0, false)
}

//Init init namespace
func (ns *NS) Init(skipCreate bool) {

	ns._CmdPrefix = []string{"ip", "netns", "exec", ns.Name}
	if !skipCreate {
		ns.Reset()
		cmdArgs := []string{"ip", "netns", "add", ns.Name}
		Common.Run(cmdArgs, 0, false)
	}
	ns._Interfaces = make(map[string]*Interface)

}

//AddVlan add vlan to interface
func (ns *NS) AddVlan(intfName string, vlan int) {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		panic("Interface not found!")
	}
	vlanIntf := intf.Name + "_" + strconv.Itoa(vlan)
	intf.AddVlanInterface(vlanIntf, vlan)
	addVlanCmd := []string{"ip", "link", "add", "link", intfName, "name", vlanIntf,
		"type", "vlan", "id", strconv.Itoa(vlan)}
	ns.RunCommand(addVlanCmd, 0, false)
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
func (ns *NS) SetMacAddress(intfName string, macAddress string, vlan int) {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		panic("Interface not found!")
	}

	if vlan != 0 {
		intf = intf.GetVlanInterface(vlan)
		if intf == nil {
			panic("Vlan interface not found")
		}
	}
	setMacAddrCmd := []string{"ifconfig", intf.Name, "hw", "ether", macAddress}
	ns.RunCommand(setMacAddrCmd, 0, false)
	intf.MacAddress = macAddress
}

//SetIPAddress set ip address of the interface
func (ns *NS) SetIPAddress(intfName string, ipAddress string,
	prefixLen int, vlan int) {
	intf := ns._Interfaces[intfName]
	if intf == nil {
		panic("Primary Interface not found")
	}
	if vlan != 0 {
		intf = intf.GetVlanInterface(vlan)
		if intf == nil {
			panic(" Vlan interface not found")
		}
	}
	cmd := []string{"ifconfig", intf.Name, ipAddress + "/" + strconv.Itoa(prefixLen)}
	ns.RunCommand(cmd, 0, false)
	intf.SetIPAddress(ipAddress, prefixLen)

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
func (ns *NS) RunCommand(cmd []string, timeout int, background bool) (Common.CmdHandle, error) {
	nsCmd := ns._CmdPrefix
	nsCmd = append(nsCmd, cmd...)
	return Common.Run(nsCmd, timeout, background)
}

//StopCommand stop command
func (ns *NS) StopCommand(cmdHandle Common.CmdHandle) {
	Common.Stop(cmdHandle)
}

//BringUp bring up name space
func (ns *NS) BringUp() {
	ns.Init(false)
}

//Stop stop namespace
func (ns *NS) Stop() {
	ns.Delete()
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
func (ctr *Container) StopCommand(cmdHandle Common.CmdHandle) {
	//TODO
}

//GetName get name of the interface
func (ctr *Container) GetName() string {
	return ctr.ContainerName
}

//RunCommand run command on the container
func (ctr *Container) RunCommand(cmd []string, timeout int, background bool) (Common.CmdHandle, error) {
	resp, err := ctr.client.ContainerExecCreate(ctr.ctx, ctr.ContainerName, types.ExecConfig{
		AttachStdout: true,
		AttachStderr: true,
		Tty:          false,
		Detach:       background,
		Cmd:          cmd,
	})

	printline := func(reader *bufio.Reader) {
		for {
			line, _, ferr := reader.ReadLine()
			if ferr == io.EOF {
				return
			}
			fmt.Printf("%s \n", line)
		}
	}

	hResp, err := ctr.client.ContainerExecAttach(ctr.ctx, resp.ID,
		types.ExecConfig{})

	if err != nil {
		panic(err)
	}

	err = ctr.client.ContainerExecStart(ctr.ctx, resp.ID, types.ExecStartCheck{})
	if err != nil {
		panic(err)
	}

	for !background {
		cmdResp, _ := ctr.client.ContainerExecInspect(ctr.ctx, resp.ID)
		printline(hResp.Reader)
		if cmdResp.Running == false {
			fmt.Println("Command exit code : ", cmdResp.ExitCode)
			if cmdResp.ExitCode != 0 {
				err = errors.New("Command Failed")
			}
			break
		}
	}

	//TOOD still
	return nil, err
}

//E2EApp image to use
type E2EApp struct {
	Register string `json:"registry"`
	Image    string `json:"image"`
}

//E2EAppCfg App Information to use
type E2EAppCfg struct {
	App E2EApp `json:"App"`
}

func _BringUpAppContainer(name string) string {
	jsonFile, err := os.Open(Common.E2eAppConfigFile)
	if err != nil {
		panic("E2E App config file not fond")
	}
	// defer the closing of our jsonFile so that we can parse it later on
	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)
	var appCfg E2EAppCfg

	json.Unmarshal(byteValue, &appCfg)

	opts := types.ImagePullOptions{}
	out, err := _DockerClient.ImagePull(_DockerCtx,
		appCfg.App.Register+appCfg.App.Image, opts)
	if err != nil {
		panic(err)
	}
	io.Copy(os.Stdout, out)
	resp, err := _DockerClient.ContainerCreate(_DockerCtx, &container.Config{
		Image:           appCfg.App.Register + appCfg.App.Image,
		NetworkDisabled: true,
		AttachStdin:     true,
		AttachStdout:    true,
		Tty:             true,
		StopSignal:      "SIGKILL",
	}, &container.HostConfig{AutoRemove: true},
		nil, name)
	if err != nil {
		panic(err)
	}

	fmt.Println("Created container with id : ", resp.ID)

	if err := _DockerClient.ContainerStart(_DockerCtx, resp.ID,
		types.ContainerStartOptions{}); err != nil {
		panic(err)
	}

	return resp.ID
}

//NewContainer Create a new instance
func NewContainer(name string,
	configFile string, containerID string) *Container {
	_container := new(Container)
	_container.client = _DockerClient
	_container.ctx = _DockerCtx

	if containerID != "" {
		_container.ContainerName = containerID
		_container.ctrID = containerID
		_container.NS.Init(false)
	} else {
		_container.ctrID = _BringUpAppContainer(name)
	}

	//_container.ctrID = resp.ID
	_container.ContainerName = name
	inspectData, err := _DockerClient.ContainerInspect(_DockerCtx,
		_container.ctrID)
	if err != nil {
		panic(err)
	}

	_container.Name = strconv.Itoa(inspectData.State.Pid)
	nsCmd := []string{"ln", "-s", "/proc/" + _container.Name + "/ns/net",
		"/var/run/netns/" + _container.Name}
	Common.Run(nsCmd, 0, false)
	_container.NS.Init(true)
	return _container
}

func init() {
	_DockerCtx = context.Background()
	_DockerClient, _ = client.NewEnvClient()
}
