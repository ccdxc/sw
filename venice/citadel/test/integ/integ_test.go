// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package integ_test

import (
	"context"
	"flag"
	"fmt"
	"os"
	"sync"
	"testing"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"
)

// integTestSuite is the state of integ test
type integTestSuite struct {
	cfg      *meta.ClusterConfig
	dnodes   []*data.DNode
	brokers  []*broker.Broker
	numNodes int
	waitGrp  sync.WaitGroup
	logger   log.Logger
}

const (
	DefaultNumNodes = 5
	nodeURLFormat   = "localhost:74%02d"
	restURLFormat   = "localhost:75%02d"
	dbpathFormat    = "/tmp/citadel/%d"
)

var logger = log.GetNewLogger(log.GetDefaultConfig("citadel-integ-test"))
var numNodes = flag.Int("nodes", DefaultNumNodes, "Number of agents")

// Hook up gocheck into the "go test" runner.
func TestCitadelInteg(t *testing.T) {
	// integ test suite
	var sts = &integTestSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

// SetUpSuite sets up data nodes and brokers
func (it *integTestSuite) SetUpSuite(c *C) {
	// set num nodes
	it.numNodes = *numNodes
	it.logger = logger

	// cluster config
	cfg := meta.DefaultClusterConfig()
	// cfg.EnableKstoreMeta = false
	// cfg.EnableKstore = false
	// cfg.EnableTstoreMeta = false
	// cfg.EnableTstore = false
	cfg.NumShards = uint32(it.numNodes)
	cfg.DesiredReplicas = 2
	cfg.DeadInterval = time.Millisecond * 500
	cfg.NodeTTL = 5
	cfg.RebalanceDelay = time.Millisecond * 100
	cfg.RebalanceInterval = time.Millisecond * 10
	it.cfg = cfg

	// create all the nodes and brokers
	for i := 0; i < it.numNodes; i++ {
		// create data node
		dn, err := data.NewDataNode(cfg, fmt.Sprintf("dnode-%d", i), fmt.Sprintf(nodeURLFormat, i), fmt.Sprintf(dbpathFormat, i), it.logger)
		AssertOk(c, err, "Error creating datanode")
		it.dnodes = append(it.dnodes, dn)

		// create broker
		br, err := broker.NewBroker(cfg, fmt.Sprintf("broker-%d", i), it.logger)
		AssertOk(c, err, "Error creating broker")
		it.brokers = append(it.brokers, br)
	}

	// verify all nodes have converged
	it.verifyConverged(c, "SetUpSuite")
	time.Sleep(time.Second)

	// create a default database
	err := it.brokers[0].CreateDatabase(context.Background(), "default")
	AssertOk(c, err, "Error creating the database")
}

func (it *integTestSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
	it.logger = logger.WithContext("t_name", c.TestName())
}

func (it *integTestSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
}

// TearDownSuite stops all nodes
func (it *integTestSuite) TearDownSuite(c *C) {
	// stop all data nodes and brokers
	it.stopAllNodes(c, "TearDownSuite")
	it.dnodes = nil
	it.brokers = nil

	time.Sleep(time.Millisecond * 100)

	// cleanup directories
	for i := 0; i < it.numNodes; i++ {
		os.RemoveAll(fmt.Sprintf(dbpathFormat, i))
	}

	meta.DestroyClusterState(it.cfg, meta.ClusterTypeTstore)
	meta.DestroyClusterState(it.cfg, meta.ClusterTypeKstore)
}
