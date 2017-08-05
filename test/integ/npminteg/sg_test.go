// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import

// . "github.com/pensando/sw/utils/testutils"
(
	"fmt"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/api/generated/network"
	. "gopkg.in/check.v1"
)

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmSgCreateDelete(c *C) {
	// expect sg calls in datapath
	for _, ag := range it.agents {
		ag.datapath.Sgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityPolicyRuleCreate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityPolicyRuleDelete(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	// create sg in watcher
	err := it.ctrler.Watchr.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// verify all agents have the security group
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.SgDB), Equals, 1)
		c.Assert(len(ag.datapath.SgDB[fmt.Sprintf("%s|%s", "default", "testsg")].Request), Equals, 1)
	}

	// incoming rule
	inrules := []network.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}
	outrules := []network.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}

	// create sg policy
	err = it.ctrler.Watchr.CreateSgpolicy("default", "testpolicy", []string{"testsg"}, inrules, outrules)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// verify datapath has the rules
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.SgRule), Equals, 2)
	}

	// delete the sg policy
	err = it.ctrler.Watchr.DeleteSgpolicy("default", "testpolicy")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// verify rules are gone from datapath
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.SgRule), Equals, 0)
	}

	// delete the security group
	err = it.ctrler.Watchr.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// verify sg is removed from datapath
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.SgDB), Equals, 0)
	}
}

func (it *integTestSuite) TestNpmSgEndpointAttach(c *C) {
	// expect sg & ep calls in datapath
	for _, ag := range it.agents {
		ag.datapath.Netclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Epclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		ag.datapath.Epclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
		ag.datapath.Sgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
		ag.datapath.Epclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).MaxTimes(1).Return(nil, nil)
		ag.datapath.Netclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)

	}

	// create a network in controller
	err := it.ctrler.Watchr.CreateNetwork("default", "testNetwork", "10.1.0.0/16", "10.1.1.254")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// create sg in watcher
	err = it.ctrler.Watchr.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// create endpoint
	err = it.ctrler.Watchr.CreateEndpoint("default", "testNetwork", "testEndpoint1", "testVm1", "01:01:01:01:01:01", "host1", "20.1.1.1", []string{"env:production", "app:procurement"}, 2)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 5 * time.Duration(it.numAgents))

	// verify endpoint is present in all agents
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.EndpointDB), Equals, 1)
		ep, ok := ag.datapath.EndpointDB[fmt.Sprintf("%s|%s", "default", "testEndpoint1")]
		c.Assert(ok, Equals, true)
		c.Assert(len(ep.Request), Equals, 1)
		c.Assert(len(ep.Request[0].SecurityGroup), Equals, 1)
		c.Assert(ep.Request[0].UsegVlan, Equals, uint32(2))
	}

	// create second endpoint
	err = it.ctrler.Watchr.CreateEndpoint("default", "testNetwork", "testEndpoint2", "testVm2", "02:02:02:02:02:02", "host2", "20.2.2.2", []string{"env:production", "app:procurement"}, 3)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 5 * time.Duration(it.numAgents))

	// verify new endpoint is present in all agents
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.EndpointDB), Equals, 2)
		ep, ok := ag.datapath.EndpointDB[fmt.Sprintf("%s|%s", "default", "testEndpoint2")]
		c.Assert(ok, Equals, true)
		c.Assert(len(ep.Request), Equals, 1)
		c.Assert(len(ep.Request[0].SecurityGroup), Equals, 1)
		c.Assert(ep.Request[0].UsegVlan, Equals, uint32(3))
	}

	// delete the second endpoint
	err = it.ctrler.Watchr.DeleteEndpoint("default", "testNetwork", "testEndpoint2", "testVm2", "02:02:02:02:02:02", "host2", "20.2.2.2")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// delete the security group
	err = it.ctrler.Watchr.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// verify sg is removed from the endpoint
	for _, ag := range it.agents {
		c.Assert(len(ag.datapath.EndpointDB), Equals, 1)
		ep, ok := ag.datapath.EndpointDB[fmt.Sprintf("%s|%s", "default", "testEndpoint1")]
		c.Assert(ok, Equals, true)
		c.Assert(len(ep.Request), Equals, 1)
		c.Assert(len(ep.Request[0].SecurityGroup), Equals, 0)
	}

	// delete endpoint
	err = it.ctrler.Watchr.DeleteEndpoint("default", "testNetwork", "testEndpoint1", "testVm1", "01:01:01:01:01:01", "host1", "20.1.1.1")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * time.Duration(it.numAgents))
}
