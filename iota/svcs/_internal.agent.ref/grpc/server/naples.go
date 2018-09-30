package server

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pkg/errors"

	Common "github.com/pensando/sw/iota/common"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
	Globals "github.com/pensando/sw/iota/svcs/globals"
)

const (
	naplesCfgDir              = "/naples/nic/conf"
	hntapCfgFile              = "hntap-cfg.json"
	hntapQemCfgFile           = "hntap-with-qemu.json"
	containerHntapRestartCmd  = "naples/nic/tools/restart-hntap.sh"
	hntapTempCfgFile          = "/tmp/hntap-cfg.json"
	bootStrapNaplesCmd        = "/usr/bin/bootstrap-naples.sh"
	bootStrapNaplesQemuOption = "--qemu"
)

//HntapCfg hntap config structure
type hntapCfg struct {
	Devices []struct {
		Name  string `json:"name"`
		Local bool   `json:"local"`
		Port  int    `json:"port"`
		LifID int    `json:"lif_id"`
	} `json:"devices"`
	Switch struct {
		OutInterface string   `json:"out-interface"`
		Peers        []string `json:"peers"`
	} `json:"switch"`
}

// NaplesSim is used to implement Naples Sim
type NaplesSim struct {
	server
	simName         string
	container       *Common.Container
	withQemu        bool
	passThroughMode bool
	//Hntap     Common.Hntap
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

func (s *NaplesSim) bringUpNaples(name string, nodeID int, ctrlIntf string,
	ctrlIP string, dataIntfs []string, dataIPs []string, veniceIPs []string,
	withQemu bool, passThroughMode bool) error {
	os.Chdir(Globals.RemoteNaplesDirectory + "/images")
	s.log("Untar image : " + Globals.NaplesContainerImage)
	untar := []string{"tar", "-xvzf", Globals.NaplesContainerImage}
	if _, stdout, err := Common.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}
	s.log("Untar successfull")
	env := []string{"NAPLES_HOME=" + Globals.RemoteNaplesDirectory + "/images"}
	cmd := []string{"sudo", "-E", "python", Globals.NaplesVMBringUpScript, "--node-id", strconv.Itoa(nodeID),
		"--data-intfs", strings.Join(dataIntfs, ",")}

	if ctrlIntf != "" {
		cmd = append(cmd, "--control-intf")
		cmd = append(cmd, ctrlIntf)
		cmd = append(cmd, "--control-ip")
		cmd = append(cmd, ctrlIP)
	}

	if len(dataIPs) != 0 {
		cmd = append(cmd, "--data-ips")
		cmd = append(cmd, strings.Join(dataIPs, ","))
	}

	if len(veniceIPs) != 0 {
		cmd = append(cmd, "--venice-ips")
		cmd = append(cmd, strings.Join(veniceIPs, ","))
	}

	if withQemu {
		cmd = append(cmd, "--qemu")
		s.withQemu = true
	}

	cmd = append(cmd, "--hntap-mode")
	if passThroughMode {
		cmd = append(cmd, "passthrough")
		s.passThroughMode = true
	} else {
		cmd = append(cmd, "tunnel")
	}

	if _, stdout, err := Common.Run(cmd, 0, false, false, env); err != nil {
		return errors.Wrap(err, stdout)
	}
	var err error
	if s.container, err = Common.NewContainer(name, "", name); err != nil {
		return errors.Wrap(err, "Naples sim not running!")
	}
	s.simName = name
	s.log("Naples bring script up succesful.")

	return nil

}

func (s *NaplesSim) configureNewNaplesDataNetwork(intfName string, peers []string) error {

	var hntapFile string
	if s.withQemu {
		hntapFile = naplesCfgDir + "/" + hntapQemCfgFile
	} else {
		hntapFile = naplesCfgDir + "/" + hntapCfgFile
	}
	cpCmd := []string{"docker", "cp", s.simName + ":" + hntapFile, hntapTempCfgFile}
	if _, _, err := Common.Run(cpCmd, 0, false, false, nil); err != nil {
		return errors.Wrap(err, "Error in coping hntap config file to host")
	}

	jsonFile, err := os.Open(hntapTempCfgFile)
	if err != nil {
		return errors.Wrap(err, "Error opening hntap cfg file")
	}
	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)

	var hntapcfg hntapCfg
	json.Unmarshal(byteValue, &hntapcfg)
	hntapcfg.Switch.OutInterface = intfName
	for _, peer := range peers {
		hntapcfg.Switch.Peers = append(hntapcfg.Switch.Peers, peer)
	}

	hntapJSON, _ := json.Marshal(hntapcfg)
	if err := ioutil.WriteFile(hntapTempCfgFile, hntapJSON, 0644); err != nil {
		return errors.Wrap(err, "Error in marshalling hntap cfg")
	}

	cpCmd = []string{"docker", "cp", hntapTempCfgFile, s.simName + ":" + hntapFile}
	if _, _, err := Common.Run(cpCmd, 0, false, false, nil); err != nil {
		return errors.Wrap(err, "Error in coping hntap config file to host")
	}

	killBootStrapNaples := []string{"pkill", "-9", "-f", "bootstrap-nap"}
	Common.Run(killBootStrapNaples, 0, false, false, nil)

	retCode, stdout, _, _ := s.container.RunCommand([]string{containerHntapRestartCmd}, 0, false, true)
	if retCode != 0 {
		errors.New("Restart hntap failed..." + stdout)
	}

	bootstrapCmd := []string{bootStrapNaplesCmd}
	if s.withQemu {
		bootstrapCmd = append(bootstrapCmd, bootStrapNaplesQemuOption)
	}

	if _, _, err := Common.Run(bootstrapCmd, 0, false, true, nil); err != nil {
		errors.New("Bootstrap start cmd failed..." + stdout)
	}

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

	if err := s.bringUpNaples(in.Name, int(in.NodeID), in.ControlIntf,
		in.ControlIP, in.DataIntfs, in.DataIPs, in.VeniceIPs,
		in.WithQemu, in.PassThroughMode); err != nil {
		resp := "Naples bring up failed : " + err.Error()
		s.log(resp)
		return &pb.NaplesStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}
	s.log("Naples bring script up succesful.")

	//s.Hntap = Common.HntapGet(Common.HntapTypeContainer, hntapCfgFile, s.container)
	resp := "Naples bring up successfull"
	s.log(resp)

	return &pb.NaplesStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *NaplesSim) Teardown(ctx context.Context, in *pb.NaplesSimConfig) (*pb.NaplesStatus, error) {
	s.log("Teardown Naples sim request received for : " + in.Name)
	if s.container == nil {
		container, err := Common.NewContainer(in.Name, "", in.Name)
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

	var hntapcfg []Common.HntapCfg
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
