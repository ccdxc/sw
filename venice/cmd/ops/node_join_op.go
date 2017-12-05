package ops

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/errors"
)

func hasRole(nodeSpec *cmd.NodeSpec, role cmd.NodeSpec_NodeRole) bool {
	if nodeSpec != nil && nodeSpec.Roles != nil {
		for _, r := range nodeSpec.Roles {
			if role.String() == r {
				return true
			}
		}
	}
	return false
}

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
	if o.node.Status.Phase == cmd.NodeStatus_JOINED.String() {
		// Node is already part of cluster
		return o.node, nil
	}

	// Transport key is an asymmetric key that allows multiple CMD instances to securely agree on a
	// symmetric key that can be used to transport secrets across CMD instances.
	var transportKeyBytes []byte
	if hasRole(&o.node.Spec, cmd.NodeSpec_QUORUM) || hasRole(&o.node.Spec, cmd.NodeSpec_CONTROLLER) {
		if !env.CertMgr.IsReady() {
			return nil, errors.NewInternalError(fmt.Errorf("CertMgr not ready"))
		}
		transportKey, err := env.CertMgr.GetKeyAgreementKey(o.node.Name)
		defer env.CertMgr.DestroyKeyAgreementKey(o.node.Name)
		if err != nil {
			return nil, errors.NewInternalError(fmt.Errorf("Error getting key-agreement-key: %v", err))
		}
		transportKeyBytes = env.CertMgr.MarshalKeyAgreementKey(transportKey)
	}

	// Send prejoin request to the new node.
	preJoinReq := &grpc.ClusterPreJoinReq{
		Name:         o.cluster.Name,
		Uuid:         o.cluster.UUID,
		VirtualIp:    o.cluster.Spec.VirtualIP,
		TransportKey: transportKeyBytes,
	}

	nodeTransportKeys := make(map[string][]byte)
	err := sendPreJoins(nil, preJoinReq, []string{o.node.Name}, nodeTransportKeys)
	if err != nil {
		return nil, errors.NewBadRequest(err.Error())
	}

	// Send join request to all nodes.
	joinReq := &grpc.ClusterJoinReq{
		Name:       o.cluster.Name,
		Uuid:       o.cluster.UUID,
		VirtualIp:  o.cluster.Spec.VirtualIP,
		NTPServers: o.cluster.Spec.NTPServers,
		NodeId:     o.node.Name,
	}

	err = sendJoins(nil, joinReq, []string{o.node.Name}, nodeTransportKeys)
	if err != nil {
		return nil, errors.NewInternalError(err)
	}
	o.node.Status.Phase = cmd.NodeStatus_JOINED.String()
	n, err := env.CfgWatcherService.APIClient().Node().Update(context.Background(), o.node)
	log.Infof("Wrote node %v to kvstore. err %v", *n, err)
	return n, err
}
