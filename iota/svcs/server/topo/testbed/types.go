package testbed

import (
	iota "github.com/pensando/sw/iota/protos/gogen"
)

// TestNode wraps an iota TestNode
type TestNode struct {
	Node *iota.Node
	//Workloads   []*iota.Workload
	AgentClient iota.IotaAgentApiClient
	TriggerInfo *iota.TriggerMsg
}
