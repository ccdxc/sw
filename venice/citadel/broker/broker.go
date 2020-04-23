// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker

// this file contains the broker code that distributes data and query to data nodes

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc/connectivity"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Inf defines the broker interface for generating mocks
type Inf interface {
	GetCluster(clusterType string) *meta.TscaleCluster
	IsStopped() bool
	Stop() error

	// Methods defined in broker_ts
	CreateDatabase(ctx context.Context, database string) error
	CreateDatabaseWithRetention(ctx context.Context, database string, retention uint64) error
	DeleteDatabase(ctx context.Context, database string) error
	WritePoints(ctx context.Context, database string, points []models.Point) error
	ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error)
	WriteLines(ctx context.Context, database string, lines []string) error

	// Methods defined in broker_kv
	ClusterCheck() error
	WriteKvs(ctx context.Context, table string, kvs []*tproto.KeyValue) error
	ReadKvs(ctx context.Context, table string, keys []*tproto.Key) ([]*tproto.KeyValue, error)
	ListKvs(ctx context.Context, table string) ([]*tproto.KeyValue, error)
	DeleteKvs(ctx context.Context, table string, keys []*tproto.Key) error
}

// Broker is the api broker that distributes writes and queries to data nodes
type Broker struct {
	sync.Mutex
	nodeUUID    string
	metaWatcher *meta.Watcher // metadata watcher
	rpcClients  map[string]*rpckit.RPCClient
	logger      log.Logger

	cqCtx                context.Context
	cqCtxCancelFunc      context.CancelFunc
	cqRoutineCreated     bool
	metricsWithCQCreated map[string]bool   // original metrics name : true
	cqInfoMap            map[string]string // cq name : query string
}

// broker retry constants
const (
	numBrokerRetries = 3
	brokerRetryDelay = time.Millisecond * 10
)

// NewBroker creates a new broker instance
func NewBroker(cfg *meta.ClusterConfig, nodeUUID string, logger log.Logger) (*Broker, error) {
	// start the metadata watcher
	metaWatcher, err := meta.NewWatcher(nodeUUID, cfg)
	if err != nil {
		logger.Errorf("Error creating the watcher. Err: %v", err)
		return nil, err
	}

	// TODO: enable by default in build
	models.EnableUintSupport()

	// create the broker instance
	broker := Broker{
		nodeUUID:    nodeUUID,
		metaWatcher: metaWatcher,
		rpcClients:  make(map[string]*rpckit.RPCClient),
		logger:      logger.WithContext("brokeruuid", nodeUUID),
	}

	// continuous query
	broker.metricsWithCQCreated = make(map[string]bool)
	broker.cqInfoMap = make(map[string]string)
	broker.cqCtx, broker.cqCtxCancelFunc = context.WithCancel(context.Background())
	go broker.continuousQueryWaitDB()

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

	// cancel continuous query context
	br.cqCtxCancelFunc()

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
	if ok && rclient.ClientConn != nil && rclient.ClientConn.GetState() == connectivity.Ready {
		return rclient.ClientConn, nil
	}

	// get node info
	cl := br.GetCluster(clusterType)
	if cl == nil {
		br.logger.Errorf("Could not find the cluster while looking up node %s.", nodeUUID)
		return nil, errors.New("Cluster not found")
	}
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		br.logger.Errorf("Could not find the node %s. Err: %v", nodeUUID, err)
		return nil, err
	}

	// dial the connection
	rclient, err = rpckit.NewRPCClient(fmt.Sprintf("broker-%s", br.nodeUUID), node.NodeURL, rpckit.WithLoggerEnabled(false), rpckit.WithRemoteServerName(globals.Citadel))
	if err != nil {
		br.logger.Errorf("Error connecting to rpc server %s. err: %v", node.NodeURL, err)
		return nil, err
	}

	// save the rpc client for future use
	br.rpcClients[node.NodeUUID] = rclient

	return rclient.ClientConn, nil
}

// DeleteRPCClient removes rpc client for a node
func (br *Broker) DeleteRPCClient(nodeUUID string) {
	br.Lock()
	delete(br.rpcClients, nodeUUID)
	br.Unlock()
}
