// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

import (
	"errors"
	"strings"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/venice/citadel/kstore"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

// ReadReq reads from kstore
func (dn *DNode) ReadReq(ctx context.Context, req *tproto.KeyMsg) (*tproto.KeyValueMsg, error) {
	var resp tproto.KeyValueMsg

	log.Debugf("%s Received ReadReq req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// read from kstore
	kvs, err := shard.kstore.Read(req.Table, req.Keys)
	if err != nil {
		log.Errorf("Error reading from kstore. Err: %v", err)
		return &resp, err
	}

	// build resp
	resp.ClusterType = req.ClusterType
	resp.ReplicaID = req.ReplicaID
	resp.ShardID = req.ShardID
	resp.Kvs = kvs

	return &resp, nil
}

// ListReq lists from kstore
func (dn *DNode) ListReq(ctx context.Context, req *tproto.TableMsg) (*tproto.KeyValueMsg, error) {
	var resp tproto.KeyValueMsg

	log.Debugf("%s Received ListReq req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// read from kstore
	kvs, err := shard.kstore.List(req.Table)
	if err != nil {
		if err == kstore.ErrTableNotFound {
			// build an empty response if table was not found
			resp.ClusterType = req.ClusterType
			resp.ReplicaID = req.ReplicaID
			resp.ShardID = req.ShardID
			resp.Kvs = []*tproto.KeyValue{}
			return &resp, nil
		}

		log.Errorf("Error listing from kstore. Err: %v", err)
		return &resp, err
	}

	// build resp
	resp.ClusterType = req.ClusterType
	resp.ReplicaID = req.ReplicaID
	resp.ShardID = req.ShardID
	resp.Kvs = kvs

	return &resp, nil
}

// WriteReq writes to kstore
func (dn *DNode) WriteReq(ctx context.Context, req *tproto.KeyValueMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received WriteReq req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// Check if we are the primary for this shard group
	if !shard.isPrimary {
		// FIXME: forward the message to real primary
		log.Errorf("non-primary received kv write message. Shard: %+v.", shard)
		return &resp, errors.New("Non-primary received points write")
	}

	// first acquire sync lock
	// lock the shard
	shard.syncLock.Lock()
	defer shard.syncLock.Unlock()

	// read from kstore
	err := shard.kstore.Write(req.Table, req.Kvs)
	if err != nil {
		log.Errorf("Error writing to kstore. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// replicate to secondaries
	err = dn.replicateWrite(ctx, req, shard)
	if err != nil {
		log.Errorf("Error replicating to secondaries. Err: %v", err)
	}

	return &resp, nil
}

// replicateWrite replicates key-value pairs to secondary replicas
func (dn *DNode) replicateWrite(ctx context.Context, req *tproto.KeyValueMsg, shard *KshardState) error {
	// get cluster state from watcher
	cl := dn.watcher.GetCluster(meta.ClusterTypeKstore)

	// make a copy of the replicas, in case if it changes while we are walking it
	replicas := ref.DeepCopy(shard.replicas).([]*tproto.ReplicaInfo)

	// walk all replicas
	// FIXME: look into how to do quorum write
	for _, se := range replicas {
		var isNodeAlive = true
		if cl != nil && cl.ShardMap != nil && !cl.IsNodeAlive(se.NodeUUID) {
			isNodeAlive = false
		}

		if se.ReplicaID != shard.replicaID && isNodeAlive {
			// get rpc client
			dnclient, err := dn.getDnclient(meta.ClusterTypeKstore, se.NodeUUID)
			if err != nil {
				continue
			}

			// message with new replica id
			newReq := tproto.KeyValueMsg{
				ClusterType: req.ClusterType,
				ReplicaID:   se.ReplicaID,
				ShardID:     req.ShardID,
				Table:       req.Table,
				Kvs:         req.Kvs,
			}

			// replicate the kv-pairs
			// FIXME: if replica is not yet marked unreachable and we fail to replicate to it, keep it in a pending queue.
			// when it comes back up, we should send the kv-pairs in pending queue to the replica
			_, err = dnclient.WriteReplicate(ctx, &newReq)
			if err != nil && strings.Contains(err.Error(), "the connection is unavailable") {
				// try reconnecting if this was a connection error
				dnclient, err = dn.reconnectDnclient(meta.ClusterTypeKstore, se.NodeUUID)
				if err != nil {
					log.Errorf("Error replicating key-values to node error connecting to %s. Err: %v", se.NodeUUID, err)
					continue
				}

				// try again
				_, err = dnclient.WriteReplicate(ctx, &newReq)
			}
			if err != nil {
				log.Errorf("Error replicating key-values to node %s. Err: %v", se.NodeUUID, err)
				continue
			}
		}
	}

	return nil
}

// WriteReplicate handles write replication message
func (dn *DNode) WriteReplicate(ctx context.Context, req *tproto.KeyValueMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received WriteReplicate req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// read from kstore
	err := shard.kstore.Write(req.Table, req.Kvs)
	if err != nil {
		log.Errorf("Error writing to kstore. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	return &resp, nil
}

// DelReq deletes from kstore
func (dn *DNode) DelReq(ctx context.Context, req *tproto.KeyMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received DelReq req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// Check if we are the primary for this shard group
	if !shard.isPrimary {
		// FIXME: forward the message to real primary
		log.Errorf("non-primary received points write message. Shard: %+v.", shard)
		return &resp, errors.New("Non-primary received points write")
	}

	// first acquire sync lock
	// lock the shard
	shard.syncLock.Lock()
	defer shard.syncLock.Unlock()

	// read from kstore
	err := shard.kstore.Delete(req.Table, req.Keys)
	if err != nil {
		log.Errorf("Error deleting from kstore. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// replicate to secondaries
	err = dn.replicateDelete(ctx, req, shard)
	if err != nil {
		log.Errorf("Error replicating to secondaries. Err: %v", err)
	}

	return &resp, nil
}

// replicateDelete replicates deletes to secondary replicas
func (dn *DNode) replicateDelete(ctx context.Context, req *tproto.KeyMsg, shard *KshardState) error {
	// get cluster state from watcher
	cl := dn.watcher.GetCluster(meta.ClusterTypeKstore)
	if cl != nil && cl.ShardMap != nil {
		sgrp, err := cl.ShardMap.FindShardByID(shard.shardID)
		if err != nil {
			return err
		}

		// walk all replicas
		for _, se := range sgrp.Replicas {
			if se.ReplicaID != shard.replicaID && cl.IsNodeAlive(se.NodeUUID) {
				// get rpc client
				dnclient, err := dn.getDnclient(meta.ClusterTypeKstore, se.NodeUUID)
				if err != nil {
					continue
				}

				// message with new replica id
				newReq := tproto.KeyMsg{
					ClusterType: req.ClusterType,
					ReplicaID:   se.ReplicaID,
					ShardID:     req.ShardID,
					Table:       req.Table,
					Keys:        req.Keys,
				}

				// replicate the points
				// FIXME: if replica is not yet marked unreachable and we fail to replicate to it, keep it in a pending queue.
				// when it comes back up, we should send the keys in pending delete request to the replica
				_, err = dnclient.DelReplicate(ctx, &newReq)
				if err != nil && strings.Contains(err.Error(), "the connection is unavailable") {
					// try reconnecting if this was a connection error
					dnclient, err = dn.reconnectDnclient(meta.ClusterTypeKstore, se.NodeUUID)
					if err != nil {
						log.Errorf("Error replicating key-value deletes to node error connecting to %s. Err: %v", se.NodeUUID, err)
						continue
					}

					// try again
					_, err = dnclient.DelReplicate(ctx, &newReq)
				}
				if err != nil {
					log.Errorf("Error replicating key-value deletes to node %s. Err: %v", se.NodeUUID, err)
					continue
				}
			}
		}
	}

	return nil
}

// DelReplicate handles delete replication message
func (dn *DNode) DelReplicate(ctx context.Context, req *tproto.KeyMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received DelReplicate req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.kshards.Load(req.ReplicaID)
	if !ok || val.(*KshardState).kstore == nil || req.ClusterType != meta.ClusterTypeKstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*KshardState)

	// read from kstore
	err := shard.kstore.Delete(req.Table, req.Keys)
	if err != nil {
		log.Errorf("Error deleting from kstore. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	return &resp, nil
}
