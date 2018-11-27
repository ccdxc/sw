package testbed

import (
	"context"
	"errors"
	"fmt"
	"net"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/runner"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	restartTimeout = 300 //300 seconds for node restart
)

// AddNode adds a node to the topology
func (n *TestNode) AddNode() error {
	resp, err := n.AgentClient.AddNode(context.Background(), n.Node)
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

//RestartNode Restart node
func (n *TestNode) RestartNode() error {

	waitForNodeUp := func(timeout time.Duration) error {
		cTimeout := time.After(time.Second * time.Duration(timeout))
		for {
			conn, _ := net.DialTimeout("tcp", net.JoinHostPort(n.Node.GetIpAddress(), "22"), 1)
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

	runner := runner.NewRunner(n.SSHCfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)
	command := fmt.Sprintf("sudo shutdown -r now")

	runner.Run(addr, command, constants.RunCommandForeground)

	time.Sleep(3 * time.Second)

	return waitForNodeUp(restartTimeout)
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
	resp, err = n.AgentClient.ReloadNode(context.Background(), n.Node)
	n.RespNode = resp
	log.Infof("TOPO SVC | ReloadNode | ReloadNode Agent . Received Response Msg: %v", resp)
	if err != nil {
		log.Errorf("Reload node %v failed. Err: %v", n.Node.Name, err)
		return err
	}
	return nil
}
