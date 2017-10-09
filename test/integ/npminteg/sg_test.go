// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/api/generated/network"
	. "github.com/pensando/sw/venice/utils/testutils"
	. "gopkg.in/check.v1"
)

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmSgCreateDelete(c *C) {
	// expect sg calls in datapath
	if it.datapathKind.String() == "mock" {
		for _, ag := range it.agents {
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
		}
	}

	// create sg in watcher
	err := it.ctrler.Watchr.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	c.Assert(err, IsNil)

	// verify all agents have the security group
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return len(ag.datapath.DB.SgDB) == 1, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
		c.Assert(len(ag.datapath.DB.SgDB[fmt.Sprintf("%s|%s", "default", "testsg")].Request), Equals, 1)
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

	// verify datapath has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			sg, ok := ag.datapath.DB.SgDB[fmt.Sprintf("%s|%s", "default", "testsg")]
			if !ok {
				return false, nil
			}

			return ((len(sg.Request[0].IngressPolicy.FwRules) == 1) && (len(sg.Request[0].EgressPolicy.FwRules) == 1)), nil
		}, "Sg rules not found on agent", "10ms", it.pollTimeout())
	}

	// delete the sg policy
	err = it.ctrler.Watchr.DeleteSgpolicy("default", "testpolicy")
	c.Assert(err, IsNil)

	// verify rules are gone from datapath
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			sg, ok := ag.datapath.DB.SgDB[fmt.Sprintf("%s|%s", "default", "testsg")]
			if !ok {
				return false, nil
			}

			return ((len(sg.Request[0].IngressPolicy.FwRules) == 0) && (len(sg.Request[0].EgressPolicy.FwRules) == 0)), nil
		}, "Sg rules still found on agent", "10ms", it.pollTimeout())
	}
	// delete the security group
	err = it.ctrler.Watchr.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from datapath
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return (len(ag.datapath.DB.SgDB) == 0), nil
		}, "Sg still found on agent", "10ms", it.pollTimeout())
	}
}

func (it *integTestSuite) TestNpmSgEndpointAttach(c *C) {
	// expect sg & ep calls in datapath
	//FIXME with real hal
	// expect sg calls in datapath
	if it.datapathKind.String() == "mock" {
		for _, ag := range it.agents {
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockNetclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockEpclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).MaxTimes(2).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockEpclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).MaxTimes(1).Return(nil, nil)
			ag.datapath.Hal.MockClients.MockNetclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
		}
	}

	// create a network in controller
	err := it.ctrler.Watchr.CreateNetwork("default", "testNetwork", "10.1.0.0/16", "10.1.1.254")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, []interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// create sg in watcher
	err = it.ctrler.Watchr.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, []interface{}) {
		_, serr := it.ctrler.StateMgr.FindSecurityGroup("default", "testsg")
		return (serr == nil), nil
	}, "Sg not found in statemgr")

	// create endpoint
	err = it.ctrler.Watchr.CreateEndpoint("default", "testNetwork", "testEndpoint1", "testVm1", "01:01:01:01:01:01", "host1", "20.1.1.1", []string{"env:production", "app:procurement"}, 2)
	c.Assert(err, IsNil)

	// verify endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return (len(ag.datapath.DB.EndpointDB) == 1), nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
		ep, ok := ag.datapath.DB.EndpointDB[fmt.Sprintf("%s|%s", "default", "testEndpoint1")]
		c.Assert(ok, Equals, true)
		c.Assert(len(ep.Request), Equals, 1)
		c.Assert(len(ep.Request[0].SecurityGroup), Equals, 1)
		c.Assert(ep.Request[0].UsegVlan, Equals, uint32(2))
	}

	// create second endpoint
	err = it.ctrler.Watchr.CreateEndpoint("default", "testNetwork", "testEndpoint2", "testVm2", "02:02:02:02:02:02", "host2", "20.2.2.2", []string{"env:production", "app:procurement"}, 3)
	c.Assert(err, IsNil)

	// verify new endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return (len(ag.datapath.DB.EndpointDB) == 2), nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
		ep, ok := ag.datapath.DB.EndpointDB[fmt.Sprintf("%s|%s", "default", "testEndpoint2")]
		c.Assert(ok, Equals, true)
		c.Assert(len(ep.Request), Equals, 1)
		c.Assert(len(ep.Request[0].SecurityGroup), Equals, 1)
		c.Assert(ep.Request[0].UsegVlan, Equals, uint32(3))
	}

	// delete the second endpoint
	err = it.ctrler.Watchr.DeleteEndpoint("default", "testNetwork", "testEndpoint2", "testVm2", "02:02:02:02:02:02", "host2", "20.2.2.2")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return (len(ag.datapath.DB.EndpointDB) == 1), nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the security group
	err = it.ctrler.Watchr.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from the endpoint
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			ep, ok := ag.datapath.DB.EndpointUpdateDB[fmt.Sprintf("%s|%s", "default", "testEndpoint1")]
			if ok && len(ep.Request) == 1 && len(ep.Request[0].SecurityGroup) == 0 {
				return true, nil
			}
			return false, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete endpoint
	err = it.ctrler.Watchr.DeleteEndpoint("default", "testNetwork", "testEndpoint1", "testVm1", "01:01:01:01:01:01", "host1", "20.1.1.1")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, []interface{}) {
			return (len(ag.datapath.DB.EndpointDB) == 0), nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.ctrler.Watchr.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, []interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "endpoint still found on agent", "10ms", it.pollTimeout())
}
