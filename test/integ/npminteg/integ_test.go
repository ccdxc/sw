// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"flag"
	"fmt"
	"testing"

	log "github.com/Sirupsen/logrus"
	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm"

	. "github.com/pensando/sw/utils/testutils"
	. "gopkg.in/check.v1"
)

// integ test suite parameters
const (
	numIntegTestAgents = 3
	integTestRPCURL    = "localhost:9595"
)

// integTestSuite is the state of integ test
type integTestSuite struct {
	ctrler    *npm.Netctrler
	agents    []*Dpagent
	numAgents int
}

// test args
var numAgents = flag.Int("agents", numIntegTestAgents, "Number of agents")

// Hook up gocheck into the "go test" runner.
func TestNpmInteg(t *testing.T) {
	// integ test suite
	var sts = &integTestSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

func (it *integTestSuite) SetUpSuite(c *C) {
	// test parameters
	it.numAgents = *numAgents

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestRPCURL, "", "")
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numAgents)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		agent, err := CreateAgent(fmt.Sprintf("dummy-uuid-%d", i), integTestRPCURL)
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

	log.Infof("Total number of agents: %v", len(it.agents))
}

func (it *integTestSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *integTestSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
}

func (it *integTestSuite) TearDownSuite(c *C) {
	// stop the agents
	for _, ag := range it.agents {
		ag.nagent.Stop()
	}
	it.agents = []*Dpagent{}

	// stop server and client
	it.ctrler.RPCServer.Stop()
	it.ctrler = nil

	log.Infof("Stopped all servers and clients")
}

// basic connectivity tests between NPM and agent
func (it *integTestSuite) TestNpmAgentBasic(c *C) {
	// expect a network create/delete call in data path
	for _, ag := range it.agents {
		ag.datapath.Netclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Netclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	// create a network in controller
	err := it.ctrler.Watchr.CreateNetwork("default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() bool {
			_, nerr := ag.nagent.Netagent.FindNetwork(api.ObjectMeta{Tenant: "default", Name: "testNetwork"})
			return (nerr == nil)
		}, "Network not found on agent", "10ms", fmt.Sprintf("%dms", 3000+100*it.numAgents))
		nt, nerr := ag.nagent.Netagent.FindNetwork(api.ObjectMeta{Tenant: "default", Name: "testNetwork"})
		AssertOk(c, nerr, "error finding network")
		Assert(c, (nt.Spec.IPv4Subnet == "10.1.1.0/24"), "Network params didnt match", nt)
	}

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() bool {
			_, nerr := ag.nagent.Netagent.FindNetwork(api.ObjectMeta{Tenant: "default", Name: "testNetwork"})
			return (nerr != nil)
		}, "Network still found on agent", "100ms", fmt.Sprintf("%dms", 3000+100*it.numAgents))
	}
}

// test endpoint create workflow e2e
func (it *integTestSuite) TestNpmEndpointCreateDelete(c *C) {
	// expect network and endpoint calls
	for _, ag := range it.agents {
		ag.datapath.Netclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Epclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).MaxTimes(it.numAgents+1).Return(nil, nil)
		ag.datapath.Epclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).MaxTimes(it.numAgents+1).Return(nil, nil)
		ag.datapath.Netclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	// create a network in controller
	err := it.ctrler.Watchr.CreateNetwork("default", "testNetwork", "10.1.0.0/16", "10.1.1.254")
	c.Assert(err, IsNil)
	AssertEventually(c, func() bool {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil)
	}, "Network not found in statemgr")

	// wait till agent has the network
	for _, ag := range it.agents {
		AssertEventually(c, func() bool {
			ometa := api.ObjectMeta{Tenant: "default", Name: "testNetwork"}
			_, nerr := ag.nagent.Netagent.FindNetwork(ometa)
			return (nerr == nil)
		}, "Network not found in agent")
	}

	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create one endpoint from each agent
	for i, ag := range it.agents {
		go func(i int, ag *Dpagent) {
			epname := fmt.Sprintf("testEndpoint-%d", i)
			hostName := fmt.Sprintf("testHost-%d", i)

			// make the call
			ep, cerr := ag.createEndpointReq("default", "testNetwork", epname, hostName)
			if cerr != nil {
				waitCh <- fmt.Errorf("endpoint create failed: %v", cerr)
				return
			}
			if ep.Name != epname {
				waitCh <- fmt.Errorf("Endpoint name did not match")
				return
			}
			if ep.Status.HomingHostName != hostName {
				waitCh <- fmt.Errorf("host name did not match")
				return
			}

			// verify endpoint was added to datapath
			eps, ok := ag.datapath.EndpointDB[objKey(ep.ObjectMeta)]
			if !ok || len(eps.Request) != 1 {
				waitCh <- fmt.Errorf("Endpoint not found in datapath")
				return
			}
			waitCh <- nil
		}(i, ag)
	}

	// wait for all endpoint creates to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Error during endpoint create")
	}

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() bool {
				return (len(ag.datapath.EndpointDB) == it.numAgents)
			}, "10ms", fmt.Sprintf("%dms", 3000+100*it.numAgents))
			if !found {
				waitCh <- fmt.Errorf("Endpoint cound incorrect in datapath")
				return
			}
			for i := range it.agents {
				epname := fmt.Sprintf("testEndpoint-%d", i)
				eps, ok := ag.datapath.EndpointDB[fmt.Sprintf("%s|%s", "default", epname)]
				if !ok || len(eps.Request) != 1 {
					waitCh <- fmt.Errorf("Endpoint not found in datapath")
					return
				}
			}
			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")

	}

	// now delete the endpoints
	for i, ag := range it.agents {
		go func(i int, ag *Dpagent) {
			epname := fmt.Sprintf("testEndpoint-%d", i)
			hostName := fmt.Sprintf("testHost-%d", i)

			// make the call
			ep, cerr := ag.deleteEndpointReq("default", "testNetwork", epname, hostName)
			if cerr != nil && cerr != netagent.ErrEndpointNotFound {
				waitCh <- fmt.Errorf("Endpoint delete failed: %v", err)
				return
			}

			// verify endpoint was deleted from datapath
			eps, ok := ag.datapath.EndpointDelDB[objKey(ep.ObjectMeta)]
			if !ok || len(eps.Request) != 1 {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}
			waitCh <- nil
		}(i, ag)
	}

	// wait for all endpoint deletes to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete failed")

	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() bool {
				return (len(ag.datapath.EndpointDB) == 0)
			}, "10ms", fmt.Sprintf("%dms", 3000+100*it.numAgents)) {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	AssertEventually(c, func() bool {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil)
	}, "Network still found in statemgr")
}
