// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

import (
	"context"
	"encoding/json"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// this file contains code for metadata mgr that manages cluster metadata

// MetadataMgr is the metadata manager instance
type MetadataMgr struct {
	sync.Mutex
	waitGrp        sync.WaitGroup             // wait group to wait on all go routines to exit
	clusterType    string                     // type of the cluster
	nodeUUID       string                     // my id
	kvs            kvstore.Interface          // kvstore client
	cluster        TscaleCluster              // metadata cluster state
	nodes          map[string]*TscaleNodeInfo // list of all nodes
	rpcClients     sync.Map                   // rpc connections, map[string]*rpckit.RPCClient
	rebalnceCancel context.CancelFunc         // stop rebalance thread
	cfg            ClusterConfig              // cluster config
}

// DefaultRPCTimeout is the default RPC call timeout
const DefaultRPCTimeout = time.Second * 5

// NewMetadataMgr returns a new metadatamgr
func NewMetadataMgr(clusterType, nodeUUID string, kvs kvstore.Interface, cfg *ClusterConfig) (*MetadataMgr, error) {
	// create the metadatamgr instance
	mgr := MetadataMgr{
		clusterType: clusterType,
		nodeUUID:    nodeUUID,
		kvs:         kvs,
		cluster: TscaleCluster{
			TypeMeta:   api.TypeMeta{Kind: "TscaleCluster"},
			ObjectMeta: api.ObjectMeta{Name: clusterType},
			NodeMap:    make(map[string]*NodeState),
		},
		nodes:      make(map[string]*TscaleNodeInfo),
		rpcClients: sync.Map{},
		cfg:        *cfg,
	}

	// set metadata manager as the mclient
	mgr.cluster.mclient = &mgr

	// create new shard map
	smap, err := NewShardMap(cfg.NumShards, cfg.DesiredReplicas, &mgr.cluster)
	if err != nil {
		return nil, err
	}
	mgr.cluster.ShardMap = smap

	// start the metadata mgr
	err = mgr.start()
	if err != nil {
		return nil, err
	}

	return &mgr, nil
}

// Stop stops the metadata mgr
func (md *MetadataMgr) Stop() error {
	md.rebalnceCancel()
	md.waitGrp.Wait()

	// close the rpc clients
	md.rpcClients.Range(func(k, v interface{}) bool {
		if rc, ok := v.(*rpckit.RPCClient); ok {
			rc.Close()
		}
		return true
	})

	return nil
}

// WriteMetadata writes the shard to kvstore
func (md *MetadataMgr) WriteMetadata(cluster *TscaleCluster) error {
	// FIXME: improve this
	return md.updateCluster()
}

// SetPrimaryReplica informs data node that replica became primary
func (md *MetadataMgr) SetPrimaryReplica(repl *Replica) error {
	log.Infof("%s Setting replica %d on node %s as primary for shard %d", md.clusterType, repl.ReplicaID, repl.NodeUUID, repl.shard.ShardID)

	// grpc connect
	grpcClient, err := md.getRPCClient(repl.NodeUUID)
	if err != nil {
		log.Errorf("Error getting rpc client to %s. Err: %v", repl.NodeUUID, err)
		return err
	}

	// collect all replicas in this shard
	var replicaList []*tproto.ReplicaInfo
	for _, sr := range repl.shard.Replicas {
		// get node for url
		node, nerr := md.cluster.GetNode(sr.NodeUUID)
		if nerr != nil {
			return nerr
		}

		// build replica info
		rpinfo := tproto.ReplicaInfo{
			ClusterType: md.clusterType,
			ReplicaID:   sr.ReplicaID,
			ShardID:     sr.ShardID,
			IsPrimary:   sr.IsPrimary,
			NodeUUID:    sr.NodeUUID,
			NodeURL:     node.NodeURL,
		}

		replicaList = append(replicaList, &rpinfo)
	}

	// updated shard info
	req := tproto.ShardReq{
		ClusterType: md.clusterType,
		ReplicaID:   repl.ReplicaID,
		ShardID:     repl.ShardID,
		IsPrimary:   repl.IsPrimary,
		Replicas:    replicaList,
	}

	// make the rpc call to set the shard as primary
	dnclient := tproto.NewDataNodeClient(grpcClient)
	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	_, err = dnclient.UpdateShard(ctx, &req)
	if err != nil {
		log.Errorf("Error making UpdateShard  rpc call to node %s. Err: %v", repl.NodeUUID, err)
		return err
	}

	// loop thru all the other replicas and send them update too..
	for _, sr := range repl.shard.Replicas {
		if sr.ReplicaID != repl.ReplicaID && md.cluster.IsNodeAlive(sr.NodeUUID) {

			log.Infof("%s Sending replica %+v to node %s", md.clusterType, sr, sr.NodeUUID)

			// grpc connect
			grpcClient, err := md.getRPCClient(sr.NodeUUID)
			if err != nil {
				log.Errorf("Error getting rpc client to %s. Err: %v", sr.NodeUUID, err)
				return err
			}

			// updated shard info
			req := tproto.ShardReq{
				ClusterType: md.clusterType,
				ReplicaID:   sr.ReplicaID,
				ShardID:     sr.ShardID,
				IsPrimary:   sr.IsPrimary,
				Replicas:    replicaList,
			}

			// make the rpc call to set the shard as primary
			dnclient := tproto.NewDataNodeClient(grpcClient)
			ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
			defer cancel()
			_, err = dnclient.UpdateShard(ctx, &req)
			if err != nil {
				log.Errorf("Error making UpdateShard  rpc call to node %s. Err: %v", sr.NodeUUID, err)
				return err
			}
		}
	}

	return nil
}

// CopyDataFromPrimaryReplica asks a secondary node to copy data from primary node
func (md *MetadataMgr) CopyDataFromPrimaryReplica(prepl *Replica, serepl *Replica) error {
	log.Infof("%s Copying data from primary shard %d on node %s to secondary %d on node %s", md.clusterType, prepl.ReplicaID, prepl.NodeUUID, serepl.ReplicaID, serepl.NodeUUID)

	// grpc connect to primary replica
	grpcClient, err := md.getRPCClient(prepl.NodeUUID)
	if err != nil {
		log.Errorf("Error getting rpc client to %s. Err: %v", prepl.NodeUUID, err)
		return err
	}

	// find the destination node
	dnode, err := md.cluster.GetNode(serepl.NodeUUID)
	if err != nil {
		log.Errorf("Could not find the node %s. Err: %v", serepl.NodeUUID, err)
		return err
	}

	// collect all replicas in this shard
	var replicaList []*tproto.ReplicaInfo
	for _, sr := range prepl.shard.Replicas {
		// get node for url
		node, nerr := md.cluster.GetNode(sr.NodeUUID)
		if nerr != nil {
			return nerr
		}

		// build replica info
		rpinfo := tproto.ReplicaInfo{
			ClusterType: md.clusterType,
			ReplicaID:   sr.ReplicaID,
			ShardID:     sr.ShardID,
			IsPrimary:   sr.IsPrimary,
			NodeUUID:    sr.NodeUUID,
			NodeURL:     node.NodeURL,
		}

		replicaList = append(replicaList, &rpinfo)
	}

	// shard sync req
	req := tproto.SyncShardMsg{
		ClusterType:   md.clusterType,
		SrcReplicaID:  prepl.ReplicaID,
		DestReplicaID: serepl.ReplicaID,
		ShardID:       prepl.shard.ShardID,
		DestNodeUUID:  serepl.NodeUUID,
		DestNodeURL:   dnode.NodeURL,
		SrcIsPrimary:  prepl.IsPrimary,
		Replicas:      replicaList,
	}

	// make the rpc call to copy data from primary to secondary
	dnclient := tproto.NewDataNodeClient(grpcClient)
	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	_, err = dnclient.SyncShardReq(ctx, &req)
	if err != nil {
		log.Errorf("Error making SyncShardReq rpc call to node %s. Err: %v", prepl.NodeUUID, err)
		return err
	}

	return nil
}

// start the metadata mgr
func (md *MetadataMgr) start() error {
	// read the current cluster state
	err := md.kvs.Get(context.Background(), ClusterMetastoreURL+md.clusterType, &md.cluster)
	if err != nil && !kvstore.IsKeyNotFoundError(err) {
		log.Errorf("Error reading cluster state. Err: %v", err)
		return err
	} else if kvstore.IsKeyNotFoundError(err) {
		// create the cluster if it doesnt exist
		err = md.createCluster()
		if err != nil {
			log.Errorf("Error creating the cluster")
			return err
		}
	} else {
		log.Infof("%s New leader %s is resuming from cluster state: %+v", md.clusterType, md.nodeUUID, md.cluster.NodeMap)

		// restore the shard state
		md.cluster.ShardMap.cluster = &md.cluster
		for _, shard := range md.cluster.ShardMap.Shards {
			for _, sr := range shard.Replicas {
				sr.shard = shard
			}
		}
	}

	// read all nodes
	var nodeList NodeList
	err = md.kvs.List(context.Background(), NodesMetastoreURL, &nodeList)
	if err != nil {
		log.Errorf("Error getting node list. Err: %v", err)
		// continue even if there are errors
	} else {
		log.Infof("%s Leader got node list: %+v", md.clusterType, &nodeList)

		// if cluster has a node that doesnt exist in the nodelist, mark it unreachable
		for _, enode := range md.cluster.NodeMap {
			if !nodeList.HasNode(enode.NodeUUID) {
				err = md.cluster.NodeUnreachable(enode.NodeUUID)
				if err != nil {
					log.Errorf("Error marking node %s unreachable. Err: %v", enode.NodeUUID, err)
				}
			}
		}
		// add the nodes to the db
		for _, node := range nodeList.Items {
			err = md.addNode(node)
			if err != nil {
				log.Errorf("Error adding node %s. Err: %v", node.NodeUUID, err)
			}
		}
	}

	ctx, cancel := context.WithCancel(context.Background())
	md.rebalnceCancel = cancel

	// run a rebalancing loop
	md.waitGrp.Add(1)
	go md.rebalanceLoop(ctx)

	return nil
}

// addNode adds a node to the cluster
// FIXME: re-evaluate the locking requirements here. We are modifying cluster state
// from different gorouting than rebalance loop
func (md *MetadataMgr) addNode(node *TscaleNodeInfo) error {
	log.Infof("%s Adding node to nodemap %+v", md.clusterType, node)

	// lock metadatamgr from doing other things
	md.Lock()
	defer md.Unlock()

	// just add it to db, rebalancing loop will pick it up
	_, err := md.cluster.AddNode(node.NodeUUID, node.NodeURL)
	if err != nil {
		log.Errorf("Error adding node %s. Err: %v", node.NodeUUID, err)
		return err
	}

	// update cluster
	return md.updateCluster()
}

// delNode deletes a node from the cluster
// FIXME: re-evaluate the locking requirements here. We are modifying cluster state
// from different gorouting than rebalance loop
func (md *MetadataMgr) delNode(node *TscaleNodeInfo) error {
	log.Infof("%s node %+v is unreachable. Cluster: %+v", md.clusterType, node, &md.cluster)

	// lock metadatamgr from doing other things
	md.Lock()
	defer md.Unlock()

	// mark the node as down and switch all primary nodes
	err := md.cluster.NodeUnreachable(node.NodeUUID)
	if err != nil {
		log.Errorf("Error marking node %s unreachable. Err: %v", node.NodeUUID, err)
		return err
	}

	// remove rpc client
	md.rpcClients.Delete(node.NodeUUID)

	// update cluster
	return md.updateCluster()
}

// createCluster inits the cluster
// FIXME: re-evaluate this
func (md *MetadataMgr) createCluster() error {
	log.Infof("Creating cluster %s with nodes: %+v", md.cluster.Name, md.cluster.NodeMap)
	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	return md.kvs.Create(ctx, ClusterMetastoreURL+md.clusterType, &md.cluster)
}

// updateCluster writes cluster info to kvstore
// FIXME: re-evaluate this func
func (md *MetadataMgr) updateCluster() error {
	md.cluster.Lock()
	smstr, _ := json.Marshal(md.cluster.ShardMap)
	nstr, _ := json.Marshal(md.cluster.NodeMap)
	md.cluster.Unlock()
	log.Debugf("Updating cluster %s with \n Shardmap: %s \n Nodemap: %s", md.cluster.Name, string(smstr), string(nstr))

	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	return md.kvs.Update(ctx, ClusterMetastoreURL+md.clusterType, &md.cluster)
}

// getRPCClient returns the rpc client for a node
func (md *MetadataMgr) getRPCClient(nodeUUID string) (*grpc.ClientConn, error) {
	// see if we already have an rpc client for this node
	// FIXME: we should check if rpc client is still connected and retry
	if v, ok := md.rpcClients.Load(nodeUUID); ok {
		if rclient, ok := v.(*rpckit.RPCClient); ok {
			if rclient.ClientConn != nil {
				return rclient.ClientConn, nil
			}
		}
	}

	// get node info for primary shard
	node, err := md.cluster.GetNode(nodeUUID)
	if err != nil {
		log.Errorf("Could not find the node %s. Err: %v", nodeUUID, err)
		return nil, err
	}

	// dial the connection
	rclient, err := rpckit.NewRPCClient(fmt.Sprintf("metanode-%s", md.nodeUUID), node.NodeURL, rpckit.WithLoggerEnabled(false), rpckit.WithRemoteServerName(globals.Citadel))
	if err != nil {
		log.Errorf("Error connecting to rpc server %s. err: %v", node.NodeURL, err)
		return nil, err
	}

	// save the rpc client for future use
	md.rpcClients.Store(node.NodeUUID, rclient)

	return rclient.ClientConn, nil
}

/* Rebalance v2 rough sketch:
 * 1. Get a list of shards with numReplicas < desiredReplicas
 *  1.2. create new replica on different node from all other replica (pick nodes based on least-used criteria)
 *  1.3. Add new replica to shard
 *  1.4 sync data from primary replica to new replica
 * 2. Get most used and least used nodes and see if they differ by atleast two shards
 *  2.1 If they do, move a replica from most used node to least used node
 * 3. Scan all nodes in node map and see if they were unreachable for longer than dead-interval
 *  3.1 If they are, remove the replicas on those nodes and go back to step (1) which will create new replicas
 */
func (md *MetadataMgr) rebalanceLoop(ctx context.Context) error {
	defer md.waitGrp.Done()

	// start after one second delay after this node becomes the leader
	time.Sleep(md.cfg.RebalanceDelay)

	// loop forever
	for {
		select {
		case <-ctx.Done():
			log.Warnf("%s Stopping rebalance loop", md.nodeUUID)
			return nil
		default:
			time.Sleep(md.cfg.RebalanceInterval)

			log.Debugf("%s Running rebalancing loop for cluster %s", md.nodeUUID, md.clusterType)

			// check if any shard has less replicas than desired
			md.checkNumReplicas()

			// check if any nodes needs to be rebalanced
			md.rebalanceNodes()

			// check if any node is down for longer than dead interval
			md.checkUnreachableNodes()

			// do some error checking. If nodemap is empty, we are done
			if len(md.cluster.NodeMap) == 0 {
				log.Warnf("%s Exiting rebalance loop since nodemap is empty", md.nodeUUID)
				return nil
			}
		}
	}
}

// checkNumReplicas checks if any shard has less replicas than desired
func (md *MetadataMgr) checkNumReplicas() error {
	// lock metadatamgr from doing other things
	md.Lock()
	defer md.Unlock()

	// scan all shards and see if any of them have less replicas than desired
	for _, shard := range md.cluster.ShardMap.Shards {
		if shard.NumReplicas < md.cluster.ShardMap.DesiredReplicas && len(md.cluster.GetActiveNodes()) > 0 {
			// create a closure so that error handling is easier..
			func() {
				// if shard has one replica already and we have only one node, we are done
				if shard.NumReplicas > 0 && len(md.cluster.GetActiveNodes()) == 1 {
					return
				}

				// find the least used node to create a replica
				luNode := md.cluster.GetLeastUsedNodeForShard(shard)
				if luNode == nil {
					log.Errorf("%s: Error getting least used node from cluster", md.nodeUUID)
					shstr, _ := json.Marshal(shard)
					nstr, _ := json.Marshal(md.cluster.NodeMap)
					log.Errorf("Shard: %s,\nNodemap: %s ", shstr, nstr)
					return
				}

				log.Infof("%s/%s Creating replica for shard %d on node %s", md.clusterType, md.nodeUUID, shard.ShardID, luNode.NodeUUID)

				// create the replica
				_, err := md.createNewReplica(shard, luNode.NodeUUID)
				if err != nil {
					log.Errorf("Error creating new replica. Err: %v", err)
					return
				}
			}()
		}
	}

	return nil
}

// rebalanceNodes checks if any node is unbalanced and rebalances them
func (md *MetadataMgr) rebalanceNodes() error {
	// lock metadatamgr from doing other things
	md.Lock()
	defer md.Unlock()

	// get most used and least used nodes and see if they differ by atleast two
	moNode := md.cluster.GetMostUsedNode()
	luNode := md.cluster.GetLeastUsedNode()
	if moNode != nil && luNode != nil && (moNode.NumShards-luNode.NumShards) >= 2 {
		// pick a shard on this node that doesnt have a replica on target node
		movingShard, err := md.cluster.FindMoveCandidateOnNode(moNode, luNode.NodeUUID)
		if err != nil {
			log.Errorf("Could not find a shard to move on node %s. Err: %v", moNode.NodeUUID, err)
			return err
		}

		log.Infof("%s/%s Moving replica %d/%d from node %s to %s", md.clusterType, md.nodeUUID, movingShard.ShardID, movingShard.ReplicaID, movingShard.NodeUUID, luNode.NodeUUID)

		// create a new replica (this will sync data from primary)
		_, err = md.createNewReplica(movingShard.shard, luNode.NodeUUID)
		if err != nil {
			log.Errorf("Error creating new replica. Err: %v", err)
			return err
		}

		// remove replica
		err = md.removeReplica(movingShard, movingShard.NodeUUID)
		if err != nil {
			log.Errorf("Error removing the replica %+v. Err: %v", movingShard, err)
			return err
		}
	}

	return nil
}

// checkUnreachableNodes checks if any unreachable node has exceeded dead interval
func (md *MetadataMgr) checkUnreachableNodes() error {
	// lock metadatamgr from doing other things
	md.Lock()
	defer md.Unlock()

	// see if any node is down for longer than dead interval
	for _, node := range md.cluster.NodeMap {
		if node.NodeStatus == NodeStatusUnreachable {
			// check if its been down for longer than dead interval
			unreach, _ := time.Parse(time.RFC3339Nano, node.UnreachableSince)
			if time.Since(unreach) > md.cfg.DeadInterval {
				log.Infof("%s/%s Node %s was unreachable for %v since %s. Removing it", md.clusterType, md.nodeUUID, node.NodeUUID, time.Since(unreach), node.UnreachableSince)

				// remove the node
				err := md.cluster.RemoveNode(node.NodeUUID)
				if err != nil {
					log.Errorf("Error removing the node %+v. Err: %v", node, err)
				}

				// write cluster update
				md.updateCluster()
			}
		}
	}

	return nil
}

// createNewReplica creates new replica
func (md *MetadataMgr) createNewReplica(shard *Shard, nodeUUID string) (*Replica, error) {
	// create a replica
	repl, err := md.cluster.ShardMap.NewReplica(nodeUUID)
	if err != nil {
		log.Errorf("Error creating the replica. Err: %v", err)
		return nil, err
	}

	// tell the node about new replica
	grpcClient, err := md.getRPCClient(nodeUUID)
	if err != nil {
		log.Errorf("Error getting rpc client to %s. Err: %v", nodeUUID, err)
		return nil, err
	}

	// collect all replicas in this shard
	var replicaList []*tproto.ReplicaInfo
	for _, sr := range shard.Replicas {
		// get node for url
		node, nerr := md.cluster.GetNode(sr.NodeUUID)
		if nerr != nil {
			return nil, nerr
		}

		// build replica info
		rpinfo := tproto.ReplicaInfo{
			ClusterType: md.clusterType,
			ReplicaID:   sr.ReplicaID,
			ShardID:     sr.ShardID,
			IsPrimary:   sr.IsPrimary,
			NodeUUID:    sr.NodeUUID,
			NodeURL:     node.NodeURL,
		}

		replicaList = append(replicaList, &rpinfo)
	}

	// create shard request
	req := tproto.ShardReq{
		ClusterType: md.clusterType,
		ReplicaID:   repl.ReplicaID,
		ShardID:     shard.ShardID,
		IsPrimary:   repl.IsPrimary,
		Replicas:    replicaList,
	}

	// make the rpc call to create the replica
	dnclient := tproto.NewDataNodeClient(grpcClient)
	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	_, err = dnclient.CreateShard(ctx, &req)
	if err != nil {
		return nil, err
	}

	// add the replica to the shard
	err = shard.AddReplicas([]*Replica{repl})
	if err != nil {
		log.Errorf("Error adding replica %+v to shard %d. Err: %v, shard:%+v ", repl, shard.ShardID, err, shard)
		return nil, err
	}

	// add shard to node
	err = md.cluster.AddReplica(repl)
	if err != nil {
		return nil, err
	}

	// write cluster update
	md.updateCluster()

	return repl, nil
}

// removeReplica removes a replica from shard and node
func (md *MetadataMgr) removeReplica(repl *Replica, nodeUUID string) error {
	// remove the candidate replica from shard
	err := repl.shard.RemoveReplica(repl)
	if err != nil {
		log.Errorf("Error removing shard %+v. Err: %v", repl, err)
		return err
	}

	// remove the shard from node
	err = md.cluster.RemoveReplica(repl)
	if err != nil {
		log.Errorf("Error removing shard %+v from node. Err: %v", repl, err)
		return err
	}

	// Send a message to remove the replica in background after a delay
	go md.sendRemoveReplica(repl, nodeUUID)

	// write cluster update
	return md.updateCluster()
}

// sendRemoveReplica sends remove replica message for a replica and all other replicas in the shard
func (md *MetadataMgr) sendRemoveReplica(repl *Replica, nodeUUID string) {
	// wait a little before removing old shard so that information is synced to everyone
	time.Sleep(md.cfg.RebalanceInterval)

	// tell the node about new shard
	grpcClient, err := md.getRPCClient(nodeUUID)
	if err != nil {
		log.Errorf("Error getting rpc client to %s. Err: %v", nodeUUID, err)
		return
	}

	// delete shard request
	req := tproto.ShardReq{
		ClusterType: md.clusterType,
		ReplicaID:   repl.ReplicaID,
		ShardID:     repl.shard.ShardID,
	}

	// make the rpc call to create the shard
	dnclient := tproto.NewDataNodeClient(grpcClient)
	ctx, cancel := context.WithTimeout(context.Background(), DefaultRPCTimeout)
	defer cancel()
	_, err = dnclient.DeleteShard(ctx, &req)
	if err != nil {
		log.Errorf("Error deleting replica %d of shard %d from node %s. Err: %v", repl.ReplicaID, repl.shard.ShardID, nodeUUID, err)
	}

	// collect all replicas in this shard
	var replicaList []*tproto.ReplicaInfo
	for _, sr := range repl.shard.Replicas {
		// get node for url
		node, nerr := md.cluster.GetNode(sr.NodeUUID)
		if nerr != nil {
			continue
		}

		// build replica info
		rpinfo := tproto.ReplicaInfo{
			ClusterType: md.clusterType,
			ReplicaID:   sr.ReplicaID,
			ShardID:     sr.ShardID,
			IsPrimary:   sr.IsPrimary,
			NodeUUID:    sr.NodeUUID,
			NodeURL:     node.NodeURL,
		}

		replicaList = append(replicaList, &rpinfo)
	}

	// walk all the other replicas in this shard and tell them about removed replica
	for _, re := range repl.shard.Replicas {
		// create shard request
		ureq := tproto.ShardReq{
			ClusterType: md.clusterType,
			ReplicaID:   re.ReplicaID,
			ShardID:     repl.shard.ShardID,
			IsPrimary:   re.IsPrimary,
			Replicas:    replicaList,
		}

		grc, err := md.getRPCClient(re.NodeUUID)
		if err != nil {
			log.Errorf("Error getting rpc client to %s. Err: %v", re.NodeUUID, err)
			continue
		}

		// make the rpc call to create the replica
		dnclient := tproto.NewDataNodeClient(grc)
		ctxre, cancelre := context.WithTimeout(context.Background(), DefaultRPCTimeout)
		defer cancelre()
		_, err = dnclient.UpdateShard(ctxre, &ureq)
		if err != nil {
			log.Errorf("%s: Error sending UpdateShard for replica %d shrd %d to node %s. Err: %v", md.nodeUUID, re.ReplicaID, re.shard.ShardID, re.NodeUUID, err)
		}
	}
}
