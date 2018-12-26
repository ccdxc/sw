// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmSgCreateDelete(c *C) {
	// create sg in watcher
	err := it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	c.Assert(err, IsNil)

	// verify all agents have the security group
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListSecurityGroup()) == 1, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
	}

	// incoming rule
	rules := []security.SGRule{
		{
			Action:          "PERMIT",
			FromIPAddresses: []string{"10.1.1.1/24"},
			ToIPAddresses:   []string{"10.1.1.1/24"},
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
			},
		},
	}

	// create sg policy
	err = it.CreateSgpolicy("default", "default", "testpolicy", true, []string{}, rules)
	c.Assert(err, IsNil)

	// construct object meta
	policyMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      "testpolicy",
	}

	// verify agent state has the policy has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindSGPolicy(policyMeta)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListSGPolicy()), "10ms", it.pollTimeout())
	}

	// delete the sg policy
	err = it.DeleteSgpolicy("default", "default", "testpolicy")
	c.Assert(err, IsNil)

	// verify rules are gone from agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindSGPolicy(policyMeta)
			if err == nil {
				return false, nil
			}
			return true, nil
		}, "Sg rules still found on agent", "10ms", it.pollTimeout())
	}
	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from datapath
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListSGPolicy()) == 0, nil
		}, "Sg still found on agent", "10ms", it.pollTimeout())
	}
}

func (it *integTestSuite) TestNpmSgEndpointAttach(c *C) {
	// create a network in controller
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.0.0/22", "10.1.1.254")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// create sg in watcher
	err = it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, serr := it.ctrler.StateMgr.FindSecurityGroup("default", "testsg")
		return (serr == nil), nil
	}, "Sg not found in statemgr")

	// create endpoint
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint1", "testVm1", "01:01:01:01:01:01", "host1", "20.1.1.1", map[string]string{"env": "production", "app": "procurement"}, 2)
	c.Assert(err, IsNil)

	// verify endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListEndpoint()) == 1, nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
		_, err := ag.nagent.NetworkAgent.FindEndpoint("default", "default", "testEndpoint1")
		c.Assert(err, Equals, nil)
	}

	// create second endpoint
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint2", "testVm2", "02:02:02:02:02:02", "host2", "20.2.2.2", map[string]string{"env": "production", "app": "procurement"}, 3)
	c.Assert(err, IsNil)

	// verify new endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListEndpoint()) == 2, nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
	}

	// delete the second endpoint
	err = it.DeleteEndpoint("default", "default", "testEndpoint2")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListEndpoint()) == 1, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from the endpoint
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			ep, err := ag.nagent.NetworkAgent.FindEndpoint("default", "default", "testEndpoint1")
			if err != nil {
				return false, nil
			}
			for _, sgName := range ep.Spec.SecurityGroups {
				if sgName == "testsg" {
					return false, nil
				}
			}
			return true, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete endpoint
	err = it.DeleteEndpoint("default", "default", "testEndpoint1")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListEndpoint()) == 0, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "endpoint still found on agent", "10ms", it.pollTimeout())
}
