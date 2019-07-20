package ops

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
)

// NodeJoinOp contains state for node addition
type NodeJoinOp struct {
	cluster *cmd.Cluster
	node    *cmd.Node
}

// NewNodeJoinOp creates an op for node addition
func NewNodeJoinOp(node *cmd.Node) Op {
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

// isQuorumNode returns true if the node name is in the list of quorum nodes
func isQuorumNode(nodeName string) bool {
	for _, qn := range env.QuorumNodes {
		if qn == nodeName {
			return true
		}
	}
	return false
}

// Run executes the cluster creation steps.
func (o *NodeJoinOp) Run() (interface{}, error) {
	if o.node.Status.Phase == cmd.NodeStatus_JOINED.String() {
		// Node is already part of cluster
		return o.node, nil
	}

	if env.Quorum == nil {
		return nil, fmt.Errorf("Quorum is not available")
	}

	var quorumConfig *grpc.QuorumConfig
	var err error

	if isQuorumNode(o.node.Name) {
		// Generate etcd quorum configuration.
		quorumConfig, err = makeQuorumConfig(o.cluster.UUID, o.cluster.Spec.QuorumNodes, true)
		if err != nil {
			return nil, errors.NewInternalError(err)
		}
		o.node.Status.Quorum = true
	}

	// Transport key is an asymmetric key that allows multiple CMD instances to securely agree on a
	// symmetric key that can be used to transport secrets across CMD instances.
	var transportKeyBytes []byte
	if env.CertMgr == nil || !env.CertMgr.IsReady() {
		return nil, errors.NewInternalError(fmt.Errorf("CertMgr not ready"))
	}
	transportKey, err := env.CertMgr.GetKeyAgreementKey(o.node.Name)
	defer env.CertMgr.DestroyKeyAgreementKey(o.node.Name)
	if err != nil {
		return nil, errors.NewInternalError(fmt.Errorf("Error getting key-agreement-key: %v", err))
	}
	transportKeyBytes = env.CertMgr.MarshalKeyAgreementKey(transportKey)

	// Send prejoin request to the new node.
	preJoinReq := &grpc.ClusterPreJoinReq{
		Name:         o.cluster.Name,
		Uuid:         o.cluster.UUID,
		VirtualIp:    o.cluster.Spec.VirtualIP,
		TransportKey: transportKeyBytes,
		NtpServers:   o.cluster.Spec.NTPServers,
	}

	nodeTransportKeys := make(map[string][]byte)
	err = sendPreJoins(nil, preJoinReq, []string{o.node.Name}, nodeTransportKeys)
	if err != nil {
		return nil, errors.NewBadRequest(err.Error())
	}
	log.Infof("Sent PreJoin message to node %s", o.node.Name)

	if quorumConfig != nil {
		// Add the node to the quorum
		for _, mcfg := range quorumConfig.QuorumMembers {
			if mcfg.Name == o.node.Name {
				member := &quorum.Member{
					Name:       o.node.Name,
					PeerURLs:   mcfg.PeerUrls,
					ClientURLs: mcfg.ClientUrls,
				}
				err = env.Quorum.Add(member)
				if err != nil {
					werr := fmt.Errorf("Error adding quorum members: %v", err)
					log.Errorf(werr.Error())
					return nil, errors.NewInternalError(werr)
				}
				log.Infof("Added quorum member %+v", member)
				recorder.Event(eventtypes.QUORUM_MEMBER_ADD, fmt.Sprintf("Member %s added to quorum, ID: %x", member.Name, member.ID), o.cluster)
			}
		}

		// Check if we have enough members after the addition
		quorumMembers, err := env.Quorum.List()
		if err != nil {
			log.Errorf("Error listing quorum members: %v", err)
		}
		if len(quorumMembers) < globals.MinSupportedQuorumSize {
			errStr := fmt.Sprintf("Quorum size %d is below minimum supported %d", len(quorumMembers), globals.MinSupportedQuorumSize)
			log.Errorf(errStr)
			recorder.Event(eventtypes.UNSUPPORTED_QUORUM_SIZE, errStr, o.cluster)
		}
	}

	// Send join request to all nodes.
	joinReq := &grpc.ClusterJoinReq{
		Name:         o.cluster.Name,
		Uuid:         o.cluster.UUID,
		VirtualIp:    o.cluster.Spec.VirtualIP,
		NTPServers:   o.cluster.Spec.NTPServers,
		QuorumNodes:  o.cluster.Spec.QuorumNodes,
		QuorumConfig: quorumConfig,
		NodeId:       o.node.Name,
	}
	err = sendJoins(nil, joinReq, []string{o.node.Name}, nodeTransportKeys)
	if err != nil {
		return nil, errors.NewInternalError(err)
	}
	log.Infof("Sent Join message to node %s", o.node.Name)

	o.node.Status.Phase = cmd.NodeStatus_JOINED.String()
	recorder.Event(eventtypes.NODE_JOINED, fmt.Sprintf("Node %s joined cluster %s", o.node.Name, o.cluster.Name), o.node)

	err = env.StateMgr.UpdateNode(o.node, true)
	if err != nil {
		werr := fmt.Errorf("Error updating Node object %s: %v", o.node.Name, err)
		log.Errorf(werr.Error())
		return nil, errors.NewInternalError(err)
	}

	return o.node, err
}
