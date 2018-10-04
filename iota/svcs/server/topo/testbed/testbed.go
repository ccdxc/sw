package testbed

import (
	"fmt"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/iota/svcs/common/runner"
	"github.com/pensando/sw/venice/utils/log"
	"golang.org/x/crypto/ssh"
)

// InitNode initializes an iota test node. It copies over IOTA Agent binary and starts it on the remote node
func (n *Node) InitNode(c *ssh.ClientConfig) error {
	log.Infof("TOPO SVC | InitTestBed | Running init for Node: %v, IPAddress: %v", n.NodeName, n.IpAddress)
	// Copy Agent Binary to the remote node
	if err := n.CopyTo(constants.IotaAgentBinaryPath, c); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy agent binary: %v, to Node: %v, at IPAddress: %v", constants.IotaAgentBinaryPath, n.NodeName, n.IpAddress)
		return err
	}

	log.Infof("TOPO SVC | InitTestBed | Starting IOTA Agent on Node: %v, IPAddress: %v", n.NodeName, n.IpAddress)
	if err := n.StartAgent(constants.DstIotaAgentBinary, c); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to start agent binary: %v, on Node: %v, at IPAddress: %v", constants.IotaAgentBinaryPath, n.NodeName, n.IpAddress)
		return err
	}

	return nil
}

// CopyTo copies a file to the node
func (n *Node) CopyTo(file string, cfg *ssh.ClientConfig) error {
	copier := copier.NewCopier(cfg)
	addr := fmt.Sprintf("%s:%d", n.IpAddress, constants.SSHPort)

	if err := copier.Copy(addr, file, constants.DstIotaAgentBinary); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | CopyTo node %v failed, IPAddress: %v , Err: %v", n.NodeName, n.IpAddress, err)
		return fmt.Errorf("CopyTo node failed, Node: %v, IPAddress: %v , Err: %v", n.NodeName, n.IpAddress, err)
	}
	return nil
}


func (n *Node) StartAgent(command string, cfg *ssh.ClientConfig) error {
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.IpAddress, constants.SSHPort)

	if err := runner.Run(addr, command, constants.RunCommandBackground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | StartAgent on node %v failed, IPAddress: %v , Err: %v", n.NodeName, n.IpAddress, err)
		return fmt.Errorf("StartAgent on node failed. Node: %v, IPAddress: %v , Err: %v", n.NodeName, n.IpAddress, err)
	}
	return nil
}


// AllocateVLANS allocates vlans based on the switch port ID
func AllocateVLANS(switchPortID uint32) (vlans []uint32, err error) {
	for i := switchPortID; i < switchPortID+constants.VlansPerTestBed; i++ {
		vlans = append(vlans, i)
	}
	return
}

// InitSSHConfig establishes the SSH Config requrired for remote logging in of the nodes
func InitSSHConfig(user, pass string) *ssh.ClientConfig {
	return &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pass),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
}
