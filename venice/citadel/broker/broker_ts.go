// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker

import (
	"bytes"
	"context"
	"errors"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
)

// createDatabaseInReplica creates the database in replica
func (br *Broker) createDatabaseInReplica(ctx context.Context, database string, repl *meta.Replica) error {
	var err error

	// retry creation if there are transient errors
	for i := 0; i < numBrokerRetries; i++ {
		// get the rpc client for the node
		rpcClient, cerr := br.getRPCClient(repl.NodeUUID, meta.ClusterTypeTstore)
		if err != nil {
			return cerr
		}

		// req message
		req := tproto.DatabaseReq{
			ClusterType: meta.ClusterTypeTstore,
			ReplicaID:   repl.ReplicaID,
			ShardID:     repl.ShardID,
			Database:    database,
		}

		// make create database call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		_, err = dnclient.CreateDatabase(ctx, &req)
		if err == nil {
			return nil
		}

		log.Warnf("Error creating the database on node %s. Err: %v. Retrying..", repl.NodeUUID, err)

		time.Sleep(brokerRetryDelay)
	}

	return err
}

// CreateDatabase creates the database
func (br *Broker) CreateDatabase(ctx context.Context, database string) error {
	// get cluster
	cl := br.GetCluster(meta.ClusterTypeTstore)
	if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
		return errors.New("Shard map is empty")
	}

	// walk all shards
	for _, shard := range cl.ShardMap.Shards {
		// walk all replicas in the shard
		for _, repl := range shard.Replicas {
			err := br.createDatabaseInReplica(ctx, database, repl)
			if err != nil {
				log.Errorf("Error creating the database in replica %v. Err: %v", repl, err)
				return err
			}
		}
	}

	return nil
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

		// get shard for the measurement
		shard, err := cl.ShardMap.GetShardForPoint(database, msrmt)
		if err != nil {
			log.Errorf("Error getting shard for %s/%s. Err: %v", database, msrmt, err)
			return err
		}

		// get the primary shard
		pri, err := shard.GetPrimaryreplica()
		if err != nil {
			log.Errorf("Could not get the primary replica for %+v. Err: %v", shard, err)
			return err
		}

		// append it to per shard list
		pointsMap[pri.ShardID] = append(pointsMap[pri.ShardID], pt)
		replMap[pri.ShardID] = pri
	}

	// walk the per shard list and write it to each node
	for sid, points := range pointsMap {
		// get the rpc client for the node
		rpcClient, err := br.getRPCClient(replMap[sid].NodeUUID, meta.ClusterTypeTstore)
		if err != nil {
			return err
		}

		// converts points to string
		var data bytes.Buffer
		for _, pt := range points {
			_, err = data.WriteString(pt.PrecisionString("n"))
			if err != nil {
				return err
			}
			err = data.WriteByte('\n')
			if err != nil {
				return err
			}
		}

		// build the request
		req := tproto.PointsWriteReq{
			ClusterType: meta.ClusterTypeTstore,
			Database:    database,
			ReplicaID:   replMap[sid].ReplicaID,
			ShardID:     replMap[sid].ShardID,
			Points:      data.String(),
		}

		// make the rpc call
		dnclient := tproto.NewDataNodeClient(rpcClient)
		resp, err := dnclient.PointsWrite(ctx, &req)
		if err != nil || resp.Status != "" {
			log.Errorf("Error making PointsWrite rpc call. Err: %v", err)
			return err
		}
	}

	return nil
}

// queryShard queries replicas in a shard till it gets a successfull response
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
			log.Errorf("Error during ExecuteQuery rpc call. Err: %v", err)
			continue
		}

		return resp, nil
	}

	return nil, errors.New("Query to allreplicas failed")
}

// ExecuteQuery executes a query on data nodes
func (br *Broker) ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	// parse the query
	pq, err := influxql.ParseQuery(qry)
	if err != nil {
		return nil, err
	}

	// parse each statement
	var results []*query.Result
	for _, stmt := range pq.Statements {
		if selStmt, ok := stmt.(*influxql.SelectStatement); ok {
			// get the measurement name
			for _, measurement := range selStmt.Sources.Measurements() {
				// get the cluster
				cl := br.GetCluster(meta.ClusterTypeTstore)
				if cl == nil || cl.ShardMap == nil || len(cl.ShardMap.Shards) == 0 {
					return nil, errors.New("Shard map is empty")
				}

				// get the shard
				shard, err := cl.ShardMap.GetShardForPoint(database, measurement.Name)
				if err != nil {
					log.Errorf("Error getting shard for %s/%s. Err: %v", database, measurement.Name, err)
					return nil, err
				}

				resp, err := br.queryShard(ctx, shard, database, qry)
				if err != nil {
					log.Errorf("Error during ExecuteQuery rpc call. Err: %v", err)
					return nil, err
				}

				// parse the response
				for _, rs := range resp.Result {
					rslt := query.Result{}
					err := rslt.UnmarshalJSON(rs.Data)
					if err != nil {
						return nil, err
					}
					results = append(results, &rslt)
				}
			}
		}
	}

	return results, nil
}
