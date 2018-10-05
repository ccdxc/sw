package testbed

import (
	"fmt"

	"golang.org/x/crypto/ssh"

	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/iota/svcs/common/runner"
	"github.com/pensando/sw/venice/utils/log"
)

// InitNode initializes an iota test node. It copies over IOTA Agent binary and starts it on the remote node
func (n *TestNode) InitNode(c *ssh.ClientConfig, artifacts []string) error {
	log.Infof("TOPO SVC | InitTestBed | Running init for TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	// Copy Agent Binary to the remote node
	if err := n.CopyTo(c, artifacts); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy agent binary: %v, to TestNode: %v, at IPAddress: %v", constants.IotaAgentBinaryPath, n.Node.Name, n.Node.IpAddress)
		return err
	}

	log.Infof("TOPO SVC | InitTestBed | Starting IOTA Agent on TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	sudoAgtCmd := fmt.Sprintf("sudo %s", constants.DstIotaAgentBinary)
	if err := n.StartAgent(sudoAgtCmd, c); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to start agent binary: %v, on TestNode: %v, at IPAddress: %v", constants.IotaAgentBinaryPath, n.Node.Name, n.Node.IpAddress)
		return err
	}

	return nil
}

// CopyTo copies a file to the node
func (n *TestNode) CopyTo(cfg *ssh.ClientConfig, files []string) error {
	copier := copier.NewCopier(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := copier.Copy(addr, constants.DstIotaAgentDir, files); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | CopyTo node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		return fmt.Errorf("CopyTo node failed, TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
	}

	return nil
}

// StartAgent starts IOTA agent on the remote node
func (n *TestNode) StartAgent(command string, cfg *ssh.ClientConfig) error {
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := runner.Run(addr, command, constants.RunCommandBackground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | StartAgent on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		return fmt.Errorf("StartAgent on node failed. TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
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
