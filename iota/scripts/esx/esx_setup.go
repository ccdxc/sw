package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/iota/svcs/common/runner"
	vmware "github.com/pensando/sw/iota/svcs/common/vmware"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/spf13/cobra"
)

const (
	restartTimeout = 300 //300 seconds for node restart
)

var (
	esxHost, esxUsername, esxPassword       string
	macHint, naplesUsername, naplesPassword string
	ctrlVMUsername, ctrlVMPassword          string
	esxOutFile                              string
)

// EsxSetup contains info about a setup
type EsxSetup struct {
	CtrlVMIP       string `json:"ctrlVMIP"`
	CtrlVMUsername string `json:"ctrlVMUsername"`
	CtrlVMPassword string `json:"ctrlVMPassword"`
}

func cleanupEsxNode() error {

	host, err := vmware.NewHost(context.Background(), esxHost, esxUsername, esxPassword)
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | Clean Esx node, failed to get host handle  %v", err.Error())
		return err
	}

	vms, err := host.GetAllVms()
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | unable to list VMS %v", err.Error())
		return err
	}

	for _, vm := range vms {
		vm.Destroy()
		if err != nil {
			log.Errorf("TOPO SVC | CleanTestBed | Destroy vm node failed %v", err.Error())
		}
	}

	if nws, err := host.ListNetworks(); err == nil {
		delNws := []vmware.NWSpec{}
		for _, nw := range nws {
			if nw.Name != "VM Network" {
				delNws = append(delNws, vmware.NWSpec{Name: nw.Name})
			}
		}
		host.RemoveNetworks(delNws)
	}

	if vswitches, err := host.ListVswitchs(); err == nil {
		for _, vswitch := range vswitches {
			if vswitch.Name != "vSwitch0" {
				host.RemoveVswitch(vswitch)
			}
		}
	}

	return nil
}

func waitForNodeUp(ip string, timeout time.Duration) error {
	cTimeout := time.After(time.Second * time.Duration(timeout))
	for {
		conn, _ := net.DialTimeout("tcp", net.JoinHostPort(ip, "22"), 2*time.Second)
		if conn != nil {
			log.Printf("Connected to host : %s", ip)
			conn.Close()
			break
		}
		select {
		case <-cTimeout:
			msg := fmt.Sprintf("Timeout system to be up %s ", ip)
			log.Errorf(msg)
			return errors.New(msg)
		default:
			time.Sleep(100 * time.Millisecond)
		}
	}
	return nil
}

func downloadControlVMImage() (string, error) {

	ctrlVMDir := constants.ControlVMImageDirectory + "/" + constants.EsxControlVMImage + "_" + esxHost
	imageName := constants.EsxControlVMImage + ".ova"
	cwd, _ := os.Getwd()
	mkdir := []string{"mkdir", "-p", ctrlVMDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	os.Chdir(ctrlVMDir)
	defer os.Chdir(cwd)

	buildIt := []string{constants.BuildItBinary, "-t", constants.BuildItURL, "image", "pull", "-o", imageName, constants.EsxControlVMImage}
	if stdout, err := exec.Command(buildIt[0], buildIt[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	tarCmd := []string{"tar", "-xvf", imageName}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	if stdout, err := exec.Command("/bin/sync").CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	return ctrlVMDir, nil
}

func initEsxCtrlVM() (string, error) {
	var ctrlVMDir string
	var err error

	if ctrlVMDir, err = downloadControlVMImage(); err != nil {
		return "", errors.Wrap(err, "Download control image failed")
	}

	host, err := vmware.NewHost(context.Background(), esxHost, esxUsername, esxPassword)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Esx node, failed to get host handle  %v", err.Error())
		return "", err
	}

	err = host.DestoryVM(constants.EsxControlVMName)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Delete control node failed %v ", err.Error())
	}

	vsname := constants.EsxIotaCtrlSwitch
	vsspec := vmware.VswitchSpec{Name: vsname}
	err = host.AddVswitch(vsspec)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to create vswitch %v ", err.Error())
	}
	nws := []vmware.NWSpec{{Name: constants.EsxDefaultNetwork, Vlan: int32(constants.EsxDefaultNetworkVlan)}, {Name: constants.EsxVMNetwork, Vlan: int32(constants.EsxVMNetworkVlan)}}
	_, err = host.AddNetworks(nws, vsspec)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to create networks %v ", err.Error())
	}

	vmInfo, err := host.DeployVM(constants.EsxControlVMName, constants.EsxControlVMCpus, constants.EsxControlVMMemory, constants.EsxControlVMNetworks, ctrlVMDir)

	if err != nil {

		log.Errorf("TOPO SVC | InitTestBed | Add control node failed %v", err.Error())
		return "", err
	}

	log.Printf("Init ESX node complete with IP : %v", vmInfo.IP)

	if err := waitForNodeUp(vmInfo.IP, restartTimeout); err != nil {
		return "", err
	}

	return vmInfo.IP, nil
}

func getIotaAgentClient(ip, username, password string) (iota.IotaAgentApiClient, error) {

	addr := fmt.Sprintf("%s:%d", ip, constants.SSHPort)
	cfg := &ssh.ClientConfig{
		User: username,
		Auth: []ssh.AuthMethod{
			ssh.Password(password),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = password
				}

				return answers, nil
			}),
		}, HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	copier := copier.NewCopier(cfg)

	agentBinary := constants.IotaAgentBinaryPathLinux

	if err := copier.CopyTo(addr, constants.DstIotaAgentDir, []string{agentBinary}); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed, IPAddress: %v , Err: %v", ip, err)
		return nil, fmt.Errorf("CopyTo node failed IPAddress: %v , Err: %v", ip, err)
	}

	//Copy Nic configuration
	if err := copier.CopyTo(addr, constants.DstIotaAgentDir, []string{constants.NicFinderConf}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to Nic conf file: %v, to  IPAddress: %v", constants.NicFinderConf, ip)
		return nil, err
	}

	//Copy  NicFinderScript
	if err := copier.CopyTo(addr, constants.DstIotaAgentDir, []string{constants.NicFinderScript}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to  NicFinderScript file: %v, to  IPAddress: %v", constants.NicFinderScript, ip)
		return nil, err
	}

	log.Infof("TOPO SVC | InitTestBed | Starting IOTA Agent on IPAddress: %v", ip)
	sudoAgtCmd := fmt.Sprintf("sudo %s", constants.DstIotaAgentBinary)

	runner := runner.NewRunner(cfg)

	if err := runner.Run(addr, sudoAgtCmd, constants.RunCommandBackground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | StartAgent failed, IPAddress: %v , Err: %v", ip, err)
		return nil, fmt.Errorf("StartAgent on node failed., IPAddress: %v , Err: %v", ip, err)
	}

	agentURL := fmt.Sprintf("%s:%d", ip, constants.IotaAgentPort)

	c, err := constants.CreateNewGRPCClient("esxInit", agentURL, 0)
	if err != nil {
		log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent. Err: %v", err)
		return nil, errors.New("Failed to establish GRPC connectio")
	}

	return iota.NewIotaAgentApiClient(c.Client), nil

}

func provideNaplesEsxPersonality(client iota.IotaAgentApiClient) error {

	req := &iota.Node{Name: "naples-esx-test", IpAddress: "",
		NodeInfo: &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{NaplesIpAddress: "", NicHint: macHint,
			Name: "naples", NicType: "pensando", NaplesUsername: naplesUsername, NaplesPassword: naplesPassword}}}},
		EsxConfig: &iota.VmwareESXConfig{IpAddress: esxHost, Username: esxUsername, Password: esxPassword},
		Entities: []*iota.Entity{&iota.Entity{Name: "host", Type: iota.EntityType_ENTITY_TYPE_HOST},
			&iota.Entity{Name: "naples", Type: iota.EntityType_ENTITY_TYPE_NAPLES}},
		Type: iota.PersonalityType_PERSONALITY_NAPLES, Os: iota.TestBedNodeOs_TESTBED_NODE_OS_ESX,
	}

	resp, err := client.AddNode(context.Background(), req)
	if err != nil {
		log.Errorf("Failed add not grpc messeage ")
		return err
	}

	if resp.NodeStatus.GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to add naples esx personality %v", resp.NodeStatus.GetErrorMsg())
		return err
	}

	log.Printf("Add naples personality..")
	return nil
}

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "esx-setup",
	Short: "sets up ESX for IOTA",
	Long:  `sets up ESX for IOTA`,
	RunE: func(cmd *cobra.Command, args []string) error {

		if esxHost == "" || esxOutFile == "" {
			cmd.Usage()
			return errors.New("Invalid command usage")
		}

		if err := cleanupEsxNode(); err != nil {
			log.Error("Failed to clean up esx node")
			return err
		}
		ip, err := initEsxCtrlVM()
		if err != nil {
			log.Error("Failed to initialize control VM")
			return err

		}

		client, err := getIotaAgentClient(ip, ctrlVMUsername, ctrlVMPassword)
		if err != nil {
			log.Error("Failed to setup iota agent client")
			return err
		}

		if err := provideNaplesEsxPersonality(client); err != nil {
			log.Error("Failed to provide naples personality")
			return err
		}

		setUPOut := &EsxSetup{CtrlVMIP: ip, CtrlVMUsername: ctrlVMUsername, CtrlVMPassword: ctrlVMPassword}
		setUPOutJSON, err := json.Marshal(setUPOut)
		if err != nil {
			log.Errorf("Failed to convert to json %v", err)
		}
		return ioutil.WriteFile(esxOutFile, setUPOutJSON, 0644)
	},
}

func main() {

	if err := RootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

}

func init() {
	RootCmd.Flags().StringVarP(&esxHost, "esx-host", "", "", "Esx Host IP address")
	RootCmd.Flags().StringVarP(&esxOutFile, "esx-outfile", "", "", "Esx Setup output file")
	RootCmd.Flags().StringVarP(&esxUsername, "esx-username", "", "root", "Esx Username")
	RootCmd.Flags().StringVarP(&esxPassword, "esx-password", "", "ubuntu123", "Esx Password")

	RootCmd.Flags().StringVarP(&macHint, "mac-hint", "", "", "Mac hint")
	RootCmd.Flags().StringVarP(&naplesUsername, "naples-username", "", "root", "Naples Username")
	RootCmd.Flags().StringVarP(&naplesPassword, "naples-password", "", "pen123", "Naples Password")

	RootCmd.Flags().StringVarP(&ctrlVMUsername, "ctrl-vm-username", "", "vm", "Ctrl Username")
	RootCmd.Flags().StringVarP(&ctrlVMPassword, "ctrl-vm-password", "", "vm", "Ctrl Password")
}
