// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"
	"strconv"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmSgCreateDelete(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
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
			_, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
	}

	// delete the sg policy
	err = it.DeleteSgpolicy("default", "default", "testpolicy")
	c.Assert(err, IsNil)

	// verify rules are gone from agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
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
			return len(ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()) == 0, nil
		}, "Sg still found on agent", "10ms", it.pollTimeout())
	}
}

// TestNpmSgCreateUpdateDelete
func (it *integTestSuite) TestNpmSgCreateDeleteWitApps(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	// create sg in watcher
	err := it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	c.Assert(err, IsNil)
	numApps := 200
	addApps := 100
	ports := []string{}
	apps := []string{}
	for i := 100; i < 100+numApps; i++ {
		port := strconv.Itoa(i)
		name := "app-" + port
		apps = append(apps, name)
		ports = append(ports, port)
	}

	// verify all agents have the security group
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListSecurityGroup()) == 1, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
	}

	//Stop App Watch so that we  miss apps update.

	it.ctrler.StateMgr.StopAppWatch()
	for index, app := range apps {
		err := it.CreateApp("default", "default", app, ports[index])
		c.Assert(err, IsNil)
		if index+1 == addApps {
			break
		}
	}

	for _, ag := range it.agents {
		for index, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				_, err := ag.nagent.NetworkAgent.FindApp(policyMeta)
				if err == nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
			if index+1 == addApps {
				break
			}
		}
	}

	// incoming rule
	rules := []security.SGRule{
		{
			Action:          "PERMIT",
			Apps:            apps[:addApps],
			FromIPAddresses: []string{"10.1.1.1/24"},
			ToIPAddresses:   []string{"10.1.1.1/24"},
		},
	}

	// create sg policy
	err = it.CreateSgpolicy("default", "default", "testpolicy", true, []string{}, rules)
	c.Assert(err, IsNil)

	policyMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      "testpolicy",
	}

	//Sleep for a while to make sure we don't receive the policy
	time.Sleep(500 * time.Millisecond)
	// verify agent state does not have the policy has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
			if err == nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("Sg rules found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
	}

	//Now we will allow apps to resolve
	it.ctrler.StateMgr.StartAppWatch()
	// construct object meta
	for _, ag := range it.agents {
		for index, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				_, err := ag.nagent.NetworkAgent.FindApp(policyMeta)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
			if index+1 == addApps {
				break
			}
		}
	}

	// verify agent state has the policy has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
	}

	//Do Update
	// incoming rule

	for index, app := range apps[addApps:] {
		err := it.CreateApp("default", "default", app, ports[index])
		c.Assert(err, IsNil)
		if index+1 == addApps {
			break
		}
	}

	//Now we should have all the apps
	for _, ag := range it.agents {
		for _, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				_, err := ag.nagent.NetworkAgent.FindApp(policyMeta)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App not found on agent. DB: %v", len(ag.nagent.NetworkAgent.ListApp())), "10ms", it.pollTimeout())
		}
	}

	rules = []security.SGRule{
		{
			Action:          "PERMIT",
			Apps:            apps[addApps:],
			FromIPAddresses: []string{"10.1.1.2/24"},
			ToIPAddresses:   []string{"10.1.1.3/24"},
		},
	}

	err = it.UpdateSgpolicy("default", "default", "testpolicy", true, []string{}, rules)
	c.Assert(err, IsNil)

	// verify agent state has the policy has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			gsgp, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
			if err != nil {
				return false, nil
			}
			if len(gsgp.Spec.Rules) != numApps {
				return false, gsgp.Spec.Rules
			}
			return true, nil
		}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
	}

	//Stop Policy watch so that npm does not receive delete
	it.ctrler.StopNetworkSecurityPolicyWatch()
	// delete the sg policy
	err = it.DeleteSgpolicy("default", "default", "testpolicy")
	c.Assert(err, IsNil)

	//Lets delete the apps from api server
	for _, app := range apps {
		err := it.DeleteApp("default", app)
		c.Assert(err, IsNil)
	}

	time.Sleep(500 * time.Millisecond)

	//Make sure apps not deleted from agent yet
	for _, ag := range it.agents {
		for _, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				_, err := ag.nagent.NetworkAgent.FindApp(policyMeta)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App found on agent. DB: %v", len(ag.nagent.NetworkAgent.ListApp())), "100ms", it.pollTimeout())
		}
	}

	it.ctrler.StartNetworkSecurityPolicyWatch()
	time.Sleep(100 * time.Millisecond)

	// verify rules are not gone from agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, err := ag.nagent.NetworkAgent.FindNetworkSecurityPolicy(policyMeta)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Sg rules still found on agent", "10ms", it.pollTimeout())
	}

	//Make sure apps not deleted from agent yet
	for _, ag := range it.agents {
		for _, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				_, err := ag.nagent.NetworkAgent.FindApp(policyMeta)
				if err != nil {
					return true, nil
				}
				return false, nil
			}, fmt.Sprintf("Sg rules not found on agent. DB: %v", ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()), "10ms", it.pollTimeout())
		}
	}

	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from datapath
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListNetworkSecurityPolicy()) == 0, nil
		}, "Sg still found on agent", "10ms", it.pollTimeout())
	}
}

func (it *integTestSuite) TestNpmSgEndpointAttach(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	// create a network in controller
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.0.0/22", "10.1.1.254")
	//c.Assert(err, IsNil)
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
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint1", "testVm1", "0101.0101.0101", "host1", "20.1.1.1", map[string]string{"env": "production", "app": "procurement"}, 2)
	c.Assert(err, IsNil)

	// verify endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return len(ag.nagent.NetworkAgent.ListEndpoint()) == 1, nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
		epmeta := api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint1",
		}
		_, err := ag.nagent.NetworkAgent.FindEndpoint(epmeta)
		c.Assert(err, Equals, nil)
	}

	// create second endpoint
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint2", "testVm2", "0202.0202.0202", "host2", "20.2.2.2", map[string]string{"env": "production", "app": "procurement"}, 3)
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
			epmeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testEndpoint1",
			}
			ep, err := ag.nagent.NetworkAgent.FindEndpoint(epmeta)
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
