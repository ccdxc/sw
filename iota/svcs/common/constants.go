package common

import (
	"fmt"
	"os"
)

// global constants
const (

	// IotaSvcPort is the default IOTA Server Port
	IotaSvcPort = 60000

	// IotaAgentPort is the default IOTA Agent Port
	IotaAgentPort = 60001

	// IotaAgentBinaryName is the name of of the IOTA Agent Binary
	IotaAgentBinaryName = "iota_agent"

	// VlansPerTestBed vlans that a testbed can manage
	VlansPerTestBed = 10

	// SSHPort captures the port where SSH is running
	SSHPort = 22

	// DstIotaAgentDir captures the top level dir where all the agent copies happen
	DstIotaAgentDir = "/tmp/iota"
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
	// IotaAgentBinaryPath captures the location of the build IOTA Agent Binary
	IotaAgentBinaryPath = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/bin/agent/%s", os.Getenv("GOPATH"), IotaAgentBinaryName)

	// DstIotaAgentBinary captures the location of agent on the remote nodes
	DstIotaAgentBinary = fmt.Sprintf("%s/%s", DstIotaAgentDir, IotaAgentBinaryName)
)
