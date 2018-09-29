package main

import (
	"context"

	pb "github.com/pensando/sw/iota/protos/gogen"
)

type iotaAgent struct {
}

// AddNode brings up the node with the personality
func (*iotaAgent) AddNode(context.Context, *pb.IotaNode) (*pb.IotaNode, error) {
	return nil, nil
}

// DeleteNode, remove the personaltiy set
func (*iotaAgent) DeleteNode(context.Context, *pb.Node) (*pb.Node, error) {

	return nil, nil
}

// AddWorkloads brings up a workload type on a given node
func (*iotaAgent) AddWorkload(context.Context, *pb.Workload) (*pb.Workload, error) {

	return nil, nil
}

// DeleteWorkloads deletes a given workload
func (*iotaAgent) DeleteWorkload(context.Context, *pb.Workload) (*pb.Workload, error) {
	return nil, nil

}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (*iotaAgent) Trigger(context.Context, *pb.TriggerMsg) (*pb.TriggerMsg, error) {
	return nil, nil

}

// CheckClusterHealth returns the cluster health
func (*iotaAgent) CheckHealth(context.Context, *pb.NodeHealth) (*pb.NodeHealth, error) {
	return nil, nil

}

func newiotaAgent() *iotaAgent {
	return &iotaAgent{}
}
