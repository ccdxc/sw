package server

import (
	"os"

	context "golang.org/x/net/context"

	Common "github.com/pensando/sw/iota/common"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
	Globals "github.com/pensando/sw/iota/svcs/globals"
	"github.com/pkg/errors"
)

// Venice is used to implement Naples Sim
type Venice struct {
	server
}

func (s *Venice) bringUpVenice(ctrlIntf string, ctrlIP string) error {

	if ctrlIntf != "" {
		Common.DisableDhcpOnInterface(ctrlIntf)
		s.log("Configuring intf : " + ctrlIntf + " with " + ctrlIP)
		ifConfigCmd := []string{"ifconfig", ctrlIntf, ctrlIP, "up"}
		if _, stdout, err := Common.Run(ifConfigCmd, 0, false, true, nil); err != nil {
			errors.New("Setting control interface IP to venice node failed.." + stdout)
		}
	}

	os.Chdir(Globals.RemoteVeniceDirectory + "/bin")
	s.log("Untar image : " + Globals.VeniceContainerImage)
	untar := []string{"tar", "-xvzf", Globals.VeniceContainerImage}
	if _, stdout, err := Common.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	s.log("Running Install Script : " + Globals.VeniceStartScript)
	install := []string{"./" + Globals.VeniceStartScript, "--clean"}
	if _, stdout, err := Common.Run(install, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	return nil
}

// BringUp Bring up app
func (s *Venice) BringUp(ctx context.Context, in *pb.VeniceConfig) (*pb.VeniceStatus, error) {

	if err := s.bringUpVenice(in.ControlIntf, in.ControlIP); err != nil {
		resp := "Venice bring up failed : " + err.Error()
		s.log(resp)
		return &pb.VeniceStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	//s.Hntap = infra.HntapGet(infra.HntapTypeContainer, hntapCfgFile, s.container)
	resp := "Venice bring up successfull"
	s.log(resp)

	return &pb.VeniceStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *Venice) Teardown(ctx context.Context, in *pb.VeniceConfig) (*pb.VeniceStatus, error) {
	s.log("Teardown Naples sim request received for : " + in.Name)
	resp := "Venice Node stopped"
	s.log(resp)
	return &pb.VeniceStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *Venice) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	return &pb.CommandStatus{
		Status:  pb.ApiStatus_API_STATUS_OK,
		RetCode: 0,
		Stdout:  "",
	}, nil
}

//NewVenice Returns Naples Sim context
func NewVenice() *Venice {
	return &Venice{server: server{name: "Venice"}}
}
