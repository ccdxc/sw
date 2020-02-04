package utils

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/docker/go-connections/nat"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/mount"
	"github.com/docker/docker/api/types/network"
	"github.com/docker/docker/client"
	"github.com/pkg/errors"
)

var _DockerCtx context.Context
var _DockerClient *client.Client

//Container Stop timeout
var _StopTimeout = (20)

const (
	baseDir        = "/home/"
	sysFsCgroupDir = "/sys/fs/cgroup"
)

//Interface structure
type Interface struct {
	Name       string
	Vlan       int
	MacAddress string
	IPAddress  string
	PrefixLen  int
	_VlanIntfs map[int]*Interface
}

// CommandHandle is the handle
type CommandHandle string

//CommandResp returns command response context
type CommandResp struct {
	RetCode  int32
	Stdout   string
	Stderr   string
	Timedout bool
	Handle   CommandHandle
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
	RunCommand(cmd []string, timeout int, background bool) (CommandResp, error)
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
func (ns *NS) SetIPAddress(intfName string, ipAddress string, vlan int, ipv6 bool) error {
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

	var cmd []string
	if ipv6 {
		cmdlist := []string{"ifconfig", intf.Name, "inet6", "add", ipAddress}
		cmd = append(cmd, cmdlist...)
	} else {
		cmdlist := []string{"ifconfig", intf.Name, ipAddress}
		cmd = append(cmd, cmdlist...)
	}

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
	baseDir       string
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
	if err != nil {
		return false
	}

	if inspectData.State.Health != nil {
		if inspectData.State.Health.Status != "healthy" {
			return false
		}
	} else {
		/* There is no health , check whether its running. */
		if !inspectData.State.Running {
			return false
		}
	}

	return true
}

//WaitForHealthy waits until container healthy
func (ctr *Container) WaitForHealthy(healthyTimeout time.Duration) error {
	timeout := time.After(healthyTimeout * time.Second)
	tick := time.Tick(1 * time.Second)

	for {
		select {
		case <-timeout:
			return errors.New("container not healthy")
		case <-tick:
			if ctr.IsHealthy() {
				return nil
			}
		}

	}

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

//SetUpCommand setups command to run later.
func (ctr *Container) SetUpCommand(cmd []string, dir string, background bool, shell bool) (CommandHandle, error) {

	cmd = []string{"cd", ctr.baseDir + "/" + dir, "&&", strings.Join(cmd, " ")}
	cmd = []string{"sh", "-c", strings.Join(cmd, " ")}

	fmt.Println("CMD ", strings.Join(cmd, " "))
	maxRetries := 3
	var err error
	for i := 0; i < maxRetries; i++ {
		resp, rerr := ctr.client.ContainerExecCreate(context.Background(), ctr.ContainerName, types.ExecConfig{
			AttachStdout: true,
			AttachStderr: true,
			Tty:          true,
			Detach:       background,
			Cmd:          cmd,
		})
		if rerr == nil {
			return (CommandHandle)(resp.ID), nil
		}
		err = rerr
	}

	return "", err

}

//StopCommand stop a command started
func (ctr *Container) StopCommand(cmdHandle CommandHandle) error {

	for true {
		cResp, err := ctr.client.ContainerExecInspect(context.Background(), (string)(cmdHandle))
		if err != nil || !cResp.Running {
			return err
		}
		/* Pid may no be immediately available */
		if cResp.Pid != 0 {
			cmd := []string{"kill", "-9", strconv.Itoa(cResp.Pid)}
			RunCmd(cmd, 0, false, false, nil)
			break
		}
		time.Sleep(1000 * time.Millisecond)
	}

	return nil
}

//RunCommand run command on the container
func (ctr *Container) RunCommand(cmdHandle CommandHandle, timeout uint32) (CommandResp, error) {

	var stdoutBuf, stderrBuf bytes.Buffer
	var cTimeout <-chan time.Time
	stdout := io.MultiWriter(&stdoutBuf)
	stderr := io.MultiWriter(&stderrBuf)

	maxAttempts := 3
	i := 0
	var hResp types.HijackedResponse
	var err error
	for true {
		hResp, err = ctr.client.ContainerExecAttach(context.Background(), (string)(cmdHandle),
			types.ExecConfig{})
		if err == nil {
			defer hResp.Close()
			break
		}
		i++
		if i == maxAttempts {
			return CommandResp{RetCode: -1}, err
		}
	}

	err = ctr.client.ContainerExecStart(context.Background(), (string)(cmdHandle), types.ExecStartCheck{})
	if err != nil {
		return CommandResp{RetCode: -1}, err
	}

	if timeout != 0 {
		cTimeout = time.After(time.Second * time.Duration(timeout))
	}
	retCode := 0
	go StdCopy(stdout, stderr, hResp.Reader)
	for true {
		cmdResp, err := ctr.client.ContainerExecInspect(context.Background(), (string)(cmdHandle))
		if err != nil || cmdResp.Running == false {
			retCode = cmdResp.ExitCode
			if cmdResp.ExitCode != 0 {
				err = errors.New("Command Failed")
			}
			return CommandResp{RetCode: ((int32)(retCode)), Stdout: stdoutBuf.String(), Stderr: stderrBuf.String()}, err
		}
		select {
		case <-cTimeout:
			ctr.StopCommand(cmdHandle)
			time.Sleep(1 * time.Second)
			return CommandResp{RetCode: 127, Timedout: true, Stdout: stdoutBuf.String(), Stderr: stderrBuf.String()}, err
		default:
			time.Sleep(100 * time.Millisecond)
		}
	}

	/* It should never come here */
	return CommandResp{}, nil
}

type PortBinding struct {
	Proto string
	Port  string
}

func bringUpAppContainer(name string, registry string, mountTarget string, privileged bool, pBindings []PortBinding) (*string, error) {
	var entryPoint []string
	opts := types.ImagePullOptions{}
	out, err := _DockerClient.ImagePull(_DockerCtx,
		registry, opts)
	if err != nil {
		return nil, err
	}
	io.Copy(os.Stdout, out)

	mountDir := ""
	if mountTarget == "" {
		mountDir = os.Getenv("HOME") + "/" + name
	} else {
		mountDir = mountTarget
	}
	os.Mkdir(mountDir, 0777)

	mounts := []mount.Mount{
		{
			Type:   mount.TypeBind,
			Source: mountDir,
			Target: baseDir,
		},
	}

	if privileged {
		mount := mount.Mount{
			Type:   mount.TypeBind,
			Source: sysFsCgroupDir,
			Target: sysFsCgroupDir,
		}
		mounts = append(mounts, mount)
		entryPoint = append(entryPoint, "/usr/sbin/init")
	}

	portBindings := nat.PortMap{}
	exposedPorts := nat.PortSet{}
	if pBindings != nil {
		for _, bind := range pBindings {
			key := bind.Port + "/" + bind.Proto
			portBindings[nat.Port(key)] = []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: bind.Port}}
			exposedPorts[nat.Port(key)] = struct{}{}
		}

	}

	resp, err := _DockerClient.ContainerCreate(_DockerCtx, &container.Config{
		Image:           registry,
		NetworkDisabled: false,
		AttachStdin:     true,
		AttachStdout:    true,
		Tty:             true,
		StopSignal:      "SIGKILL",
		Entrypoint:      entryPoint,
		ExposedPorts:    exposedPorts,
	}, &container.HostConfig{AutoRemove: true,
		Mounts:       mounts,
		Privileged:   privileged,
		PortBindings: portBindings},
		nil, name)
	if err != nil {
		fmt.Println(err.Error(), mountDir)
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
	registry string, containerID string, mount string, privileged bool, pbindings []PortBinding) (*Container, error) {
	_container := new(Container)
	_container.client = _DockerClient
	_container.ctx = _DockerCtx
	var err error

	if containerID != "" {
		_container.ContainerName = containerID
		_container.ctrID = containerID
		_container.NS.Init(false)
	} else {
		id, cErr := bringUpAppContainer(name, registry, mount, privileged, pbindings)
		if cErr != nil {
			return nil, cErr
		}
		_container.ctrID = *id
	}

	_container.baseDir = baseDir
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

func createNetwork(name, parent, driver, ipRange, ipGateway, ipSubnet string) (string, error) {

	nwOptions := types.NetworkCreate{Driver: driver}

	ipamConfig := network.IPAMConfig{IPRange: ipRange, Gateway: ipGateway, Subnet: ipSubnet}

	nwOptions.Options = make(map[string]string)

	nwOptions.Options["parent"] = parent
	nwOptions.IPAM = &network.IPAM{}
	nwOptions.IPAM.Config = append(nwOptions.IPAM.Config, ipamConfig)
	fmt.Printf("Create network %v \n", nwOptions)
	resp, err := _DockerClient.NetworkCreate(_DockerCtx, name, nwOptions)

	if err != nil {
		return "", errors.Wrap(err, "Failed to create docker network")
	}

	return resp.ID, nil
}

//DeleteDockerNetworkByName delete docker network by name
func DeleteDockerNetworkByName(name string) error {

	nws, err := _DockerClient.NetworkList(_DockerCtx, types.NetworkListOptions{})

	if err != nil {
		return errors.Wrap(err, "Failed to list network")
	}

	for _, nw := range nws {
		if nw.Name == name {
			err = _DockerClient.NetworkRemove(_DockerCtx, nw.ID)
			if err != nil {
				return errors.Wrap(err, "Failed to delete network")
			}
			break
		}
	}
	return nil
}

//CreateMacVlanDockerNetwork create macvlan network
func CreateMacVlanDockerNetwork(name, parent, ipRange, ipGateway, ipSubnet string) (string, error) {
	return createNetwork(name, parent, "macvlan", ipRange, ipGateway, ipSubnet)
}

//ConnectToDockerNetwork connect container to network
func ConnectToDockerNetwork(ctr *Container, networkID string) error {
	return _DockerClient.NetworkConnect(_DockerCtx, networkID, ctr.ctrID, nil)
}

//GetContainer handle
var GetContainer = NewContainer

func init() {
	_DockerCtx = context.Background()
	_DockerClient, _ = client.NewEnvClient()
}
