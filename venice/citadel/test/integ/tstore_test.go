// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package integ_test

import (
	. "gopkg.in/check.v1"
)

func (it *integTestSuite) TestTstoreReadWrite(c *C) {

	readWriteCase := TestCase{
		TestName: "ReadWriteTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "ReadWriteTest",
						},
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "ReadWriteTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &readWriteCase)
}

func (it *integTestSuite) TestTstoreNodeRestartQuick(c *C) {

	nodeRestartQuickCase := TestCase{
		TestName: "NodeRestartQuickTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "NodeRestartQuickTest",
						},
					},
				},
			},
			{
				StepName: "NodeRestatQuick",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionRestartQuickAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "1s",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "NodeRestartQuickTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeRestartQuickCase)
}

func (it *integTestSuite) TestTstoreNodeStopStart(c *C) {

	nodeStopStartCase := TestCase{
		TestName: "NodeStopStartTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "NodeStopStartTest",
						},
					},
				},
			},
			{
				StepName: "StopStartAllNodes",
				Repeat:   2,
				Actions: []TestAction{
					{
						Action: ActionStopAllNodes,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "1s",
						},
					},
					{
						Action: ActionStartAllNodesAsync,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "1s",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "NodeStopStartTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeStopStartCase)
}

func (it *integTestSuite) TestTstoreNodeRestartQuickWriteLoss(c *C) {

	nodeRestartQuickWriteLossCase := TestCase{
		TestName: "NodeRestartQuickWriteLossTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPatternAsync,
						Params: ActionParams{
							"rate":  "2",
							"count": "20",
							"table": "NodeRestartQuickWriteLossTest",
						},
					},
				},
			},
			{
				StepName: "NodeRestatQuick",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionRestartQuickAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "1s",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionWaitForTasks,
						Params: ActionParams{
							"timeout": "40s", // wait for async write point to complete
						},
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "20",
							"table": "NodeRestartQuickWriteLossTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeRestartQuickWriteLossCase)
}

func (it *integTestSuite) TestTstoreNodeRestartSlow(c *C) {

	nodeRestartSlowCase := TestCase{
		TestName: "NodeRestartSlowTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "NodeRestartSlowTest",
						},
					},
				},
			},
			{
				StepName: "NodeRestatSlow",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionRestartSlowAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "NodeRestartSlowTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeRestartSlowCase)

}

func (it *integTestSuite) TestTstoreStoppedNodeRecovery(c *C) {

	stoppedNodeRecoveryCase := TestCase{
		TestName: "StoppedNodeRecovery",
		Steps: []TestStep{
			{
				StepName: "NodeStop1",
				Actions: []TestAction{
					{
						Action: ActionStopAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
				},
			},
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "StoppedNodeRecovery",
						},
					},
				},
			},
			{
				StepName: "StartAllNodes1",
				Actions: []TestAction{
					{
						Action: ActionStartAllNodes,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "NodeStop2",
				Actions: []TestAction{
					{
						Action: ActionStopAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
				},
			},
			{
				StepName: "StartAllNodes2",
				Actions: []TestAction{
					{
						Action: ActionStartAllNodes,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "StoppedNodeRecovery",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &stoppedNodeRecoveryCase)

}

func (it *integTestSuite) TestTstoreNodeSoftRestart(c *C) {

	nodeSoftRestartCase := TestCase{
		TestName: "NodeSoftRestartTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPattern,
						Params: ActionParams{
							"rate":  "100",
							"count": "10",
							"table": "NodeSoftRestartTest",
						},
					},
				},
			},
			{
				StepName: "NodeSoftRestart",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionSoftRestartAnyNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "10",
							"table": "NodeSoftRestartTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeSoftRestartCase)
}

func (it *integTestSuite) TestTstoreNodeRestartLeaderQuick(c *C) {

	nodeRestartLeaderQuickCase := TestCase{
		TestName: "NodeRestartLeaderQuickTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPatternAsync,
						Params: ActionParams{
							"rate":  "2",
							"count": "20",
							"table": "NodeRestartLeaderQuickTest",
						},
					},
				},
			},
			{
				StepName: "NodeRestatQuickLeader",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionRestartQuickLeaderNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionWaitForTasks,
						Params: ActionParams{
							"timeout": "40s", // wait for async write point to complete
						},
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "20",
							"table": "NodeRestartLeaderQuickTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeRestartLeaderQuickCase)
}

func (it *integTestSuite) TestTstoreNodeSoftRestartLeader(c *C) {
	c.Skip("skipping due to known issue replicating live writes during soft-restart")

	nodeSoftRestartLeaderCase := TestCase{
		TestName: "NodeSoftRestartLeaderTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPatternAsync,
						Params: ActionParams{
							"rate":  "2",
							"count": "20",
							"table": "NodeSoftRestartLeaderTest",
						},
					},
				},
			},
			{
				StepName: "NodeSoftRestatLeader",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionSoftRestartLeaderNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "100ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionWaitForTasks,
						Params: ActionParams{
							"timeout": "40s", // wait for async write point to complete
						},
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "20",
							"table": "NodeSoftRestartLeaderTest",
						},
					},
				},
			},
		},
	}

	it.runTestCase(c, &nodeSoftRestartLeaderCase)
}

func (it *integTestSuite) TestTstoreNodeRestartLeaderSlow(c *C) {
	c.Skip("skipping due to known issues")

	nodeRestartLeaderSlowCase := TestCase{
		TestName: "NodeRestartLeaderSlowTest",
		Steps: []TestStep{
			{
				StepName: "WritePoints",
				Actions: []TestAction{
					{
						Action: ActionWritePointPatternAsync,
						Params: ActionParams{
							"rate":  "2",
							"count": "20",
							"table": "NodeRestartLeaderSlowTest",
						},
					},
				},
			},
			{
				StepName: "NodeRestatSlowLeader",
				Repeat:   it.numNodes,
				Actions: []TestAction{
					{
						Action: ActionRestartSlowLeaderNode,
					},
					{
						Action: ActionSleep,
						Params: ActionParams{
							"time": "300ms",
						},
					},
					{
						Action: ActionVerifyConverged,
					},
				},
			},
			{
				StepName: "VerifyPoints",
				Actions: []TestAction{
					{
						Action: ActionVerifyConverged,
					},
					{
						Action: ActionWaitForTasks,
						Params: ActionParams{
							"timeout": "40s", // wait for async write point to complete
						},
					},
					{
						Action: ActionVerifyPointPattern,
						Params: ActionParams{
							"count": "20",
							"table": "NodeRestartLeaderSlowTest",
						},
					},
				},
			},
		},
	}

	// run the test cases
	it.runTestCase(c, &nodeRestartLeaderSlowCase)

}
