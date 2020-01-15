// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

import (
	"context"
	"crypto/tls"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// ReplicaState is the fsm state of the replica
type ReplicaState string

// NodeStatusType is enum of node states
type NodeStatusType string

// ClusterType is tstore or kstore
type ClusterType string

// fsm state of the shard
const (
	ReplicaStateInit        ReplicaState = "Init"
	ReplicaStateCandidate   ReplicaState = "Candidate"
	ReplicaStateSyncing     ReplicaState = "Syncing"
	ReplicaStateReady       ReplicaState = "Ready"
	ReplicaStateUnreachable ReplicaState = "Unreachable"
	ReplicaStateRemoving    ReplicaState = "Removing"
)

// node fsm states
const (
	NodeStatusAlive       NodeStatusType = "Alive"
	NodeStatusUnreachable NodeStatusType = "Unreachable"
	NodeStatusDead        NodeStatusType = "Dead"
)

// cluster types
const (
	ClusterTypeTstore = "tstore"
	ClusterTypeKstore = "kstore"
)

// Replica provides information about a replica
type Replica struct {
	ReplicaID uint64       // Unique id for the replica
	ShardID   uint64       // shard identifier
	NodeUUID  string       // Node id
	IsPrimary bool         // is this the primary replica for the shard?
	State     ReplicaState // current state of the replica
	shard     *Shard       // shard this replica belongs to
}

// Shard contains info about a shard
type Shard struct {
	ShardID        uint64              // shard  identifier
	NumReplicas    uint32              // number of replicas
	PrimaryReplica uint64              // id of primary replica
	Replicas       map[uint64]*Replica // list of replicas
	smap           *ShardMap           // shard map this belongs to
}

// ShardMap keeps mapping of keys to shards
type ShardMap struct {
	NumShards       uint32         // number of shards (Can not change this after cluster creation)
	DesiredReplicas uint32         // desired number of replicas
	LastReplicaID   uint64         // running counter of replica ids
	LastShardID     uint64         // running counter of shard ids
	Shards          []*Shard       // shards in this map
	cluster         *TscaleCluster // cluster holding this shard map
}

// NodeState has shard information per node
type NodeState struct {
	NodeUUID           string            // node id
	NodeURL            string            // URL to reach the node
	NodeStatus         NodeStatusType    // current state of the node
	AliveSince         string            // timestamp since this node is alive
	UnreachableSince   string            // timestamp since this node is unreachable
	Replicas           map[uint64]uint64 // list of replicas on this node
	NumShards          uint32            // number of shards on this node
	NumPrimaryReplicas uint32            // number of primary replicas
}

// TscaleCluster represents a cluster of nodes
type TscaleCluster struct {
	sync.Mutex                           // lock the cluster object
	api.TypeMeta                         // type meta
	api.ObjectMeta                       // object meta
	ShardMap       *ShardMap             // shard map
	NodeMap        map[string]*NodeState // list of nodes
	mclient        metaclient            // metadata mgmt apis
}

// TscaleNodeInfo is information about nodes as published by the node
type TscaleNodeInfo struct {
	api.TypeMeta          // type meta
	api.ObjectMeta        // object meta
	NodeUUID       string // node id
	NodeURL        string // URL to reach the node
}

// NodeList is the list object
type NodeList struct {
	api.ListMeta                // list meta
	api.TypeMeta                // type meta
	ObjectMeta   api.ObjectMeta // object meta
	Items        []*TscaleNodeInfo
}

// ClusterConfig config parameters
type ClusterConfig struct {
	EnableKstore        bool               // enable key-value store
	EnableTstore        bool               // enable time-series store
	EnableKstoreMeta    bool               // enable kstore metadata mgr (used for testing purposes)
	EnableTstoreMeta    bool               // enable tstore metadata mgr (used for testing purposes)
	MetastoreType       string             // metadata store type
	MetaStoreTLSConfig  *tls.Config        // tls config for kv store
	MetaStoreRetry      int                // number of retries before generating event
	MetaStoreURLs       []string           // metastore URLs to connect to
	NumShards           uint32             // number of shards in shardmap
	DesiredReplicas     uint32             // desired number of replicas
	NodeTTL             uint64             // TTL for the node keepalives
	MaxSyncMsgSize      int                // Max sync message size
	MaxSyncRetry        int                // Max sync retry attempts
	MaxSyncBuffSize     int                // Max sync buffer size
	DeadInterval        time.Duration      // duration after which we declare a node as dead
	RebalanceInterval   time.Duration      // rebalance interval
	RebalanceDelay      time.Duration      // delay before starting rebalance loop
	RetentionPeriod     time.Duration      // how long TSDB keeps the data
	RetentionPolicyName string             // retention policy name in TSDB
	ResolverClient      resolver.Interface // resolver client
}

// metaclient api provided by meta data mgr
type metaclient interface {
	WriteMetadata(cluster *TscaleCluster) error
	SetPrimaryReplica(repl *Replica) error
	CopyDataFromPrimaryReplica(prepl *Replica, serepl *Replica) error
}

// clusterMetastoreURL is the kvstore URL for cluster info
const (
	ClusterMetastoreURL = "/venice/citadel/meta/cluster/"
	NodesMetastoreURL   = "/venice/citadel/nodes/"
)

// default values for cluster config
const (
	DefaultShardCount          = 16
	DefaultReplicaCount        = 2
	DefaultNodeDeadInterval    = time.Second * 30
	DefaultNodeTTL             = 30
	DefaultRetentionPeriod     = 7 * 24 * time.Hour
	DefaultRetentionPolicyName = "default"
	DefaultMetaStoreRetry      = 180
	DefaultMaxSyncMsgSize      = 20 * 1024 * 1024 // 20MB
	DefaultMaxSyncRetry        = 5
	DefaultMaxSyncBuffSize     = 1000
)

// DefaultClusterConfig returns default cluster config params
func DefaultClusterConfig() *ClusterConfig {
	metaStoreTLSConfig, err := etcd.GetEtcdClientCredentials()
	// warn and proceed
	if err != nil {
		log.Warnf("failed to load etcd credentials")
	}

	return &ClusterConfig{
		EnableKstore:        true,
		EnableTstore:        true,
		EnableKstoreMeta:    true,
		EnableTstoreMeta:    true,
		MetastoreType:       store.KVStoreTypeMemkv,
		MetaStoreTLSConfig:  metaStoreTLSConfig,
		MetaStoreRetry:      DefaultMetaStoreRetry,
		NumShards:           DefaultShardCount,
		DesiredReplicas:     DefaultReplicaCount,
		NodeTTL:             DefaultNodeTTL,
		MaxSyncMsgSize:      DefaultMaxSyncMsgSize,
		MaxSyncRetry:        DefaultMaxSyncRetry,
		MaxSyncBuffSize:     DefaultMaxSyncBuffSize,
		DeadInterval:        DefaultNodeDeadInterval,
		RebalanceInterval:   time.Second,
		RebalanceDelay:      time.Second * 5,
		RetentionPeriod:     DefaultRetentionPeriod,
		RetentionPolicyName: DefaultRetentionPolicyName,
	}
}

// DefaultQueryLimit is the default number of points returned from the query
const DefaultQueryLimit = 50000 // limited by GRPC limit

// AddNode adds a node to
func (cl *TscaleCluster) AddNode(nodeUUID, nodeURL string) (*NodeState, error) {
	// check if we already have the node
	cl.Lock()
	enode, ok := cl.NodeMap[nodeUUID]
	cl.Unlock()
	if ok && enode.NodeStatus == NodeStatusAlive {
		return enode, nil
	}

	// if node was unreachable, mark it alive now
	if ok && enode.NodeStatus == NodeStatusUnreachable {
		enode.NodeStatus = NodeStatusAlive
		enode.AliveSince = time.Now().String()
		enode.UnreachableSince = ""

		// save the updated metadata
		cl.mclient.WriteMetadata(cl)

		// walk all the replicas and mark them active
		for replID := range enode.Replicas {
			// find the replica
			repl, err := cl.ShardMap.GetReplica(replID)
			if err != nil {
				log.Errorf("Error finding the replica %d. err: %v", replID, err)
				continue
			}

			// mark the replica as activated
			err = repl.shard.ReplicaActivate(repl)
			if err != nil {
				log.Errorf("Error marking shard %+v as active. Err: %v", repl, err)
			}
		}

		return enode, nil
	}

	// create node state
	node := NodeState{
		NodeUUID:   nodeUUID,
		NodeURL:    nodeURL,
		NodeStatus: NodeStatusAlive,
		AliveSince: time.Now().String(),
		Replicas:   make(map[uint64]uint64),
	}

	// add it to node map
	cl.Lock()
	cl.NodeMap[nodeUUID] = &node
	cl.Unlock()

	return &node, nil
}

// NodeUnreachable mark a node unreachable
func (cl *TscaleCluster) NodeUnreachable(nodeUUID string) error {
	log.Infof("%s Marking %s unreachable", cl.Name, nodeUUID)

	// find the node
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		return err
	}

	// change node state
	node.UnreachableSince = time.Now().Format(time.RFC3339Nano)
	node.NodeStatus = NodeStatusUnreachable
	node.AliveSince = ""

	// mark all the replicas unreachable
	for replID := range node.Replicas {
		// find the replica
		repl, err := cl.ShardMap.GetReplica(replID)
		if err != nil {
			log.Errorf("Error finding the replica %d. err: %v", replID, err)
			continue
		}

		// mark the replica down
		err = repl.shard.ReplicaUnreachable(repl)
		if err != nil {
			log.Errorf("Error marking shard %+v as unreachable. Err: %v", repl, err)
		}
	}

	return nil
}

// RemoveNode remove the node from cluster
func (cl *TscaleCluster) RemoveNode(nodeUUID string) error {
	// find the node
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		return err
	}

	// change node state
	node.NodeStatus = NodeStatusDead

	// walk all the shards on this node and remove it
	for replID := range node.Replicas {
		// find the shard
		repl, err := cl.ShardMap.GetReplica(replID)
		if err != nil {
			log.Errorf("Error finding the shard %d. err: %v", replID, err)
			continue
		}

		// mark the shard down
		err = repl.shard.RemoveReplica(repl)
		if err != nil {
			log.Errorf("Error removing shard %+v. Err: %v", repl, err)
		}
	}

	// delete the node from node map
	cl.Lock()
	delete(cl.NodeMap, nodeUUID)
	cl.Unlock()

	return nil
}

// AddReplica adds a replica info to nodemap
func (cl *TscaleCluster) AddReplica(repl *Replica) error {
	// find the node
	node, err := cl.GetNode(repl.NodeUUID)
	if err != nil {
		return err
	}

	// add shard to node
	node.Replicas[repl.ReplicaID] = repl.ReplicaID
	node.NumShards++
	if repl.IsPrimary {
		node.NumPrimaryReplicas++
	}

	return nil
}

// RemoveReplica removes a replica from nodemap
func (cl *TscaleCluster) RemoveReplica(repl *Replica) error {
	// find the node
	node, err := cl.GetNode(repl.NodeUUID)
	if err != nil {
		return err
	}

	// add shard to node
	delete(node.Replicas, repl.ReplicaID)
	node.NumShards--
	if repl.IsPrimary {
		// FIXME: there is a bug here. When a shard transition from primary to secondary or vice-versa we're not updating this counter
		node.NumPrimaryReplicas--
	}

	return nil
}

// GetLeastUsedNode returns the least used node
func (cl *TscaleCluster) GetLeastUsedNode() *NodeState {
	var luNode *NodeState

	// lock the cluster while we walk nodemap
	cl.Lock()
	defer cl.Unlock()

	// loop thru all nodes
	for _, node := range cl.NodeMap {
		// check if this node has fewer shards
		if luNode == nil || node.NumShards < luNode.NumShards {
			luNode = node
		}
	}

	return luNode
}

// GetLeastUsedNodeForShard finds the least used node excluding the nodes where a shard already has a replica
func (cl *TscaleCluster) GetLeastUsedNodeForShard(shard *Shard) *NodeState {
	var luNode *NodeState

	// lock the cluster for access
	cl.Lock()
	defer cl.Unlock()

	// loop thru all nodes
	for _, node := range cl.NodeMap {
		// check if this node has fewer shards
		if luNode == nil || node.NumShards < luNode.NumShards {
			if !shard.HasReplicaOnNode(node.NodeUUID) {
				luNode = node
			}
		}
	}

	return luNode
}

// GetMostUsedNode returns the most used node
func (cl *TscaleCluster) GetMostUsedNode() *NodeState {
	var luNode *NodeState

	// lock the cluster for access
	cl.Lock()
	defer cl.Unlock()

	// loop thru all nodes
	for _, node := range cl.NodeMap {
		// check if this node has fewer shards
		if luNode == nil || node.NumShards > luNode.NumShards {
			luNode = node
		}
	}

	return luNode
}

// FindMoveCandidateOnNode find a candidate shard to move
func (cl *TscaleCluster) FindMoveCandidateOnNode(node *NodeState, excludeNode string) (*Replica, error) {
	// loop thru all shards
	for replID := range node.Replicas {
		repl, err := cl.ShardMap.GetReplica(replID)
		if err != nil {
			return nil, err
		}

		// if the shard doesnt have a replica on target node, this is a candidate
		if !repl.shard.HasReplicaOnNode(excludeNode) {
			return repl, nil
		}

	}

	return nil, errors.New("Could not find a move candidate")
}

// GetNode returns a node from UUID
func (cl *TscaleCluster) GetNode(nodeUUID string) (*NodeState, error) {
	// lock the cluster for access
	cl.Lock()
	defer cl.Unlock()

	// find the node
	node, ok := cl.NodeMap[nodeUUID]
	if !ok {
		return nil, errors.New("Node not found")
	}

	return node, nil
}

// GetReplicaFromNode returns a replica from Node
func (cl *TscaleCluster) GetReplicaFromNode(nodeUUID string) (*Replica, error) {
	ns, err := cl.GetNode(nodeUUID)
	if err != nil {
		return nil, err
	}

	for r := range ns.Replicas {
		repl, err := cl.ShardMap.GetReplica(r)
		if err != nil {
			continue
		}

		if repl.State != ReplicaStateReady {
			continue
		}

		return repl, nil
	}

	log.Errorf("failed to find any replica in node %v in ready state ", nodeUUID)
	return nil, fmt.Errorf("failed to find any replica")
}

// IsNodeAlive checks if a node is alive
func (cl *TscaleCluster) IsNodeAlive(nodeUUID string) bool {
	// find the node
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		return false
	}

	// check if node is alive
	if node.NodeStatus == NodeStatusAlive {
		return true
	}

	return false
}

// GetActiveNodes returns a list of active nodes
func (cl *TscaleCluster) GetActiveNodes() []*NodeState {
	var nodes []*NodeState

	// lock the cluster for access
	cl.Lock()
	defer cl.Unlock()

	// walk all nodes
	for _, node := range cl.NodeMap {
		if node.NodeStatus == NodeStatusAlive {
			nodes = append(nodes, node)
		}
	}

	return nodes
}

// HasNode checks if a node exists in node list
func (nl *NodeList) HasNode(nodeUUID string) bool {
	for _, node := range nl.Items {
		if node.NodeUUID == nodeUUID {
			return true
		}
	}

	return false
}

// GetClusterState reads the current cluster state from kvstore
func GetClusterState(cfg *ClusterConfig, clusterType string) (*TscaleCluster, error) {
	var kvs kvstore.Interface
	var err error

	// connect to kvstore
	config := store.Config{
		Type:        cfg.MetastoreType,
		Servers:     GetMetastoreURLs(context.Background(), cfg),
		Credentials: cfg.MetaStoreTLSConfig,
		Codec:       runtime.NewJSONCodec(runtime.NewScheme()),
	}

	for i := 0; i < cfg.MetaStoreRetry; i++ {
		kvs, err = store.New(config)
		if err == nil {
			defer kvs.Close()
			// read cluster state
			var cluster TscaleCluster
			err = kvs.Get(context.Background(), ClusterMetastoreURL+clusterType, &cluster)
			if err == nil {
				return &cluster, nil
			}

			log.Errorf("Error reading cluster state. Err: %v", err)
		}
	}

	// reset URLs to fetch from api server
	cfg.MetaStoreURLs = nil
	return nil, err
}

// DestroyClusterState destroys the cluster state in kvstore
func DestroyClusterState(cfg *ClusterConfig, clusterType string) error {
	config := store.Config{
		Type:        cfg.MetastoreType,
		Servers:     GetMetastoreURLs(context.Background(), cfg),
		Credentials: cfg.MetaStoreTLSConfig,
		Codec:       runtime.NewJSONCodec(runtime.NewScheme()),
	}
	kvs, err := store.New(config)
	if err != nil {
		return err
	}
	defer kvs.Close()

	return kvs.Delete(context.Background(), ClusterMetastoreURL+clusterType, nil)
}

// GetMetastoreURLs retrieves kv store URL from cluster config
func GetMetastoreURLs(ctx context.Context, cfg *ClusterConfig) []string {
	if cfg.MetastoreType != store.KVStoreTypeEtcd { // for tests
		log.Infof("get URL for meta store type %v", cfg.MetastoreType)
		return nil
	}

	if len(cfg.MetaStoreURLs) > 0 {
		return cfg.MetaStoreURLs
	}

	for ctx.Err() == nil {
		for i := 0; i < cfg.MetaStoreRetry && ctx.Err() == nil; i++ {
			// create a grpc client
			bl := balancer.New(cfg.ResolverClient)
			apicl, err := apiclient.NewGrpcAPIClient(globals.Citadel, globals.APIServer, log.WithContext("pkg", globals.Citadel+"-grpc"), rpckit.WithBalancer(bl))
			if err != nil {
				bl.Close()
				log.Errorf("failed to connect to %s, %s", globals.APIServer, err)
				time.Sleep(time.Second)
				continue
			}

			defer apicl.Close()

			log.Infof("connected to %s", globals.APIServer)

			cluster, err := apicl.ClusterV1().Cluster().List(context.Background(), &api.ListWatchOptions{})
			if err != nil {
				log.Errorf("failed to get cluster info %s", err)
				time.Sleep(time.Second)
				continue
			}

			// pick the first entry, singleton object
			for _, c := range cluster {
				var kvstore []string
				for _, q := range c.Spec.QuorumNodes {
					kvstore = append(kvstore, q+":"+globals.KVStoreClientPort)
				}
				log.Infof("metastore urls : %+v", kvstore)
				cfg.MetaStoreURLs = kvstore
				return kvstore
			}
			time.Sleep(time.Second)
		}
	}
	return nil
}
