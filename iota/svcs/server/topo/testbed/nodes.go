package testbed

import (
	"context"
	"errors"
	"fmt"
	"net"
	"time"

	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/runner"
	vmware "github.com/pensando/sw/iota/svcs/common/vmware"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	restartTimeout = 300 //300 seconds for node restart
)

// AddNode adds a node to the topology
func (n *TestNode) AddNode() error {
	var ip string
	var err error

	if ip, err = n.GetNodeIP(); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed to get node IP")
		return fmt.Errorf("TOPO SVC | CopyTo node failed to get node IP")
	}

	addr := fmt.Sprintf("%s:%d", ip, constants.SSHPort)
	sshclient, err := ssh.Dial("tcp", addr, n.SSHCfg)
	if sshclient == nil || err != nil {
		msg := fmt.Sprintf("SSH connect to %v (%v) failed", n.Node.Name, n.Node.IpAddress)
		log.Error(msg)
		n.RespNode = &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return err
	}

	n.SSHClient = sshclient
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Minute)
	defer cancel()
	resp, err := n.AgentClient.AddNode(ctx, n.Node)
	n.RespNode = resp
	log.Infof("TOPO SVC | DEBUG | AddNode Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Adding node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if resp.NodeStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.NodeStatus.ApiStatus)
		return fmt.Errorf("adding node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.NodeStatus.ApiStatus)
	}
	n.Node.NodeUuid = resp.NodeUuid
	return nil
}

func (n *TestNode) waitForNodeUp(timeout time.Duration) error {
	cTimeout := time.After(time.Second * time.Duration(timeout))
	for {
		conn, _ := net.DialTimeout("tcp", net.JoinHostPort(n.Node.GetIpAddress(), "22"), 2*time.Second)
		if conn != nil {
			log.Printf("Connected to host : ", n.Node.GetIpAddress())
			conn.Close()
			break
		}
		select {
		case <-cTimeout:
			msg := fmt.Sprintf("Timeout system to be up %s ", n.Node.GetIpAddress())
			log.Errorf(msg)
			return errors.New(msg)
		default:
			time.Sleep(100 * time.Millisecond)
		}
	}
	return nil
}

//GetAgentURL get agents URL
func (n *TestNode) GetAgentURL() (string, error) {

	var ip string
	var err error
	if ip, err = n.GetNodeIP(); err != nil {
		return "", err
	}

	agentURL := fmt.Sprintf("%s:%d", ip, constants.IotaAgentPort)
	return agentURL, nil
}

//GetNodeIP gets node IP
func (n *TestNode) GetNodeIP() (string, error) {

	var ip string
	var err error
	var host *vmware.Host

	if n.Node.GetOs() != iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		ip = n.Node.IpAddress
	} else {
		if n.Node.EsxConfig == nil {
			log.Errorf("TOPO SVC | GetNodeIP | No ESX config present %v", err.Error())
			return "", err
		}
		host, err = vmware.NewHost(context.Background(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
		if err != nil {
			log.Errorf("TOPO SVC | GetNodeIP | Failed to  connect to ESX host  %v %v %v %v", err.Error(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
			return "", err
		}

		ip, err = host.GetVMIP(constants.EsxControlVMName)
		if err != nil {
			log.Errorf("TOPO SVC | GetNodeIP | Failed to get IP address for control VM  %v", err.Error())
			return "", err
		}
	}

	return ip, nil
}

//RestartNode Restart node
func (n *TestNode) RestartNode() error {

	runner := runner.NewRunner(n.SSHCfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)
	command := fmt.Sprintf("sudo shutdown -r now")

	runner.Run(addr, command, constants.RunCommandForeground)

	time.Sleep(3 * time.Second)

	if err := n.waitForNodeUp(restartTimeout); err != nil {
		return err
	}

	sshclient, err := ssh.Dial("tcp", addr, n.SSHCfg)
	if sshclient == nil || err != nil {
		log.Errorf("SSH connect to %v (%v) failed", n.Node.Name, n.Node.IpAddress)
		return err
	}

	n.SSHClient = sshclient

	return nil
}

// ReloadNode saves and reboots the nodes.
func (n *TestNode) ReloadNode() error {
	var agentBinary string

	resp, err := n.AgentClient.SaveNode(context.Background(), n.Node)
	n.RespNode = resp
	log.Infof("TOPO SVC | DEBUG | SaveNode Agent . Received Response Msg: %v", resp)
	if err != nil {
		log.Errorf("Saving node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if err = n.RestartNode(); err != nil {
		log.Errorf("Restart node %v failed. Err: %v", n.Node.Name, err)
		return err
	}

	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD {
		agentBinary = constants.IotaAgentBinaryPathFreebsd
	} else {
		agentBinary = constants.IotaAgentBinaryPathLinux
	}

	log.Infof("TOPO SVC | ReloadNode | Starting IOTA Agent on TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	sudoAgtCmd := fmt.Sprintf("sudo %s", constants.DstIotaAgentBinary)
	if err = n.StartAgent(sudoAgtCmd, n.SSHCfg); err != nil {
		log.Errorf("TOPO SVC | RestartNode | Failed to start agent binary: %v, on TestNode: %v, at IPAddress: %v", agentBinary, n.Node.Name, n.Node.IpAddress)
		return err
	}

	agentURL := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.IotaAgentPort)
	c, err := constants.CreateNewGRPCClient(n.Node.Name, agentURL)
	if err != nil {
		errorMsg := fmt.Sprintf("Could not create GRPC Connection to IOTA Agent. Err: %v", err)
		log.Errorf("TOPO SVC | ReloadNode | ReloadNode call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Node.Name, errorMsg)
		return err
	}

	n.AgentClient = iota.NewIotaAgentApiClient(c.Client)
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Minute)
	defer cancel()
	resp, err = n.AgentClient.ReloadNode(ctx, n.Node)
	n.RespNode = resp
	log.Infof("TOPO SVC | ReloadNode | ReloadNode Agent . Received Response Msg: %v", resp)
	if err != nil {
		log.Errorf("Reload node %v failed. Err: %v", n.Node.Name, err)
		return err
	}
	return nil
}
