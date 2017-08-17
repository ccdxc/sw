package ops

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/log"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/grpc"
	"github.com/pensando/sw/utils/errors"
)

// NodeJoinOp contains state for node addition
type NodeJoinOp struct {
	cluster *cmd.Cluster
	node    *cmd.Node
}

// NewNodeJoinOp creates an op for node addition
func NewNodeJoinOp(node *cmd.Node) *NodeJoinOp {
	return &NodeJoinOp{
		node: node,
	}
}

// Validate is a method to validate the op
func (o *NodeJoinOp) Validate() error {
	// Check if in cluster.
	if env.MasterService == nil {
		return errors.NewBadRequest(fmt.Sprintf("Not a master to add node +%v to cluster", o.node.Name))
	}
	opt := api.ListWatchOptions{}
	cList, err := env.CfgWatcherService.APIClient().Cluster().List(context.Background(), &opt)
	if err != nil {
		return errors.NewBadRequest(fmt.Sprintf("error %v  getting Cluster properties while adding node +%v to cluster", err, o.node.Name))
	}
	if (len(cList) <= 0) || (cList[0] == nil) {
		return errors.NewBadRequest(fmt.Sprintf("Clusteris nil  while adding node +%v to cluster", o.node.Name))
	}
	o.cluster = cList[0]
	return nil
}

// Run executes the cluster creation steps.
func (o *NodeJoinOp) Run() (interface{}, error) {
	if o.node.Status != nil && o.node.Status.Status == "OK" {
		// Node is already part of cluster
		return o.node, nil
	}
	// Send prejoin request to all nodes.
	preJoinReq := &grpc.ClusterPreJoinReq{
		Name:      o.cluster.Name,
		Uuid:      o.cluster.UUID,
		VirtualIp: o.cluster.Spec.VirtualIP,
	}

	err := sendPreJoins(nil, preJoinReq, []string{o.node.Name})
	if err != nil {
		return nil, errors.NewBadRequest(err.Error())
	}

	// Send join request to all nodes.
	joinReq := &grpc.ClusterJoinReq{
		Name:       o.cluster.Name,
		Uuid:       o.cluster.UUID,
		VirtualIp:  o.cluster.Spec.VirtualIP,
		NTPServers: o.cluster.Spec.NTPServers,
	}

	err = sendJoins(nil, joinReq, []string{o.node.Name})
	if err != nil {
		return nil, errors.NewInternalError(err)
	}
	if o.node.Status == nil {
		o.node.Status = &cmd.NodeStatus{Status: "OK"}
	} else {
		o.node.Status.Status = "OK"
	}
	o.node.Status.Status = "OK"
	n, err := env.CfgWatcherService.APIClient().Node().Update(context.Background(), o.node)
	log.Infof("Wrote node %v to kvstore. err %v", *n, err)
	return n, err
}
