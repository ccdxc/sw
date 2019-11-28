// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

// this file contains time series data handling

import (
	"encoding/json"
	"errors"
	"fmt"
	"path/filepath"
	"strings"
	"time"

	meta2 "github.com/influxdata/influxdb/services/meta"

	"github.com/google/uuid"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/toml"
	"github.com/influxdata/influxdb/tsdb"
	"github.com/influxdata/influxql"
	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/citadel/tstore"
	"github.com/pensando/sw/venice/utils/ref"
)

// CreateDatabase creates a database
func (dn *DNode) CreateDatabase(ctx context.Context, req *tproto.DatabaseReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received CreateDatabase req %+v", dn.nodeUUID, req)

	// find the shard from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		dn.logger.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	retentionPeriod := time.Duration(req.RetentionPeriod)

	// set cluster default if duration is below minimum or unlimited (0)
	if retentionPeriod < meta2.MinRetentionPolicyDuration {
		retentionPeriod = dn.clusterCfg.RetentionPeriod
	}

	rp := &meta2.RetentionPolicySpec{
		Name:     dn.clusterCfg.RetentionPolicyName,
		Duration: &retentionPeriod,
	}

	// create the database in the datastore
	if err := shard.store.CreateDatabase(req.Database, rp); err != nil {
		dn.logger.Errorf("Error creating the database in %s. Err: %v", req.Database, err)
		return &resp, err
	}

	return &resp, nil
}

// DeleteDatabase deletes a database
func (dn *DNode) DeleteDatabase(ctx context.Context, req *tproto.DatabaseReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received DeleteDatabase req %+v", dn.nodeUUID, req)

	// find the shard from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	err := shard.store.DeleteDatabase(req.Database)
	return &resp, err
}

// ReadDatabases reads all databases
func (dn *DNode) ReadDatabases(ctx context.Context, req *tproto.DatabaseReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Infof("%s Received read database req %+v", dn.nodeUUID, req)

	// find the shard from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	dbinfo := shard.store.ReadDatabases()
	data, err := json.Marshal(dbinfo)
	if err != nil {
		return &resp, err
	}
	resp.Status = string(data)
	return &resp, err
}

// PointsWrite writes points to data store
// FIXME: this needs to replicate the data to secondary nodes
func (dn *DNode) PointsWrite(ctx context.Context, req *tproto.PointsWriteReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Debugf("%s Received PointsWrite req %+v", dn.nodeUUID, req)

	// check if datanode is already stopped
	if dn.isStopped {
		dn.logger.Errorf("Received PointsWrite on stopped datanode %s", dn.nodeUUID)
		return &resp, errors.New("Datanode is stopped")
	}

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		dn.logger.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// Check if we are the primary for this shard group
	if !shard.isPrimary {
		// FIXME: forward the message to real primary
		dn.logger.Errorf("non-primary received points write message. Shard: %+v.", shard)
		return &resp, errors.New("Non-primary received points write")
	}

	// parse the points
	points, err := models.ParsePointsWithPrecision([]byte(req.Points), time.Now().UTC(), "n")
	if err != nil {
		dn.logger.Errorf("Error parsing the points. Err: %v", err)
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
		dn.logger.Errorf("Error writing the points to db. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}
	dn.logger.Debugf("shard %v replica %v wrote %d points", req.ShardID, req.ReplicaID, len(points))

	// replicate to all secondaries
	err = dn.replicatePoints(ctx, req, shard)
	if err != nil {
		dn.logger.Errorf("Error replicating to secondaries. Err: %v", err)
		return &resp, err
	}

	return &resp, nil
}

// isGrpcConnectErr checks if the grpc error is because of connection issues
func (dn *DNode) isGrpcConnectErr(err error) bool {
	if err == nil {
		return false
	}

	return strings.Contains(err.Error(), "the connection is unavailable") ||
		strings.Contains(err.Error(), "connection error")
}

// replicatePoints replicates points to secondary replicas
func (dn *DNode) replicatePoints(ctx context.Context, req *tproto.PointsWriteReq, shard *TshardState) error {
	// get cluster state from watcher
	cl := dn.watcher.GetCluster(meta.ClusterTypeTstore)

	// make a copy of the replicas, in case if it changes while we are walking it
	replicas := ref.DeepCopy(shard.replicas).([]*tproto.ReplicaInfo)

	secRepl := []uint64{}
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
			// if replica is not yet marked unreachable and we fail to replicate to it, keep it in a pending queue.
			// when it comes back up, we should send the points in pending queue to the replica
			_, err = dnclient.PointsReplicate(ctx, &newReq)
			if err != nil && dn.isGrpcConnectErr(err) {
				// try reconnecting if this was a connection error
				dnclient, err = dn.reconnectDnclient(meta.ClusterTypeTstore, se.NodeUUID)
				if err == nil { // try again
					_, err = dnclient.PointsReplicate(ctx, &newReq)
				}
			}

			if err != nil {
				dn.logger.Warnf("failed to replicate points to node %s. Err: %v", se.NodeUUID, err)
				// add to pending queue
				dn.addSyncBuffer(&shard.syncBuffer, se.NodeUUID, &newReq)
				continue
			}
			secRepl = append(secRepl, se.ReplicaID)
		}
	}

	log.Debugf("shard:%d replica:%d replicated points to %v", req.ShardID, req.ReplicaID, secRepl)
	return nil
}

// replicateFailedPoints replicates failed points from the pending queue
func (dn *DNode) replicateFailedPoints(sb *syncBufferState) error {
	if sb.queue.Len() == 0 {
		return nil
	}
	dn.logger.Infof("%s sync buffer queue len:%d for %+v", dn.nodeUUID, sb.queue.Len(), sb)

	cl := dn.watcher.GetCluster(meta.ClusterTypeTstore)
	if _, err := cl.GetNode(sb.nodeUUID); err != nil {
		// drain the buffer
		sb.queue.RemoveAll(func(k interface{}) {})
		return fmt.Errorf("unable to find node %v", sb.nodeUUID)
	}

	if !cl.IsNodeAlive(sb.nodeUUID) {
		return fmt.Errorf("unable to reach node %v", sb.nodeUUID)
	}

	dnclient, err := dn.getDnclient(meta.ClusterTypeTstore, sb.nodeUUID)
	if err != nil {
		return err
	}

	if ok := sb.queue.RemoveTill(func(c int, el interface{}) bool {
		req, ok := el.(*tproto.PointsWriteReq)
		if !ok {
			return false
		}
		_, err = dnclient.PointsReplicate(sb.ctx, req)
		if err != nil && dn.isGrpcConnectErr(err) {
			// try reconnecting if this was a connection error
			dnclient, err = dn.reconnectDnclient(meta.ClusterTypeTstore, sb.nodeUUID)
			if err != nil {
				return false
			}
			if _, err = dnclient.PointsReplicate(sb.ctx, req); err != nil {
				return false
			}
		}
		return true

	}); !ok {
		return fmt.Errorf("sync buffer failed to replicate points from pending ts queue")
	}

	return nil
}

// PointsReplicate is a replicate message from primary to secondaries
func (dn *DNode) PointsReplicate(ctx context.Context, req *tproto.PointsWriteReq) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp

	dn.logger.Debugf("%s Received PointsReplicate req %+v", dn.nodeUUID, req)

	// check if datanode is already stopped
	if dn.isStopped {
		dn.logger.Errorf("Received PointsWrite on stopped datanode %s", dn.nodeUUID)
		return &resp, errors.New("Datanode is stopped")
	}

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s", req.ReplicaID, req.ClusterType)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// TODO: remove the debug message
	if shard.syncPending {
		dn.logger.Warnf("replicate points in %s, shard: %+v while sync is pending", dn.nodeUUID, shard)
	}

	// parse the points
	points, err := models.ParsePointsWithPrecision([]byte(req.Points), time.Now().UTC(), "n")
	if err != nil {
		dn.logger.Errorf("Error parsing the points. Err: %v", err)
		resp.Status = err.Error()
		return &resp, err
	}

	// write points to datastore
	if err = shard.store.WritePoints(req.Database, points); err == nil {
		return &resp, nil
	}

	dn.logger.Errorf("Error writing %d points to db. Err: %v", len(points), err)
	if strings.Contains(err.Error(), "database not found") {
		dn.logger.Errorf("creating database %v in replica %v shard %v",
			req.Database, req.ReplicaID, req.ShardID)
		if _, dbErr := dn.CreateDatabase(ctx, &tproto.DatabaseReq{
			ClusterType:     req.ClusterType,
			ReplicaID:       req.ReplicaID,
			ShardID:         req.ShardID,
			Database:        req.Database,
			RetentionPeriod: uint64(dn.clusterCfg.RetentionPeriod),
		}); dbErr != nil {
			dn.logger.Errorf("failed to create database %s, %v", req.Database, dbErr)
		} else {
			if err = shard.store.WritePoints(req.Database, points); err == nil {
				return &resp, nil
			}
		}
	}

	resp.Status = err.Error()
	return &resp, err
}

// ExecuteQuery executes a query on the data node
func (dn *DNode) ExecuteQuery(ctx context.Context, req *tproto.QueryReq) (*tproto.QueryResp, error) {
	var resp tproto.QueryResp
	dn.logger.Infof("%s Received ExecuteQuery req %+v", dn.nodeUUID, req)

	// find the data store from shard id
	val, ok := dn.tshards.Load(req.ReplicaID)
	if !ok || val.(*TshardState).store == nil || req.ClusterType != meta.ClusterTypeTstore {
		dn.logger.Errorf("Shard %d not found for cluster %s in %v", req.ReplicaID, req.ClusterType, dn.nodeUUID)
		jstr, _ := json.Marshal(dn.watcher.GetCluster(meta.ClusterTypeTstore))
		dn.logger.Errorf("Nodemap: %s", jstr)
		return &resp, errors.New("Shard not found")
	}
	shard := val.(*TshardState)

	// execute the query
	ch, err := shard.store.ExecuteQuery(req.Query, req.Database)
	if err != nil {
		dn.logger.Errorf("Error executing the query %s on db %s. Err: %v", req.Query, req.Database, err)
		return &resp, err
	}

	// read the result
	var respResults []*query.Result
	for r := range ch {
		l := len(respResults)
		if l == 0 {
			respResults = append(respResults, r)
		} else if respResults[l-1].StatementID == r.StatementID {
			if r.Err != nil {
				respResults[l-1] = r
				continue
			}

			cr := respResults[l-1]
			rowsMerged := 0
			if len(cr.Series) > 0 {
				lastSeries := cr.Series[len(cr.Series)-1]

				for _, row := range r.Series {
					if !lastSeries.SameSeries(row) {
						// Next row is for a different series than last.
						break
					}
					// Values are for the same series, so append them.
					lastSeries.Values = append(lastSeries.Values, row.Values...)
					rowsMerged++
				}
			}

			// Append remaining rows as new rows.
			r.Series = r.Series[rowsMerged:]
			cr.Series = append(cr.Series, r.Series...)
			cr.Messages = append(cr.Messages, r.Messages...)
			cr.Partial = r.Partial
		} else {
			respResults = append(respResults, r)
		}

	}

	var result []*tproto.Result
	for _, res := range respResults {
		s, jerr := res.MarshalJSON()
		if jerr != nil {
			dn.logger.Errorf("Error marshaling the output. Err: %v", err)
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

// newQueryStore creates ts engine for query
func (dn *DNode) newQueryStore() error {
	if dn.querydbPath == "" {
		dn.querydbPath = dn.getQueryDbPath(meta.ClusterTypeTstore)
	}
	// get default
	cfg := tsdb.NewConfig()

	// set custom parameters
	cfg.WALDir = filepath.Join(dn.querydbPath, "wal")

	// set engine config

	cfg.CacheMaxMemorySize = 2 * tsdb.DefaultCacheMaxMemorySize
	cfg.CacheSnapshotMemorySize = tsdb.DefaultCacheSnapshotMemorySize
	cfg.CacheSnapshotWriteColdDuration = toml.Duration(time.Duration(time.Hour))
	cfg.CompactFullWriteColdDuration = toml.Duration(time.Duration(12 * time.Hour))
	cfg.MaxSeriesPerDatabase = 5 * tsdb.DefaultMaxSeriesPerDatabase
	cfg.MaxValuesPerTag = 0

	// query tstore for this data node
	ts, err := tstore.NewTstoreWithConfig(dn.querydbPath, cfg)
	if err != nil {
		dn.logger.Errorf("Error creating tstore at %s. Err: %v", dn.querydbPath, err)
		return fmt.Errorf("error creating tstore at %s. Err: %v", dn.querydbPath, err)
	}

	dn.tsQueryStore = ts
	dn.logger.Infof("created query aggregator with db %v", dn.querydbPath)
	return err
}

// executeAggQuery executes a query on query store
func (dn *DNode) executeAggQuery(ctx context.Context, req *tproto.QueryReq, querystr string, database string) (*tproto.QueryResp, error) {
	var resp tproto.QueryResp

	// aggregated query
	ch, err := dn.tsQueryStore.ExecuteQuery(querystr, database)
	if err != nil {
		dn.logger.Errorf("Error executing query aggregator %s on db %s. Err: %v", querystr, database, err)
		return &resp, err
	}

	// read the result
	var respResults []*query.Result
	for r := range ch {
		l := len(respResults)
		if l == 0 {
			respResults = append(respResults, r)
		} else if respResults[l-1].StatementID == r.StatementID {
			if r.Err != nil {
				respResults[l-1] = r
				continue
			}

			cr := respResults[l-1]
			rowsMerged := 0
			if len(cr.Series) > 0 {
				lastSeries := cr.Series[len(cr.Series)-1]

				for _, row := range r.Series {
					if !lastSeries.SameSeries(row) {
						// Next row is for a different series than last.
						break
					}
					// Values are for the same series, so append them.
					lastSeries.Values = append(lastSeries.Values, row.Values...)
					rowsMerged++
				}
			}

			// Append remaining rows as new rows.
			r.Series = r.Series[rowsMerged:]
			cr.Series = append(cr.Series, r.Series...)
			cr.Messages = append(cr.Messages, r.Messages...)
			cr.Partial = r.Partial
		} else {
			respResults = append(respResults, r)
		}

	}

	var result []*tproto.Result
	for _, res := range respResults {
		s, jerr := res.MarshalJSON()
		if jerr != nil {
			dn.logger.Errorf("Error marshaling the output. Err: %v", err)
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

// writePointsInAggregator writes points in aggregator db
func (dn *DNode) writePointsInAggregator(queryDb string, measurement string, resultCh []<-chan *query.Result) error {
	numPoints := 0

	// read results, go one by one shard to avoid out of order timestamp in batches
	for _, ch := range resultCh {
		tagKeys := map[string]int{}
		for res := range ch {
			if res.Err != nil {
				dn.logger.Errorf("query %s failed, %s", measurement, res.Err)
				return res.Err
			}

			// StatementID=0 is the tags query
			if res.StatementID == 0 {
				for _, s := range res.Series {
					for _, dx := range s.Values {
						for i, x := range dx {
							if s, ok := x.(string); ok {
								tagKeys[s] = i
							}
						}
					}
				}
			} else {
				for _, s := range res.Series {
					// did we reach the limit, continue to drain the channels
					if numPoints > meta.DefaultQueryLimit {
						break
					}

					points, err := convertRowToPoints(measurement, tagKeys, s)
					if err != nil {
						dn.logger.Errorf("failed to convert points %s", err)
						continue
					}

					if len(points) > 0 {
						if err := dn.tsQueryStore.WritePoints(queryDb, points); err != nil {
							dn.logger.Errorf("failed to write points to %v, %s", queryDb, err)
							continue
						}
						numPoints += len(points)
					}
				}
			}
		}
	}

	return nil
}

// executeQueryRemote executes a query on the remote replica
func (dn *DNode) executeQueryRemote(ctx context.Context, req *tproto.QueryReq, nodeUUID string) (<-chan *query.Result, error) {
	dnclient, err := dn.getDnclient(meta.ClusterTypeTstore, nodeUUID)
	if err != nil {
		return nil, err
	}

	ch := make(chan *query.Result, 1)

	go func() {
		defer close(ch)

		resp, err := dnclient.ExecuteQuery(ctx, req)
		if err != nil && dn.isGrpcConnectErr(err) {
			// try reconnecting if this was a connection error
			dnclient, err = dn.reconnectDnclient(meta.ClusterTypeTstore, nodeUUID)
			if err == nil {
				resp, err = dnclient.ExecuteQuery(ctx, req)
			}
		}

		if err != nil {
			ch <- &query.Result{
				Err: err,
			}
			return
		}

		// parse the response
		for _, rs := range resp.Result {
			rslt := &query.Result{}
			err := rslt.UnmarshalJSON(rs.Data)
			if err != nil {
				ch <- &query.Result{
					Err: err,
				}
				return
			}
			ch <- rslt
		}
	}()

	return ch, nil
}

// convertRowToPoints will convert a query result Row into Points that can be written back in.
func convertRowToPoints(measurementName string, tagKeys map[string]int, row *models.Row) ([]models.Point, error) {
	// figure out which parts of the result are the time and which are the fields
	timeIndex := -1
	fieldIndexes := make(map[string]int)

	for i, c := range row.Columns {
		if c == "time" {
			timeIndex = i
		} else {
			fieldIndexes[c] = i
		}
	}

	if timeIndex == -1 {
		return nil, errors.New("error finding time index in result")
	}

	points := make([]models.Point, 0, len(row.Values))

	for _, v := range row.Values {
		tags := make(map[string]string)
		vals := make(map[string]interface{})

		for fieldName, fieldIndex := range fieldIndexes {
			val := v[fieldIndex]
			if val != nil {
				if _, ok := tagKeys[fieldName]; ok {
					tags[fieldName] = fmt.Sprintf("%v", val)
				} else {
					vals[fieldName] = val
				}
			}
		}

		var ts time.Time
		if t, ok := v[timeIndex].(string); ok {
			if nts, err := time.Parse(time.RFC3339, t); err == nil {
				ts = nts
			}
		} else {
			ts = v[timeIndex].(time.Time)
		}

		p, err := models.NewPoint(measurementName, models.NewTags(tags), vals, ts)
		if err != nil {
			// Drop points that can't be stored
			log.Errorf("failed to convert points tags:%+v vals:%+v", tags, vals)
			continue
		}

		points = append(points, p)
	}

	return points, nil
}

// queryAllShards sends queries to all shards and returns a list of result channels
func (dn *DNode) queryAllShards(ctx context.Context, req *tproto.QueryReq) ([]<-chan *query.Result, error) {

	queryShards := map[uint64]bool{}
	resultsCh := []<-chan *query.Result{}

	// todo: query local replicas to optimize
	// requires new marshal functions to handle int/float
	// Use grpc for local and remote shards for now

	// check if any shards are remote
	cl := dn.watcher.GetCluster(meta.ClusterTypeTstore)

	// walk thru each shard and query it
	for _, shard := range cl.ShardMap.Shards {
		// query one of the repicas in each shard
		for _, replica := range shard.Replicas {
			dn.logger.Debugf("query remote shard %d, replica: %v, node: %v", shard.ShardID, replica.ReplicaID, replica.NodeUUID)

			if !cl.IsNodeAlive(replica.NodeUUID) {
				// go to the next replica
				continue
			}
			nreq := *req
			nreq.ShardID = replica.ShardID
			nreq.ReplicaID = replica.ReplicaID
			ch, err := dn.executeQueryRemote(ctx, &nreq, replica.NodeUUID)
			if err != nil {
				continue
			}
			queryShards[replica.ShardID] = true
			resultsCh = append(resultsCh, ch)
			break
		}
	}

	dn.logger.Debugf("number of query result channels: %d", len(queryShards))

	return resultsCh, nil
}

// ExecuteAggQuery queries all shards and aggregates the response
func (dn *DNode) ExecuteAggQuery(ctx context.Context, req *tproto.QueryReq) (*tproto.QueryResp, error) {
	dn.queryMutex.Lock()
	defer dn.queryMutex.Unlock()

	if req.ClusterType != meta.ClusterTypeTstore {
		return nil, fmt.Errorf("invalid cluster type %v", req.ClusterType)
	}

	// save measurement name
	pq, err := influxql.ParseQuery(req.Query)
	if err != nil {
		return nil, err
	}

	// todo: more than 1 statement in query

	// broker splits query to multiple statements & execute one by one on data nodes
	if len(pq.Statements) != 1 {
		return nil, errors.New("Query must have only one statement")
	}

	// parse statement
	stmt := pq.Statements[0]
	selStmt, ok := stmt.(*influxql.SelectStatement)
	if !ok {
		return nil, fmt.Errorf("invalid select statement %+v", stmt)
	}

	// get the measurement name
	if len(selStmt.Sources.Measurements()) != 1 {
		return nil, errors.New("Query must have only one measurement")
	}

	measurement := selStmt.Sources.Measurements()[0].Name

	// run wildcard query
	wildcardStmnt := selStmt.Clone()
	wildcardStmnt.Fields = influxql.Fields{{Expr: &influxql.Wildcard{}}}
	wildcardStmnt.Sources[0] = selStmt.Sources.Measurements()[0]
	wildcardStmnt.Dimensions = nil
	wildcardStmnt.Limit = (selStmt.Limit + 4) / 4

	_, err = influxql.ParseQuery(wildcardStmnt.String())
	if err != nil {
		return nil, err
	}

	// save original query & create new query
	origQuery := req.Query
	req.Query = fmt.Sprintf("show tag keys from %s; %s", measurement, wildcardStmnt.String())

	resultCh, err := dn.queryAllShards(ctx, req)
	if err != nil {
		return nil, err
	}

	// drain channels to unblock
	defer func() {
		for _, ch := range resultCh {
			for range ch {
			}
		}
	}()

	dn.logger.Infof("process query: %s, shard query: %s", origQuery, req.Query)
	// create a unique db
	queryDb := req.Database + uuid.New().String()

	d := time.Duration(0)
	rp := &meta2.RetentionPolicySpec{
		Name:     dn.clusterCfg.RetentionPolicyName,
		Duration: &d,
	}

	if err := dn.tsQueryStore.CreateDatabase(queryDb, rp); err != nil {
		return nil, fmt.Errorf("failed to create query db %s", err)
	}
	defer dn.tsQueryStore.DeleteDatabase(queryDb)

	// read results, go one by one shard to avoid out of order timestamp if timestamp was used in sharding
	if err := dn.writePointsInAggregator(queryDb, measurement, resultCh); err != nil {
		return nil, err
	}

	// aggregated query
	return dn.executeAggQuery(ctx, req, origQuery, queryDb)
}
