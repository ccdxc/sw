// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"math/rand"
	"reflect"
	"strings"
	"time"

	"github.com/imdario/mergo"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	influxmeta "github.com/influxdata/influxdb/services/meta"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
)

// createDatabaseInReplica creates the database in replica
func (br *Broker) createDatabaseInReplica(ctx context.Context, database string, retention uint64, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.DatabaseReq{
			ClusterType:     meta.ClusterTypeTstore,
			ReplicaID:       repl.ReplicaID,
			ShardID:         repl.ShardID,
			Database:        database,
			RetentionPeriod: retention,
		}

		// make create database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		_, err = dnclient.CreateDatabase(ctx, &req)
		if err == nil {
			br.logger.Infof("=>created the database on node %s.", repl.NodeUUID)
			return nil
		}

		br.logger.Warnf("Error creating the database on node %s. Err: %v. Retrying..", repl.NodeUUID, err)

		time.Sleep(brokerRetryDelay)
	}

	return err
}

// CreateDatabase creates the database
func (br *Broker) CreateDatabase(ctx context.Context, database string) error {
	return br.CreateDatabaseWithRetention(ctx, database, 0)
}

// CreateDatabaseWithRetention creates the database with retention
func (br *Broker) CreateDatabaseWithRetention(ctx context.Context, database string, retention uint64) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.createDatabaseInReplica(ctx, database, retention, repl)
			if err != nil {
				br.logger.Errorf("Error creating the database in replica %v. Err: %v", repl, err)
				return err
			}
		}
	}

	return nil
}

// deleteDatabaseInReplica deletes the database in replica
func (br *Broker) deleteDatabaseInReplica(ctx context.Context, database string, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Database:    database,
		}

		// delete database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		_, err = dnclient.DeleteDatabase(ctx, &req)
		if err == nil {
			br.logger.Infof("=>deleted the database on node %s.", repl.NodeUUID)
			return nil
		}

		br.logger.Warnf("Error deleting the database on node %s. Err: %v. Retrying..", repl.NodeUUID, err)

		time.Sleep(brokerRetryDelay)
	}

	return err
}

// DeleteDatabase drops the database
func (br *Broker) DeleteDatabase(ctx context.Context, database string) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("shard map is empty")
	}

	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.deleteDatabaseInReplica(ctx, database, repl)
			if err != nil {
				br.logger.Errorf("Error deleting the database in replica %v. Err: %v", repl, err)
				return err
			}
		}
	}

	return nil
}

// readDatabaseInReplica reads databases in replica
func (br *Broker) readDatabaseInReplica(ctx context.Context, repl *meta.Replica) ([]*influxmeta.DatabaseInfo, error) {
	var dbInfo []*influxmeta.DatabaseInfo

	// retry read if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {

		if ctx.Err() != nil {
			return dbInfo, fmt.Errorf("context cancelled %s", ctx.Err())
		}

		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return dbInfo, cerr
		}

		// req message
		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
		}

		// read database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.ReadDatabases(ctx, &req)
		if err == nil {
			if err = json.Unmarshal([]byte(resp.Status), &dbInfo); err == nil {
				return dbInfo, nil
			}
		}

		br.logger.Warnf("failed reading databases on node %s. Err: %v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}

	return dbInfo, fmt.Errorf("failed reading databases")
}

// ReadDatabases reads all databases
func (br *Broker) ReadDatabases(ctx context.Context) ([]*influxmeta.DatabaseInfo, error) {

	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("shard map is empty")
	}

	// select random shard
	shard := cl.ShardMap.Shards[rand.Int63n(int64(len(cl.ShardMap.Shards)))]
	// read from primary
	repl, ok := shard.Replicas[shard.PrimaryReplica]
	if ok != true {
		br.logger.Errorf("failed to find replicas for %v from %+v", shard.PrimaryReplica, shard.Replicas)
		return nil, fmt.Errorf("failed to get replicas")
	}

	br.logger.Infof("reading database from shard %d primary replica %d", repl.ShardID, repl.ReplicaID)
	dbInfo, err := br.readDatabaseInReplica(ctx, repl)
	if err != nil {
		br.logger.Errorf("Error reading databases from replica %v. Err: %v", repl, err)
		return nil, err
	}
	return dbInfo, nil
}

// WritePoints writes points to data nodes
func (br *Broker) WritePoints(ctx context.Context, database string, points []models.Point) error {
	pointsMap := make(map[uint64]models.Points)
	replMap := make(map[uint64]*meta.Replica)

	// get the shardmap
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// walk all the points and break it up into per shard list
	for _, pt := range points {
		msrmt := string(pt.Name())

		// get shard for the point, note: tag keys are sorted
		shard, err := cl.ShardMap.GetShardForPoint(database, msrmt, string(pt.Tags().HashKey()))
		if err != nil {
			br.logger.Errorf("Error getting shard for %s/%s. Err: %v", database, msrmt, err)
			return err
		}

		// get the primary shard
		pri, err := shard.GetPrimaryreplica()
		if err != nil {
			br.logger.Errorf("Could not get the primary replica for %+v. Err: %v", shard, err)
			return err
		}

		// append it to per shard list
		pointsMap[pri.ShardID] = append(pointsMap[pri.ShardID], pt)
		replMap[pri.ShardID] = pri
	}

	//TODO: batch process

	// walk the per shard list and write it to each node
	for sid, points := range pointsMap {
		// converts points to string
		var data bytes.Buffer
		for _, pt := range points {
			_, err := data.WriteString(pt.PrecisionString("n"))
			if err != nil {
				return err
			}
			err = data.WriteByte('\n')
			if err != nil {
				return err
			}
		}

		// build the request
		req := &tproto.PointsWriteReq{
			ClusterType: meta.ClusterTypeTstore,
			Database:    database,
			ReplicaID:   replMap[sid].ReplicaID,
			ShardID:     replMap[sid].ShardID,
			Points:      data.String(),
		}

		if err := br.writePointsInReplica(ctx, replMap[sid].NodeUUID, req); err != nil {
			return err
		}
	}

	return nil
}

// writePointsInReplica writes points to a replica in the data node
func (br *Broker) writePointsInReplica(ctx context.Context, nodeuuid string, req *tproto.PointsWriteReq) error {
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(nodeuuid, meta.ClusterTypeTstore)
		if err != nil {
			return err
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.PointsWrite(ctx, req)
		if err == nil {
			if resp.Status == "" {
				return nil
			}
			br.logger.Errorf("points write failed with status %v", resp.Status)
			continue
		}

		br.logger.Errorf("Error making PointsWrite rpc call. Err: %v, rpc status: %v, Node: %v", err, rpcClient.GetState(), nodeuuid)
		// trigger db creation
		if strings.Contains(err.Error(), "database not found") {
			if dbErr := br.CreateDatabase(ctx, req.Database); dbErr != nil {
				br.logger.Errorf("failed to create database %s, %v", req.Database, dbErr)
			}
			// continue
		}

		// reconnect
		if strings.Contains(err.Error(), "connection error") {
			br.DeleteRPCClient(nodeuuid)
			// continue
		}
	}
	return fmt.Errorf("retries exhaused for points write to %v, %+v", nodeuuid, req)
}

// queryShard queries replicas in a shard till it gets a successful response
func (br *Broker) queryShard(ctx context.Context, shard *meta.Shard, database, qry string) (*tproto.QueryResp, error) {
	for _, repl := range shard.Replicas {
		// get an rpc client
		rpcClient, err := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if err != nil {
			continue
		}

		// rpc request
		req := tproto.QueryReq{
			ClusterType: meta.ClusterTypeTstore,
			Database:    database,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Query:       qry,
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.ExecuteQuery(ctx, &req)
		if err != nil {
			br.logger.Errorf("Error during ExecuteQuery rpc call. Err: %v", err)
			continue
		}

		return resp, nil
	}

	return nil, errors.New("Query to allreplicas failed")
}

// ExecuteQuerySingle executes a query on data nodes, deprecated
func (br *Broker) ExecuteQuerySingle(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	// parse the query
	pq, err := influxql.ParseQuery(qry)
	if err != nil {
		return nil, err
	}

	// get the cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("Shard map is empty")
	}

	// parse each statement
	var results []*query.Result
	for i, stmt := range pq.Statements {
		if selStmt, ok := stmt.(*influxql.SelectStatement); ok {
			// get the measurement name
			if len(selStmt.Sources.Measurements()) != 1 {
				return nil, errors.New("Query must have only one measurement")
			}

			for _, measurement := range selStmt.Sources.Measurements() {
				if measurement.Name == "Fwlogs" {
					return nil, errors.New("fwlogs not supported")
				}

				shard, err := cl.ShardMap.GetShardForPoint(database, measurement.Name, "")
				if err != nil {
					br.logger.Errorf("Error getting shard for %s/%s. Err: %v", database, measurement.Name, err)
					return nil, err
				}

				resp, err := br.queryShard(ctx, shard, database, selStmt.String())
				if err != nil {
					br.logger.Errorf("Error during ExecuteQuery rpc call. Err: %v", err)
					return nil, err
				}

				// parse the response
				for _, rs := range resp.Result {
					rslt := query.Result{}
					err := rslt.UnmarshalJSON(rs.Data)
					if err != nil {
						return nil, err
					}
					// Since we are making each query individually,
					// we need to manually set the StatementID
					rslt.StatementID = i
					results = append(results, &rslt)
				}
			}
		}
	}

	return results, nil
}

// ExecuteQuery executes a query on data nodes
func (br *Broker) ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	return br.ExecuteQuerySingle(ctx, database, qry)
}

// WriteLines writes influx line protocol
func (br *Broker) WriteLines(ctx context.Context, database string, lines []string) error {
	return nil
}

// ExecuteShowCmd executes a show command on data nodes
func (br *Broker) ExecuteShowCmd(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	if database == "" {
		return nil, fmt.Errorf("database name is requird")
	}

	if !strings.HasPrefix(qry, "SHOW") {
		return nil, fmt.Errorf("invalid show command")
	}

	// parse the query
	pq, err := influxql.ParseQuery(qry)
	if err != nil {
		return nil, err
	}

	if len(pq.Statements) != 1 {
		return nil, fmt.Errorf("invalid show command %+v", pq)
	}

	stmt := pq.Statements[0]

	// handle only these commands
	switch stmt.(type) {
	case *influxql.ShowSeriesStatement:
	case *influxql.ShowMeasurementsStatement:
	case *influxql.ShowFieldKeysStatement:
	case *influxql.ShowTagKeysStatement:
	case *influxql.ShowTagValuesStatement:
	default:
		return nil, fmt.Errorf("invalid show command %s", stmt.String())
	}

	// get the cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("Shard map is empty")
	}

	// todo: use channel for async query
	var result query.Result
	for _, shard := range cl.ShardMap.Shards {
		resp, err := br.queryShard(ctx, shard, database, stmt.String())
		if err != nil {
			br.logger.Errorf("shard [%d] Error during ExecuteQuery rpc call. Err: %v", shard.ShardID, err)
			return nil, err
		}

		// show series: {Name: Tags:map[] Columns:[key] Values:[[SmartNIC,Kind=SmartNIC,Name=f60b.595f.eca3,reporterID=Node-venice]] Partial:false}
		// show measurements {Name:measurements Tags:map[] Columns:[name] Values:[[SmartNIC]] Partial:false}
		// show field keys {Name:SmartNIC Tags:map[] Columns:[fieldKey fieldType] Values:[[CPUUsedPercent float] [DiskFree float] [DiskTotal float] [DiskUsed float] [DiskUsedPercent float] [InterfaceName string] [InterfaceRxBytes float] [InterfaceTxBytes float] [MemAvailable float] [MemFree float] [MemTotal float] [MemUsed float] [MemUsedPercent float]] Partial:false}
		// show tag keys {Name:Node Tags:map[] Columns:[tagKey] Values:[[Kind] [Name] [reporterID]] Partial:false}

		// parse the response
		for _, rs := range resp.Result {
			rslt := query.Result{}
			err := rslt.UnmarshalJSON(rs.Data)

			if err != nil {
				return nil, fmt.Errorf("shard [%d] unmarshal error: %s", shard.ShardID, err)
			}

			if rslt.Err != nil {
				return nil, fmt.Errorf("shard [%d] query returned error: %s", shard.ShardID, rslt.Err)
			}

			// merge fields
			if err := mergo.Merge(&result, rslt, mergo.WithAppendSlice); err != nil {
				return nil, fmt.Errorf("shard [%d] failed to merge results %s", shard.ShardID, err)
			}
		}
	}

	// merge rows having the same name
	valMap := map[string]*models.Row{}
	for _, s := range result.Series {
		if val, ok := valMap[s.Name]; ok {
			// remove duplicates
			for _, nVal := range s.Values {
				if func() bool {
					for _, v := range val.Values {
						if reflect.DeepEqual(nVal, v) {
							return true
						}
					}
					return false
				}() != true {
					val.Values = append(val.Values, nVal)
				}
			}
		} else {
			valMap[s.Name] = s
		}
	}

	series := models.Rows{}
	for _, r := range valMap {
		series = append(series, r)
	}
	result.Series = series
	return []*query.Result{&result}, nil
}
