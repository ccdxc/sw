package ops

import (
	"context"
	"fmt"
	"path"

	log "github.com/Sirupsen/logrus"
	"github.com/pborman/uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/grpc"
	"github.com/pensando/sw/cmd/utils"
	"github.com/pensando/sw/cmd/validation"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/errors"
)

const (
	defaultNTPServer = "pool.ntp.org"
)

// clusterCreateOp contains state for creating a cluster.
type clusterCreateOp struct {
	cluster *api.Cluster
}

// NewClusterCreateOp creates an op for creating a cluster.
func NewClusterCreateOp(cluster *api.Cluster) Op {
	return &clusterCreateOp{
		cluster: cluster,
	}
}

// Validate is a method to validate the cluster object.
func (o *clusterCreateOp) Validate() error {
	// Check if in cluster.
	if cluster, err := utils.GetCluster(); err != nil {
		return errors.NewInternalError(err)
	} else if cluster != nil {
		return errors.NewBadRequest(fmt.Sprintf("Already part of cluster +%v", cluster))
	}

	// Validate arguments.
	if errs := validation.ValidateCluster(o.cluster); len(errs) != 0 {
		return errors.NewInvalid("cluster", "", errs)
	}
	return nil
}

// populateClusterDefaults fills in the defaults for cluster object.
func (o *clusterCreateOp) populateClusterDefaults() {
	o.cluster.Kind = "Cluster"
	o.cluster.UUID = uuid.New()
	if len(o.cluster.Spec.NTPServers) == 0 {
		o.cluster.Spec.NTPServers = append(o.cluster.Spec.NTPServers, defaultNTPServer)
	}
}

// Run executes the cluster creation steps.
func (o *clusterCreateOp) Run() (interface{}, error) {
	// Populate defaults (UUID, NTP Servers etc)
	o.populateClusterDefaults()

	// Generate etcd quorum configuration.
	quorumConfig, err := makeQuorumConfig(o.cluster.UUID, o.cluster.Spec.QuorumNodes)
	if err != nil {
		return nil, errors.NewInternalError(err)
	}

	// Send prejoin request to all nodes.
	preJoinReq := &grpc.ClusterPreJoinReq{
		Name:      o.cluster.Name,
		Uuid:      o.cluster.UUID,
		VirtualIp: o.cluster.Spec.VirtualIP.String(),
	}

	err = sendPreJoins(nil, preJoinReq, o.cluster.Spec.QuorumNodes)
	if err != nil {
		return nil, errors.NewBadRequest(err.Error())
	}

	// Send join request to all nodes.
	joinReq := &grpc.ClusterJoinReq{
		Name:         o.cluster.Name,
		Uuid:         o.cluster.UUID,
		VirtualIp:    o.cluster.Spec.VirtualIP.String(),
		QuorumConfig: quorumConfig,
	}

	err = sendJoins(nil, joinReq, o.cluster.Spec.QuorumNodes)
	if err != nil {
		return nil, errors.NewInternalError(err)
	}

	// Store Cluster and Node objects in kv store.
	// FIXME: Do this in a transaction when kv store supports transactions.
	err = env.KVStore.Create(context.Background(), globals.ClusterKey, o.cluster, 0, o.cluster)
	if err != nil {
		log.Errorf("Failed to add cluster to kvstore, error: %v", err)
		sendDisjoins(nil, o.cluster.Spec.QuorumNodes)
		return nil, errors.NewInternalError(err)
	}

	for ii := range o.cluster.Spec.QuorumNodes {
		name := o.cluster.Spec.QuorumNodes[ii]
		node := makeNode(name)
		err = env.KVStore.Create(context.Background(), path.Join(globals.NodesKey, name), node, 0, node)
		if err != nil {
			// FIXME: With txn, error handling will be merged with above block.
			log.Errorf("Failed to add node %v to kvstore, error: %v", name, err)
			return nil, errors.NewInternalError(err)
		}
	}

	return o.cluster, nil
}
