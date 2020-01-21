package testbed

import (
	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
	common "github.com/pensando/sw/iota/svcs/common"
)

// TestNode wraps an iota TestNode
type TestNode struct {
	SSHCfg    *ssh.ClientConfig
	SSHClient *ssh.Client
	Node      *iota.Node
	RespNode  *iota.Node
	//Workloads   []*iota.Workload
	GrpcClient   *common.GRPCClient
	AgentClient  iota.IotaAgentApiClient
	WorkloadInfo *iota.WorkloadMsg
	WorkloadResp *iota.WorkloadMsg
	Os           iota.TestBedNodeOs
	CimcUserName string
	CimcPassword string
	CimcIP       string
    ApcInfo      *iota.ApcInfo
    RestartMethod string
}
