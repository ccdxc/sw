// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"
	"strconv"
	"time"

	. "gopkg.in/check.v1"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmSgCreateDelete(c *C) {

	// clean up stale SG Policies
	policies, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	AssertOk(c, err, "failed to list policies")

	for _, p := range policies {
		_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Delete(context.Background(), &p.ObjectMeta)
		AssertOk(c, err, "failed to clean up policy. NSP: %v | Err: %v", p.GetKey(), err)
	}

	// Check agent doesn't have any SG Policies
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
			}
			policies, _ := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.List, nsgp)
			if len(policies) != 0 {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("SG Policies not deleted from agent"), "10ms", it.pollTimeout())
	}

	// if not present create the default tenant
	it.CreateTenant("default")
	// create sg in watcher
	err = it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	c.Assert(err, IsNil)

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
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			_, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("SG Policy not found on agent. SGPolicy: %v", policyMeta), "10ms", it.pollTimeout())
	}

	// delete the sg policy
	err = it.DeleteSgpolicy("default", "default", "testpolicy")
	c.Assert(err, IsNil)

	// verify rules are gone from agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			_, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if err == nil {
				return false, nil
			}
			return true, nil
		}, "Sg rules still found on agent", "10ms", it.pollTimeout())
	}
	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)
}

// TestNpmSgCreateUpdateDelete
func (it *integTestSuite) TestNpmSgCreateDeleteWitApps(c *C) {
	// clean up stale SG Policies
	policies, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	AssertOk(c, err, "failed to list policies")

	for _, p := range policies {
		_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Delete(context.Background(), &p.ObjectMeta)
		AssertOk(c, err, "failed to clean up policy. NSP: %v | Err: %v", p.GetKey(), err)
	}

	// if not present create the default tenant
	it.CreateTenant("default")
	// create sg in watcher
	err = it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	//c.Assert(err, IsNil)
	numApps := 2
	addApps := 1
	ports := []string{}
	apps := []string{}
	for i := 100; i < 100+numApps; i++ {
		port := strconv.Itoa(i)
		name := "app-" + port
		apps = append(apps, name)
		ports = append(ports, port)
	}

	//Stop App Watch so that we  miss apps update.

	it.npmCtrler.StateMgr.StopAppWatch()
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
				napp := netproto.App{
					TypeMeta:   api.TypeMeta{Kind: "App"},
					ObjectMeta: policyMeta,
				}
				_, err := ag.dscAgent.PipelineAPI.HandleApp(agentTypes.Get, napp)
				if err == nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App not found in agent. App: %v", policyMeta), "10ms", it.pollTimeout())
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
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			_, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if err == nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("SGPolicy still found in agent. SGP: %v", policyMeta), "10ms", it.pollTimeout())
	}

	//Now we will allow apps to resolve
	it.npmCtrler.StateMgr.StartAppWatch()
	// construct object meta
	for _, ag := range it.agents {
		for index, app := range apps {
			policyMeta := api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      app,
			}

			AssertEventually(c, func() (bool, interface{}) {
				napp := netproto.App{
					TypeMeta:   api.TypeMeta{Kind: "App"},
					ObjectMeta: policyMeta,
				}
				_, err := ag.dscAgent.PipelineAPI.HandleApp(agentTypes.Get, napp)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App found on agent. App: %v", policyMeta), "10ms", it.pollTimeout())
			if index+1 == addApps {
				break
			}
		}
	}

	// verify agent state has the policy has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			_, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, fmt.Sprintf("SGpolicy not found in agent. SGP: %v", policyMeta), "10ms", it.pollTimeout())
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
				napp := netproto.App{
					TypeMeta:   api.TypeMeta{Kind: "App"},
					ObjectMeta: policyMeta,
				}
				_, err := ag.dscAgent.PipelineAPI.HandleApp(agentTypes.Get, napp)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App not found on agent. App: %v", policyMeta), "10ms", it.pollTimeout())
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
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			gsgp, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if err != nil {
				return false, nil
			}
			if len(gsgp[0].Spec.Rules) != numApps {
				return false, gsgp[0].Spec.Rules
			}
			return true, nil
		}, fmt.Sprintf("Sg rules not found on agent. SGP: %v", policyMeta), "10ms", it.pollTimeout())
	}

	//Stop Policy watch so that npm does not receive delete
	it.npmCtrler.StopNetworkSecurityPolicyWatch()
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
				napp := netproto.App{
					TypeMeta:   api.TypeMeta{Kind: "App"},
					ObjectMeta: policyMeta,
				}
				_, err := ag.dscAgent.PipelineAPI.HandleApp(agentTypes.Get, napp)
				if err != nil {
					return false, nil
				}
				return true, nil
			}, fmt.Sprintf("App found on agent. DB: %v", policyMeta), "100ms", it.pollTimeout())
		}
	}

	it.npmCtrler.StartNetworkSecurityPolicyWatch()
	time.Sleep(2 * time.Second)

	// verify rules are not gone from agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: policyMeta,
			}
			_, err := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
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
				napp := netproto.App{
					TypeMeta:   api.TypeMeta{Kind: "App"},
					ObjectMeta: policyMeta,
				}
				_, err := ag.dscAgent.PipelineAPI.HandleApp(agentTypes.Get, napp)
				if err != nil {
					return true, nil
				}
				return false, nil
			}, fmt.Sprintf("App not found on agent. App: %v", policyMeta), "10ms", it.pollTimeout())
		}
	}

	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)
}

func (it *integTestSuite) TestNpmSgEndpointAttach(c *C) {

	// if not present create the default tenant
	it.CreateTenant("default")
	// create a network in controller
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.0.0/22", "10.1.1.254")
	//c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// create sg in watcher
	err = it.CreateSecurityGroup("default", "default", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, serr := it.npmCtrler.StateMgr.FindSecurityGroup("default", "testsg")
		return (serr == nil), nil
	}, "Sg not found in statemgr")

	// create endpoint
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint1", "testVm1", "0101.0101.0101", "host1", "20.1.1.1", map[string]string{"env": "production", "app": "procurement"}, 2)
	c.Assert(err, IsNil)

	// verify endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 1, nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
		epmeta := netproto.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testEndpoint1",
			},
		}
		_, err := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
		c.Assert(err, Equals, nil)
	}

	// create second endpoint
	err = it.CreateEndpoint("default", "default", "testNetwork", "testEndpoint2", "testVm2", "0202.0202.0202", "host2", "20.2.2.2", map[string]string{"env": "production", "app": "procurement"}, 3)
	c.Assert(err, IsNil)

	// verify new endpoint is present in all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 2, nil
		}, "endpoint not found on agent", "10ms", it.pollTimeout())
	}

	// delete the second endpoint
	err = it.DeleteEndpoint("default", "default", "testEndpoint2")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 1, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the security group
	err = it.DeleteSecurityGroup("default", "testsg")
	c.Assert(err, IsNil)

	// verify sg is removed from the endpoint
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epmeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      "testEndpoint1",
				},
			}
			_, err := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete endpoint
	err = it.DeleteEndpoint("default", "default", "testEndpoint1")
	c.Assert(err, IsNil)
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 0, nil
		}, "endpoint still found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "endpoint still found on agent", "10ms", it.pollTimeout())
}

// TestNpmSgCreateDelete
func (it *integTestSuite) TestNpmFwProfileCreateDelete(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	err := it.npmCtrler.StateMgr.EnableSelectivePushForKind("SecurityProfile")
	c.Assert(err, IsNil)
	// create sg in watcher
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:        "90s",
			TCPConnectionSetupTimeout: "30s",
			TCPCloseTimeout:           "15s",
			TCPHalfClosedTimeout:      "120s",
			TCPDropTimeout:            "90s",
			UDPDropTimeout:            "60s",
			ICMPDropTimeout:           "60s",
			DropTimeout:               "60s",
			TcpTimeout:                "3m",
			UdpTimeout:                "3m",
			IcmpTimeout:               "3m",
			TcpHalfOpenSessionLimit:   20000,
			UdpActiveSessionLimit:     20000,
			IcmpActiveSessionLimit:    20000,
			OtherActiveSessionLimit:   20000,
			DetectApp:                 true,
		},
	}

	_, err = it.apisrvClient.SecurityV1().FirewallProfile().Delete(context.Background(), &fwp.ObjectMeta)

	c.Assert(err, IsNil)

	// verify all agents have the security group
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			p := netproto.SecurityProfile{TypeMeta: api.TypeMeta{Kind: "SecurityProfile"}}
			profiles, _ := ag.dscAgent.PipelineAPI.HandleSecurityProfile(agentTypes.List, p)

			return len(profiles) == 0, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
	}

	_, err = it.apisrvClient.SecurityV1().FirewallProfile().Create(context.Background(), &fwp)

	c.Assert(err, IsNil)

	//Check whetheer we have receivers

	for _, ag := range it.agents {
		ok, err := it.npmCtrler.StateMgr.DSCAddedAsReceiver(ag.dscAgent.InfraAPI.GetDscName())
		c.Assert(err, IsNil)
		Assert(c, ok, "DSC not added")
	}
	// verify that agents have received
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			p := netproto.SecurityProfile{TypeMeta: api.TypeMeta{Kind: "SecurityProfile"}}
			profiles, err := ag.dscAgent.PipelineAPI.HandleSecurityProfile(agentTypes.List, p)
			logrus.Infof("profiles : %v err:%v", profiles, err)
			return len(profiles) == 1, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
	}

	// invalid range checks
	fwp.Spec.TcpHalfOpenSessionLimit = 128001
	_, err = it.apisrvClient.SecurityV1().FirewallProfile().Update(context.Background(), &fwp)
	Assert(c, err != nil, "Invalid range above 128000 must fail for SessionLimit")

	// change conn track and session timeout
	fwp.Spec.SessionIdleTimeout = "5m"
	fwp.Spec.TcpHalfOpenSessionLimit = 128000
	fwp.Spec.DetectApp = true
	_, err = it.apisrvClient.SecurityV1().FirewallProfile().Update(context.Background(), &fwp)
	AssertOk(c, err, "Error updating firewall profile")

	// verify params got updated in agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			profileMeta := netproto.SecurityProfile{
				TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
				ObjectMeta: fwp.ObjectMeta,
			}
			secp, cerr := ag.dscAgent.PipelineAPI.HandleSecurityProfile(agentTypes.Get, profileMeta)

			if (cerr != nil) || (secp[0].Spec.Timeouts.SessionIdle != fwp.Spec.SessionIdleTimeout) || (secp[0].Spec.RateLimits.TcpHalfOpenSessionLimit != fwp.Spec.TcpHalfOpenSessionLimit) || (secp[0].Spec.DetectApp != fwp.Spec.DetectApp) {
				return false, secp
			}
			return true, nil
		}, "Sg not found on agent", "10ms", it.pollTimeout())
	}

	/*
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
	*/
}
