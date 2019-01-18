package testbed

import (
	"context"
	"fmt"
	"strings"
	"time"

	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/iota/svcs/common/runner"
	vmware "github.com/pensando/sw/iota/svcs/common/vmware"
	"github.com/pensando/sw/venice/utils/log"
)

func (n *TestNode) cleanupEsxNode() error {

	host, err := vmware.NewHost(context.Background(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
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

	/* Delete all invalid VMS too */
	pass := n.Node.EsxConfig.Password
	cfg := &ssh.ClientConfig{
		User: n.Node.EsxConfig.Username,
		Auth: []ssh.AuthMethod{
			ssh.Password(n.Node.EsxConfig.Password),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = pass
				}

				return answers, nil
			}),
		}, HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.EsxConfig.IpAddress, constants.SSHPort)
	cmd := `vim-cmd /vmsvc/getallvm  2>&1 >/dev/null  | cut -d' ' -f 4`
	output, err := runner.RunWithOutput(addr, cmd, constants.RunCommandForeground)
	log.Infof("Invalid VMS out %v", output)
	if err != nil {
		log.Errorf("TOPO SVC | CleanTestBed | Failed to find invalid VMs %v", err.Error())
		return err
	}
	for _, vmID := range strings.Split(output, "\n") {
		cmd = "vim-cmd /vmsvc/unregister " + strings.Replace(vmID, "'", "", -1)
		log.Infof("Running Delete Invalid Command : %v", cmd)
		runner.Run(addr, cmd, constants.RunCommandForeground)
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

func (n *TestNode) initEsxNode() error {
	var err error

	ctrlVMDir := constants.ControlVMImageDirectory + "/" + constants.EsxControlVMImage

	host, err := vmware.NewHost(context.Background(), n.Node.EsxConfig.IpAddress, n.Node.EsxConfig.Username, n.Node.EsxConfig.Password)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Esx node, failed to get host handle  %v", err.Error())
		return err
	}

	err = host.DestoryVM(constants.EsxControlVMName)
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Delete control node failed %v", err.Error())
	}

	vsname := constants.EsxIotaCtrlSwitch
	vsspec := vmware.VswitchSpec{Name: vsname}
	host.AddVswitch(vsspec)

	nws := []vmware.NWSpec{{Name: constants.EsxDefaultNetwork, Vlan: int32(constants.EsxDefaultNetworkVlan)}, {Name: constants.EsxVMNetwork, Vlan: int32(constants.EsxVMNetworkVlan)}}
	host.AddNetworks(nws, vsspec)

	vmInfo, err := host.DeployVM(constants.EsxControlVMName, constants.EsxControlVMCpus, constants.EsxControlVMMemory, constants.EsxControlVMNetworks, ctrlVMDir)

	if err != nil {

		log.Errorf("TOPO SVC | InitTestBed | Add control node failed %v", err.Error())
		return err
	}

	log.Println("Init ESX node complete with IP : %s", vmInfo.IP)
	n.Node.IpAddress = vmInfo.IP

	return n.waitForNodeUp(restartTimeout)
}

// InitNode initializes an iota test node. It copies over IOTA Agent binary and starts it on the remote node
func (n *TestNode) InitNode(c *ssh.ClientConfig, dstDir string, commonArtifacts []string) error {
	var agentBinary string

	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		if err := n.cleanupEsxNode(); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Clean up ESX node failed :  %v", err.Error())
			return err
		}
		if err := n.initEsxNode(); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Init ESX node failed :  %v", err.Error())
			return err
		}
	}

	runner := runner.NewRunner(c)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := runner.Run(addr, "sudo rm -rf /tmp/iota", constants.RunCommandForeground); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | StartAgent on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		return fmt.Errorf("StartAgent on node failed. TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
	}

	// Copy Common Artifacts to the remote node
	if err := n.CopyTo(c, dstDir, commonArtifacts); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy common artifacts, to TestNode: %v, at IPAddress: %v", n.Node.Name, n.Node.IpAddress)
		return err
	}

	// Copy Agent Binary to the remote node
	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD {
		agentBinary = constants.IotaAgentBinaryPathFreebsd
	} else {
		agentBinary = constants.IotaAgentBinaryPathLinux
	}

	log.Infof("TOPO SVC | InitTestBed | Running init for TestNode: %v, IPAddress: %v AgentBinary: %v", n.Node.Name, n.Node.IpAddress, agentBinary)
	if err := n.CopyTo(c, dstDir, []string{agentBinary}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to copy agent binary: %v, to TestNode: %v, at IPAddress: %v", agentBinary, n.Node.Name, n.Node.IpAddress)
		return err
	}

	//Copy Nic configuration
	log.Infof("TOPO SVC | InitTestBed | Running init for TestNode: %v, IPAddress: %v Nic Finder Conf: %v", n.Node.Name, n.Node.IpAddress, constants.NicFinderConf)
	if err := n.CopyTo(c, dstDir, []string{constants.NicFinderConf}); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to Nic conf file: %v, to TestNode: %v, at IPAddress: %v", constants.NicFinderConf, n.Node.Name, n.Node.IpAddress)
		return err
	}

	log.Infof("TOPO SVC | InitTestBed | Starting IOTA Agent on TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	sudoAgtCmd := fmt.Sprintf("sudo %s", constants.DstIotaAgentBinary)
	if err := n.StartAgent(sudoAgtCmd, c); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Failed to start agent binary: %v, on TestNode: %v, at IPAddress: %v", agentBinary, n.Node.Name, n.Node.IpAddress)
		return err
	}

	return nil
}

// CleanUpNode cleans up the node
func (n *TestNode) CleanUpNode(cfg *ssh.ClientConfig) error {
	log.Infof("TOPO SVC | CleanupNode | Running clean up for TestNode: %v, IPAddress: %v", n.Node.Name, n.Node.IpAddress)
	runner := runner.NewRunner(cfg)
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if n.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
		if err := n.cleanupEsxNode(); err != nil {
			log.Errorf("TOPO SVC | CleanUpNode | Clean up  ESX node failed :  %v", err.Error())
			return err
		}
	} else {
		// Dont enforce error handling for clean up path
		for _, cmd := range constants.CleanupCommands {
			err := runner.Run(addr, cmd, constants.RunCommandForeground)
			log.Errorf("TOPO SVC | CleanUpNode | Clean up on node %v failed, IPAddress: %v , Err: %v", n.Node.Name, n.Node.IpAddress, err)
		}
	}

	return nil
}

// CopyTo copies a file to the node
func (n *TestNode) CopyTo(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	var ip string
	var err error
	var copyHandle *copier.Copier

	if ip, err = n.GetNodeIP(); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed to get node IP")
		return fmt.Errorf("TOPO SVC | CopyTo node failed to get node IP")
	}
	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient)
	}
	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := copyHandle.CopyTo(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | CopyTo node %v failed, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
		return fmt.Errorf("CopyTo node failed, TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
	}

	return nil
}

// CopyFrom copies a file to the node
func (n *TestNode) CopyFrom(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	//copier := copier.NewCopier(cfg)
	var ip string
	var err error
	var copyHandle *copier.Copier
	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient)
	}

	if ip, err = n.GetNodeIP(); err != nil {
		log.Errorf("TOPO SVC | CopyTo node failed to get node IP")
		return fmt.Errorf("TOPO SVC | CopyTo node failed to get node IP")
	}

	addr := fmt.Sprintf("%s:%d", n.Node.IpAddress, constants.SSHPort)

	if err := copyHandle.CopyFrom(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | CopyFrom node %v failed, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
		return fmt.Errorf("CopyFrom node failed, TestNode: %v, IPAddress: %v , Err: %v", n.Node.Name, ip, err)
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

// InitSSHConfig establishes the SSH Config required for remote logging in of the nodes
func InitSSHConfig(user, pass string) *ssh.ClientConfig {
	return &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pass),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = pass
				}
				return answers, nil
			}),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
		Timeout:         time.Second * 30,
	}
}
