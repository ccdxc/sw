// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package integ_test

import (
	"context"
	"encoding/json"
	"fmt"
	"math/rand"
	"strconv"
	"sync"
	"time"

	. "gopkg.in/check.v1"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// ActionType action types
type ActionType string

// ActionParams are action specific parameters
type ActionParams map[string]string

// TestAction is an action to be taken in a test step
type TestAction struct {
	Action ActionType
	Params ActionParams
}

// TestStep is a step in test case
type TestStep struct {
	StepName string
	Repeat   int
	Actions  []TestAction
}

// TestCase is a test case
type TestCase struct {
	TestName string
	Steps    []TestStep
}

// action types
const (
	ActionVerifyConverged        ActionType = "VerifyConverged"
	ActionWritePointPattern      ActionType = "WritePointPattern"
	ActionWritePointPatternAsync ActionType = "WritePointPatternAsync"
	ActionVerifyPointPattern     ActionType = "VerifyPointPattern"
	ActionWriteKvPattern         ActionType = "WriteKvPattern"
	ActionWriteKvPatternAsync    ActionType = "WriteKvPatternAsync"
	ActionVerifyKvPattern        ActionType = "VerifyKvPattern"
	ActionRestartSlowAllNodes    ActionType = "RestartSlowAllNodes"
	ActionRestartSlowAnyNode     ActionType = "RestartSlowAnyNode"
	ActionRestartSlowLeaderNode  ActionType = "RestartSlowLeaderNode"
	ActionRestartQuickAllNodes   ActionType = "RestartQuickAllNodes"
	ActionRestartQuickAnyNode    ActionType = "RestartQuickAnyNode"
	ActionRestartQuickLeaderNode ActionType = "RestartQuickLeaderNode"
	ActionSoftRestartAnyNode     ActionType = "SoftRestartAnyNode"
	ActionSoftRestartLeaderNode  ActionType = "SoftRestartLeaderNode"
	ActionWaitForTasks           ActionType = "WaitForTasks"
	ActionStopAnyNode            ActionType = "StopAnyNode"
	ActionStartAllNodes          ActionType = "StartAllNodes"
	ActionStartAllNodesAsync     ActionType = "StartAllNodesAsync"
	ActionStopAllNodes           ActionType = "StopAllNodes"
	ActionSleep                  ActionType = "Sleep"
)

// execTestAction executes one action
func (it *integTestSuite) execTestAction(c *C, action *TestAction, locator string) {
	switch action.Action {
	case ActionVerifyConverged:
		it.verifyConverged(c, locator)
	case ActionRestartSlowAllNodes:
		it.restartSlowAllNodes(c)
	case ActionRestartSlowAnyNode:
		it.restartSlowAnyNode(c)
	case ActionRestartSlowLeaderNode:
		it.restartSlowLeaderNode(c)
	case ActionRestartQuickAllNodes:
		it.restartQuickAllNodes(c)
	case ActionRestartQuickAnyNode:
		it.restartQuickAnyNode(c)
	case ActionRestartQuickLeaderNode:
		it.restartQuickLeaderNode(c)
	case ActionSoftRestartAnyNode:
		it.softRestartAnyNode(c)
	case ActionSoftRestartLeaderNode:
		it.softRestartLeaderNode(c)
	case ActionWritePointPattern:
		it.writePointsPattern(c, locator, action.Params)
	case ActionVerifyPointPattern:
		it.verifyPointsPattern(c, locator, action.Params)
	case ActionWritePointPatternAsync:
		it.writePointsPatternAsync(c, locator, action.Params)
	case ActionWriteKvPattern:
		it.writeKvPattern(c, locator, action.Params)
	case ActionWriteKvPatternAsync:
		it.writeKvPatternAsync(c, locator, action.Params)
	case ActionVerifyKvPattern:
		it.verifyKvPattern(c, locator, action.Params)
	case ActionWaitForTasks:
		it.waitForTasks(c, locator, action.Params)
	case ActionStopAnyNode:
		it.stopAnyNode(c)
	case ActionStartAllNodes:
		it.startAllNodes(c)
	case ActionStartAllNodesAsync:
		it.startAllNodesAsync(c)
	case ActionStopAllNodes:
		it.stopAllNodes(c, locator)
	case ActionSleep:
		it.sleepAction(c, locator, action.Params)
	default:
		c.Fatalf("Unknown test step action %s", action.Action)
	}
}

// runTestCase runs a test case with N steps
func (it *integTestSuite) runTestCase(c *C, testcase *TestCase) {
	log.Infof("################## Starting test case %s ###################", testcase.TestName)

	// verify everything is converged
	verifyConverged := TestAction{Action: ActionVerifyConverged}
	it.execTestAction(c, &verifyConverged, fmt.Sprintf("%s: start", testcase.TestName))

	log.Infof("###### %s: Nodes are in steady state. starting test steps", testcase.TestName)

	// walk thru each step and execute it
	for idx, step := range testcase.Steps {
		log.Infof("###### Executing step %s/%d %s", testcase.TestName, idx, step.StepName)
		count := step.Repeat
		if count == 0 {
			count = 1
		}

		// repeate the actions if required
		for iter := 0; iter < count; iter++ {
			// execute each action
			for aid, action := range step.Actions {
				locator := fmt.Sprintf("%s: step %d/%s, iter %d, action %d/%s", testcase.TestName, idx, step.StepName, iter, aid, action.Action)

				log.Infof("###### Executing Action %s", locator)

				it.execTestAction(c, &action, locator)
			}
		}
	}
}

// ------------------------- action functions --------------------------

// verifyConverged verifies cluster has converged
func (it *integTestSuite) verifyConverged(c *C, locator string) {
	// verify atleast one node is a leader
	AssertEventually(c, func() (bool, interface{}) {
		for _, node := range it.dnodes {
			if node.IsLeader() {
				return true, nil
			}
		}
		return false, nil
	}, fmt.Sprintf("%s: No leader node", locator), "300ms", "30s")

	// verify there is no sync pending
	AssertEventually(c, func() (bool, interface{}) {
		for _, node := range it.dnodes {
			if !node.IsStopped() && node.HasPendingSync() {
				return false, node
			}
		}
		return true, nil
	}, fmt.Sprintf("%s: Nodes have pending sync", locator), "300ms", "30s")

	// verify nodemap has all nodes and each node has expected number of shards
	AssertEventually(c, func() (bool, interface{}) {
		// get the metadata for each store
		tcl := it.brokers[0].GetCluster(meta.ClusterTypeTstore)
		kcl := it.brokers[0].GetCluster(meta.ClusterTypeKstore)

		// check if the nodemap has all the nodes
		if len(tcl.NodeMap) != it.numNodes || len(kcl.NodeMap) != it.numNodes {
			return false, []interface{}{tcl.NodeMap, kcl.NodeMap}
		}

		// check if each nodemap has expected number of shards
		for _, node := range tcl.NodeMap {
			if node.NodeStatus != meta.NodeStatusAlive {
				return false, []interface{}{tcl, kcl}
			}
			if node.NumShards != ((it.cfg.NumShards / uint32(it.numNodes)) * it.cfg.DesiredReplicas) {
				return false, []interface{}{tcl, kcl}
			}
		}
		for _, node := range kcl.NodeMap {
			if node.NodeStatus != meta.NodeStatusAlive {
				return false, []interface{}{tcl, kcl}
			}
			if node.NumShards != ((it.cfg.NumShards / uint32(it.numNodes)) * it.cfg.DesiredReplicas) {
				return false, []interface{}{tcl, kcl}
			}
		}

		return true, nil
	}, fmt.Sprintf("%s: Incorrect Nodemap", locator), "300ms", "30s")

	log.Infof("###### All nodes are in steady state")
}

// stopNode stops a node
func (it *integTestSuite) stopNode(c *C, idx int) {
	// stop dnode
	err := it.dnodes[idx].Stop()
	AssertOk(c, err, "Error stopping datanode")

	// stop broker
	err = it.brokers[idx].Stop()
	AssertOk(c, err, "Error stopping broker")
}

// startNode starts a node
func (it *integTestSuite) startNode(c *C, idx int) {
	var err error
	// create data node
	it.dnodes[idx], err = data.NewDataNode(it.cfg, fmt.Sprintf("dnode-%d", idx), fmt.Sprintf(nodeURLFormat, idx), fmt.Sprintf(dbpathFormat, idx), it.logger)
	AssertOk(c, err, "Error creating datanode")

	// create broker
	it.brokers[idx], err = broker.NewBroker(it.cfg, fmt.Sprintf("broker-%d", idx), it.logger)
	AssertOk(c, err, "Error creating broker")
}

// softRestartNode soft restart a node without expiring the lease
func (it *integTestSuite) softRestartNode(c *C, idx int) {
	err := it.dnodes[idx].SoftRestart()
	AssertOk(c, err, "Error soft restarting datanode")

	// stop broker
	err = it.brokers[idx].Stop()
	AssertOk(c, err, "Error stopping broker")

	// recreate broker
	it.brokers[idx], err = broker.NewBroker(it.cfg, fmt.Sprintf("broker-%d", idx), it.logger)
	AssertOk(c, err, "Error creating broker")
}

// restartNode restarts a specified node
func (it *integTestSuite) restartNode(c *C, idx int, delayStr string) {
	log.Infof("###### Restarting node %s", fmt.Sprintf("dnode-%d", idx))

	// stop the node
	it.stopNode(c, idx)

	// wait before starting back up, if required
	if delayStr != "" {
		delay, derr := time.ParseDuration(delayStr)
		AssertOk(c, derr, "Error parsing delay string")

		time.Sleep(delay)
	}

	// start the node
	it.startNode(c, idx)
}

// stopAnyNode stops any one node
func (it *integTestSuite) stopAnyNode(c *C) {
	ridx := rand.Intn(it.numNodes)
	it.stopNode(c, ridx)
}

// softRestartAnyNode soft restarts a node
func (it *integTestSuite) softRestartAnyNode(c *C) {
	ridx := rand.Intn(it.numNodes)
	it.softRestartNode(c, ridx)
}

// softRestartLeaderNode soft restart leader node restarts leader node without expiring the lease
func (it *integTestSuite) softRestartLeaderNode(c *C) {
	for idx, node := range it.dnodes {
		if node.IsLeader() {
			it.softRestartNode(c, idx)
			return
		}
	}
}

// restartQuickAllNodes restarts all nodes in the cluster without waiting for dead interval
func (it *integTestSuite) restartQuickAllNodes(c *C) {
	for idx := range it.dnodes {
		it.restartNode(c, idx, "")
	}
}

// restartQuickAnyNode restarts a random node without dead interval delay
func (it *integTestSuite) restartQuickAnyNode(c *C) {
	ridx := rand.Intn(it.numNodes)
	it.restartNode(c, ridx, "")
}

// restartQuickLeaderNode restarts leader quickly without delay
func (it *integTestSuite) restartQuickLeaderNode(c *C) {
	for idx, node := range it.dnodes {
		if node.IsLeader() {
			it.restartNode(c, idx, "")
			return
		}
	}
}

// restartSlowAllNodes restarts all nodes with a dead interval delay
func (it *integTestSuite) restartSlowAllNodes(c *C) {
	for idx := range it.dnodes {
		delay := it.cfg.DeadInterval * 3
		it.restartNode(c, idx, delay.String())
	}
}

// restartAnyNode restarts a random node with dead interval delay
func (it *integTestSuite) restartSlowAnyNode(c *C) {
	ridx := rand.Intn(it.numNodes)
	delay := it.cfg.DeadInterval * 3
	it.restartNode(c, ridx, delay.String())
}

// restartSlowLeaderNode restarts leader node with dead interval delay
func (it *integTestSuite) restartSlowLeaderNode(c *C) {
	for idx, node := range it.dnodes {
		if node.IsLeader() {
			delay := it.cfg.DeadInterval * 3
			it.restartNode(c, idx, delay.String())
			return
		}
	}
}

// stopAllNodes stops all nodes
func (it *integTestSuite) stopAllNodes(c *C, locator string) {
	var waitGrp sync.WaitGroup
	for idx := range it.dnodes {
		waitGrp.Add(1)
		go func(i int) {
			defer waitGrp.Done()
			it.stopNode(c, i)
		}(idx)
	}
	waitGrp.Wait()
}

// startAllNodes starts all stopped nodes
func (it *integTestSuite) startAllNodes(c *C) {
	for idx, dn := range it.dnodes {
		if dn.IsStopped() {
			it.startNode(c, idx)
		}
	}
}

// startAllNodesAsync starts all nodes in parallel
func (it *integTestSuite) startAllNodesAsync(c *C) {
	for idx, dn := range it.dnodes {
		if dn.IsStopped() {
			go it.startNode(c, idx)
		}
	}
}

// waitForTasks waits for all background tasks to complete
func (it *integTestSuite) waitForTasks(c *C, locator string, params ActionParams) {
	// parse timeout params
	timeoutStr, ok := params["timeout"]
	Assert(c, ok, "Error parsing params")
	timeout, err := time.ParseDuration(timeoutStr)
	AssertOk(c, err, "Error parsing timeout value")

	ch := make(chan struct{})
	go func() {
		defer close(ch)
		it.waitGrp.Wait()
	}()

	log.Infof("Waiting %s for all tasks to complete", timeoutStr)

	select {
	case <-ch:
		log.Infof("All waitgroups finished")
		return
	case <-time.After(timeout):
		c.Fatalf("%s: Timeout(%s) while waiting for tasks to complete", locator, timeoutStr)
		return
	}
}

// writePoints writes a point using one of the brokers
func (it *integTestSuite) writePoints(points []models.Point) error {
	var err error
	for i, broker := range it.brokers {
		if !broker.IsStopped() {
			err = broker.WritePoints(context.Background(), "default", points)
			if err == nil {
				return nil
			}
			log.Warnf("write failed on broker-%d, %s", i, err)
			// retry next broker after a small delay
			time.Sleep(time.Millisecond * 100)
		}
	}

	return err
}

// writePointsPattern writes points at specified rate and pattern
func (it *integTestSuite) writePointsPattern(c *C, locator string, params ActionParams) {
	// setup waitGrp
	it.waitGrp.Add(1)
	defer it.waitGrp.Done()

	// parse params
	rate, err := strconv.Atoi(params["rate"])
	AssertOk(c, err, "Error parsing params")
	count, err := strconv.Atoi(params["count"])
	AssertOk(c, err, "Error parsing params")
	table, ok := params["table"]
	Assert(c, ok, "Error parsing params")

	delay := time.Second / time.Duration(rate)

	for iter := 0; iter < count; iter++ {
		select {
		case <-time.After(delay):
			data := ""
			for i := 0; i < it.numNodes; i++ {
				data += fmt.Sprintf("%s%d,key1=key1%d,key2=key2%d value1=%d,value2=%d %d\n", table, i, iter, iter, iter, iter, time.Now().UnixNano())
			}
			points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
			AssertOk(c, err, "Error parsing points")
			err = it.writePoints(points)
			AssertOk(c, err, fmt.Sprintf("%s: Error writing points", locator))
		}
	}
}

// writePointsPatternAsync writes points in the background
func (it *integTestSuite) writePointsPatternAsync(c *C, locator string, params ActionParams) {
	go it.writePointsPattern(c, locator, params)
}

// executeQuery executes a query on one of the brokers
func (it *integTestSuite) executeQuery(q string) ([]*query.Result, error) {
	var err error
	var results []*query.Result
	for _, broker := range it.brokers {
		if !broker.IsStopped() {
			results, err = broker.ExecuteQuery(context.Background(), "default", q)
			if err == nil {
				return results, nil
			}
			// retry next broker after a small delay
			time.Sleep(time.Millisecond)
		}
	}

	return results, err
}

// verifyPointsPattern verifies all points in a pattern can be read back
func (it *integTestSuite) verifyPointsPattern(c *C, locator string, params ActionParams) {
	count, err := strconv.Atoi(params["count"])
	AssertOk(c, err, "Error parsing params")
	table, ok := params["table"]
	Assert(c, ok, "Error parsing params")

	for i := 0; i < it.numNodes; i++ {
		results, err := it.executeQuery(fmt.Sprintf("SELECT * FROM %s%d", table, i))
		AssertOk(c, err, fmt.Sprintf("%s: Error executing the query", locator))
		jstr, _ := json.Marshal(results)
		log.Infof("Got query results: %s", jstr)
		Assert(c, len(results) == 1, fmt.Sprintf("%s: Invalid query result", locator), results)
		Assert(c, len(results[0].Series) == 1, fmt.Sprintf("%s: Invalid query result", locator), results)
		Assert(c, (results[0].Series[0].Name == fmt.Sprintf("%s%d", table, i)), fmt.Sprintf("%s: Invalid measurement name", locator), results)
		Assert(c, len(results[0].Series[0].Columns) == 5, fmt.Sprintf("%s: Invalid number of columns", locator), results)
		Assert(c, (results[0].Series[0].Columns[0] == "time" && results[0].Series[0].Columns[1] == "key1" && results[0].Series[0].Columns[2] == "key2" &&
			results[0].Series[0].Columns[3] == "value1" && results[0].Series[0].Columns[4] == "value2"), fmt.Sprintf("%s: Invalid column names", locator), results)
		Assert(c, len(results[0].Series[0].Values) == count, fmt.Sprintf("%s: Invalid number of rows, expected:%d, got:%d", locator, count, len(results[0].Series[0].Values)), results)

		// verify each key and value
		for ridx := 0; ridx < count; ridx++ {
			row := results[0].Series[0].Values[ridx]
			col1, ok := row[1].(string)
			Assert(c, (ok && col1 == fmt.Sprintf("key1%d", ridx)), fmt.Sprintf("%s: Invalid key1 value in row", locator), row)
			col2, ok := row[2].(string)
			Assert(c, (ok && col2 == fmt.Sprintf("key2%d", ridx)), fmt.Sprintf("%s: Invalid key2 value in row", locator), row)
			col3, ok := row[3].(float64)
			Assert(c, (ok && col3 == float64(ridx)), fmt.Sprintf("%s: Invalid value1 in row", locator), row)
			col4, ok := row[4].(float64)
			Assert(c, (ok && col4 == float64(ridx)), fmt.Sprintf("%s: Invalid value2 in row", locator), row)
		}
	}
}

// writeKvs writes list of key-value pairs using one of the brokers
func (it *integTestSuite) writeKvs(table string, kvs []*tproto.KeyValue) error {
	var err error
	for _, broker := range it.brokers {
		if !broker.IsStopped() {
			err = broker.WriteKvs(context.Background(), table, kvs)
			if err == nil {
				return nil
			}
			// retry next broker after a small delay
			time.Sleep(time.Millisecond * 100)
		}
	}

	return err
}

// writeKvPattern writes key-value pairs at specified rate and pattern
func (it *integTestSuite) writeKvPattern(c *C, locator string, params ActionParams) {
	// setup waitGrp
	it.waitGrp.Add(1)
	defer it.waitGrp.Done()

	// parse params
	rate, err := strconv.Atoi(params["rate"])
	AssertOk(c, err, "Error parsing params")
	count, err := strconv.Atoi(params["count"])
	AssertOk(c, err, "Error parsing params")
	table, ok := params["table"]
	Assert(c, ok, "Error parsing params")

	delay := time.Second / time.Duration(rate)

	for iter := 0; iter < count; iter++ {
		select {
		case <-time.After(delay):
			for i := 0; i < it.numNodes; i++ {
				kvs := []*tproto.KeyValue{
					{
						Key:   []byte(fmt.Sprintf("Key%d-%d", i, iter)),
						Value: []byte(fmt.Sprintf("Value%d-%d", i, iter)),
					},
				}
				err = it.writeKvs(fmt.Sprintf("%s%d", table, i), kvs)
				AssertOk(c, err, fmt.Sprintf("%s: Error writing key-value pairs", locator))
			}
		}
	}
}

// writeKvPatternAsync writes kv pattern in background
func (it *integTestSuite) writeKvPatternAsync(c *C, locator string, params ActionParams) {
	go it.writeKvPattern(c, locator, params)
}

// listKvs lists all key-value pairs in a table using one of the brokers
func (it *integTestSuite) listKvs(table string) ([]*tproto.KeyValue, error) {
	var err error
	var results []*tproto.KeyValue
	for _, broker := range it.brokers {
		if !broker.IsStopped() {
			results, err = broker.ListKvs(context.Background(), table)
			if err == nil {
				return results, nil
			}
			// retry next broker after a small delay
			time.Sleep(time.Millisecond)
		}
	}

	return results, err
}

// verifyKvPattern verifies all key-value pairs in a pattern can be read back
func (it *integTestSuite) verifyKvPattern(c *C, locator string, params ActionParams) {
	count, err := strconv.Atoi(params["count"])
	AssertOk(c, err, "Error parsing params")
	table, ok := params["table"]
	Assert(c, ok, "Error parsing params")

	for i := 0; i < it.numNodes; i++ {
		results, err := it.listKvs(fmt.Sprintf("%s%d", table, i))
		AssertOk(c, err, fmt.Sprintf("%s: Error listing key-values", locator))
		jstr, _ := json.Marshal(results)
		log.Infof("Got list results: %s", jstr)
		Assert(c, (len(results) == count), fmt.Sprintf("Got %d number of kv pairs, expected %d, {%+v}",
			len(results), count, results))

		// verify each key and value
		/*
			for ridx := 0; ridx < count; ridx++ {
				for i := 0; i < it.numNodes; i++ {
				}
			}
		*/
	}
}

// sleepAction sleeps for specified amount of time
func (it *integTestSuite) sleepAction(c *C, locator string, params ActionParams) {
	// parse time params
	timeStr, ok := params["time"]
	Assert(c, ok, "Error parsing params")
	sleepTime, err := time.ParseDuration(timeStr)
	AssertOk(c, err, "Error parsing timeout value")

	time.Sleep(sleepTime)
}
