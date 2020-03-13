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
	"strconv"
	"strings"
	"time"

	"github.com/imdario/mergo"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	influxmeta "github.com/influxdata/influxdb/services/meta"
	"github.com/influxdata/influxql"

	cq "github.com/pensando/sw/venice/citadel/broker/continuous_query"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
)

func init() {
	cq.InitContinuousQueryMap()
}

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

	err := br.RunContinuousQueries(ctx)
	if err != nil {
		br.logger.Errorf("Error launching continuous query for database %v. Err: %v", database, err)
		return err
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

// sendQueryRequest sned query request for specific replica
func (br *Broker) sendQueryRequest(ctx context.Context, database string, qry string, repl *meta.Replica) (*tproto.QueryResp, error) {
	// get an rpc client
	rpcClient, err := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
	if err != nil {
		return nil, err
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
		return nil, err
	}

	return resp, nil
}

// queryShard queries replicas in a shard till it gets a successful response
func (br *Broker) queryShard(ctx context.Context, shard *meta.Shard, database, qry string) (*tproto.QueryResp, error) {
	for _, repl := range shard.Replicas {
		resp, err := br.sendQueryRequest(ctx, database, qry, repl)
		if err != nil {
			continue
		}
		return resp, nil
	}

	return nil, errors.New("Query to all replicas failed")
}

// ExecuteQuerySingle executes a query on data nodes
func (br *Broker) ExecuteQuerySingle(ctx context.Context, database string, qry string, shardIn *meta.Shard) ([]*query.Result, error) {
	shard := shardIn
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

				originMeasurementName := measurement.Name
				if cq.IsContinuousQueryMeasurement(measurement.Name) {
					originMeasurementName = strings.Split(measurement.Name, "_")[0]
					suffix := strings.Split(measurement.Name, "_")[1]
					// lookup whether it is a valid CQ suffix or not
					_, ok := cq.RetentionPolicyMap[suffix]
					if !ok {
						br.logger.Errorf("Error using invalid suffix %v to query continuous query table. Err: %v", suffix, err)
						return nil, err
					}
					measurement.Database = database
					measurement.RetentionPolicy = cq.RetentionPolicyMap[suffix].Name
				}

				if shardIn == nil {
					shard, err = cl.ShardMap.GetShardForPoint(database, originMeasurementName, "")
					if err != nil {
						br.logger.Errorf("Error getting shard for %s/%s. Err: %v", database, measurement.Name, err)
						return nil, err
					}
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

// ExecuteQueryShard executes a query on shard, debug only
func (br *Broker) ExecuteQueryShard(ctx context.Context, database string, qry string, shardID uint) ([]*query.Result, error) {
	// get the cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("shard map is empty")
	}

	sm := cl.ShardMap

	if shardID >= uint(sm.NumShards) {
		return nil, fmt.Errorf("invalid shard, valid range 1-%d", sm.NumShards)
	}

	shard := sm.Shards[shardID]
	if shard == nil {
		return nil, fmt.Errorf("shard %d not found", shardID)
	}

	return br.ExecuteQuerySingle(ctx, database, qry, shard)
}

// queryReplica queries replicas in a replica till it gets a successful response
func (br *Broker) queryReplica(ctx context.Context, shard *meta.Shard, database, qry string, isPrimary bool) (*tproto.QueryResp, error) {
	if isPrimary {
		// run query for primary replica
		repl, ok := shard.Replicas[shard.PrimaryReplica]
		if !ok {
			return nil, fmt.Errorf("Error Failed to get primary replica for shard %v", shard)
		}
		return br.sendQueryRequest(ctx, database, qry, repl)
	}
	for _, repl := range shard.Replicas {
		// only run query for non-primary replica
		// return result for the first successful query on non-primary replica
		if repl.ReplicaID != shard.PrimaryReplica {
			resp, err := br.sendQueryRequest(ctx, database, qry, repl)
			if err != nil {
				continue
			}
			return resp, nil
		}
	}

	return nil, errors.New("Query to all replicas failed")
}

// ExecuteQuerySingleReplica executes a query on data nodes
func (br *Broker) ExecuteQuerySingleReplica(ctx context.Context, database string, qry string, shardIn *meta.Shard, isPrimary bool) ([]*query.Result, error) {
	shard := shardIn
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

				originMeasurementName := measurement.Name
				if cq.IsContinuousQueryMeasurement(measurement.Name) {
					originMeasurementName = strings.Split(measurement.Name, "_")[0]
					suffix := strings.Split(measurement.Name, "_")[1]
					// lookup whether it is a valid CQ suffix or not
					_, ok := cq.RetentionPolicyMap[suffix]
					if !ok {
						br.logger.Errorf("Error using invalid suffix %v to query continuous query table. Err: %v", suffix, err)
						return nil, err
					}
					measurement.Database = database
					measurement.RetentionPolicy = cq.RetentionPolicyMap[suffix].Name
				}

				if shardIn == nil {
					shard, err = cl.ShardMap.GetShardForPoint(database, originMeasurementName, "")
					if err != nil {
						br.logger.Errorf("Error getting shard for %s/%s. Err: %v", database, measurement.Name, err)
						return nil, err
					}
				}

				resp, err := br.queryReplica(ctx, shard, database, selStmt.String(), isPrimary)
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

// ExecuteQueryReplica executes a query on replica, debug only
func (br *Broker) ExecuteQueryReplica(ctx context.Context, database string, qry string, shardID uint, isPrimary bool) ([]*query.Result, error) {
	// get the cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("shard map is empty")
	}

	sm := cl.ShardMap

	if shardID >= uint(sm.NumShards) {
		return nil, fmt.Errorf("invalid shard, valid range 1-%d", sm.NumShards)
	}

	shard := sm.Shards[shardID]
	if shard == nil {
		return nil, fmt.Errorf("shard %d not found", shardID)
	}

	return br.ExecuteQuerySingleReplica(ctx, database, qry, shard, isPrimary)
}

// ExecuteQuery executes a query on data nodes
func (br *Broker) ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	return br.ExecuteQuerySingle(ctx, database, qry, nil)
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

// createRetentionPolicyInReplica create retention policy in replica
func (br *Broker) createRetentionPolicyInReplica(ctx context.Context, database string, rpName string, rpPeriod uint64, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.RetentionPolicyReq{
			ClusterType:     meta.ClusterTypeTstore,
			ReplicaID:       repl.ReplicaID,
			ShardID:         repl.ShardID,
			Database:        database,
			RetentionName:   rpName,
			RetentionPeriod: rpPeriod,
		}

		// make create retention policy call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.CreateRetentionPolicy(ctx, &req)
		if err == nil {
			br.logger.Infof("=>created the retention policy in database %+v on node %+v.", database, repl.NodeUUID)
			if resp.Status != "" {
				br.logger.Infof("Receive response status: %+v", resp.Status)
			}
			return nil
		}

		br.logger.Warnf("Error creating the retention policy on node %+v. Err: %+v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}

	return err
}

// CreateRetentionPolicy creates retention policy in database
func (br *Broker) CreateRetentionPolicy(ctx context.Context, database string, rpName string, rpPeriod uint64) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.createRetentionPolicyInReplica(ctx, database, rpName, rpPeriod, repl)
			if err != nil {
				br.logger.Errorf("Error creating retention policy in replica %v. Err: %v", repl, err)
				return err
			}
		}
	}
	return nil
}

// getRetentionPolicyInReplica create retention policy in replica
func (br *Broker) getRetentionPolicyInReplica(ctx context.Context, database string, repl *meta.Replica) ([]string, error) {
	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return nil, cerr
		}

		// req message
		req := tproto.RetentionPolicyReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Database:    database,
		}

		// make create retention policy call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.GetRetentionPolicy(ctx, &req)
		if err == nil {
			if resp.Status != "" {
				br.logger.Infof("Receive response status: %+v", resp.Status)
			}
			return strings.Split(resp.Status, " "), nil
		}

		br.logger.Warnf("Error getting the retention policy on node %+v. Err: %+v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}

	return nil, errors.New("Broker reaches max retry times")
}

// GetRetentionPolicy creates retention policy in database
func (br *Broker) GetRetentionPolicy(ctx context.Context, database string) ([]string, error) {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("Shard map is empty")
	}

	// select random shard
	shard := cl.ShardMap.Shards[rand.Int63n(int64(len(cl.ShardMap.Shards)))]
	// read from primary
	repl, ok := shard.Replicas[shard.PrimaryReplica]
	if !ok {
		return nil, fmt.Errorf("Error getting primary replica for shard %v", shard)
	}
	rpList, err := br.getRetentionPolicyInReplica(ctx, database, repl)
	if err != nil {
		br.logger.Errorf("Error getting retention policy in replica %v. Err: %v", repl, err)
		return nil, err
	}
	return rpList, nil
}

// deleteRetentionPolicyInReplica delete retention policy in replica
func (br *Broker) deleteRetentionPolicyInReplica(ctx context.Context, database string, rpName string, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.RetentionPolicyReq{
			ClusterType:   meta.ClusterTypeTstore,
			ReplicaID:     repl.ReplicaID,
			ShardID:       repl.ShardID,
			Database:      database,
			RetentionName: rpName,
		}

		// make create retention policy call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.DeleteRetentionPolicy(ctx, &req)
		if err == nil {
			br.logger.Infof("=>deleted the retention policy in database %+v on node %+v.", database, repl.NodeUUID)
			if resp.Status != "" {
				br.logger.Infof("Receive response status: %+v", resp.Status)
			}
			return nil
		}

		br.logger.Warnf("Error deleting the retention policy on node %+v. Err: %+v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}

	return err
}

// DeleteRetentionPolicy delete retention policy in database
func (br *Broker) DeleteRetentionPolicy(ctx context.Context, database string, rpName string) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.deleteRetentionPolicyInReplica(ctx, database, rpName, repl)
			if err != nil {
				br.logger.Errorf("Error deleting retention policy in replica %v. Err: %v", repl, err)
				return err
			}
		}
	}
	return nil
}

// createContinuousQueryInReplica creates the continuous query in replica
func (br *Broker) createContinuousQueryInReplica(ctx context.Context, database string, cqName string, rpName string, rpPeriod uint64, query string, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.ContinuousQueryReq{
			ClusterType:         meta.ClusterTypeTstore,
			ReplicaID:           repl.ReplicaID,
			ShardID:             repl.ShardID,
			Database:            database,
			ContinuousQueryName: cqName,
			RetentionName:       rpName,
			Query:               query,
		}

		// make create continuous query call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.CreateContinuousQuery(ctx, &req)
		if err == nil {
			br.logger.Infof("=>created the continuous query in database %+v on node %+v.", database, repl.NodeUUID)
			if resp.Status != "" {
				br.logger.Infof("Receive response status: %+v", resp.Status)
			}
			return nil
		}

		br.logger.Warnf("Error creating the continuous query on node %+v. Err: %+v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}

	return err
}

// CreateContinuousQuery creates continuous query in database
func (br *Broker) CreateContinuousQuery(ctx context.Context, database string, cqName string, rpName string, rpPeriod uint64, query string) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	parser := influxql.NewParser(strings.NewReader(query))
	stmt, err := parser.ParseStatement()
	if err != nil {
		return fmt.Errorf("Failed to parse query. Error: %v", err)
	}
	if st, ok := stmt.(*influxql.CreateContinuousQueryStatement); ok {
		measurement := strings.Trim(st.Source.Sources[0].String(), `"`)
		parsedSource := strings.Split(measurement, ".")
		if len(parsedSource) > 1 {
			measurement = parsedSource[len(parsedSource)-1]
		}
		shard, err := cl.ShardMap.GetShardForPoint(st.Database, measurement, "")
		if err != nil {
			return fmt.Errorf("Error get shard for creating continuous query")
		}
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.createContinuousQueryInReplica(ctx, database, cqName, rpName, rpPeriod, query, repl)
			if err != nil {
				br.logger.Errorf("Error creating the continuous query in replica %v. Err: %v", repl, err)
				return err
			}
		}
		return nil
	}
	return fmt.Errorf("Error receive invalid query for create continuous query api")
}

// getContinuousQueryInReplica reads cotinuous queries in replica
func (br *Broker) getContinuousQueryInReplica(ctx context.Context, database string, repl *meta.Replica) ([]string, error) {
	// retry read if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {

		if ctx.Err() != nil {
			return nil, fmt.Errorf("context cancelled %s", ctx.Err())
		}

		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return nil, cerr
		}

		// req message
		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Database:    database,
		}

		// read database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.GetContinuousQuery(ctx, &req)
		if err == nil {
			br.logger.Infof("=>get continuous queries from database on node %+v", repl.NodeUUID)
			if resp.Status != "" {
				return strings.Split(resp.Status, " "), nil
			}
			// If there is no continuous query in the replica, just return nil
			return nil, nil
		}

		br.logger.Warnf("failed reading databases on node %s. Err: %v. Retrying..", repl.NodeUUID, err)
		time.Sleep(brokerRetryDelay)
	}
	return nil, fmt.Errorf("failed reading continuous queries")
}

// GetContinuousQuery reads all continuous queries
func (br *Broker) GetContinuousQuery(ctx context.Context, database string, replicaID string) ([]string, error) {

	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return nil, errors.New("shard map is empty")
	}

	cqMap := map[string]bool{}
	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		for _, repl := range shard.Replicas {
			// if there is no replica specified, only query primary replica
			if replicaID == "" && !repl.IsPrimary {
				continue
			}
			// if there is replica specified, query expected replica
			if replicaID != "" && strconv.FormatUint(repl.ReplicaID, 10) != replicaID {
				continue
			}
			br.logger.Infof("reading continuous queries from shard %+v primary replica %+v", repl.ShardID, repl.ReplicaID)
			result, err := br.getContinuousQueryInReplica(ctx, database, repl)
			if err != nil {
				br.logger.Errorf("Error getting continuous query in the database in replica %v. Err: %v", repl, err)
				return nil, err
			}
			if result != nil {
				for _, cq := range result {
					cqMap[cq] = true
				}
			}
		}
	}
	allCQ := []string{}
	for k := range cqMap {
		allCQ = append(allCQ, k)
	}

	return allCQ, nil
}

// deleteContinuousQueryInReplica deletes the continuous query in replica
func (br *Broker) deleteContinuousQueryInReplica(ctx context.Context, database string, cq string, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if cerr != nil {
			return cerr
		}

		// req message
		req := tproto.ContinuousQueryReq{
			ClusterType:         meta.ClusterTypeTstore,
			ReplicaID:           repl.ReplicaID,
			ShardID:             repl.ShardID,
			Database:            database,
			ContinuousQueryName: cq,
		}

		// delete database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		_, err = dnclient.DeleteContinuousQuery(ctx, &req)
		if err == nil {
			br.logger.Infof("=>deleted the continuous query %+v in database %+v on node %+v.", cq, database, repl.NodeUUID)
			return nil
		}

		br.logger.Warnf("Error deleting the continuous query on node %s. Err: %v. Retrying..", repl.NodeUUID, err)

		time.Sleep(brokerRetryDelay)
	}

	return err
}

// DeleteContinuousQuery drops the continuous query
func (br *Broker) DeleteContinuousQuery(ctx context.Context, database string, cq string, targetMeasurement string) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("shard map is empty")
	}

	shard, err := cl.ShardMap.GetShardForPoint(database, targetMeasurement, "")
	if err != nil {
		return fmt.Errorf("Error get shard for deleting continuous query")
	}
	// walk all replicas in the shard
	for _, repl := range shard.Replicas {
		err := br.deleteContinuousQueryInReplica(ctx, database, cq, repl)
		if err != nil {
			br.logger.Errorf("Error deleting the database in replica %v. Err: %v", repl, err)
			return err
		}
	}

	return nil
}

// RunContinuousQueries Run continuous query through existed broker
func (br *Broker) RunContinuousQueries(ctx context.Context) error {
	// create corresponded continuous query for database
	for _, rpSpec := range cq.RetentionPolicyMap {
		err := br.CreateRetentionPolicy(ctx, "default", rpSpec.Name, rpSpec.Hours)
		if err != nil {
			br.logger.Errorf("Error creating retention policy for database default. Err: %v", err)
			return err
		}
	}

	for _, cqSpec := range cq.ContinuousQueryMap {
		err := br.CreateContinuousQuery(ctx, cqSpec.DBName, cqSpec.CQName,
			cqSpec.RetentionPolicyName, cqSpec.RetentionPolicyInHours, cqSpec.Query)
		if err != nil {
			return fmt.Errorf("Error creating continuous query %v. Err: %v", cqSpec.CQName, err)
		}
	}
	return nil
}
