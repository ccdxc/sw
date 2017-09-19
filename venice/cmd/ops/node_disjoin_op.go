package ops

import (
	"fmt"

	"github.com/pensando/sw/venice/utils/log"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/utils/errors"
)

// NodeDisjoinOp contains state for Op for node leaving the cluster
type NodeDisjoinOp struct {
	cluster *cmd.Cluster
	node    *cmd.Node
}

// NewNodeDisjoinOp creates an op for a node leaving a cluster.
func NewNodeDisjoinOp(node *cmd.Node) *NodeDisjoinOp {
	return &NodeDisjoinOp{
		node: node,
	}
}

// Validate is a method to validate the op
func (o *NodeDisjoinOp) Validate() error {
	// Check if in cluster.
	if env.MasterService == nil {
		return errors.NewBadRequest(fmt.Sprintf("Not a master to delete node +%v from cluster", o.node.Name))
	}
	return nil
}

// Run executes the cluster leaving steps.
func (o *NodeDisjoinOp) Run() (interface{}, error) {
	_, err := sendDisjoins(nil, []string{o.node.Name})
	if err != nil {
		return nil, errors.NewInternalError(err)
	}
	log.Infof("node %v disjoin from cluster. err %v", o.node.Name, err)
	return o.node.Name, err
}
