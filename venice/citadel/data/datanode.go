// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

// this file contains the datanode management code

import (
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strconv"
	"sync"

	"golang.org/x/net/context"

	"github.com/cenkalti/backoff"

	"github.com/pensando/sw/venice/citadel/kstore"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/citadel/tstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/safelist"
)

// TshardState is state of a tstore shard
type TshardState struct {
	syncLock    sync.Mutex            // lock for pending sync operations
	replicaID   uint64                // replica id
	shardID     uint64                // shard this replica belongs to
	isPrimary   bool                  // is this primary replica? // FIXME: who updates this when node is disconnected?
	syncPending bool                  // is sync pending on this shard?
	store       *tstore.Tstore        // data store
	syncBuffer  sync.Map              // sync buffer for failed writes
	replicas    []*tproto.ReplicaInfo // other replicas of this shard
	// FIXME: do we need to keep a write log for replicas that are temporarily offline?
}

// KshardState is state of a kstore shard
type KshardState struct {
	syncLock    sync.Mutex            // lock for pending sync operations
	replicaID   uint64                // replica id
	shardID     uint64                // shard this replica belongs to
	isPrimary   bool                  // is this primary replica? // FIXME: who updates this when node is disconnected?
	syncPending bool                  // is sync pending on this shard?
	kstore      kstore.Kstore         // key-value store
	syncBuffer  sync.Map              // sync buffer for failed writes
	replicas    []*tproto.ReplicaInfo // other replicas of this shard
	// FIXME: do we need to keep a write log for replicas that are temporarily offline?
}

// StoreAPI is common api provided by both kstore and tstore
type StoreAPI interface {
	GetShardInfo(sinfo *tproto.SyncShardInfoMsg) error
	RestoreShardInfo(sinfo *tproto.SyncShardInfoMsg) error
	BackupChunk(chunkID uint64, w io.Writer) error
	RestoreChunk(chunkID uint64, r io.Reader) error
}

// DNode represents a backend data node instance
type DNode struct {
	nodeUUID     string // unique id for the data node
	nodeURL      string // URL to reach the data node
	dbPath       string // data store path
	querydbPath  string // query data store path
	clusterCfg   *meta.ClusterConfig
	metaNode     *meta.Node        // pointer to metadata node instance
	watcher      *meta.Watcher     // metadata watcher
	rpcServer    *rpckit.RPCServer // grpc server
	tsQueryStore *tstore.Tstore    // ts query store
	tshards      sync.Map          // tstore shards
	kshards      sync.Map          // kstore shards
	rpcClients   sync.Map          // rpc connections
	logger       log.Logger
	queryMutex   sync.Mutex // query lock

	isStopped bool // is the datanode stopped?
}

// syncBufferState is the common structure used by ts/kvstore to queue failed writes
// replica id is used as the key to access syncBufferState from the map
type syncBufferState struct {
	ctx         context.Context             // context used by the retry functions
	cancel      context.CancelFunc          // to cancel sync buffer
	wg          sync.WaitGroup              // for the syncbuffer go routine
	nodeUUID    string                      // unique id for the data node
	shardID     uint64                      // shard id
	replicaID   uint64                      // replica id
	clusterType string                      // ts or kv store
	queue       *safelist.SafeList          // pending queue
	backoff     *backoff.ExponentialBackOff // back-off config
}

// NewDataNode creates a new data node instance
func NewDataNode(cfg *meta.ClusterConfig, nodeUUID, nodeURL, dbPath string, querydbPath string, logger log.Logger) (*DNode, error) {
	// Start a rpc server
	rpcSrv, err := rpckit.NewRPCServer(globals.Citadel, nodeURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		logger.Errorf("failed to listen to %s: Err %v", nodeURL, err)
		return nil, err
	}

	// create watcher
	watcher, err := meta.NewWatcher(nodeUUID, cfg)
	if err != nil {
		return nil, err
	}

	// If nodeURL was passed with :0, then update the nodeURL to the real URL
	nodeURL = rpcSrv.GetListenURL()

	// create a data node
	dn := DNode{
		nodeUUID:    nodeUUID,
		nodeURL:     nodeURL,
		dbPath:      dbPath,
		querydbPath: querydbPath,
		clusterCfg:  cfg,
		watcher:     watcher,
		rpcServer:   rpcSrv,
		logger:      logger.WithContext("nodeuuid", nodeUUID),
	}

	// read all shard state from metadata store and restore it
	err = dn.readAllShards(cfg)
	if err != nil {
		dn.logger.Errorf("Error reading state from metadata store. Err: %v", err)
		return nil, err
	}

	// register RPC handlers
	tproto.RegisterDataNodeServer(rpcSrv.GrpcServer, &dn)
	rpcSrv.Start()
	dn.logger.Infof("Datanode RPC server is listening on: %s", nodeURL)

	// register the node metadata
	metaNode, err := meta.NewNode(cfg, nodeUUID, nodeURL)
	if err != nil {
		dn.logger.Errorf("Error creating metanode. Err: %v", err)
		return nil, err
	}
	dn.metaNode = metaNode
	return &dn, nil
}

// getDbFiles returns db files for the store type
func (dn *DNode) getDbFiles(clusterType string) map[string]string {
	dbfiles := map[string]string{}

	if fds, err := ioutil.ReadDir(fmt.Sprintf("%s/%s", dn.dbPath, clusterType)); err == nil {
		for _, f := range fds {
			if f.Name() != "qdb" {
				dbfiles[fmt.Sprintf("%s/%s/%v", dn.dbPath, clusterType, f.Name())] = f.Name()
			}
		}
	}

	return dbfiles
}

// getDbPath returns the db path for store type
func (dn *DNode) getDbPath(clusterType string, replicaID uint64) string {
	return fmt.Sprintf("%s/%s/%d", dn.dbPath, clusterType, replicaID)
}

// getQueryDbPath returns the query db path
func (dn *DNode) getQueryDbPath(clusterType string) string {
	return fmt.Sprintf("%s/%s/qdb", dn.dbPath, clusterType)
}

// readAllShards reads all shard state from metadata store and restores state
func (dn *DNode) readAllShards(cfg *meta.ClusterConfig) error {
	// read current state of the cluster and restore the shards
	// FIXME: if etcd was unreachable when we come up we need to handle it
	if cfg.EnableTstore {
		dbfiles := dn.getDbFiles(meta.ClusterTypeTstore)

		// query aggregator for this data node
		if err := dn.newQueryStore(); err != nil {
			return err
		}

		cluster, err := meta.GetClusterState(cfg, meta.ClusterTypeTstore)
		if err == nil {
			for _, shard := range cluster.ShardMap.Shards {
				for _, repl := range shard.Replicas {
					if repl.NodeUUID == dn.nodeUUID {
						dbPath := dn.getDbPath(meta.ClusterTypeTstore, repl.ReplicaID)
						delete(dbfiles, dbPath)
						ts, serr := tstore.NewTstore(dbPath)
						if serr != nil {
							dn.logger.Errorf("Error creating tstore at %s. Err: %v", dbPath, serr)
						} else {
							// create a shard instance
							tshard := TshardState{
								replicaID: repl.ReplicaID,
								shardID:   repl.ShardID,
								isPrimary: repl.IsPrimary,
								store:     ts,
							}

							dn.logger.Infof("Restored tstore replica %d shard %d", repl.ReplicaID, repl.ShardID)

							// collect all replicas in this shard
							var replicaList []*tproto.ReplicaInfo
							for _, sr := range shard.Replicas {
								// get node for url
								node, nerr := cluster.GetNode(sr.NodeUUID)
								if nerr != nil {
									// recover if it is our own node missing in the node map
									if sr.NodeUUID == dn.nodeUUID {
										log.Warnf("failed to find node %v in node list", sr.NodeUUID)
										// recover
										node = &meta.NodeState{
											NodeUUID: dn.nodeUUID,
											NodeURL:  dn.nodeURL,
										}
									} else {
										log.Errorf("failed to find node %v in node list", sr.NodeUUID)
										return nerr
									}
								}

								// build replica info
								rpinfo := tproto.ReplicaInfo{
									ClusterType: meta.ClusterTypeTstore,
									ReplicaID:   sr.ReplicaID,
									ShardID:     sr.ShardID,
									IsPrimary:   sr.IsPrimary,
									NodeUUID:    sr.NodeUUID,
									NodeURL:     node.NodeURL,
								}

								replicaList = append(replicaList, &rpinfo)
							}
							tshard.replicas = replicaList

							// save the datastore
							dn.tshards.Store(repl.ReplicaID, &tshard)
						}
					}
				}
			}
		}
		for k := range dbfiles {
			log.Infof("deleting replica file %v", k)
			if err := os.RemoveAll(k); err != nil {
				log.Errorf("failed to delete %v, %v", k, err)
			}
		}

	}
	// restore all kstore shards
	if cfg.EnableKstore {
		dbfiles := dn.getDbFiles(meta.ClusterTypeKstore)

		cluster, err := meta.GetClusterState(cfg, meta.ClusterTypeKstore)
		if err == nil {
			for _, shard := range cluster.ShardMap.Shards {
				for _, repl := range shard.Replicas {
					if repl.NodeUUID == dn.nodeUUID {
						dbPath := dn.getDbPath(meta.ClusterTypeKstore, repl.ReplicaID)
						delete(dbfiles, dbPath)

						ks, serr := kstore.NewKstore(kstore.BoltDBType, dbPath)
						if serr != nil {
							dn.logger.Errorf("Error creating kstore at %s. Err: %v", dbPath, serr)
						} else {
							// create a shard instance
							kshard := KshardState{
								replicaID: repl.ReplicaID,
								shardID:   repl.ShardID,
								isPrimary: repl.IsPrimary,
								kstore:    ks,
							}

							dn.logger.Infof("Restored kstore replica %d shard %d", repl.ReplicaID, repl.ShardID)

							// collect all replicas in this shard
							var replicaList []*tproto.ReplicaInfo
							for _, sr := range shard.Replicas {
								// get node for url
								node, nerr := cluster.GetNode(sr.NodeUUID)
								if nerr != nil {
									// recover if it is our own node missing in the node map
									if sr.NodeUUID == dn.nodeUUID {
										log.Warnf("failed to find node %v in node list", sr.NodeUUID)
										// recover
										node = &meta.NodeState{
											NodeUUID: dn.nodeUUID,
											NodeURL:  dn.nodeURL,
										}
									} else {
										log.Errorf("failed to find node %v in node list", sr.NodeUUID)
										return nerr
									}
								}

								// build replica info
								rpinfo := tproto.ReplicaInfo{
									ClusterType: meta.ClusterTypeKstore,
									ReplicaID:   sr.ReplicaID,
									ShardID:     sr.ShardID,
									IsPrimary:   sr.IsPrimary,
									NodeUUID:    sr.NodeUUID,
									NodeURL:     node.NodeURL,
								}

								replicaList = append(replicaList, &rpinfo)
							}
							kshard.replicas = replicaList

							// save the datastore
							dn.kshards.Store(repl.ReplicaID, &kshard)
						}
					}
				}
			}
		}
		for k := range dbfiles {
			log.Infof("deleting replica file %v", k)
			if err := os.RemoveAll(k); err != nil {
				log.Errorf("failed to delete %v, %v", k, err)
			}
		}

	}

	return nil
}

// CreateShard creates a shard
func (dn *DNode) CreateShard(ctx context.Context, req *tproto.ShardReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received CreateShard req %+v", dn.nodeUUID, req)

	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		if dn.clusterCfg.EnableTstore {
			// find the shard from replica id
			val, ok := dn.tshards.Load(req.ReplicaID)
			if ok && val.(*TshardState).store != nil {
				dn.logger.Warnf("Replica %d already exists", req.ReplicaID)
				return &resp, nil
			}

			// delete old replicas of the shard if it exists
			delReplicas := []uint64{}
			dn.tshards.Range(func(key, val interface{}) bool {
				replID := key.(uint64)
				tState := val.(*TshardState)

				// find old replica of the shard
				if tState.shardID == req.ShardID {
					delReplicas = append(delReplicas, replID)
				}
				return true
			})

			for _, r := range delReplicas {
				log.Infof("delete prev. replica %d of shard %v, new replica %d", r, req.ShardID, req.ReplicaID)
				if _, err := dn.DeleteShard(ctx, &tproto.ShardReq{
					ClusterType: req.ClusterType,
					ShardID:     req.ShardID,
					ReplicaID:   r,
				}); err != nil {
					log.Errorf("failed to delete replica %d of shard %v, err: %v", r, req.ShardID, err)
					continue
				}
			}

			// create the data store
			dbPath := dn.getDbPath(meta.ClusterTypeTstore, req.ReplicaID)
			ts, err := tstore.NewTstore(dbPath)
			if err != nil {
				dn.logger.Errorf("Error creating tstore at %s. Err: %v", dbPath, err)
				resp.Status = err.Error()
				return &resp, err
			}

			// create a shard instance
			shard := TshardState{
				replicaID: req.ReplicaID,
				shardID:   req.ShardID,
				isPrimary: req.IsPrimary,
				store:     ts,
				replicas:  req.Replicas,
			}

			// save the datastore
			dn.tshards.Store(req.ReplicaID, &shard)
		}
	case meta.ClusterTypeKstore:
		if dn.clusterCfg.EnableKstore {
			// find the shard from replica id
			val, ok := dn.kshards.Load(req.ReplicaID)
			if ok && val.(*KshardState).kstore != nil {
				dn.logger.Warnf("Replica %d already exists", req.ReplicaID)
				return &resp, nil
			}

			// create the shard
			dbPath := dn.getDbPath(meta.ClusterTypeKstore, req.ReplicaID)
			ks, err := kstore.NewKstore(kstore.BoltDBType, dbPath)
			if err != nil {
				dn.logger.Errorf("Error creating kstore %s. Err: %v", dbPath, err)
				resp.Status = err.Error()
				return &resp, err
			}

			// create a shard instance
			shard := KshardState{
				replicaID: req.ReplicaID,
				shardID:   req.ShardID,
				isPrimary: req.IsPrimary,
				kstore:    ks,
				replicas:  req.Replicas,
			}

			// save the datastore
			dn.kshards.Store(req.ReplicaID, &shard)
		}
	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
	}

	return &resp, nil
}

// UpdateShard updates shard info
func (dn *DNode) UpdateShard(ctx context.Context, req *tproto.ShardReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received UpdateShard req %+v", dn.nodeUUID, req)

	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		// find the shard from replica id
		val, ok := dn.tshards.Load(req.ReplicaID)
		if !ok || val.(*TshardState).store == nil {
			dn.logger.Errorf("Shard %d not found", req.ReplicaID)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*TshardState)

		// set it as primary
		shard.isPrimary = req.IsPrimary

		// set replicas
		shard.replicas = req.Replicas

		// update sync buffer
		dn.updateSyncBuffer(&shard.syncBuffer, req)

	case meta.ClusterTypeKstore:
		// find the shard from replica id
		val, ok := dn.kshards.Load(req.ReplicaID)
		if !ok || val.(*KshardState).kstore == nil {
			dn.logger.Errorf("Shard %d not found", req.ReplicaID)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*KshardState)

		// set it as primary
		shard.isPrimary = req.IsPrimary

		// set replicas
		shard.replicas = req.Replicas

		// update sync buffer
		dn.updateSyncBuffer(&shard.syncBuffer, req)

	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
	}

	return &resp, nil
}

// DeleteShard deletes a shard from the data node
func (dn *DNode) DeleteShard(ctx context.Context, req *tproto.ShardReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received DeleteShard req %+v", dn.nodeUUID, req)

	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		// find the shard from replica id
		val, ok := dn.tshards.Load(req.ReplicaID)
		if !ok || val.(*TshardState).store == nil {
			dn.logger.Errorf("Shard %d not found", req.ReplicaID)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*TshardState)
		dn.deleteShardSyncBuffer(&shard.syncBuffer)

		dn.tshards.Delete(req.ReplicaID)

		// acquire sync lock to make sure there are no outstanding sync
		shard.syncLock.Lock()
		defer shard.syncLock.Unlock()

		// close the databse
		err := shard.store.Close()
		if err != nil {
			dn.logger.Errorf("Error closing the database for shard %d. Err: %v", req.ReplicaID, err)
		}
		shard.store = nil

		// remove the db files
		dbPath := dn.getDbPath(meta.ClusterTypeTstore, req.ReplicaID)
		os.RemoveAll(dbPath)
	case meta.ClusterTypeKstore:
		// find the shard from replica id
		val, ok := dn.kshards.Load(req.ReplicaID)
		if !ok || val.(*KshardState).kstore == nil {
			dn.logger.Errorf("Shard %d not found", req.ReplicaID)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*KshardState)

		// delete sync buffer
		dn.deleteShardSyncBuffer(&shard.syncBuffer)

		dn.kshards.Delete(req.ReplicaID)

		// acquire sync lock to make sure there are no outstanding sync
		shard.syncLock.Lock()
		defer shard.syncLock.Unlock()

		// close the databse
		err := shard.kstore.Close()
		if err != nil {
			dn.logger.Errorf("Error closing the database for shard %d. Err: %v", req.ReplicaID, err)
		}
		shard.kstore = nil

		// remove the db files
		dbPath := dn.getDbPath(meta.ClusterTypeKstore, req.ReplicaID)
		os.RemoveAll(dbPath)
	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
	}

	return &resp, nil
}

// GetCluster returns cluster state from the watcher
func (dn *DNode) GetCluster(clusterType string) *meta.TscaleCluster {
	return dn.watcher.GetCluster(clusterType)
}

// IsLeader returns true if this node is the leader node
func (dn *DNode) IsLeader() bool {
	return dn.metaNode.IsLeader()
}

// IsStopped returns true if data node is stopped
func (dn *DNode) IsStopped() bool {
	return dn.isStopped
}

// HasPendingSync checks if any shards on this node has sync pending
func (dn *DNode) HasPendingSync() bool {
	var retVal bool
	dn.tshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*TshardState)
		shard.syncLock.Lock()
		defer shard.syncLock.Unlock()
		if shard.syncPending {
			dn.logger.Infof("Datanode %s tstore shard %d replica %d has sync pending", dn.nodeUUID, shard.shardID, shard.replicaID)
			retVal = true
		}
		return true
	})
	dn.kshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*KshardState)
		shard.syncLock.Lock()
		defer shard.syncLock.Unlock()
		if shard.syncPending {
			dn.logger.Infof("Datanode %s kstore shard %d replica %d has sync pending", dn.nodeUUID, shard.shardID, shard.replicaID)
			retVal = true
		}
		return true
	})

	return retVal
}

// SoftRestart does a soft restart of the nodes without releasing the lease
// Note: this is to be used by testing purposes only
func (dn *DNode) SoftRestart() error {
	var err error

	// first stop everythig
	dn.isStopped = true
	dn.tsQueryStore.Close()
	dn.watcher.Stop()
	dn.tshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*TshardState)
		// clean up syncbuffer
		dn.deleteShardSyncBuffer(&shard.syncBuffer)

		if shard.store != nil {
			shard.store.Close()
			shard.store = nil
		}
		return true
	})
	dn.kshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*KshardState)
		// clean up syncbuffer
		dn.deleteShardSyncBuffer(&shard.syncBuffer)
		if shard.kstore != nil {
			shard.kstore.Close()
			shard.kstore = nil
		}
		return true
	})

	// close the rpc clients
	dn.rpcClients.Range(func(key interface{}, value interface{}) bool {
		rc, ok := value.(*rpckit.RPCClient)
		if ok {
			rc.Close()
			dn.rpcClients.Delete(key)
		}
		return true
	})

	// stop rpc server
	dn.rpcServer.Stop()

	// restart everything back up
	dn.isStopped = false

	// Start a rpc server
	dn.rpcServer, err = rpckit.NewRPCServer(globals.Citadel, dn.nodeURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		dn.logger.Errorf("failed to listen to %s: Err %v", dn.nodeURL, err)
		return err
	}

	// create watcher
	dn.watcher, err = meta.NewWatcher(dn.nodeUUID, dn.clusterCfg)
	if err != nil {
		return err
	}

	// register RPC handlers
	tproto.RegisterDataNodeServer(dn.rpcServer.GrpcServer, dn)
	dn.rpcServer.Start()
	dn.logger.Infof("Datanode RPC server is listening on: %s", dn.nodeURL)

	// read all shard state from metadata store and restore it
	err = dn.readAllShards(dn.clusterCfg)
	if err != nil {
		dn.logger.Errorf("Error reading state from metadata store. Err: %v", err)
		return err
	}

	return nil
}

// Stop stops the data node
func (dn *DNode) Stop() error {
	dn.isStopped = true
	dn.tsQueryStore.Close()
	dn.metaNode.Stop()
	dn.watcher.Stop()
	dn.tshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*TshardState)
		// clean up sync buffer
		dn.deleteShardSyncBuffer(&shard.syncBuffer)

		if shard.store != nil {
			shard.store.Close()
			shard.store = nil
		}
		return true
	})
	dn.kshards.Range(func(key interface{}, value interface{}) bool {
		shard := value.(*KshardState)
		// clean up sync buffer
		dn.deleteShardSyncBuffer(&shard.syncBuffer)
		if shard.kstore != nil {
			shard.kstore.Close()
			shard.kstore = nil
		}
		return true
	})

	// close the rpc clients
	dn.rpcClients.Range(func(key interface{}, value interface{}) bool {
		rc, ok := value.(*rpckit.RPCClient)
		if ok {
			rc.Close()
			dn.rpcClients.Delete(key)
		}
		return true
	})

	dn.rpcServer.Stop()

	return nil
}

// ------------- internal functions --------

// getDnclient returns a grpc client
func (dn *DNode) getDnclient(clusterType, nodeUUID string) (tproto.DataNodeClient, error) {
	// if we already have a connection, just return it
	eclient, ok := dn.rpcClients.Load(nodeUUID)
	if ok {
		return tproto.NewDataNodeClient(eclient.(*rpckit.RPCClient).ClientConn), nil
	}

	// find the node
	cl := dn.watcher.GetCluster(clusterType)
	node, err := cl.GetNode(nodeUUID)
	if err != nil {
		return nil, err
	}

	// dial the connection
	rclient, err := rpckit.NewRPCClient(fmt.Sprintf("datanode-%s", dn.nodeUUID), node.NodeURL, rpckit.WithLoggerEnabled(false), rpckit.WithRemoteServerName(globals.Citadel))
	if err != nil {
		dn.logger.Errorf("Error connecting to rpc server %s. err: %v", node.NodeURL, err)
		return nil, err
	}
	dn.rpcClients.Store(nodeUUID, rclient)

	return tproto.NewDataNodeClient(rclient.ClientConn), nil
}

// reconnectDnclient close existing rpc client and try reconnecting
func (dn *DNode) reconnectDnclient(clusterType, nodeUUID string) (tproto.DataNodeClient, error) {
	// close and delete existing rpc client
	eclient, ok := dn.rpcClients.Load(nodeUUID)
	if ok {
		eclient.(*rpckit.RPCClient).Close()
		dn.rpcClients.Delete(nodeUUID)
	}

	return dn.getDnclient(clusterType, nodeUUID)
}

// retry ts/kv write based on the cluster type
func (dn *DNode) replicateFailedRequest(sb *syncBufferState) error {
	switch sb.clusterType {
	case meta.ClusterTypeTstore:
		return dn.replicateFailedPoints(sb)
	case meta.ClusterTypeKstore:
		return dn.replicateFailedWrite(sb)
	default:
		dn.logger.Fatalf("unknown cluster type: %+v in sync buffer", sb)
	}
	return nil
}

// String displays data node
func (dn *DNode) String() string {
	dbg := map[string]string{
		"node-uuid": dn.nodeUUID,
		"node-url":  dn.nodeURL,
		"leader":    strconv.FormatBool(dn.IsLeader()),
		"stopped":   strconv.FormatBool(dn.IsStopped()),
	}

	rp := map[uint64]uint64{}
	dn.tshards.Range(func(key, val interface{}) bool {
		replID := key.(uint64)
		tState := val.(*TshardState)
		rp[replID] = tState.shardID
		return true
	})

	rpj, err := json.Marshal(rp)
	if err == nil {
		dbg["replicas"] = string(rpj)
	}

	dbgj, err := json.Marshal(dbg)
	if err != nil {
		return "{}"
	}

	return string(dbgj)
}
