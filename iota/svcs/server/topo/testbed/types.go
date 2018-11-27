package testbed

import (
	iota "github.com/pensando/sw/iota/protos/gogen"
	"golang.org/x/crypto/ssh"
)

// TestNode wraps an iota TestNode
type TestNode struct {
	SSHCfg   *ssh.ClientConfig
	Node     *iota.Node
	RespNode *iota.Node
	//Workloads   []*iota.Workload
	AgentClient  iota.IotaAgentApiClient
	TriggerInfo  []*iota.TriggerMsg
	TriggerResp  []*iota.TriggerMsg
	WorkloadInfo iota.WorkloadMsg
	WorkloadResp iota.WorkloadMsg
	Os           iota.TestBedNodeOs
}
