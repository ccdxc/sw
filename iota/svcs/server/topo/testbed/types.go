package testbed

import (
	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// TestNode wraps an iota TestNode
type TestNode struct {
	SSHCfg    *ssh.ClientConfig
	SSHClient *ssh.Client
	Node      *iota.Node
	RespNode  *iota.Node
	//Workloads   []*iota.Workload
	AgentClient  iota.IotaAgentApiClient
	TriggerInfo  []*iota.TriggerMsg
	TriggerResp  []*iota.TriggerMsg
	WorkloadInfo *iota.WorkloadMsg
	WorkloadResp *iota.WorkloadMsg
	Os           iota.TestBedNodeOs
}
