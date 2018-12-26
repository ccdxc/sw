// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"time"

	. "gopkg.in/check.v1"
	check "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// this test simulates API server restarting while NPM is up and runinng
func (it *integTestSuite) TestNpmApiServerRestart(c *C) {
	// create a network in controller
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
		nt, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
		AssertOk(c, nerr, "error finding network")
		Assert(c, (nt.Spec.IPv4Subnet == "10.1.1.0/24"), "Network params didnt match", nt)
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

	// verify NPM still has the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}

	// recreate the network in API server
	err = it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify NPM recreates the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
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
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "Network still found in NPM", "10ms", it.pollTimeout())

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}

	// recreate the network in API server
	err = it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify NPM recreates the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
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
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}
}

// this test simulates NPM restarting while API server and agents remain up and running
func (it *integTestSuite) TestNpmRestart(c *C) {
	// create a network in controller
	err := it.CreateNetwork("default", "default", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "error creating network")

	// verify agent receives the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
		nt, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
		AssertOk(c, nerr, "error finding network")
		Assert(c, (nt.Spec.IPv4Subnet == "10.1.1.0/24"), "Network params didnt match", nt)
	}

	// stop NPM
	err = it.ctrler.Stop()
	AssertOk(c, err, "Error stopping NPM")

	// verify agents are all disconnected
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return !ag.nagent.IsNpmClientConnected(), nil
		}, "agents are not disconnected from NPM", "10ms", it.pollTimeout())
	}

	// restart the NPM
	it.ctrler, err = npm.NewNetctrler(integTestRPCURL, integTestRESTURL, integTestApisrvURL, "", it.resolverClient)
	c.Assert(err, IsNil)
	time.Sleep(time.Millisecond * 100)

	// verify NPM still has the network
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found on NPM", "10ms", it.pollTimeout())

	// verify agents are all connected back
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return ag.nagent.IsNpmClientConnected(), nil
		}, "agents are not connected to NPM", "10ms", it.pollTimeout())
	}

	// verify agents have the network too
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr == nil), nil
		}, "Network not found on agent", "10ms", it.pollTimeout())
	}

	// delete the network
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)

	// verify network is removed from all agents
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"})
			return (nerr != nil), nil
		}, "Network still found on agent", "100ms", it.pollTimeout())
	}
}
