// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

// this file contains time series data handling

import (
	"encoding/json"
	"errors"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

// CreateDatabase creates a database
func (dn *DNode) CreateDatabase(ctx context.Context, req *tproto.DatabaseReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Infof("%s Received CreateDatabase req %+v", dn.nodeUUID, req)

	// find the shard from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		log.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// create the database in the datastore
	err := shard.store.CreateDatabase(req.Database, nil)
	if err != nil {
		log.Errorf("Error creating the database in %s. Err: %v", req.Database, err)
		return &resp, err
	}

	return &resp, nil
}

// DeleteDatabase deletes a database
func (dn *DNode) DeleteDatabase(ctx context.Context, req *tproto.DatabaseReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Infof("%s Received DeleteDatabase req %+v", dn.nodeUUID, req)

	// find the shard from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	err := shard.store.DeleteDatabase(req.Database)
	return &resp, err
}

// PointsWrite writes points to data store
// FIXME: this needs to replicate the data to secondary nodes
func (dn *DNode) PointsWrite(ctx context.Context, req *tproto.PointsWriteReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received PointsWrite req %+v", dn.nodeUUID, req)

	// check if datanode is already stopped
	if dn.isStopped {
		log.Errorf("Received PointsWrite on stopped datanode %s", dn.nodeUUID)
		return &resp, errors.New("Datanode is stopped")
	}

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		log.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// Check if we are the primary for this shard group
	if !shard.isPrimary {
		// FIXME: forward the message to real primary
		log.Errorf("non-primary received points write message. Shard: %+v.", shard)
		return &resp, errors.New("Non-primary received points write")
	}

	// parse the points
	points, err := models.ParsePointsWithPrecision([]byte(req.Points), time.Time{}, "n")
	if err != nil {
		log.Errorf("Error parsing the points. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// first acquire sync lock
	shard.syncLock.Lock()
	defer shard.syncLock.Unlock()

	// write points to datastore
	// FIXME: should we use the low level shard write points APIs here?
	err = shard.store.WritePoints(req.Database, points)
	if err != nil {
		log.Errorf("Error writing the points to db. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// replicate to all secondaries
	err = dn.replicatePoints(ctx, req, shard)
	if err != nil {
		log.Errorf("Error replicating to secondaries. Err: %v", err)
		return &resp, err
	}

	return &resp, nil
}

// replicatePoints replicates points to secondary replicas
func (dn *DNode) replicatePoints(ctx context.Context, req *tproto.PointsWriteReq, shard *TshardState) error {
	// get cluster state from watcher
	cl := dn.watcher.GetCluster(meta.ClusterTypeTstore)

	// make a copy of the replicas, in case if it changes while we are walking it
	replicas := ref.DeepCopy(shard.replicas).([]*tproto.ReplicaInfo)

	// walk all replicas
	// FIXME: look into how to do quorum write
	for _, se := range replicas {
		var isNodeAlive = true
		if cl != nil && cl.ShardMap != nil && !cl.IsNodeAlive(se.NodeUUID) {
			isNodeAlive = false
		}

		// replicate to other replicas that are not yet dead
		if se.ReplicaID != shard.replicaID && isNodeAlive {
			// get rpc client
			dnclient, err := dn.getDnclient(meta.ClusterTypeTstore, se.NodeUUID)
			if err != nil {
				continue
			}

			// message with new replica id
			newReq := tproto.PointsWriteReq{
				ClusterType: req.ClusterType,
				ReplicaID:   se.ReplicaID,
				ShardID:     req.ShardID,
				Database:    req.Database,
				Points:      req.Points,
			}

			// replicate the points
			// FIXME: if replica is not yet marked unreachable and we fail to replicate to it, keep it in a pending queue.
			// when it comes back up, we should send the points in pending queue to the replica
			_, err = dnclient.PointsReplicate(ctx, &newReq)
			if err != nil && strings.Contains(err.Error(), "the connection is unavailable") {
				// try reconnecting if this was a connection error
				dnclient, err = dn.reconnectDnclient(meta.ClusterTypeTstore, se.NodeUUID)
				if err != nil {
					log.Errorf("Error replicating points to node error connecting to %s. Err: %v", se.NodeUUID, err)
					continue
				}

				// try again
				_, err = dnclient.PointsReplicate(ctx, &newReq)
			}
			if err != nil {
				log.Errorf("Error replicating points to node %s. Err: %v", se.NodeUUID, err)
				continue
			}
		}
	}

	return nil
}

// PointsReplicate is a replicate message from primary to secondaries
func (dn *DNode) PointsReplicate(ctx context.Context, req *tproto.PointsWriteReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	log.Debugf("%s Received PointsReplicate req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// parse the points
	points, err := models.ParsePointsWithPrecision([]byte(req.Points), time.Time{}, "n")
	if err != nil {
		log.Errorf("Error parsing the points. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// write points to datastore
	err = shard.store.WritePoints(req.Database, points)
	if err != nil {
		log.Errorf("Error writing the points to db. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	return &resp, nil
}

// ExecuteQuery executes a query on the data node
func (dn *DNode) ExecuteQuery(ctx context.Context, req *tproto.QueryReq) (*tproto.QueryResp, error) {
	var resp tproto.QueryResp

	log.Debugf("%s Received ExecuteQuery req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		log.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		log.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// execute the query
	ch, err := shard.store.ExecuteQuery(req.Query, req.Database)
	if err != nil {
		log.Errorf("Error executing the query %s on db %s. Err: %v", req.Query, req.Database, err)
		return &resp, err
	}

	// read the result
	var result []*tproto.Result
	for res := range ch {
		s, jerr := res.MarshalJSON()
		if jerr != nil {
			log.Errorf("Error marshaling the output. Err: %v", err)
			return &resp, err
		}
		data := tproto.Result{
			Data: s,
		}
		result = append(result, &data)
	}

	// build query resp
	resp.Database = req.Database
	resp.ReplicaID = req.ReplicaID
	resp.TxnID = req.TxnID
	resp.Result = result

	return &resp, nil
}
