// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	"github.com/pensando/sw/api/generated/security"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/utils/featureflags"

	"context"
	//"fmt"
	"time"

	"gopkg.in/check.v1"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	//"github.com/pensando/sw/api/generated/security"
	//"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	//"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	//"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// this test simulates API server restarting while NPM is up and runinng
func (it *integTestSuite) TestNpmApiServerRestart(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")

	// create a network in controller
	// if not present create the default tenant
	it.CreateTenant("default")
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// stop API server
	it.apiSrv.Stop()
	it.apisrvClient.Close()
	time.Sleep(time.Millisecond * 10)
	log.Infof("================ Stopped API server. Restarting ==============")

	// restart API server
	it.apiSrv, _, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test", it.logger)
	c.Assert(err, check.IsNil)
	it.apisrvClient, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, it.logger, rpckit.WithBalancer(balancer.New(it.resolverClient)))
	c.Assert(err, check.IsNil)
	time.Sleep(time.Second)

	// verify NPM still has the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}

	// recreate the network in API server
	err = it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify NPM recreates the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on agent", "10ms", it.pollTimeout())

	// restart API server. But, use different cluster name to emulate API server loosing state
	it.apiSrv.Stop()
	it.apisrvClient.Close()
	time.Sleep(time.Millisecond * 10)
	it.apiSrv, _, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test2", it.logger)
	c.Assert(err, check.IsNil)
	it.apisrvClient, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, it.logger, rpckit.WithBalancer(balancer.New(it.resolverClient)))
	c.Assert(err, check.IsNil)

	// wait for network to go away from NPM
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "Network still found in NPM", "100ms", it.pollTimeout())

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}
	it.CreateTenant("default")
	// recreate the network in API server
	err = it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify NPM recreates the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// one more restart API server. this time go back to old cluster name
	it.apiSrv.Stop()
	it.apisrvClient.Close()
	time.Sleep(time.Millisecond * 10)
	it.apiSrv, _, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test", it.logger)
	c.Assert(err, check.IsNil)
	it.apisrvClient, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, it.logger, rpckit.WithBalancer(balancer.New(it.resolverClient)))
	c.Assert(err, check.IsNil)

	// verify NPM still has the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}
}

// this test simulates NPM restarting while API server and agents remain up and running
func (it *integTestSuite) TestNpmRestart(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")

	// create a network in controller
	// if not present create the default tenant
	it.CreateTenant("default")
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// stop NPM
	err = it.npmCtrler.Stop()
	AssertOk(c, err, "Error stopping NPM")

	// verify agents are all disconnected
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return !cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}
	// Set feature flags to initialized
	featureflags.SetInitialized()

	// restart the NPM
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 100)

	// verify NPM still has the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not connected to NPM", "10ms", it.pollTimeout())
	}

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}
}

func (it *integTestSuite) TestNpmRestartWithNetworkSecurityPolicy(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")
	const numApps = 10
	// if not present create the default tenant
	it.CreateTenant("default")

	// app
	appNames := []string{}
	for i := 0; i < numApps; i++ {
		appName := fmt.Sprintf("ftpApp-%d", i)
		appNames = append(appNames, appName)
		ftpApp := security.App{
			TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Name:      appName,
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: security.AppSpec{
				ProtoPorts: []security.ProtoPort{
					{
						Protocol: "tcp",
						Ports:    "21",
					},
				},
				Timeout: "5m",
				ALG: &security.ALG{
					Type: "FTP",
					Ftp: &security.Ftp{
						AllowMismatchIPAddress: true,
					},
				},
			},
		}
		it.apisrvClient.SecurityV1().App().Create(context.Background(), &ftpApp)
		//AssertOk(c, err, "error creating app")
	}

	// create a policy using this alg
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         "test-sgpolicy",
			GenerationID: "1",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"2.101.0.0/22"},
					ToIPAddresses:   []string{"2.101.0.0/24"},
					Apps:            appNames,
					Action:          "PERMIT",
				},
			},
		},
	}
	time.Sleep(time.Millisecond * 100)

	// create sg policy
	_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Create(context.Background(), &sgp)
	//AssertOk(c, err, "error creating sg policy")

	// verify agent state has the policy
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			_, gerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if gerr != nil {
				return false, fmt.Errorf("Error finding sgpolicy for %+v", sgp.ObjectMeta)
			}
			return true, nil
		}, fmt.Sprintf("Sg policy not found in agent. SGPolicy: %v", sgp.GetKey()), "10ms", it.pollTimeout())
	}

	// verify sgpolicy status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		tsgp, gerr := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Get(context.Background(), &sgp.ObjectMeta)
		if gerr != nil {
			return false, gerr
		}
		if tsgp.Status.PropagationStatus.GenerationID != sgp.ObjectMeta.GenerationID {
			return false, tsgp
		}
		if (tsgp.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.Status.PropagationStatus.Pending != 0) {
			return false, tsgp
		}
		return true, nil
	}, "SgPolicy status was not updated after creating the policy", "100ms", it.pollTimeout())

	log.Infof("==================== Restarting NPM =========================")

	// stop NPM
	err = it.npmCtrler.Stop()
	AssertOk(c, err, "Error stopping NPM")

	// verify agents are all disconnected
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return !cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}

	// Set feature flags to initialized
	featureflags.SetInitialized()

	// restart the NPM
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 100)

	// verify NPM got the sgpolicy
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindSgpolicy("default", "test-sgpolicy")
		return (nerr == nil), nil
	}, "NetworkSecurityPolicy not found on NPM", "10ms", it.pollTimeout())

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not connected to NPM", "10ms", it.pollTimeout())
	}

	// verify agents have the sgpolicy too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			_, gerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if gerr != nil {
				return false, fmt.Errorf("Error finding sgpolicy for %+v", sgp.ObjectMeta)
			}
			return true, nil
		}, fmt.Sprintf("Sg policy not found in agent. %v", sgp.GetKey()), "10ms", it.pollTimeout())
	}

	// delete the policy
	time.Sleep(time.Millisecond * 100)
	err = it.DeleteSgpolicy("default", "default", "test-sgpolicy")
	c.Assert(err, IsNil)

	// verify policy is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name:      "default",
					Tenant:    "default",
					Namespace: "test-sgpolicy",
				},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			return (nerr != nil), nil
		}, "NetworkSecurityPolicy still found on agent", "100ms", "60s")
	}
}

func (it *integTestSuite) TestNpmRestartWithWorkload(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")
	const numWorkloadPerHost = 10
	// if not present create the default tenant
	it.CreateTenant("default")

	// create a host for each agent if it doesnt exist
	for idx := range it.agents {
		macAddr := fmt.Sprintf("0002.0000.%02x00", idx)
		it.CreateHost(fmt.Sprintf("testHost-%d", idx), macAddr)
	}

	// create 100 workloads on each host
	for i := range it.agents {
		for j := 0; j < numWorkloadPerHost; j++ {
			macAddr := fmt.Sprintf("0001.0203.%02x%02x", i, j)
			err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, j), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+j), 1)
			AssertOk(c, err, "Error creating workload")
		}
	}

	// wait for endpoints to be sent to agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * numWorkloadPerHost), nil
		}, "Endpoint count incorrect in agent", "100ms", it.pollTimeout())
	}

	log.Infof("==================== Restarting NPM =========================")

	// stop NPM
	err := it.npmCtrler.Stop()
	AssertOk(c, err, "Error stopping NPM")

	// verify agents are all disconnected
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return !cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}

	// Set feature flags to initialized
	featureflags.SetInitialized()

	// restart the NPM
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 100)

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}
	time.Sleep(time.Millisecond * 100)

	// verify agents have all endpoints
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * numWorkloadPerHost), len(endpoints)
		}, "Endpoint count incorrect in agent", "100ms", it.pollTimeout())
	}

	// create one more workload on each host
	for i := range it.agents {
		macAddr := fmt.Sprintf("0001.0203.%02x%02x", i, numWorkloadPerHost)
		err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, numWorkloadPerHost), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+numWorkloadPerHost), 1)
		AssertOk(c, err, "Error creating n+1 workload")
	}

	// verify agent has old and new endpoints
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * (numWorkloadPerHost + 1)), endpoints
		}, "Endpoint count incorrect in agent", "100ms", it.pollTimeout())
	}

	// delete workloads
	for i := range it.agents {
		for j := 0; j <= numWorkloadPerHost; j++ {
			err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, j))
			AssertOk(c, err, "Error deleting workload")
		}
	}

	// verify endpoints are gone
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 0, endpoints
		}, "Not all endpoints deleted from agent", "100ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "Network-Vlan-1")
	c.Assert(err, IsNil)
}

func (it *integTestSuite) TestAgentRestart(c *C) {
	const numWorkloadPerHost = 10
	// if not present create the default tenant
	it.CreateTenant("default")

	// create a host for each agent if it doesnt exist
	for idx := range it.agents {
		macAddr := fmt.Sprintf("0002.0000.%02x00", idx)
		it.CreateHost(fmt.Sprintf("testHost-%d", idx), macAddr)
	}

	// create 100 workloads on each host
	for i := range it.agents {
		for j := 0; j < numWorkloadPerHost; j++ {
			macAddr := fmt.Sprintf("0001.0203.%02x%02x", i, j)
			err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, j), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+j), 1)
			AssertOk(c, err, "Error creating workload")
		}
	}

	// wait for endpoints to be sent to agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * numWorkloadPerHost), nil
		}, "Endpoint count incorrect in agent", "100ms", it.pollTimeout())
	}

	const numApps = 10
	// app
	appNames := []string{}
	for i := 0; i < numApps; i++ {
		appName := fmt.Sprintf("ftpApp-%d", i)
		appNames = append(appNames, appName)
		ftpApp := security.App{
			TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Name:      appName,
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: security.AppSpec{
				ProtoPorts: []security.ProtoPort{
					{
						Protocol: "tcp",
						Ports:    "21",
					},
				},
				Timeout: "5m",
				ALG: &security.ALG{
					Type: "FTP",
					Ftp: &security.Ftp{
						AllowMismatchIPAddress: true,
					},
				},
			},
		}
		_, err := it.apisrvClient.SecurityV1().App().Create(context.Background(), &ftpApp)
		AssertOk(c, err, "error creating app")
	}

	// create a policy using this alg
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         "test-sgpolicy",
			GenerationID: "1",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"2.101.0.0/22"},
					ToIPAddresses:   []string{"2.101.0.0/24"},
					Apps:            appNames,
					Action:          "PERMIT",
				},
			},
		},
	}
	time.Sleep(time.Millisecond * 100)

	// create sg policy
	_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Create(context.Background(), &sgp)
	AssertOk(c, err, "error creating sg policy")

	// verify agent state has the policy
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			_, gerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if gerr != nil {
				return false, fmt.Errorf("Error finding sgpolicy for %+v", sgp.ObjectMeta)
			}
			return true, nil
		}, fmt.Sprintf("SGPolicy not found in agent. SGP: %v", sgp.GetKey()), "10ms", it.pollTimeout())
	}

	// stop all agents
	for _, ag := range it.agents {
		ag.dscAgent.Stop()
	}
	it.agents = []*Dpagent{}

	// restart all agents
	for i := 0; i < it.numAgents; i++ {
		agent, err := CreateAgent(it.logger, it.resolverSrv.GetListenURL(), fmt.Sprintf("testHost-%d", i))
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}
	time.Sleep(time.Millisecond * 100)

	// verify agents have all endpoints
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * numWorkloadPerHost), len(endpoints)
		}, "Endpoint count incorrect in agent after restart", "100ms", it.endpointPollTimeout())
	}

	// verify agent state has the policy
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			_, gerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)
			if gerr != nil {
				return false, fmt.Errorf("Error finding sgpolicy for %+v", sgp.ObjectMeta)
			}
			return true, nil
		}, fmt.Sprintf("SGPolicy not found in agent. SGP: %v", sgp.GetKey()), "10ms", it.pollTimeout())
	}

	// create one more workload on each host
	for i := range it.agents {
		macAddr := fmt.Sprintf("0001.0203.%02x%02x", i, numWorkloadPerHost)
		err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, numWorkloadPerHost), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+numWorkloadPerHost), 1)
		AssertOk(c, err, "Error creating n+1 workload")
	}

	// verify agent has old and new endpoints
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * (numWorkloadPerHost + 1)), endpoints
		}, "Endpoint count incorrect in agent after new workload", "100ms", it.pollTimeout())
	}

	// stop all agents
	for _, ag := range it.agents {
		ag.dscAgent.Stop()
	}
	it.agents = []*Dpagent{}

	// delete new workload from each host
	for i := 0; i < it.numAgents; i++ {
		err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, numWorkloadPerHost))
		AssertOk(c, err, "Error deleting workload")
	}

	// verify endpoints are gone from apiserver
	AssertEventually(c, func() (bool, interface{}) {
		listopt := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}}
		eplist, lerr := it.apisrvClient.WorkloadV1().Endpoint().List(context.Background(), &listopt)
		if lerr == nil && len(eplist) == (it.numAgents*numWorkloadPerHost) {
			return true, nil
		}
		return false, eplist
	}, "Endpoints still found in apiserver")

	// restart all agents
	for i := 0; i < it.numAgents; i++ {
		agent, err := CreateAgent(it.logger, it.resolverSrv.GetListenURL(), fmt.Sprintf("testHost-%d", i))
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.dscAgent.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}
	time.Sleep(time.Millisecond * 100)

	// verify agent new endpoint is removed from the agent
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == (it.numAgents * numWorkloadPerHost), endpoints
		}, "Deleted endpoint still found in agent", "100ms", it.endpointPollTimeout())
	}

	// delete workloads
	for i := range it.agents {
		for j := 0; j < numWorkloadPerHost; j++ {
			err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d-%d", i, j))
			AssertOk(c, err, "Error deleting workload")
		}
	}

	c.Skip("Skipping this test till we debug the intermittent failure")

	// verify endpoints are gone
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			return len(endpoints) == 0, endpoints
		}, "Not all endpoints deleted from agent", "100ms", it.pollTimeout())
	}

	// delete sg policy
	err = it.DeleteSgpolicy(sgp.Tenant, sgp.Namespace, sgp.Name)
	AssertOk(c, err, "error creating sg policy")

	// delete the network
	err = it.DeleteNetwork("default", "Network-Vlan-1")
	c.Assert(err, IsNil)
}
