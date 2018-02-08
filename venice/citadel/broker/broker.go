// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker

// this file contains the broker code that distributes data and query to data nodes

import (
	"errors"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Broker is the api broker that distributes writes and queries to data nodes
type Broker struct {
	sync.Mutex
	nodeUUID    string
	metaWatcher *meta.Watcher // metadata watcher
	rpcClients  map[string]*rpckit.RPCClient
}

// broker retry constants
const (
	numBrokerRetries = 3
	brokerRetryDelay = time.Millisecond * 10
)

// NewBroker creates a new broker instance
func NewBroker(cfg *meta.ClusterConfig, nodeUUID string) (*Broker, error) {
	// start the metadata watcher
	metaWatcher, err := meta.NewWatcher(nodeUUID, cfg)
	if err != nil {
		log.Errorf("Error creating the watcher. Err: %v", err)
		return nil, err
	}

	// create the broker instance
	broker := Broker{
		nodeUUID:    nodeUUID,
		metaWatcher: metaWatcher,
		rpcClients:  make(map[string]*rpckit.RPCClient),
	}

	return &broker, nil
}

// IsStopped returns true if broker is stopped
func (br *Broker) IsStopped() bool {
	if br.metaWatcher == nil {
		return true
	}

	return false
}

// Stop stops the broker
func (br *Broker) Stop() error {
	if br.metaWatcher != nil {
		br.metaWatcher.Stop()
		br.metaWatcher = nil
	}

	// close the rpc clients
	for idx, rc := range br.rpcClients {
		rc.Close()
		delete(br.rpcClients, idx)
	}

	return nil
}

// GetCluster returns current state of the cluster
// used for debug purposes
func (br *Broker) GetCluster(clusterType string) *meta.TscaleCluster {
	// some error checking
	if br.metaWatcher == nil {
		return nil
	}

	return br.metaWatcher.GetCluster(clusterType)
}

// getRPCClient returns the rpc client for a node
func (br *Broker) getRPCClient(nodeUUID, clusterType string) (*grpc.ClientConn, error) {
	br.Lock()
	defer br.Unlock()

	// see if we already have an rpc client for this node
	rclient, ok := br.rpcClients[nodeUUID]
	if ok {
		return rclient.ClientConn, nil
	}

	// get node info
	cl := br.GetCluster(clusterType)
	if cl == nil {
		log.Errorf("Could not find the cluster while looking up node %s.", nodeUUID)
		return nil, errors.New("Cluster not found")
	}
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		log.Errorf("Could not find the node %s. Err: %v", nodeUUID, err)
		return nil, err
	}

	// dial the connection
	rclient, err = rpckit.NewRPCClient(fmt.Sprintf("broker-%s", br.nodeUUID), node.NodeURL, rpckit.WithLoggerEnabled(false), rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Error connecting to rpc server %s. err: %v", node.NodeURL, err)
		return nil, err
	}

	// save the rpc client for future use
	br.rpcClients[node.NodeUUID] = rclient

	return rclient.ClientConn, nil
}
