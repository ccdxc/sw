// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

import (
	"context"
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// createNode creates a node
func createNode(nodeUUID, nodeURL, dbPath string) (*DNode, error) {
	// metadata config to disable metadata mgr
	cfg := meta.DefaultClusterConfig()
	cfg.EnableKstoreMeta = false
	cfg.EnableTstoreMeta = false

	// create the data node
	dn, err := NewDataNode(cfg, nodeUUID, nodeURL, dbPath)
	if err != nil {
		return nil, err
	}

	// stop the metadata service for this test
	dn.metaNode.Stop()

	return dn, nil
}

func TestDataNodeBasic(t *testing.T) {
	const numNodes = 3
	dnodes := make([]*DNode, numNodes)
	clients := make([]*rpckit.RPCClient, numNodes)
	var err error

	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		dnodes[idx], err = createNode(fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")
		defer dnodes[idx].Stop()
	}

	// create rpc client
	for idx := 0; idx < numNodes; idx++ {
		clients[idx], err = rpckit.NewRPCClient(fmt.Sprintf("datanode-%d", idx), fmt.Sprintf("localhost:730%d", idx), rpckit.WithLoggerEnabled(false), rpckit.WithTLSProvider(nil))
		AssertOk(t, err, "Error connecting to grpc server")
		defer clients[idx].Close()
	}

	// make create shard api call
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.ShardReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			IsPrimary:   true,
		}

		// create tstore shard
		resp, err := dnclient.CreateShard(context.Background(), &req)
		AssertOk(t, err, "Error making the create shard call")
		Assert(t, resp.Status == "", "Invalid resp to create shard req", resp)

		// create kstore shard
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.CreateShard(context.Background(), &req)
		AssertOk(t, err, "Error making the create shard call")
	}

	// make create database call
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
		}

		_, err := dnclient.CreateDatabase(context.Background(), &req)
		AssertOk(t, err, "Error making the create database call")
	}

	// write some points
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		data := "cpu,host=serverB,svc=nginx value1=11,value2=12 10\n" +
			"cpu,host=serverC,svc=nginx value1=21,value2=22  20\n"

		req := tproto.PointsWriteReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
			Points:      data,
		}

		_, err := dnclient.PointsWrite(context.Background(), &req)
		AssertOk(t, err, "Error writing points")
	}

	// execute some query
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.QueryReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
			TxnID:       uint64(idx + 1),
			Query:       "SELECT * FROM cpu",
		}

		resp, err := dnclient.ExecuteQuery(context.Background(), &req)
		AssertOk(t, err, "Error execuitng query")
		log.Infof("Got query resp: %+v", resp)
	}

	// make delete database call
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
		}

		_, err := dnclient.DeleteDatabase(context.Background(), &req)
		AssertOk(t, err, "Error making the delete database call")
	}

	// write some keys
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		kvl := []*tproto.KeyValue{
			{
				Key:   []byte("testKey1"),
				Value: []byte("testValue1"),
			},
			{
				Key:   []byte("testKey2"),
				Value: []byte("testValue2"),
			},
		}

		// build the request
		req := tproto.KeyValueMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Kvs:         kvl,
		}

		_, err := dnclient.WriteReq(context.Background(), &req)
		AssertOk(t, err, "Error writing kvs")
	}

	// read a key back
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		// keys to get
		kl := []*tproto.Key{
			{
				Key: []byte("testKey1"),
			},
			{
				Key: []byte("testKey2"),
			},
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Keys:        kl,
		}

		resp, err := dnclient.ReadReq(context.Background(), &req)
		AssertOk(t, err, "Error reading the keys back")
		log.Infof("Gor read response: %+v", resp)
	}

	// list all keys in a table
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.TableMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
		}

		resp, err := dnclient.ListReq(context.Background(), &req)
		AssertOk(t, err, "Error listing the keys")
		log.Infof("Gor read response: %+v", resp)
	}

	// delete the keys
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		// keys to delete
		kl := []*tproto.Key{
			{
				Key: []byte("testKey1"),
			},
			{
				Key: []byte("testKey2"),
			},
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Keys:        kl,
		}

		_, err := dnclient.DelReq(context.Background(), &req)
		AssertOk(t, err, "Error deleting the keys")
	}
}

func TestDataNodeErrors(t *testing.T) {
	const numNodes = 3
	dnodes := make([]*DNode, numNodes)
	clients := make([]*rpckit.RPCClient, numNodes)
	var err error

	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		dnodes[idx], err = createNode(fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")
		defer dnodes[idx].Stop()
	}

	// create rpc client
	for idx := 0; idx < numNodes; idx++ {
		clients[idx], err = rpckit.NewRPCClient(fmt.Sprintf("datanode-%d", idx), fmt.Sprintf("localhost:730%d", idx), rpckit.WithLoggerEnabled(false), rpckit.WithTLSProvider(nil))
		AssertOk(t, err, "Error connecting to grpc server")
		defer clients[idx].Close()
	}

	// make create shard api call
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.ShardReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			IsPrimary:   true,
		}

		// create tstore shard
		resp, err := dnclient.CreateShard(context.Background(), &req)
		AssertOk(t, err, "Error making the create shard call")
		Assert(t, resp.Status == "", "Invalid resp to create shard req", resp)

		// create kstore shard
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.CreateShard(context.Background(), &req)
		AssertOk(t, err, "Error making the create shard call")
	}

	// update & delete shard call with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.ShardReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   300,
			IsPrimary:   true,
		}

		// send update shard
		_, err := dnclient.UpdateShard(context.Background(), &req)
		Assert(t, (err != nil), "update shard with invalid params suceeded")
		_, err = dnclient.DeleteShard(context.Background(), &req)
		Assert(t, (err != nil), "delete shard with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.UpdateShard(context.Background(), &req)
		Assert(t, (err != nil), "update shard with invalid params suceeded")
		_, err = dnclient.DeleteShard(context.Background(), &req)
		Assert(t, (err != nil), "delete shard with invalid params suceeded")
	}

	// sync shard messages with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.SyncShardMsg{
			ClusterType:  meta.ClusterTypeTstore,
			SrcReplicaID: 300,
		}

		// send the message
		_, err := dnclient.SyncShardReq(context.Background(), &req)
		Assert(t, (err != nil), "SyncShardReq with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.SyncShardReq(context.Background(), &req)
		Assert(t, (err != nil), "SyncShardReq with invalid params suceeded")
	}

	// sync shard info messages with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.SyncShardInfoMsg{
			ClusterType:   meta.ClusterTypeTstore,
			SrcReplicaID:  300,
			DestReplicaID: 300,
		}

		// send the message
		_, err := dnclient.SyncShardInfo(context.Background(), &req)
		Assert(t, (err != nil), "SyncShardInfo with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.SyncShardInfo(context.Background(), &req)
		Assert(t, (err != nil), "SyncShardInfo with invalid params suceeded")
	}

	// create database with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
		}

		_, err := dnclient.CreateDatabase(context.Background(), &req)
		Assert(t, (err != nil), "create database with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeTstore
		req.ReplicaID = 100
		_, err = dnclient.CreateDatabase(context.Background(), &req)
		Assert(t, (err != nil), "create database with invalid params suceeded")
		_, err = dnclient.DeleteDatabase(context.Background(), &req)
		Assert(t, (err != nil), "delete database with invalid params suceeded")
	}

	// write points with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		data := "cpu,host=serverB,svc=nginx value1=11,value2=12 10\n" +
			"cpu,host=serverC,svc=nginx value1=21,value2=22  20\n"

		req := tproto.PointsWriteReq{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
			Points:      data,
		}

		_, err := dnclient.PointsWrite(context.Background(), &req)
		Assert(t, (err != nil), "points write with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeTstore
		req.ReplicaID = 300
		_, err = dnclient.PointsWrite(context.Background(), &req)
		Assert(t, (err != nil), "points write with invalid params suceeded")
	}

	// execute some query
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.QueryReq{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   uint64(idx + 1),
			Database:    "db0",
			TxnID:       uint64(idx + 1),
			Query:       "SELECT * FROM cpu",
		}

		_, err := dnclient.ExecuteQuery(context.Background(), &req)
		Assert(t, (err != nil), "query with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeTstore
		req.ReplicaID = 300
		_, err = dnclient.ExecuteQuery(context.Background(), &req)
		Assert(t, (err != nil), "query with invalid params suceeded")
	}

	// write keys with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		kvl := []*tproto.KeyValue{
			{
				Key:   []byte("testKey1"),
				Value: []byte("testValue1"),
			},
			{
				Key:   []byte("testKey2"),
				Value: []byte("testValue2"),
			},
		}

		// build the request
		req := tproto.KeyValueMsg{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Kvs:         kvl,
		}

		_, err := dnclient.WriteReq(context.Background(), &req)
		Assert(t, (err != nil), "kv write with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		req.ReplicaID = 300
		_, err = dnclient.WriteReq(context.Background(), &req)
		Assert(t, (err != nil), "kv write with invalid params suceeded")
	}

	// read a key with invalid params
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		// keys to get
		kl := []*tproto.Key{
			{
				Key: []byte("testKey1"),
			},
			{
				Key: []byte("testKey2"),
			},
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Keys:        kl,
		}

		_, err := dnclient.ReadReq(context.Background(), &req)
		Assert(t, (err != nil), "kv read with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		req.Table = "table1"
		_, err = dnclient.ReadReq(context.Background(), &req)
		Assert(t, (err != nil), "kv read with invalid params suceeded")
	}

	// list all keys in a table
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		req := tproto.TableMsg{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
		}

		resp, err := dnclient.ListReq(context.Background(), &req)
		Assert(t, (err != nil), "kv list with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		resp, err = dnclient.ListReq(context.Background(), &req)
		AssertOk(t, err, "Error listing table")
		Assert(t, (len(resp.Kvs) == 0), "Got invalid respons for non-existing table", resp)
	}

	// delete the keys
	for idx := 0; idx < numNodes; idx++ {
		dnclient := tproto.NewDataNodeClient(clients[idx].ClientConn)

		// keys to delete
		kl := []*tproto.Key{
			{
				Key: []byte("testKey1"),
			},
			{
				Key: []byte("testKey2"),
			},
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   uint64(idx + 1),
			ShardID:     uint64(idx + 1),
			Table:       "table0",
			Keys:        kl,
		}

		_, err := dnclient.DelReq(context.Background(), &req)
		Assert(t, (err != nil), "kv delete with invalid params suceeded")
		req.ClusterType = meta.ClusterTypeKstore
		_, err = dnclient.DelReq(context.Background(), &req)
		Assert(t, (err != nil), "kv delete with non-existing keys suceeded")
	}
}

func TestDataNodeTstoreClustering(t *testing.T) {
	const numNodes = 3
	dnodes := make([]*DNode, numNodes)
	clients := make([]*rpckit.RPCClient, numNodes)
	var err error

	// metadata config
	cfg := meta.DefaultClusterConfig()
	cfg.EnableKstoreMeta = false
	cfg.DeadInterval = time.Millisecond * 100
	cfg.NodeTTL = 5
	cfg.RebalanceDelay = time.Millisecond * 100
	cfg.RebalanceInterval = time.Millisecond * 10

	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")
	}

	// create rpc client
	for idx := 0; idx < numNodes; idx++ {
		clients[idx], err = rpckit.NewRPCClient(fmt.Sprintf("datanode-%d", idx), fmt.Sprintf("localhost:730%d", idx), rpckit.WithLoggerEnabled(false), rpckit.WithTLSProvider(nil))
		AssertOk(t, err, "Error connecting to grpc server")
		defer clients[idx].Close()
	}

	watcher, err := meta.NewWatcher("watcher", cfg)
	AssertOk(t, err, "Error creating the watcher")
	defer watcher.Stop()

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeTstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, nil
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeTstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, nil
			}

			if dnodes[idx].HasPendingSync() {
				return false, nil
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")

	// make create database call
	cl := dnodes[0].GetCluster(meta.ClusterTypeTstore)
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			dnclient, rerr := dnodes[0].getDnclient(meta.ClusterTypeTstore, repl.NodeUUID)
			AssertOk(t, rerr, "Error getting datanode client")

			req := tproto.DatabaseReq{
				ClusterType: meta.ClusterTypeTstore,
				ReplicaID:   repl.ReplicaID,
				ShardID:     repl.ShardID,
				Database:    "db0",
			}

			_, err = dnclient.CreateDatabase(context.Background(), &req)
			AssertOk(t, err, "Error making the create database call")
		}
	}

	// write some points
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			if repl.IsPrimary {

				dnclient, rerr := dnodes[0].getDnclient(meta.ClusterTypeTstore, repl.NodeUUID)
				AssertOk(t, rerr, "Error getting datanode client")

				data := "cpu,host=serverB,svc=nginx value1=11,value2=12 10\n" +
					"cpu,host=serverC,svc=nginx value1=21,value2=22  20\n"

				req := tproto.PointsWriteReq{
					ClusterType: meta.ClusterTypeTstore,
					ReplicaID:   repl.ReplicaID,
					ShardID:     repl.ShardID,
					Database:    "db0",
					Points:      data,
				}

				_, err = dnclient.PointsWrite(context.Background(), &req)
				AssertOk(t, err, "Error writing points")
			}
		}
	}

	// do a quick statefull restart
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("############### Restarting(quick) node %s", fmt.Sprintf("node-%d", idx))
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")

		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")
	}

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeTstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, nil
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeTstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, nil
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")

	// do a soft restart without expiring the lease
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("############### Restarting(soft) node %s", fmt.Sprintf("node-%d", idx))
		err = dnodes[idx].SoftRestart()
		AssertOk(t, err, "Error softrestarting data node")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")
	}

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeTstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, nil
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeTstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, nil
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")

	// restart each of the nodes
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("################## Restarting(slow) node %s", fmt.Sprintf("node-%d", idx))

		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
		time.Sleep(cfg.DeadInterval * 3)

		AssertEventually(t, func() (bool, interface{}) {
			if len(watcher.GetCluster(meta.ClusterTypeTstore).NodeMap) != (numNodes - 1) {
				return false, nil
			}

			return true, nil
		}, "node was not removed from cluster", "100ms", "30s")

		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")

		// wait till cluster state has converged
		AssertEventually(t, func() (bool, interface{}) {
			if (len(watcher.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes) ||
				(len(watcher.GetCluster(meta.ClusterTypeTstore).ShardMap.Shards) != meta.DefaultShardCount) {
				return false, nil
			}

			for idx := 0; idx < numNodes; idx++ {
				if watcher.GetCluster(meta.ClusterTypeTstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
					return false, nil
				}
			}

			return true, nil
		}, "nodes did not get cluster update", "100ms", "30s")
	}

	for idx := 0; idx < numNodes; idx++ {
		// stop the data node
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping nodes")
	}
	time.Sleep(time.Millisecond * 10)
	meta.DestroyClusterState(cfg, meta.ClusterTypeTstore)
	meta.DestroyClusterState(cfg, meta.ClusterTypeKstore)
}

func TestDataNodeKstoreClustering(t *testing.T) {
	const numNodes = 3
	dnodes := make([]*DNode, numNodes)
	clients := make([]*rpckit.RPCClient, numNodes)
	var err error

	// metadata config
	cfg := meta.DefaultClusterConfig()
	cfg.EnableTstoreMeta = false
	cfg.DeadInterval = time.Millisecond * 100
	cfg.NodeTTL = 5
	cfg.RebalanceDelay = time.Millisecond * 100
	cfg.RebalanceInterval = time.Millisecond * 10

	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")
	}

	// create rpc client
	for idx := 0; idx < numNodes; idx++ {
		clients[idx], err = rpckit.NewRPCClient(fmt.Sprintf("datanode-%d", idx), fmt.Sprintf("localhost:730%d", idx), rpckit.WithLoggerEnabled(false), rpckit.WithTLSProvider(nil))
		AssertOk(t, err, "Error connecting to grpc server")
		defer clients[idx].Close()
	}

	watcher, err := meta.NewWatcher("watcher", cfg)
	AssertOk(t, err, "Error creating the watcher")
	defer watcher.Stop()

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeKstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, nil
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeKstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, nil
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")

	// write some keys
	kcl := dnodes[0].GetCluster(meta.ClusterTypeKstore)
	for _, shard := range kcl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			if repl.IsPrimary {
				dnclient, rerr := dnodes[0].getDnclient(meta.ClusterTypeKstore, repl.NodeUUID)
				AssertOk(t, rerr, "Error getting datanode client")

				kvl := []*tproto.KeyValue{
					{
						Key:   []byte("testKey1"),
						Value: []byte("testValue1"),
					},
					{
						Key:   []byte("testKey2"),
						Value: []byte("testValue2"),
					},
				}

				// build the request
				req := tproto.KeyValueMsg{
					ClusterType: meta.ClusterTypeKstore,
					ReplicaID:   repl.ReplicaID,
					ShardID:     repl.ShardID,
					Table:       "table0",
					Kvs:         kvl,
				}

				_, err = dnclient.WriteReq(context.Background(), &req)
				AssertOk(t, err, "Error writing kvs")
			}
		}
	}
	time.Sleep(time.Second)

	// do a quick statefull restart
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("################## Restarting(quick) node %s", fmt.Sprintf("node-%d", idx))

		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")

		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")
	}

	time.Sleep(time.Second)

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeKstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, watcher.GetCluster(meta.ClusterTypeKstore)
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeKstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, watcher.GetCluster(meta.ClusterTypeKstore)
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")
	time.Sleep(time.Second)

	// do a soft restart without expiring the lease
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("############### Restarting(soft) node %s", fmt.Sprintf("node-%d", idx))
		err = dnodes[idx].SoftRestart()
		AssertOk(t, err, "Error softrestarting data node")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")
	}

	// wait till cluster state has converged
	AssertEventually(t, func() (bool, interface{}) {
		if (len(watcher.GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes) ||
			(len(watcher.GetCluster(meta.ClusterTypeKstore).ShardMap.Shards) != meta.DefaultShardCount) {
			return false, nil
		}

		for idx := 0; idx < numNodes; idx++ {
			if watcher.GetCluster(meta.ClusterTypeKstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
				return false, nil
			}
		}

		return true, nil
	}, "nodes did not get cluster update", "100ms", "30s")
	time.Sleep(time.Second)

	// restart each of the nodes
	for idx := 0; idx < numNodes; idx++ {
		log.Infof("################## Restarting(slow) node %s", fmt.Sprintf("node-%d", idx))

		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
		time.Sleep(cfg.DeadInterval * 3)

		AssertEventually(t, func() (bool, interface{}) {
			if len(watcher.GetCluster(meta.ClusterTypeKstore).NodeMap) != (numNodes - 1) {
				return false, watcher.GetCluster(meta.ClusterTypeKstore)
			}

			return true, nil
		}, "node was not removed from cluster", "100ms", "30s")

		// create the data node
		dnodes[idx], err = NewDataNode(cfg, fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:730%d", idx), fmt.Sprintf("%s/%d", path, idx))
		AssertOk(t, err, "Error creating nodes")

		// verify atleast one of the nodes is a leader
		AssertEventually(t, func() (bool, interface{}) {
			for i := 0; i < numNodes; i++ {
				if dnodes[i].metaNode.IsLeader() {
					return true, nil
				}
			}
			return false, nil
		}, "Leader election failure", "300ms", "30s")

		// wait till cluster state has converged
		AssertEventually(t, func() (bool, interface{}) {
			if (len(watcher.GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes) ||
				(len(watcher.GetCluster(meta.ClusterTypeKstore).ShardMap.Shards) != meta.DefaultShardCount) {
				return false, watcher.GetCluster(meta.ClusterTypeKstore)
			}

			for idx := 0; idx < numNodes; idx++ {
				if watcher.GetCluster(meta.ClusterTypeKstore).NodeMap[fmt.Sprintf("node-%d", idx)].NumShards < (meta.DefaultShardCount * meta.DefaultReplicaCount / numNodes) {
					return false, watcher.GetCluster(meta.ClusterTypeKstore)
				}
			}

			return true, nil
		}, "nodes did not get cluster update", "100ms", "30s")

		time.Sleep(time.Second)
	}

	// delete some keys
	kcl = dnodes[0].GetCluster(meta.ClusterTypeKstore)
	for _, shard := range kcl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			if repl.IsPrimary {
				dnclient, rerr := dnodes[0].getDnclient(meta.ClusterTypeKstore, repl.NodeUUID)
				AssertOk(t, rerr, "Error getting datanode client")

				// keys to delete
				kl := []*tproto.Key{
					{
						Key: []byte("testKey1"),
					},
					{
						Key: []byte("testKey2"),
					},
				}

				// build the request
				req := tproto.KeyMsg{
					ClusterType: meta.ClusterTypeKstore,
					ReplicaID:   repl.ReplicaID,
					ShardID:     repl.ShardID,
					Table:       "table0",
					Keys:        kl,
				}

				_, err = dnclient.DelReq(context.Background(), &req)
				AssertOk(t, err, "Error deleting the keys")
			}
		}
	}

	for idx := 0; idx < numNodes; idx++ {
		// stop the data node
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping nodes")
	}

	time.Sleep(time.Millisecond * 10)
	meta.DestroyClusterState(cfg, meta.ClusterTypeTstore)
	meta.DestroyClusterState(cfg, meta.ClusterTypeKstore)
}
