// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"flag"
	"fmt"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"

	"golang.org/x/net/context"
	"gopkg.in/check.v1"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/delphi/gosdk"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/ctrler/tsm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	rmock "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// integ test suite parameters
const (
	numIntegTestAgents = 3
	integTestNpmRPCURL = "localhost:9595"
	//integTestTpmRPCURL = "localhost:9093"
	//integTestTsmRPCURL = "localhost:9500"
	integTestRESTURL   = "localhost:9596"
	agentDatapathKind  = "mock"
	integTestApisrvURL = "localhost:8082"
	maxConnRetry       = 5
)

// integTestSuite is the state of integ test
type integTestSuite struct {
	apiSrv         apiserver.Server
	apiSrvAddr     string
	npmCtrler      *npm.Netctrler
	tsmCtrler      *tsm.TsCtrler
	tpmCtrler      *tpm.PolicyManager
	agents         []*Dpagent
	numAgents      int
	logger         log.Logger
	resolverSrv    *rpckit.RPCServer
	resolverClient resolver.Interface
	apisrvClient   apiclient.Services
	hub            gosdk.Hub
	fakehal        *fakehal.Hal
}

// test args
var numHosts = flag.Int("hosts", numIntegTestAgents, "Number of agents")
var datapathKind = flag.String("datapath", agentDatapathKind, "Specify the datapath type. mock | hal")

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("npm-integ-test"))
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("npm_integ_test", logger))
)

// Hook up gocheck into the "go test" runner.
func TestNpmInteg(t *testing.T) {
	// integ test suite
	var sts = &integTestSuite{}

	// set timeout values
	SetDefaultIntervals(time.Millisecond*500, time.Second*60)

	var _ = Suite(sts)
	TestingT(t)
}

func (it *integTestSuite) SetUpSuite(c *C) {
	// start hub
	it.hub = gosdk.NewFakeHub()
	it.hub.Start()

	// start fake hal
	var halLis netutils.TestListenAddr
	halLis.GetAvailablePort()
	it.fakehal = fakehal.NewFakeHalServer(halLis.ListenURL.String())
	if err := os.Setenv("HAL_GRPC_PORT", strings.Split(halLis.ListenURL.String(), ":")[1]); err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	// test parameters
	it.numAgents = 1

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	err := testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(context.Background(), &tsdb.Opts{ClientName: "NpmIntegTestSuite", ResolverClient: &rmock.ResolverClient{}})
	defer cancel()

	it.logger = logger

	// Create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(false))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	it.resolverSrv = resolverServer

	// populate the mock resolver with apiserver instance.
	npmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-npm-test",
		},
		Service: globals.Npm,
		Node:    "localhost",
		URL:     integTestNpmRPCURL,
	}
	m.AddServiceInstance(&npmSi)

	//tpmSi := types.ServiceInstance{
	//	TypeMeta: api.TypeMeta{
	//		Kind: "ServiceInstance",
	//	},
	//	ObjectMeta: api.ObjectMeta{
	//		Name: "pen-tpm-test",
	//	},
	//	Service: globals.Tpm,
	//	Node:    "localhost",
	//	URL:     integTestTpmRPCURL,
	//}
	//m.AddServiceInstance(&tpmSi)
	//
	//tsmSi := types.ServiceInstance{
	//	TypeMeta: api.TypeMeta{
	//		Kind: "ServiceInstance",
	//	},
	//	ObjectMeta: api.ObjectMeta{
	//		Name: "pen-tsm-test",
	//	},
	//	Service: globals.Tsm,
	//	Node:    "localhost",
	//	URL:     integTestTsmRPCURL,
	//}
	//m.AddServiceInstance(&tsmSi)

	// start API server
	it.apiSrv, it.apiSrvAddr, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test", logger.WithContext("submodule", "pen-apiserver"))
	c.Assert(err, check.IsNil)

	// populate the mock resolver with apiserver instance.
	apiSrvSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-apiserver-test",
		},
		Service: globals.APIServer,
		Node:    "localhost",
		URL:     integTestApisrvURL,
	}
	m.AddServiceInstance(&apiSrvSi)

	rc := resolver.New(&resolver.Config{Name: globals.Npm, Servers: []string{resolverServer.GetListenURL()}})

	// Set feature flags to initialized
	featureflags.SetInitialized()

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, rc, logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, IsNil)
	it.npmCtrler = ctrler
	it.resolverClient = rc

	//pm, err := tpm.NewPolicyManager(integTestTpmRPCURL, rc, "localhost:")
	//c.Assert(err, check.IsNil)
	//it.tpmCtrler = pm
	//
	//tsCtrler, err := tsm.NewTsCtrler(integTestTsmRPCURL, "localhost:", globals.APIServer, rc)
	//c.Assert(err, check.IsNil)
	//it.tsmCtrler = tsCtrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numHosts)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		agent, err := CreateAgent(it.logger, resolverServer.GetListenURL(), fmt.Sprintf("0001.%02x00.0000", i))
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

	log.Infof("Total number of agents: %v", len(it.agents))

	// wait a little for things to settle down
	time.Sleep(time.Millisecond * 100)

	// create api server client
	for i := 0; i < maxConnRetry; i++ {
		it.apisrvClient, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, logger, rpckit.WithBalancer(balancer.New(rc)))
		if err == nil {
			break
		}
	}
	if err != nil {
		c.Fatalf("cannot create grpc client. Err: %v", err)
	}

	// if not present create the default tenant
	err = it.CreateTenant("default")
	AssertOk(c, err, "Error creating default tenant")

	// create a host for each agent
	for i := 0; i < it.numAgents; i++ {
		err = it.CreateHost(fmt.Sprintf("testHost-%d", i), fmt.Sprintf("0001.%02x00.0000", i))
		AssertOk(c, err, "Error creating host")
	}

	// verify agents are all connected
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			return ag.dscAgent.InfraAPI.GetConfig().IsConnectedToVenice, nil
		}, "agents are not connected to NPM", "1s", it.pollTimeout())
	}
	time.Sleep(time.Second * 2)

	// Ensure we push an insertion enforced profile

	dscProfile1 := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "insertion.enforced1",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "INSERTION",
			FlowPolicyMode: "ENFORCED",
		},
	}

	_, err = it.apisrvClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile1)
	if err != nil {
		c.Fatalf("Failed to get APIServer handler. Err: %v", err)
	}

	meta := &api.ObjectMeta{
		Name:   it.agents[0].dscAgent.InfraAPI.GetConfig().DSCName,
		Tenant: "default",
	}

	dsc, err := it.apisrvClient.ClusterV1().DistributedServiceCard().Get(ctx, meta)
	if err != nil {
		c.Fatalf("Failed to get dsc. Err: %v", err)
	}

	dsc.Spec.DSCProfile = "insertion.enforced1"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dsc)
	if err != nil {
		c.Fatalf("Error DistributedServicesCard update failed. Err: %v", err)
	}
	time.Sleep(time.Second * 5)
}

func (it *integTestSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
	os.Remove(globals.NetAgentDBPath)
	it.logger = logger.WithContext("t_name", c.TestName())
}

func (it *integTestSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
}

func (it *integTestSuite) TearDownSuite(c *C) {
	// stop delphi hub
	it.hub.Stop()

	// stop the agents
	for _, ag := range it.agents {
		ag.dscAgent.Stop()
	}
	it.agents = []*Dpagent{}

	// stop server and client
	it.npmCtrler.Stop()
	it.npmCtrler = nil
	it.resolverSrv.Stop()
	it.resolverSrv = nil
	it.resolverClient.Stop()
	it.resolverClient = nil
	testutils.CleanupIntegTLSProvider()
	tsdb.Cleanup()
	it.fakehal.Stop()
	time.Sleep(time.Millisecond * 100) // allow goroutines to cleanup and terminate gracefully

	log.Infof("Stopped all servers and clients")
}

// basic connectivity tests between NPM and agent
func (it *integTestSuite) TestNpmAgentBasic(c *C) {
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
		}, "Network not found on agent", it.pollTimeout(), "10s")
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

func (it *integTestSuite) DeleteAllEndpoints(c *C) error {
	log.Infof("----- Delete ALL Endpoints in all agents")

	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			epMeta := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			}
			endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
			for _, ep := range endpoints {
				// make the call
				it.DeleteEndpoint("default", "default", ep.Name)
			}
			waitCh <- nil
		}(ag)
	}

	// wait for all endpoint deletes to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete failed")
	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
			}, "10ms", it.pollTimeout()) {
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
	return nil
}

// test endpoint create workflow e2e
func (it *integTestSuite) TestNpmEndpointCreateDelete(c *C) {
	// create a network in controller
	// if not present create the default tenant
	it.CreateTenant("default")
	err := it.DeleteAllEndpoints(c)
	c.Assert(err, IsNil)
	err = it.CreateNetwork("default", "default", "testNetwork", "10.1.0.0/22", "10.1.1.254")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// wait till agent has the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {

			nt := netproto.Network{
				TypeMeta:   api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "testNetwork"},
			}
			_, nerr := ag.dscAgent.PipelineAPI.HandleNetwork(agentTypes.Get, nt)
			return (nerr == nil), nil
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
			cerr := it.CreateEndpoint("default", "default", "testNetwork", epname, hostName, "0101.0101.0101", hostName, "20.1.1.1", map[string]string{"env": "production", "app": "procurement"}, 2)
			if cerr != nil {
				waitCh <- fmt.Errorf("endpoint create failed: %v", cerr)
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
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				log.Infof("Endpoint count expected [%v] found [%v]", it.numAgents, len(endpoints))
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			for i := range it.agents {
				epname := fmt.Sprintf("testEndpoint-%d", i)
				epmeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      epname,
					},
				}
				_, perr := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
				if perr != nil {
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

			// make the call
			cerr := it.DeleteEndpoint("default", "default", epname)
			if cerr != nil {
				waitCh <- fmt.Errorf("Endpoint delete failed: %v", cerr)
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
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
			}, "10ms", it.pollTimeout()) {
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
	err = it.DeleteNetwork("default", "testNetwork")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "testNetwork")
		return (nerr != nil), nil
	}, "Network still found in statemgr")
}
