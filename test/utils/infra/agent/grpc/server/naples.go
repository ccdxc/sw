package server

import (
	"os"
	"strconv"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pkg/errors"

	"github.com/pensando/sw/test/utils/infra"
	pb "github.com/pensando/sw/test/utils/infra/agent/grpc/api/pb"
	Common "github.com/pensando/sw/test/utils/infra/common"
)

const hntapCfgFile string = "/tmp/hntap-cfg.json"

// NaplesSim is used to implement Naples Sim
type NaplesSim struct {
	server
	simName   string
	container *infra.Container
	//Hntap     infra.Hntap
}

const (
	naplesUplinkCnt        = 1
	dataBridgeName         = "data-net"
	dataBridgeCheckTimeout = 20 * time.Second
)

var (
	naplesUplinkIntfs = []string{"pen-intf1"}
)

func (s *NaplesSim) configureHostDataNetwork(intfName string, ipAddress string) error {
	s.log("Disable DHCP on the data interface.")
	if err := Common.DisableDhcpOnInterface(intfName); err != nil {
		return err
	}
	cmd := []string{"service", "docker", "restart"}
	Common.Run(cmd, 0, false, false, nil)
	s.log("Configure IP address data network.")
	return Common.SetUpIPAddress(intfName, ipAddress)
}

func (s *NaplesSim) bringUpNaples(name string, nodeID int, ctrlNwIPRange string, withQemu bool) error {
	os.Chdir(infra.RemoteNaplesDirectory + "/images")
	s.log("Untar image : " + infra.NaplesContainerImage)
	untar := []string{"tar", "-xvzf", infra.NaplesContainerImage}
	if _, stdout, err := Common.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}
	s.log("Untar successfull")
	env := []string{"NAPLES_HOME=" + infra.RemoteNaplesDirectory + "/images"}
	cmd := []string{"sudo", "-E", "python", infra.NaplesVMBringUpScript, "--node-id", strconv.Itoa(nodeID),
		"--network-ip-range", ctrlNwIPRange}
	if withQemu {
		cmd = append(cmd, "--qemu")
	}
	if _, stdout, err := Common.Run(cmd, 0, false, false, env); err != nil {
		return errors.Wrap(err, stdout)
	}
	var err error
	if s.container, err = infra.NewContainer(name, "", name); err != nil {
		return errors.Wrap(err, "Naples sim not running!")
	}
	s.simName = name
	s.log("Naples bring script up succesful.")

	return nil

}

func (s *NaplesSim) configureNaplesDataNetwork() error {
	checkNaplesDataNet := func() error {
		s.log("Waiting for naples-sim interface to connect to data-net")
		cTimeout := time.After(dataBridgeCheckTimeout)
		for true {
			connected := 0
			for i := 0; i < len(naplesUplinkIntfs); i++ {
				cmd := []string{"ovs-vsctl", "list-ports", dataBridgeName, "|", "grep", naplesUplinkIntfs[i]}
				s.log("Running cmd :" + strings.Join(cmd, " "))
				retcode, stdout, _ := Common.Run(cmd, 0, false, true, nil)
				if retcode == 0 {
					connected++
				}
				s.log(stdout)
			}
			if connected == len(naplesUplinkIntfs) {
				s.log("Uplink interfaces connected to data network")
				return nil
			}
			time.Sleep(1 * time.Second)
			select {
			case <-cTimeout:
				return errors.New("Naples sim uplink interface not connected too data-net")
			default:
			}
		}
		return nil
	}

	cmd := []string{"ovs-vsctl", "set", "bridge", dataBridgeName, "flood_vlans=1-4095"}
	if _, stdout, err := Common.Run(cmd, 0, false, true, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	if err := checkNaplesDataNet(); err != nil {
		return errors.Wrap(err, "Uplink interfaces not connected to data-net "+err.Error())
	}

	return nil
}

// BringUp Bring up app
func (s *NaplesSim) BringUp(ctx context.Context, in *pb.NaplesSimConfig) (*pb.NaplesStatus, error) {
	// For now we are not actually doing bring up.

	s.log("Bring up request received for : " + in.Name)

	if err := s.configureHostDataNetwork(in.TunnelInterface, in.TunnelIpAddress+"/24"); err != nil {
		resp := "Configure Naples Host Network failed " + err.Error()
		s.log(resp)
		return &pb.NaplesStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	if err := s.bringUpNaples(in.Name, int(in.NodeID), in.CtrlNwIpRange, in.WithQemu); err != nil {
		resp := "Naples bring up failed : " + err.Error()
		s.log(resp)
		return &pb.NaplesStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}
	s.log("Naples bring script up succesful.")

	if err := s.configureNaplesDataNetwork(); err != nil {
		resp := "Configure Naples Data Network failed " + err.Error()
		s.log(resp)
		return &pb.NaplesStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	//s.Hntap = infra.HntapGet(infra.HntapTypeContainer, hntapCfgFile, s.container)
	resp := "Naples bring up successfull"
	s.log(resp)

	return &pb.NaplesStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *NaplesSim) Teardown(ctx context.Context, in *pb.NaplesSimConfig) (*pb.NaplesStatus, error) {
	s.log("Teardown Naples sim request received for : " + in.Name)
	if s.container == nil {
		container, err := infra.NewContainer(in.Name, "", in.Name)
		if err != nil {
			resp := "Naples Sim Container not running..." + err.Error()
			s.log(resp)
			return &pb.NaplesStatus{Response: resp,
				Status: pb.ApiStatus_API_STATUS_APP_NOT_FOUND}, nil
		}
		s.container = container
	}
	s.container.Stop()
	s.simName = ""
	s.container = nil
	resp := "Naples Container Stopped"
	s.log(resp)
	return &pb.NaplesStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *NaplesSim) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	s.log("Run command request :  " + in.GetCmd())
	if s.container == nil {
		return &pb.CommandStatus{
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	retCode, stdout, stderr, _ := s.container.RunCommand(strings.Split(in.Cmd, " "),
		in.Timeout, in.Background, true)

	s.log("Run command status :  " + strconv.Itoa(retCode))
	s.log("Run command stdout :  " + stdout)
	s.log("Run command stder  :  " + stderr)
	return &pb.CommandStatus{
		Status:  pb.ApiStatus_API_STATUS_OK,
		RetCode: int32(retCode),
		Stdout:  stdout,
	}, nil
}

//ConfigureHntap Configure Hntap.
func (s *NaplesSim) ConfigureHntap(ctx context.Context, in *pb.HntapConfig) (*pb.NaplesStatus, error) {
	/* if s.container == nil {
		return &pb.NaplesStatus{Response: "Naples Sim not running.",
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	var hntapcfg []infra.HntapCfg
	json.Unmarshal([]byte(in.Config), &hntapcfg)
	if len(hntapcfg) == 0 {
		return &pb.NaplesStatus{Response: "Hntap Config is empty or not in required format",
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}
	hntapJSON, _ := json.Marshal(hntapcfg)
	err := ioutil.WriteFile(hntapCfgFile, hntapJSON, 0644)
	if err != nil {
		return &pb.NaplesStatus{Response: "Error in processing Hntap Config file",
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}
	if err := s.Hntap.Run(hntapCfgFile, false); err != nil {
		return &pb.NaplesStatus{Response: "Hntap bring up failed",
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil

	} */
	return &pb.NaplesStatus{Response: "Configured HNTAP.",
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

//NewNaplesSim Returns Naples Sim context
func NewNaplesSim() *NaplesSim {
	return &NaplesSim{server: server{name: "Naples"}}
}
