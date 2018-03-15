// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker

import (
	"context"
	"errors"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
)

type kvlist []*tproto.KeyValue
type keylist []*tproto.Key

// WriteKvs writes a list of key-value pairs to backend
func (br *Broker) WriteKvs(ctx context.Context, table string, kvs []*tproto.KeyValue) error {
	kvmap := make(map[uint64]kvlist)
	replMap := make(map[uint64]*meta.Replica)

	// get the kstore cluster
	cl := br.GetCluster(meta.ClusterTypeKstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// loop thru the keys and sort them by shard
	for _, kv := range kvs {
		// get shard for the measurement
		shard, err := cl.ShardMap.GetShardForKey(table, tproto.Key{Key: kv.Key})
		if err != nil {
			log.Errorf("Error getting shard for %s/%s. Err: %v", table, string(kv.Key), err)
			return err
		}

		// get the primary replica
		pri, err := shard.GetPrimaryreplica()
		if err != nil {
			log.Errorf("Could not get the primary replica for %+v. Err: %v", shard, err)
			return err
		}

		// append it to per shard list
		kvmap[pri.ShardID] = append(kvmap[pri.ShardID], kv)
		replMap[pri.ShardID] = pri
	}

	// walk the per shard list and write it to each node
	for sid, kvl := range kvmap {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(replMap[sid].NodeUUID, meta.ClusterTypeKstore)
		if err != nil {
			return err
		}

		// build the request
		req := tproto.KeyValueMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   replMap[sid].ReplicaID,
			ShardID:     replMap[sid].ShardID,
			Table:       table,
			Kvs:         kvl,
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.WriteReq(ctx, &req)
		if err != nil {
			log.Errorf("Error making WriteReq rpc call. Err: %v", err)
			return err
		} else if resp.Status != "" {
			log.Errorf("Error making WriteReq rpc call. Err: %v", resp.Status)
			return errors.New(resp.Status)
		}
	}

	return nil
}

// ReadKvs reads a list of keys
func (br *Broker) ReadKvs(ctx context.Context, table string, keys []*tproto.Key) ([]*tproto.KeyValue, error) {
	keymap := make(map[uint64]keylist)
	replMap := make(map[uint64]*meta.Replica)
	var kvs []*tproto.KeyValue

	// get the kstore cluster
	cl := br.GetCluster(meta.ClusterTypeKstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return kvs, errors.New("Shard map is empty")
	}

	// loop thru the keys and sort them by shard
	for _, key := range keys {
		// get shard for the measurement
		shard, err := cl.ShardMap.GetShardForKey(table, *key)
		if err != nil {
			log.Errorf("Error getting shard for %s/%s. Err: %v", table, string(key.Key), err)
			return kvs, err
		}

		// get the primary shard
		pri, err := shard.GetPrimaryreplica()
		if err != nil {
			log.Errorf("Could not get the primary replica for %+v. Err: %v", shard, err)
			return kvs, err
		}

		// append it to per shard list
		keymap[pri.ShardID] = append(keymap[pri.ShardID], key)
		replMap[pri.ShardID] = pri
	}

	// walk the per shard list and read from each node
	for sid, kl := range keymap {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(replMap[sid].NodeUUID, meta.ClusterTypeKstore)
		if err != nil {
			return kvs, err
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   replMap[sid].ReplicaID,
			ShardID:     replMap[sid].ShardID,
			Table:       table,
			Keys:        kl,
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.ReadReq(ctx, &req)
		if err != nil {
			log.Errorf("Error making WriteReq rpc call. Err: %v", err)
			return kvs, err
		}

		// append to results
		kvs = append(kvs, resp.Kvs...)
	}

	return kvs, nil
}

// listKvForShard gets list from any one node in the shard
func (br *Broker) listKvForShard(ctx context.Context, table string, shard *meta.Shard) (*tproto.KeyValueMsg, error) {
	// get successfull answer from atleast one replica
	for _, repl := range shard.Replicas {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeKstore)
		if err != nil {
			return nil, err
		}

		// build the request
		req := tproto.TableMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Table:       table,
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.ListReq(ctx, &req)
		if err == nil {
			return resp, nil
		}
	}

	return nil, errors.New("list failed from all replicas")
}

// ListKvs lists all keys in a table
func (br *Broker) ListKvs(ctx context.Context, table string) ([]*tproto.KeyValue, error) {
	var kvs []*tproto.KeyValue

	// get the kstore cluster
	cl := br.GetCluster(meta.ClusterTypeKstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return kvs, errors.New("Shard map is empty")
	}

	// walk thru all shards in shard map
	for _, shard := range cl.ShardMap.Shards {
		resp, err := br.listKvForShard(ctx, table, shard)
		if err == nil {
			// append to results
			kvs = append(kvs, resp.Kvs...)
		}
	}

	return kvs, nil
}

// DeleteKvs deletes a list of keys
func (br *Broker) DeleteKvs(ctx context.Context, table string, keys []*tproto.Key) error {
	keymap := make(map[uint64]keylist)
	replMap := make(map[uint64]*meta.Replica)

	// get the kstore cluster
	cl := br.GetCluster(meta.ClusterTypeKstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// loop thru the keys and sort them by shard
	for _, key := range keys {
		// get shard for the measurement
		shard, err := cl.ShardMap.GetShardForKey(table, *key)
		if err != nil {
			log.Errorf("Error getting shard for %s/%s. Err: %v", table, string(key.Key), err)
			return err
		}

		// get the primary shard
		pri, err := shard.GetPrimaryreplica()
		if err != nil {
			log.Errorf("Could not get the primary replica for %+v. Err: %v", shard, err)
			return err
		}

		// append it to per shard list
		keymap[pri.ShardID] = append(keymap[pri.ShardID], key)
		replMap[pri.ShardID] = pri
	}

	// walk the per shard list and delete it to each node
	for sid, kl := range keymap {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(replMap[sid].NodeUUID, meta.ClusterTypeKstore)
		if err != nil {
			return err
		}

		// build the request
		req := tproto.KeyMsg{
			ClusterType: meta.ClusterTypeKstore,
			ReplicaID:   replMap[sid].ReplicaID,
			ShardID:     replMap[sid].ShardID,
			Table:       table,
			Keys:        kl,
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.DelReq(ctx, &req)
		if err != nil {
			log.Errorf("Error making DelReq rpc call. Err: %v", err)
			return err
		} else if resp.Status != "" {
			return errors.New(resp.Status)
		}
	}

	return nil
}
