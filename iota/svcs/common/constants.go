package common

import (
	"fmt"
	"os"
	"time"
)

// global constants
const (

	// IotaSvcPort is the default IOTA Server Port
	IotaSvcPort = 60000

	// IotaAgentPort is the default IOTA Agent Port
	IotaAgentPort = 60001

	// IotaAgentBinaryName is the name of of the IOTA Agent Binary
	IotaAgentBinaryName = "iota_agent"

	// IotaAgentBinaryNameLinux is the name of of the IOTA Agent Binary for Linux
	IotaAgentBinaryNameLinux = "linux/" + IotaAgentBinaryName

	// IotaAgentBinaryNameFreebsd is the name of of the IOTA Agent Binary for Freebsd
	IotaAgentBinaryNameFreebsd = "freebsd/" + IotaAgentBinaryName

	// VlansPerTestBed vlans that a testbed can manage
	VlansPerTestBed = 10

	// SSHPort captures the port where SSH is running
	SSHPort = 22

	// DstIotaAgentDir captures the top level dir where all the agent copies happen
	DstIotaAgentDir = "/var/iota"

	// DstIotaEntitiesDir has all workload related data for each workload
	DstIotaEntitiesDir = DstIotaAgentDir + "/entities"

	// DstIotaDBDir has persistance for db.
	DstIotaDBDir = DstIotaAgentDir + "/db"

	//MakeClusterTimeout waits for 5 minutes for the cluster to be up
	MakeClusterTimeout = time.Duration(time.Minute * 5)

	//WorkloadsPerNode captures the number of endpoints per node.
	WorkloadsPerNode = 4

	// IOTARandomSeed captures the fixed seed that IOTA uses to generate IP Address and MAC Addresses
	IOTARandomSeed = 42

	// IotaWorkloadImage is the docker image that will be used for workloads
	IotaWorkloadImage = "registry.test.pensando.io:5000/pensando/iota/centos:1.0"
)

// incrementing constants. List all constants whose value you don't care here
const (
	// RunCommandForeground runs a command in the foreground
	RunCommandForeground = iota

	// RunCommandBackground runs a command in the background
	RunCommandBackground
)

// global derived vars from the constants
var (
	// IotaAgentBinaryPathLinux captures the location of the build IOTA Agent Binary for Linux
	IotaAgentBinaryPathLinux = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/bin/agent/%s", os.Getenv("GOPATH"), IotaAgentBinaryNameLinux)

	// IotaAgentBinaryPathFreebsd captures the location of the build IOTA Agent Binary for Freebsd
	IotaAgentBinaryPathFreebsd = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/bin/agent/%s", os.Getenv("GOPATH"), IotaAgentBinaryNameFreebsd)

	// DstIotaAgentBinary captures the location of agent on the remote nodes
	DstIotaAgentBinary = fmt.Sprintf("%s/%s", DstIotaAgentDir, IotaAgentBinaryName)

	// CleanupCommands lists the clean up commands required to clean up an IOTA node.
	CleanupCommands = []string{
		`sudo /var/iota/INSTALL.sh --clean-only`,
		`sudo systemctl stop pen-cmd`,
		`sudo docker rm -fv $(docker ps -aq)`,
		`sudo docker system prune -f`,
		`sudo rm /etc/hosts`,
		`sudo pkill iota*`,
		`sudo rm -rf /var/iota*`,
		`sudo docker ps`,
		`sudo docker rmi -f \$(docker images -aq)`,
		`sudo rm -rf /var/run/naples`,
		`sudo iptables -F`,
		`sudo systemctl restart docker`,
	}
)
