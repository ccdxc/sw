package agent

import (
	"os"
	"strings"
	"time"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	veniceStartScript = "INSTALL.sh"
	veniceInstallLog  = "install.log"
)

type veniceNode struct {
	commandNode
}

type veniceBMNode struct {
	commandNode
}

type venicePeerNode struct {
	hostname string
	ip       string
}

func (venice *veniceNode) bringUpVenice(image string, hostname string,
	ctrlIntf string, ctrlIP string, reload bool, peers []venicePeerNode) error {

	if ctrlIntf != "" {
		utils.DisableDhcpOnInterface(ctrlIntf)
		venice.logger.Println("Configuring intf : " + ctrlIntf + " with " + ctrlIP)
		ifConfigCmd := []string{"ifconfig", ctrlIntf, ctrlIP, "up"}
		if _, stdout, err := utils.Run(ifConfigCmd, 0, false, true, nil); err != nil {
			errors.New("Setting control interface IP to venice node failed.." + stdout)
		}
	}

	// if this is a reload, we are done
	if reload {
		return nil
	}

	curDir, _ := os.Getwd()
	defer os.Chdir(curDir)
	os.Chdir(Common.ImageArtificatsDirectory)
	venice.logger.Println("Untar image : " + image)
	untar := []string{"tar", "-xvzf", image}
	if _, stdout, err := utils.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	for i := 0; true; i++ {
		setHostname := []string{"hostnamectl", "set-hostname", "--static", hostname}
		if stderr, stdout, err := utils.Run(setHostname, 0, false, false, nil); err != nil {
			venice.logger.Printf("Setting hostname failed %v %v %v", stderr, stdout, err.Error())
			if i == 5 {
				return errors.Wrap(err, stdout)
			}
			time.Sleep(1 * time.Second)
		}
		break
	}

	venice.logger.Println("Running Install Script : " + veniceStartScript)
	install := []string{"./" + veniceStartScript, "--clean", "|", "tee", veniceInstallLog}
	if _, stdout, err := utils.Run(install, 0, false, true, nil); err != nil {
		venice.logger.Println("Running Install Script failed : " + veniceStartScript)
		return errors.Wrap(err, stdout)
	}

	cmd := []string{"touch", "/etc/hosts"}
	utils.Run(cmd, 0, false, true, nil)

	for _, peer := range peers {
		if peer.hostname != "" && peer.ip != "" {
			cmd := []string{"echo", strings.Split(peer.ip, "/")[0], peer.hostname, " | sudo tee -a /etc/hosts"}
			if _, stdout, err := utils.Run(cmd, 0, false, true, nil); err != nil {
				venice.logger.Println("Setting venice peer hostnames failed")
				return errors.Wrap(err, stdout)
			}
		}
	}
	return nil
}

//Init initalize node type
func (venice *veniceNode) Init(in *iota.Node) (*iota.Node, error) {
	venice.commandNode.Init(in)
	venice.iotaNode.name = in.GetName()
	venice.iotaNode.nodeMsg = in
	venice.logger.Printf("Bring up request received for : %v. Req: %+v", in.GetName(), in)

	veniceNodes := []venicePeerNode{}

	for _, node := range in.GetVeniceConfig().GetVenicePeers() {
		veniceNodes = append(veniceNodes, venicePeerNode{hostname: node.GetHostName(),
			ip: node.GetIpAddress()})
	}

	if err := venice.bringUpVenice(in.GetImage(), in.GetName(),
		in.GetVeniceConfig().GetControlIntf(), in.GetVeniceConfig().GetControlIp(),
		in.GetReload(), veniceNodes); err != nil {
		venice.logger.Println("Venice bring up failed.")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err

	}

	venice.logger.Println("Venice bring script up successful.")

	dir := Common.DstIotaEntitiesDir + "/" + in.GetName()
	os.Mkdir(dir, 0765)
	os.Chmod(dir, 0777)

	return &iota.Node{Name: in.Name, IpAddress: in.IpAddress, Type: in.GetType(),
		NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

// AddWorkload brings up a workload type on a given node
func (venice *veniceNode) AddWorkloads(*iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	venice.logger.Println("Add workload on venice not supported.")
	return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// DeleteWorkloads deletes a given workloads
func (venice *veniceNode) DeleteWorkloads(*iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	venice.logger.Println("Delete workload on venice not supported.")
	return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

func (venice *veniceBMNode) Init(in *iota.Node) (*iota.Node, error) {
	venice.commandNode.Init(in)
	venice.iotaNode.name = in.GetName()
	venice.iotaNode.nodeMsg = in
	venice.logger.Printf("Bring up request received for BM Venice node : %v. Req: %+v", in.GetName(), in)

	dir := Common.DstIotaEntitiesDir + "/" + in.GetName()
	os.Mkdir(dir, 0765)
	os.Chmod(dir, 0777)

	return &iota.Node{Name: in.Name, IpAddress: in.IpAddress, Type: in.GetType(),
		NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}
