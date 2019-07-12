package ops

import (
	"fmt"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
)

// NodeDisjoinOp contains state for Op for node leaving the cluster
type NodeDisjoinOp struct {
	cluster *cmd.Cluster
	node    *cmd.Node
}

// NewNodeDisjoinOp creates an op for a node leaving a cluster.
func NewNodeDisjoinOp(node *cmd.Node, cluster *cmd.Cluster) *NodeDisjoinOp {
	return &NodeDisjoinOp{
		cluster: cluster,
		node:    node,
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
	// If this was a quorum node, we need to remove it from quorum member list in etcd
	quorumMembers, err := env.Quorum.List()
	if err != nil {
		werr := fmt.Errorf("Error listing quorum members: %v", err)
		log.Errorf(werr.Error())
		return nil, errors.NewInternalError(werr)
	}
	quorumSize := len(quorumMembers)
	for _, m := range quorumMembers {
		if m.Name == o.node.Name {
			err := env.Quorum.Remove(m.ID)
			if err != nil {
				werr := fmt.Errorf("Error removing quorum member %+v: %v", m, err)
				log.Errorf(werr.Error())
				return nil, errors.NewInternalError(werr)
			}
			break
		}
	}
	log.Infof("Removed quorum member %+v", o.node.Name)
	recorder.Event(eventtypes.QUORUM_MEMBER_REMOVE, fmt.Sprintf("Member %s removed from quorum", o.node.Name), o.cluster)
	quorumSize--
	if quorumSize < globals.MinSupportedQuorumSize {
		errStr := fmt.Sprintf("Quorum size %d is below minimum supported %d", quorumSize, globals.MinSupportedQuorumSize)
		log.Errorf(errStr)
		recorder.Event(eventtypes.UNSUPPORTED_QUORUM_SIZE, errStr, o.cluster)
	}

	_, err = sendDisjoins(nil, []string{o.node.Name})
	if err != nil {
		log.Errorf("error %v sending disjoins to node %v", err, o.node.Name)
	}
	err2 := env.K8sService.DeleteNode(o.node.Name)
	log.Infof("node %v disjoined from cluster. err %v", o.node.Name, err2)
	if err != nil || err2 != nil {
		recorder.Event(eventtypes.NODE_DISJOINED, fmt.Sprintf("Node %s disjoined from cluster", o.node.Name), o.cluster)
	}
	return o.node.Name, err
}
