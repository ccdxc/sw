// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package broker_test

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
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
func createDnode(nodeUUID, nodeURL, dbPath string) (*data.DNode, error) {
	cfg := meta.DefaultClusterConfig()
	cfg.DeadInterval = time.Millisecond * 500
	cfg.NodeTTL = 5
	cfg.RebalanceDelay = time.Second
	cfg.RebalanceInterval = time.Millisecond * 10

	// create the data node
	dn, err := data.NewDataNode(cfg, nodeUUID, nodeURL, dbPath)
	if err != nil {
		return nil, err
	}

	return dn, nil
}

// createBroker creates a broker
func createBroker(nodeUUID string) (*broker.Broker, error) {
	br, err := broker.NewBroker(meta.DefaultClusterConfig(), nodeUUID)
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

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("tstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)
		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:720%d", idx), path)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the brokers
	for idx := 0; idx < numNodes; idx++ {
		brokers[idx], err = createBroker(fmt.Sprintf("node-%d", idx))
		AssertOk(t, err, "Error creating broker")
	}

	// try making some calls while cluster is not ready, they should fail
	err = brokers[0].CreateDatabase(context.Background(), "db0")
	Assert(t, (err != nil), "Creating database suceeded while cluster is not ready")
	err = brokers[0].WritePoints(context.Background(), "db0", []models.Point{})
	Assert(t, (err != nil), "writing points suceeded while cluster is not ready")
	_, err = brokers[0].ExecuteQuery(context.Background(), "db0", "SELECT * FROM cpu0")
	Assert(t, (err != nil), "executing query suceeded while cluster is not ready")

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if len(brokers[0].GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range brokers[0].GetCluster(meta.ClusterTypeTstore).NodeMap {
			if nd.NumShards != 4 {
				return false, []interface{}{brokers[0].GetCluster(meta.ClusterTypeTstore), nd}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "20s")

	// create the database
	err = brokers[0].CreateDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error creating database")

	// write some points
	for idx := 0; idx < numNodes; idx++ {
		// create some points
		data := fmt.Sprintf("cpu%d,host=serverB,svc=nginx value1=11,value2=12 10\n", idx) +
			fmt.Sprintf("cpu%d,host=serverC,svc=nginx value1=21,value2=22  20\n", idx)
		points, err := models.ParsePointsWithPrecision([]byte(data), time.Time{}, "s")
		AssertOk(t, err, "Error parsing points")

		// write the points
		err = brokers[idx].WritePoints(context.Background(), "db0", points)
		AssertOk(t, err, "Error writing points")
	}

	// read the points back
	for idx := 0; idx < numNodes; idx++ {
		results, err := brokers[idx].ExecuteQuery(context.Background(), "db0", fmt.Sprintf("SELECT * FROM cpu%d", idx))
		AssertOk(t, err, "Error executing query")
		restr, _ := json.Marshal(results)
		log.Infof("Got result: %v", string(restr))
	}

	// stop all brokers and data nodes
	for idx := 0; idx < numNodes; idx++ {
		err = dnodes[idx].Stop()
		AssertOk(t, err, "Error stopping data node")
		Assert(t, brokers[idx].IsStopped() == false, "Incorrect broker state")
		err = brokers[idx].Stop()
		AssertOk(t, err, "Error stopping broker")
		Assert(t, brokers[idx].IsStopped() == true, "Incorrect broker state")
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

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("tstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)
		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:720%d", idx), path)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the brokers
	for idx := 0; idx < numNodes; idx++ {
		brokers[idx], err = createBroker(fmt.Sprintf("node-%d", idx))
		AssertOk(t, err, "Error creating broker")
	}

	// try making some calls while cluster is not ready, they should fail
	err = brokers[0].WriteKvs(context.Background(), "table0", []*tproto.KeyValue{})
	Assert(t, (err != nil), "writing keys suceeded while cluster is not ready")
	_, err = brokers[0].ReadKvs(context.Background(), "table0", []*tproto.Key{})
	Assert(t, (err != nil), "reading keys suceeded while cluster is not ready")
	_, err = brokers[0].ListKvs(context.Background(), "table0")
	Assert(t, (err != nil), "listing keys suceeded while cluster is not ready")
	err = brokers[0].DeleteKvs(context.Background(), "table0", []*tproto.Key{})
	Assert(t, (err != nil), "deleting keys suceeded while cluster is not ready")

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if len(brokers[0].GetCluster(meta.ClusterTypeKstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range brokers[0].GetCluster(meta.ClusterTypeKstore).NodeMap {
			if nd.NumShards != 4 {
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

func TestBrokerBenchmark(t *testing.T) {
	const numNodes = 4
	var batchSize = 1000
	const numIterations = 100
	dnodes := make([]*data.DNode, numNodes)
	var err error

	// create nodes
	for idx := 0; idx < numNodes; idx++ {
		// create a temp dir
		path, err := ioutil.TempDir("", fmt.Sprintf("tstore-%d-", idx))
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)
		dnodes[idx], err = createDnode(fmt.Sprintf("node-%d", idx), fmt.Sprintf("localhost:701%d", idx), path)
		AssertOk(t, err, "Error creating nodes")
	}

	// create the broker
	broker, err := createBroker(fmt.Sprintf("node-0"))
	AssertOk(t, err, "Error creating broker")

	// wait till all the shards are created
	AssertEventually(t, func() (bool, interface{}) {
		if len(broker.GetCluster(meta.ClusterTypeTstore).NodeMap) != numNodes {
			return false, nil
		}
		for _, nd := range broker.GetCluster(meta.ClusterTypeTstore).NodeMap {
			if nd.NumShards != 4 {
				return false, []interface{}{broker.GetCluster(meta.ClusterTypeTstore), nd}
			}
		}
		return true, nil
	}, "nodes have invalid number of shards", "1s", "20s")

	// create the database
	err = broker.CreateDatabase(context.Background(), "db0")
	AssertOk(t, err, "Error creating database")

	// measure how long it takes to parse the points
	points := make([]models.Points, numIterations)
	startTime := time.Now()
	for iter := 0; iter < numIterations; iter++ {
		// parse some points
		var data string
		for i := 0; i < batchSize; i++ {
			data += fmt.Sprintf("cpu%d,host=server%d,svc=nginx%d value1=%d,value2=%d %d\n", i, i, i, (i + batchSize*iter), (i + batchSize*iter), (i + 20 + batchSize*iter))
		}
		points[iter], err = models.ParsePointsWithPrecision([]byte(data), time.Now(), "s")
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
