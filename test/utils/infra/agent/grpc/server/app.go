package server

import (
	"strconv"
	"strings"

	"github.com/pkg/errors"
	context "golang.org/x/net/context"

	"github.com/pensando/sw/test/utils/infra"
	pb "github.com/pensando/sw/test/utils/infra/agent/grpc/api/pb"
)

// AppServer is used to implement App server
type AppServer struct {
	server
	containerMap map[string]*infra.Container
}

const (
	appNotFound = "App not running."
	arpTimeout  = 3000 //3 seconds
)

// BringUp Bring up app
func (s *AppServer) BringUp(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {
	s.log("App bring up request received for : " + in.GetName())
	container, err := infra.NewContainer(in.Name, in.Registry, "")
	if err != nil {
		resp := errors.Wrapf(err, "App bring up failed").Error()
		return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	s.containerMap[in.Name] = container
	resp := "App Started : " + in.Name
	s.log(resp)
	return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *AppServer) Teardown(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {

	s.log("App teardown request received for : " + in.GetName())
	contianer, f := s.containerMap[in.Name]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.AppStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}
	contianer.Stop()
	delete(s.containerMap, in.Name)
	resp := "App Stopped : " + in.Name
	s.log(resp)
	return &pb.AppStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// AttachInterface attaches the specified to the container.
func (s *AppServer) AttachInterface(ctx context.Context, in *pb.Interface) (*pb.InterfaceStatus, error) {
	s.log("Attach Interface request received for  " + in.App + " " + in.GetName())
	container, f := s.containerMap[in.App]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	err := container.AttachInterface(in.GetName())
	if err != nil {
		resp := errors.Wrapf(err, "Attach interface failed").Error()
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	if err := container.SetMacAddress(in.GetName(), in.GetMacAddress(), 0); err != nil {
		resp := errors.Wrapf(err, "Set Mac Address failed").Error()
		s.log(resp)
		return &pb.InterfaceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	if in.Vlan != 0 {
		if err := container.AddVlan(in.GetName(), in.Vlan); err != nil {
			resp := errors.Wrapf(err, "Add vlan failed").Error()
			s.log(resp)
			return &pb.InterfaceStatus{Response: resp,
				Status: pb.ApiStatus_API_STATUS_FAILED}, nil
		}
		s.log("Add vlan to interface success :" + strconv.Itoa(int(in.Vlan)))

		s.log("Setting mac adddres for vlan interface :" + in.GetMacAddress())
		if err := container.SetMacAddress(in.GetName(), in.GetMacAddress(), int(in.Vlan)); err != nil {
			resp := errors.Wrapf(err, "Set Mac Address failed").Error()
			s.log(resp)
			return &pb.InterfaceStatus{Response: resp,
				Status: pb.ApiStatus_API_STATUS_FAILED}, nil
		}

		//Below statement are hack for now to increase ARP timeout.
		cmd := []string{"sysctl", "-w", "net.ipv4.neigh." + in.GetName() + ".retrans_time_ms=" + strconv.Itoa(arpTimeout)}
		if retCode, stdout, _, _ := container.RunCommandInNS(cmd, 0, false, true); retCode != 0 {
			s.log(stdout)
		}
		cmd = []string{"sysctl", "-w", "net.ipv4.neigh." + in.GetName() + "_" + strconv.Itoa(int(in.Vlan)) + ".retrans_time_ms=" + strconv.Itoa(arpTimeout)}
		if retCode, stdout, _, _ := container.RunCommandInNS(cmd, 0, false, true); retCode != 0 {
			s.log(stdout)
		}
	}

	if in.IpAddress != "" {
		if err := container.SetIPAddress(in.GetName(), in.GetIpAddress(), int(in.GetPrefixLen()), int(in.GetVlan())); err != nil {
			resp := errors.Wrapf(err, "Set IP Address failed").Error()
			s.log(resp)
			return &pb.InterfaceStatus{Response: resp,
				Status: pb.ApiStatus_API_STATUS_FAILED}, nil
		}
		s.log("Add IP address to interface success : " + in.IpAddress)
	}

	resp := "Attach Interface success."
	s.log(resp)

	return &pb.InterfaceStatus{Response: resp,
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *AppServer) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	s.log("Run command request received for  " + in.App + " " + in.GetCmd())
	contianer, f := s.containerMap[in.App]
	if !f {
		resp := appNotFound
		s.log(resp)
		return &pb.CommandStatus{
			Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
	}

	retCode, stdout, stderr, _ := contianer.RunCommand(strings.Split(in.Cmd, " "),
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
	appServer.containerMap = make(map[string]*infra.Container)
	return appServer
}
