package globals

import "time"

const (
	RunnerDirectory       = "/tmp/iota/"
	RemoteInfraDirectory  = RunnerDirectory + "/infra"
	RemoteIotaDirectory   = RunnerDirectory + "/iota-agent"
	RemoteNaplesDirectory = RunnerDirectory + "/naples"
	RemoteQemuDirectory   = RunnerDirectory + "/qemu"
	RemoteVeniceDirectory = RunnerDirectory + "/venice"
	NaplesContainerImage  = "naples-release-v1.tgz"
	NaplesVMBringUpScript = "naples_vm_bringup.py"
	VeniceContainerImage  = "venice.tgz"
	VeniceStartScript     = "INSTALL.sh"

	LogDir
	AgentExecName   = "iota-agent"
	AgentPort       = 9876
	NaplesAgentPort = 9007
	InfraLogFile    = "infra.log"

	NaplesAgentBringUpDelay = (20 * time.Second)

	IotaSvcPort = 8888
)
