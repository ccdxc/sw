// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"flag"
	"fmt"
	"testing"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm"

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

// integ test suite
var sts = &integTestSuite{}

var _ = Suite(sts)

// Hook up gocheck into the "go test" runner.
func TestInteg(t *testing.T) {
	TestingT(t)
}

func (it *integTestSuite) SetUpSuite(c *C) {
	// test parameters
	it.numAgents = *numAgents

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestRPCURL)
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numAgents)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		agent, err := CreateAgent(integTestRPCURL)
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

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

	// stop server and client
	it.ctrler.RPCServer.Stop()
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
	c.Assert(err, IsNil)

	// verify agent receives the network
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))
	for _, ag := range it.agents {
		nt, nerr := ag.nagent.Netagent.FindNetwork(api.ObjectMeta{Tenant: "default", Name: "testNetwork"})
		c.Assert(nerr, IsNil)
		log.Infof("Found network {%+v} on agent", nt)
		c.Assert(nt.Spec.IPv4Subnet, Equals, "10.1.1.0/24")
	}

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))
	for _, ag := range it.agents {
		_, err := ag.nagent.Netagent.FindNetwork(api.ObjectMeta{Tenant: "default", Name: "testNetwork"})
		c.Assert(err, NotNil)
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
	time.Sleep(time.Millisecond * 5 * time.Duration(it.numAgents))

	// create one endpoint from each agent
	for i, ag := range it.agents {
		epname := fmt.Sprintf("testEndpoint-%d", i)
		hostName := fmt.Sprintf("testHost-%d", i)

		// make the call
		ep, cerr := ag.createEndpointReq("default", "testNetwork", epname, hostName)
		c.Assert(cerr, IsNil)
		c.Assert(ep.Name, Equals, epname)
		c.Assert(ep.Status.HomingHostName, Equals, hostName)

		// verify endpoint was added to datapath
		c.Assert(len(ag.datapath.EndpointDB[objKey(ep.ObjectMeta)].Request), Equals, 1)
	}

	// wait for all endpoints to be propagated to other agents
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.EndpointDB), Equals, it.numAgents)
		for i := range it.agents {
			epname := fmt.Sprintf("testEndpoint-%d", i)
			c.Assert(len(ag.datapath.EndpointDB[fmt.Sprintf("%s|%s", "default", epname)].Request), Equals, 1)
		}
	}

	// now delete the endpoints
	for i, ag := range it.agents {
		epname := fmt.Sprintf("testEndpoint-%d", i)
		hostName := fmt.Sprintf("testHost-%d", i)

		// make the call
		ep, cerr := ag.deleteEndpointReq("default", "testNetwork", epname, hostName)
		c.Assert((cerr == nil || cerr == netagent.ErrEndpointNotFound), Equals, true)

		// verify endpoint was added to datapath
		c.Assert(len(ag.datapath.EndpointDelDB[objKey(ep.ObjectMeta)].Request), Equals, 1)
	}
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))
}
