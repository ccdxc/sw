// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker_test

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"
	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// createNode creates a node
func createDnode(nodeUUID, nodeURL, dbPath string, logger log.Logger) (*data.DNode, error) {
	cfg := meta.DefaultClusterConfig()
	cfg.DeadInterval = time.Millisecond * 500
	cfg.NodeTTL = 5
	cfg.RebalanceDelay = time.Second
	cfg.RebalanceInterval = time.Millisecond * 10

	// create the data node
	dn, err := data.NewDataNode(cfg, nodeUUID, nodeURL, dbPath, logger)
	if err != nil {
		return nil, err
	}

	return dn, nil
}

// createBroker creates a broker
func createBroker(nodeUUID string, logger log.Logger) (*broker.Broker, error) {
	br, err := broker.NewBroker(meta.DefaultClusterConfig(), nodeUUID, logger)
	if err != nil {
		return nil, err
	}

	return br, nil
}

func TestBrokerTstoreBasic(t *testing.T) {
	const numNodes = 4
	dnodes := make([]*data.DNode, numNodes)
	brokers := make([]*broker.Broker, numNodes)
	var err error
	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("tstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		qpath, err := ioutil.TempDir("", fmt.Sprintf("qstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")

		defer os.RemoveAll(qpath)
		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), "localhost:0", path, logger)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the brokers
	for idx := 0; idx < numNodes; idx++ {
		brokers[idx], err = createBroker(fmt.Sprintf("node-%d", idx), logger)
		AssertOk(t, err, "Error creating broker")
	}

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if len(brokers[0].GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range brokers[0].GetCluster(meta.ClusterTypeTstore).NodeMap {
			if nd.NumShards != (meta.DefaultShardCount*meta.DefaultReplicaCount)/numNodes {
				return false, []interface{}{brokers[0].GetCluster(meta.ClusterTypeTstore), nd}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "20s")

	// create the database
	err = brokers[0].CreateDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error creating database")

	// create a dummy db
	err = brokers[0].CreateDatabaseWithRetention(context.Background(), "dummy", 0)
	AssertOk(t, err, "Error creating database")

	err = brokers[0].DeleteDatabase(context.Background(), "dummy")
	AssertOk(t, err, "Error deleting database")

	// read databases
	AssertEventually(t, func() (bool, interface{}) {
		dbList, err := brokers[0].ReadDatabases(context.Background())
		if err != nil {
			return false, err
		}

		if len(dbList) != 1 {
			return false, fmt.Errorf("invalid number of dbs, %+v", dbList)
		}
		for _, db := range dbList {
			if db.Name != "db0" {
				return false, fmt.Errorf("invalid db name, %+v", db)
			}
		}

		return true, nil
	}, "Error reading database")

	schema := map[string]map[string]bool{}

	// write some points
	for idx := 0; idx < numNodes; idx++ {
		// create some points
		data := fmt.Sprintf("cpu%d,host=serverB,svc=nginx value1=11,value2=12 %v\n", idx, time.Now().UnixNano()) +
			fmt.Sprintf("cpu%d,host=serverC,svc=nginx value1=21,value2=22  %v\n", idx, time.Now().UnixNano())
		points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
		AssertOk(t, err, "Error parsing points")

		// write the points
		err = brokers[idx].WritePoints(context.Background(), "db0", points)
		AssertOk(t, err, "Error writing points")

		// store schema
		schema[fmt.Sprintf("cpu%d", idx)] = map[string]bool{
			"host": true,
			"svc":  true,
		}
	}

	// read the points back
	for idx := 0; idx < numNodes; idx++ {
		results, err := brokers[idx].ExecuteQuery(context.Background(), "db0", fmt.Sprintf("SELECT * FROM cpu%d", idx))
		AssertOk(t, err, "Error executing query")
		restr, _ := json.Marshal(results)
		log.Infof("Got result: %v", string(restr))

		// query deprecated apis
		_, err = brokers[0].ExecuteQuerySingle(context.Background(), "db0", fmt.Sprintf("SELECT * FROM cpu%d", idx))
		AssertOk(t, err, "Error executing query-single")

		// test show commands

		// SHOW TAG KEYS
		results, err = brokers[idx].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SHOW TAG KEYS"))
		AssertOk(t, err, "Error executing show command")
		Assert(t, len(results) == 1, "invalid number of results %+v", results)
		AssertOk(t, results[0].Err, "error in show command results %+v", results[0])
		Assert(t, len(results[0].Series) == len(schema), "invalid series in show command %+v", results[0].Series)
		for _, series := range results[0].Series {
			sc, ok := schema[series.Name]
			Assert(t, ok, "failed to find %s in %+v", series.Name, schema)
			Assert(t, len(series.Values) == len(sc), "expected %+v, got %+v", sc, series.Values)
			for _, k := range series.Values {
				Assert(t, len(k) == 1, "invalid number of values %+v", k)
				s, ok := k[0].(string)
				Assert(t, ok, "invalid values type %+v", k)
				_, ok = sc[s]
				Assert(t, ok, "%s not found in schema values type %+v", s)
			}
			Assert(t, len(series.Columns) == 1 && series.Columns[0] == "tagKey", "invalid column %s", series.Columns)
		}
		restr, err = json.Marshal(results)
		AssertOk(t, err, "Error to marshal show command results")

		// SHOW MEASUREMENTS
		results, err = brokers[idx].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SHOW MEASUREMENTS"))
		AssertOk(t, err, "Error executing show command")
		Assert(t, len(results) == 1, "invalid number of results %+v", results)
		AssertOk(t, results[0].Err, "error in show command results %+v", results[0])
		Assert(t, len(results[0].Series) == 1, "invalid series in show command %+v", results[0].Series)

		s := results[0].Series[0]
		Assert(t, len(s.Values) == len(schema), "expected %+v, got %+v", schema[s.Name], s.Values)

		for _, k := range s.Values {
			Assert(t, len(k) == 1, "invalid number of values %+v", k)
			s, ok := k[0].(string)
			Assert(t, ok, "invalid value type %+v", k)
			_, ok = schema[s]
			Assert(t, ok, "%s not found in schema value type %+v", s)
		}
		Assert(t, len(s.Columns) == 1 && s.Columns[0] == "name", "invalid column %s", s.Columns)

		restr, err = json.Marshal(results)
		AssertOk(t, err, "Error to marshal show command results")

		// SHOW SERIES
		results, err = brokers[idx].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SHOW SERIES"))
		AssertOk(t, err, "Error executing show command")
		Assert(t, len(results) == 1, "invalid number of results %+v", results)
		AssertOk(t, results[0].Err, "error in show command results %+v", results[0])
		Assert(t, len(results[0].Series) == 1, "invalid series in show command %+v", results[0].Series)

		s = results[0].Series[0]
		Assert(t, len(s.Values) == len(schema)*2, "invalid values, got %+v", s.Values)
		Assert(t, len(s.Columns) == 1 && s.Columns[0] == "key", "invalid column %s", s.Columns)
		restr, err = json.Marshal(results)
		AssertOk(t, err, "Error to marshal show command results")

		// SHOW FIELD KEYS
		results, err = brokers[idx].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SHOW FIELD KEYS"))
		AssertOk(t, err, "Error executing show command")
		Assert(t, len(results) == 1, "invalid number of results %+v", results)
		AssertOk(t, results[0].Err, "error in show command results %+v", results[0])
		Assert(t, len(results[0].Series) == len(schema), "invalid series in show command %+v", results[0].Series)

		for _, s = range results[0].Series {
			Assert(t, len(s.Values) == 2, "invalid values, got %+v", s.Values)
			Assert(t, len(s.Columns) == 2, "invalid column %+v", s.Columns)
		}
		restr, err = json.Marshal(results)
		AssertOk(t, err, "Error to marshal show command results")
	}

	// Test errors in show command
	_, err = brokers[0].ExecuteShowCmd(context.Background(), "", fmt.Sprintf("SHOW SERIES"))
	Assert(t, err != nil, "query didin't fail for invalid db")
	_, err = brokers[0].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SERIES"))
	Assert(t, err != nil, "query didin't fail for invalid command")
	_, err = brokers[0].ExecuteShowCmd(context.Background(), "db0", fmt.Sprintf("SHOW DATABASES"))
	Assert(t, err != nil, "query didin't fail for invalid command")

	// Perform query with multiple statements
	queries := make([]string, 0)
	for idx := 0; idx < numNodes; idx++ {
		queries = append(queries, fmt.Sprintf("SELECT * FROM cpu%d", idx))
	}
	queryString := strings.Join(queries, ";")
	results, err := brokers[0].ExecuteQuery(context.Background(), "db0", queryString)
	AssertOk(t, err, "Error executing query")
	Assert(t, len(results) == numNodes, "got invalid number of results", results)
	Assert(t, len(results[0].Series) == 1, "got invalid number of series", results[0].Series)
	Assert(t, len(results[1].Series) == 1, "got invalid number of series", results[1].Series)

	// delete the database
	err = brokers[0].DeleteDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error deleting database")

	// stop all brokers and data nodes
	for idx := 0; idx < numNodes; idx++ {
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
		Assert(t, brokers[idx].IsStopped() == false, "Incorrect broker state")
		err = brokers[idx].Stop()
		AssertOk(t, err, "Error stopping broker")
		Assert(t, brokers[idx].IsStopped() == true, "Incorrect broker state")
	}

	// test broker commands
	err = brokers[0].CreateDatabase(context.Background(), "db0")
	Assert(t, err != nil, "create database didn't fail")
	err = brokers[0].DeleteDatabase(context.Background(), "db0")
	Assert(t, err != nil, "delete database didn't fail")
	_, err = brokers[0].ReadDatabases(context.Background())
	Assert(t, err != nil, "read database didn't fail")
	err = brokers[0].WritePoints(context.Background(), "db0", []models.Point{})
	Assert(t, err != nil, "query database didn't fail")
	_, err = brokers[0].ExecuteQuery(context.Background(), "db0", "SELECT * frpm cpu")
	Assert(t, err != nil, "query database didn't fail")
	_, err = brokers[0].ExecuteShowCmd(context.Background(), "db0", "SHOW mesurements")
	Assert(t, err != nil, "show commands didn't fail")

	// verify no node is a leader
	AssertEventually(t, func() (bool, interface{}) {
		for idx := 0; idx < numNodes; idx++ {
			return !dnodes[idx].IsLeader(), nil
		}
		return true, nil
	}, "Node is still a leader after stopping", "300ms", "30s")

	log.Infof("--------------Stopped all data nodes -------------")

	// delete the old cluster state
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeTstore)
	AssertOk(t, err, "Error deleting cluster state")
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeKstore)
	AssertOk(t, err, "Error deleting cluster state")
}

func TestBrokerKstoreBasic(t *testing.T) {
	const numNodes = 4
	dnodes := make([]*data.DNode, numNodes)
	brokers := make([]*broker.Broker, numNodes)
	var err error
	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("kstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		qpath, err := ioutil.TempDir("", fmt.Sprintf("qstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(qpath)

		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), "localhost:0", path, logger)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the brokers
	for idx := 0; idx < numNodes; idx++ {
		brokers[idx], err = createBroker(fmt.Sprintf("node-%d", idx), logger)
		AssertOk(t, err, "Error creating broker")
	}

	// try making some calls while cluster is not ready, they should fail
	// we really don't have a deterministic way to assert that
	// cluster won't be ready. Ignore unless we see the
	// cluster as not ready.
	validateError := func(err error, msg string) {
		if err == nil {
			err := brokers[0].ClusterCheck()
			Assert(t, err == nil, msg, err)
		}
	}

	kvs := []*tproto.KeyValue{
		{
			Key:   []byte(fmt.Sprintf("testKey0-%d", 0)),
			Value: []byte(fmt.Sprintf("testKey0-%d", 0)),
		},
	}
	keys := []*tproto.Key{
		{Key: []byte(fmt.Sprintf("testKey0-%d", 0))},
	}

	err = brokers[0].WriteKvs(context.Background(), "table0", kvs)
	validateError(err, "writing keys suceeded while cluster is not ready")
	_, err = brokers[0].ReadKvs(context.Background(), "table0", keys)
	validateError(err, "reading keys suceeded while cluster is not ready")
	listKeys, err := brokers[0].ListKvs(context.Background(), "table0")
	if err == nil && len(listKeys) > 0 {
		log.Fatalf("listing keys suceeded while cluster is not ready, %v", listKeys)
	}
	err = brokers[0].DeleteKvs(context.Background(), "table0", keys)
	validateError(err, "deleting keys suceeded while cluster is not ready")

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if brokers[0].ClusterCheck() != nil {
			return false, nil
		}

		if len(brokers[0].GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range brokers[0].GetCluster(meta.ClusterTypeKstore).NodeMap {
			if nd.NumShards != (meta.DefaultShardCount*meta.DefaultReplicaCount)/numNodes {
				return false, []interface{}{brokers[0].GetCluster(meta.ClusterTypeKstore), nd}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "20s")

	// write some key-value pairs
	for idx := 0; idx < numNodes; idx++ {
		kvs := []*tproto.KeyValue{
			{
				Key:   []byte(fmt.Sprintf("testKey1-%d", idx)),
				Value: []byte(fmt.Sprintf("testVal1-%d", idx)),
			},
			{
				Key:   []byte(fmt.Sprintf("testKey2-%d", idx)),
				Value: []byte(fmt.Sprintf("testVal2-%d", idx)),
			},
		}

		// write the kv pairs
		err = brokers[idx].WriteKvs(context.Background(), "table0", kvs)
		AssertOk(t, err, "Error writing kvs")
	}

	// read the keys back
	for idx := 0; idx < numNodes; idx++ {
		keys := []*tproto.Key{
			{Key: []byte(fmt.Sprintf("testKey1-%d", idx))},
			{Key: []byte(fmt.Sprintf("testKey2-%d", idx))},
		}

		// read the keys
		result, err := brokers[idx].ReadKvs(context.Background(), "table0", keys)
		AssertOk(t, err, "Error reading the keys")
		Assert(t, (len(result) == 2), "Got invalid number of kv-pairs")
	}

	// list all the keys
	for idx := 0; idx < numNodes; idx++ {
		kvl, err := brokers[idx].ListKvs(context.Background(), "table0")
		AssertOk(t, err, "Error listing all the keys")
		Assert(t, (len(kvl) == 2*numNodes), "Got invalid number of kv-pairs")
	}

	// delete the keys
	for idx := 0; idx < numNodes; idx++ {
		keys := []*tproto.Key{
			{Key: []byte(fmt.Sprintf("testKey1-%d", idx))},
			{Key: []byte(fmt.Sprintf("testKey2-%d", idx))},
		}

		// read the keys
		err := brokers[idx].DeleteKvs(context.Background(), "table0", keys)
		AssertOk(t, err, "Error deleting the keys")
	}

	// inject a primary replica error and attempt a write
	meta.SetErrorRet(fmt.Errorf("Fakest fake error"))
	kvsA := []*tproto.KeyValue{
		{
			Key:   []byte("key11"),
			Value: []byte("val11"),
		},
	}
	err = brokers[0].WriteKvs(context.Background(), "table0", kvsA)
	Assert(t, (err != nil), "Write succeeded without primary replica")
	meta.SetErrorRet(nil)
	err = brokers[0].WriteKvs(context.Background(), "table0", kvsA)
	Assert(t, (err == nil), "Write failed expected success")
	keys = []*tproto.Key{
		{Key: []byte("key11")},
	}
	meta.SetErrorRet(fmt.Errorf("Fakest fake error"))
	err = brokers[0].DeleteKvs(context.Background(), "table0", keys)
	Assert(t, (err != nil), "Delete succeeded without primary replica")
	meta.SetErrorRet(nil)
	err = brokers[0].DeleteKvs(context.Background(), "table0", keys)
	Assert(t, (err == nil), "Delete failed, expected success")

	// stop all brokers and data nodes
	for idx := 0; idx < numNodes; idx++ {
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
		err = brokers[idx].Stop()
		AssertOk(t, err, "Error stopping broker")
	}

	// verify no node is a leader
	AssertEventually(t, func() (bool, interface{}) {
		for idx := 0; idx < numNodes; idx++ {
			return !dnodes[idx].IsLeader(), nil
		}
		return true, nil
	}, "Node is still a leader after stopping", "300ms", "30s")

	log.Infof("--------------Stopped all data nodes -------------")

	// delete the old cluster state
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeKstore)
	AssertOk(t, err, "Error deleting cluster state")
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeTstore)
	AssertOk(t, err, "Error deleting cluster state")
}

func TestBrokerTstoreWriteRetry(t *testing.T) {
	const numNodes = 4
	dnodes := make([]*data.DNode, numNodes)
	brokers := make([]*broker.Broker, numNodes)
	var err error
	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("tstore-%s-%d-", t.Name(), idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		qpath, err := ioutil.TempDir("", fmt.Sprintf("qstore-%s-%d-", t.Name(), idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(qpath)

		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), "localhost:0", path, logger)
		AssertOk(t, err, "Error creating nodes")
		defer dnodes[idx].Stop()
	}

	// create the brokers
	for idx := 0; idx < numNodes; idx++ {
		brokers[idx], err = createBroker(fmt.Sprintf("node-%d", idx), logger)
		AssertOk(t, err, "Error creating broker")
		defer brokers[idx].Stop()
	}

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		for idx := 0; idx < numNodes; idx++ {
			if len(brokers[idx].GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes {
				return false, nil
			}
			for _, nd := range brokers[idx].GetCluster(meta.ClusterTypeTstore).NodeMap {
				if nd.NumShards != (meta.DefaultShardCount*meta.DefaultReplicaCount)/numNodes {
					return false, []interface{}{brokers[idx].GetCluster(meta.ClusterTypeTstore), nd}
				}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "60s")

	// write some points without creating db
	data := fmt.Sprintf("cpu%d,host=serverB,svc=nginx value1=11,value2=12 %v\n", 0, time.Now().UnixNano()) +
		fmt.Sprintf("cpu%d,host=serverC,svc=nginx value1=21,value2=22  %v\n", 0, time.Now().UnixNano())
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
	AssertOk(t, err, "Error parsing points")

	// write the points
	err = brokers[0].WritePoints(context.Background(), "db0", points)
	AssertOk(t, err, "Error writing points")

	// delete rpc client
	for idx := range dnodes {
		brokers[0].DeleteRPCClient(fmt.Sprintf("node-%d", idx))
	}

	// write the points, should reconnect
	err = brokers[0].WritePoints(context.Background(), "db0", points)
	AssertOk(t, err, "Error writing points")

	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeTstore)
	AssertOk(t, err, "Error deleting cluster state")
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeKstore)
	AssertOk(t, err, "Error deleting cluster state")
}

func TestBrokerBenchmark(t *testing.T) {
	const numNodes = 4
	var batchSize = 1000
	const numIterations = 100
	dnodes := make([]*data.DNode, numNodes)
	var err error
	logger := log.GetNewLogger(log.GetDefaultConfig(t.Name()))

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("%s-broker-%d-", t.Name(), idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), "localhost:0", path, logger)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the broker
	broker, err := createBroker(fmt.Sprintf("node-0"), logger)
	AssertOk(t, err, "Error creating broker")

	t.Skip("TODO: Reenable after https://github.com/pensando/sw/issues/11893 is fixed")

	AssertEventually(t, func() (bool, interface{}) {
		if err := broker.ClusterCheck(); err != nil {
			log.Errorf("cluster check %v", err)
			return false, err
		}
		return true, nil
	}, "cluster check failed", "1s", "30s")

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if len(broker.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range broker.GetCluster(meta.ClusterTypeTstore).NodeMap {
			if nd.NumShards != (meta.DefaultShardCount*meta.DefaultReplicaCount)/numNodes {
				return false, []interface{}{broker.GetCluster(meta.ClusterTypeTstore), nd}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "30s")

	// create the database
	err = broker.CreateDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error creating database")

	// measure how long it takes to parse the points
	points := make([]models.Points, numIterations)
	startTime := time.Now()
	// pick a timestamp in the retention range, chose now() - 6 days
	pointStartTime := int(startTime.UnixNano() - time.Duration(6*24*time.Hour).Nanoseconds())

	for iter := 0; iter < numIterations; iter++ {
		// parse some points
		var data string
		for i := 0; i < batchSize; i++ {
			data += fmt.Sprintf("cpu%d,host=server%d,svc=nginx%d value1=%d,value2=%d %v\n", i, i, i, (i + batchSize*iter), (i + batchSize*iter), pointStartTime+(i+20+batchSize*iter))
		}
		points[iter], err = models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
		AssertOk(t, err, "Error parsing points")
	}
	log.Warnf("%d iterations at batch size %d parsing points took %v ", numIterations, batchSize, time.Since(startTime).String())

	// measure how long writing points takes
	startTime = time.Now()
	for iter := 0; iter < numIterations; iter++ {
		// write the points
		err = broker.WritePoints(context.Background(), "db0", points[iter])
		AssertOk(t, err, "Error writing points")
	}
	log.Warnf("%d iterations at batch size %d writing points took %v ", numIterations, batchSize, time.Since(startTime).String())

	// read the points back
	for iter := 0; iter < numIterations; iter++ {
		results, err := broker.ExecuteQuery(context.Background(), "db0", fmt.Sprintf("SELECT * FROM cpu%d", iter))
		AssertOk(t, err, "Error executing query")
		Assert(t, len(results) == 1, "got invalid number of results", results)
		Assert(t, len(results[0].Series) == 1, "got invalid number of series", results[0].Series)
		Assert(t, len(results[0].Series[0].Values) == numIterations, "got invalid number of values", len(results[0].Series[0].Values))
	}
	log.Warnf("%d iterations at batch size %d executing query took %v ", numIterations, batchSize, time.Since(startTime).String())

	// delete the database
	err = broker.DeleteDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error deleting database")

	// delete non-exisiting database
	err = broker.DeleteDatabase(context.Background(), "db0")
	AssertOk(t, err, "deleting non-existing database failed")

	// stop all brokers and data nodes
	for idx := 0; idx < numNodes; idx++ {
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
	}
	err = broker.Stop()
	AssertOk(t, err, "Error stopping broker")

	// delete the old cluster state
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeTstore)
	AssertOk(t, err, "Error deleting cluster state")
	err = meta.DestroyClusterState(meta.DefaultClusterConfig(), meta.ClusterTypeKstore)
	AssertOk(t, err, "Error deleting cluster state")
}
