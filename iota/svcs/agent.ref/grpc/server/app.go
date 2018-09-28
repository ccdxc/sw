package server

import (
	"strconv"
	"strings"

	"github.com/pkg/errors"
	context "golang.org/x/net/context"

	Common "github.com/pensando/sw/iota/common"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
)

func vlanIntf(name string, vlan int) string {
	return name + "_" + strconv.Itoa(vlan)
}

//App interface
type App interface {
	BringUp(name string, args ...string) error
	RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error)
	AttachInterface(name string, macAddress string, ipaddress string, prefixLen int) error
	AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error)
	TearDown()
}

type containerApp struct {
	containerHandle *Common.Container
}

func (app *containerApp) BringUp(name string, args ...string) error {
	var err error
	app.containerHandle, err = Common.NewContainer(name, args[0], "")
	return err
}

func (app *containerApp) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {

	if parentIntf == "" {
		intfGetCmd := []string{"ip", "-o", "link", "|", "grep", parentMacAddress, "|", "awk", "'{print $2}'", "|", "sed", "'s/\\://g'"}
		retCode, stdout, _ := Common.Run(intfGetCmd, 0, false, true, nil)
		if retCode != 0 || len(stdout) == 0 {
			return "", errors.Errorf("Could not find interface with mac  %s : %s", parentMacAddress, stdout)
		}
		parentIntf = strings.TrimSuffix(stdout, "\n")

	}

	ifconfigCmd := []string{"ifconfig", parentIntf, "up"}
	if retCode, stdout, _ := Common.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s : %s", parentIntf, stdout)
	}

	vlanintf := vlanIntf(parentIntf, vlan)
	addVlanCmd := []string{"ip", "link", "add", "link", parentIntf, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
	if retCode, stdout, _ := Common.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
		return "", errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", parentIntf, vlan, stdout)
	}

	return vlanintf, nil
}

func (app *containerApp) AttachInterface(name string, macAddress string, ipaddress string, prefixLen int) error {
	err := app.containerHandle.AttachInterface(name)
	if err != nil {
		return errors.Wrapf(err, "Attach interface failed")
	}

	if err := app.containerHandle.SetMacAddress(name, macAddress, 0); err != nil {
		return errors.Wrapf(err, "Set Mac Address failed")
	}

	//Below statement are hack for now to increase ARP timeout.
	cmd := []string{"sysctl", "-w", "net.ipv4.neigh." + name + ".retrans_time_ms=" + strconv.Itoa(arpTimeout)}
	app.containerHandle.RunCommandInNS(cmd, 0, false, true)

	if ipaddress != "" {
		if err := app.containerHandle.SetIPAddress(name, ipaddress, prefixLen, 0); err != nil {
			return errors.Wrapf(err, "Set IP Address failed")
		}
	}

	return nil
}

func (app *containerApp) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error) {
	return app.containerHandle.RunCommand(cmd, timeout, background, shell)
}

func (app *containerApp) TearDown() {
	app.containerHandle.Stop()
}

type qemuApp struct {
	name      string
	cmdPrefix string
}

func (app *qemuApp) BringUp(name string, args ...string) error {

	app.name = name
	app.cmdPrefix = "ip netns exec " + name + " "
	if !QemuInstance.Running() {
		return errors.Errorf("Qemu is not running, app  %s bring up failed", name)
	}
	cmd := "ip netns del " + name
	QemuInstance.Exec(cmd, true, false)
	cmd = "ip netns add " + name
	if retCode, _, _ := QemuInstance.Exec(cmd, true, false); retCode != 0 {
		return errors.Errorf("Failed to create namespace %s", name)
	}
	return nil
}

func (app *qemuApp) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error) {
	appCmd := app.cmdPrefix + strings.Join(cmd, " ")
	retCode, stdout, _ := QemuInstance.Exec(appCmd, true, background)
	return retCode, strings.Join(stdout, " "), "", nil
}

func (app *qemuApp) AttachInterface(name string, macAddress string, ipaddress string, prefixLen int) error {
	/* First move the interface to the namespace */
	cmd := []string{"ip", "link", "set", "dev", name, "netns", app.name}
	if retCode, _, _ := QemuInstance.Exec(strings.Join(cmd, " "), true, false); retCode != 0 {
		return errors.Errorf("Failed to move interface %s to namespace %s", name, app.name)
	}

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, _, _ := QemuInstance.Exec(app.cmdPrefix+strings.Join(ifconfigCmd, " "), true, false); retCode != 0 {
		return errors.Errorf("Could not bring up interface %s", name)
	}

	if macAddress != "" {
		setMacAddrCmd := []string{"ifconfig", name, "hw", "ether", macAddress}
		if retCode, _, _ := QemuInstance.Exec(app.cmdPrefix+strings.Join(setMacAddrCmd, " "), true, false); retCode != 0 {
			return errors.Errorf("Could not bring up interface %s", name)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", name, ipaddress + "/" + strconv.Itoa(prefixLen)}
		if retCode, _, _ := QemuInstance.Exec(app.cmdPrefix+strings.Join(cmd, " "), true, false); retCode != 0 {
			return errors.Errorf("Could not bring up interface %s", name)
		}
	}

	return nil
}

func (app *qemuApp) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {

	if parentIntf == "" {
		intfGetCmd := []string{"ip", "-o", "link", "|", "grep", parentMacAddress, "|", "awk", "'{print $2}'", "|", "sed", "'s/\\://g'"}
		retCode, stdout, _ := QemuInstance.Exec(strings.Join(intfGetCmd, " "), true, false)
		if retCode != 0 || len(stdout) == 0 || len(stdout[0]) < 1 {
			return "", errors.Errorf("Could not find interface with mac  %s", parentMacAddress)
		}
		parentIntf = strings.Split(stdout[0], " ")[1]
	}

	ifconfigCmd := []string{"ifconfig", parentIntf, "up"}
	if retCode, _, _ := QemuInstance.Exec(strings.Join(ifconfigCmd, " "), true, false); retCode != 0 {
		return "", errors.Errorf("Could not bring up parent interface %s", parentIntf)
	}

	vlanintf := vlanIntf(parentIntf, vlan)
	addVlanCmd := []string{"ip", "link", "add", "link", parentIntf, "name", vlanintf,
		"type", "vlan", "id", strconv.Itoa(vlan)}
	if retCode, _, _ := QemuInstance.Exec(strings.Join(addVlanCmd, " "), true, false); retCode != 0 {
		return "", errors.Errorf("IP link create to add vlan failed %s:%d", parentIntf, vlan)
	}

	return vlanintf, nil
}

func (app *qemuApp) TearDown() {

}

// AppServer is used to implement App server
type AppServer struct {
	server
	appMap map[string]App
}

const (
	appNotFound = "App not running."
	arpTimeout  = 3000 //3 seconds
)

// BringUp Bring up app
func (s *AppServer) BringUp(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {
	s.log("App bring up request received for : " + in.GetName())

	var app App
	if !in.OnQemu {
		app = &containerApp{}
	} else {
		app = &qemuApp{}
	}

	if err := app.BringUp(in.Name, in.Registry, ""); err != nil {
		resp := errors.Wrapf(err, "App bring up failed").Error()
		s.log(resp)
		return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	s.appMap[in.Name] = app
	resp := "App Started : " + in.Name
	s.log(resp)
	return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *AppServer) Teardown(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {

	s.log("App teardown request received for : " + in.GetName())
	app, f := s.appMap[in.Name]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.AppStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}
	app.TearDown()
	delete(s.appMap, in.Name)
	resp := "App Stopped : " + in.Name
	s.log(resp)
	return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// AttachInterface attaches the specified to the container.
func (s *AppServer) AttachInterface(ctx context.Context, in *pb.Interface) (*pb.InterfaceStatus, error) {
	s.log("Attach Interface request received for  " + in.App + " " + in.GetName())
	app, f := s.appMap[in.App]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	err := app.AttachInterface(in.GetName(), in.GetMacAddress(), in.GetIpAddress(), int(in.GetPrefixLen()))
	if err != nil {
		resp := errors.Wrapf(err, "Attach interface failed").Error()
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	resp := "Attach Interface success."
	s.log(resp)

	return &pb.InterfaceStatus{Response: resp,
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// AddVlanInterface attaches the specified to the container.
func (s *AppServer) AddVlanInterface(ctx context.Context, in *pb.VlanInterface) (*pb.InterfaceStatus, error) {
	s.log("Add Vlan Interface request received for  " + in.App + " " + in.GetParentIntfName())
	app, f := s.appMap[in.App]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	vlanInf, err := app.AddVlanInterface(in.GetParentIntfName(), in.GetParentMacAddress(), int(in.GetVlan()))
	if err != nil {
		s.log(err.Error())
		return &pb.InterfaceStatus{Response: err.Error(),
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil

	}

	if err := app.AttachInterface(vlanInf, in.GetMacAddress(),
		in.GetIpAddress(), int(in.GetPrefixLen())); err != nil {
		s.log(err.Error())
		return &pb.InterfaceStatus{Response: err.Error(),
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	resp := "Attach Vlan Interface success."
	s.log(resp)

	return &pb.InterfaceStatus{Response: resp,
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *AppServer) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	s.log("Run command request received for  " + in.App + " " + in.GetCmd())
	app, f := s.appMap[in.App]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.CommandStatus{
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	retCode, stdout, stderr, _ := app.RunCommand(strings.Split(in.Cmd, " "),
		in.Timeout, in.Background, true)

	s.log("Run command status :  " + strconv.Itoa(retCode))
	s.log("Run command stdout :  " + stdout)
	s.log("Run command stder  :  " + stderr)
	return &pb.CommandStatus{
		Status:  pb.ApiStatus_API_STATUS_OK,
		RetCode: int32(retCode),
		Stdout:  stdout,
		Stderr:  stderr,
	}, nil

}

//NewAppServer Returns an App server
func NewAppServer() *AppServer {
	appServer := &AppServer{server: server{name: "App"}}
	appServer.appMap = make(map[string]App)
	return appServer
}
