// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"fmt"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/nic/agent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/generated/network/gateway"
	_ "github.com/pensando/sw/api/hooks"

	. "github.com/pensando/sw/venice/utils/testutils"
)

// integ test suite parameters
const (
	numIntegTestAgents = 3
	integTestNpmURL    = "localhost:9495"
	integTestApisrvURL = "localhost:8082"
	integTestAPIGWURL  = "localhost:9092"
)

// veniceIntegSuite is the state of integ test
type veniceIntegSuite struct {
	apiSrv       apiserver.Server
	apiGw        apigw.APIGateway
	ctrler       *npm.Netctrler
	agents       []*agent.Agent
	datapaths    []*datapath.MockHalDatapath
	numAgents    int
	restClient   apiclient.Services
	apisrvClient apiclient.Services
}

// test args
var numAgents = flag.Int("agents", numIntegTestAgents, "Number of agents")

// Hook up gocheck into the "go test" runner.
func TestVeniceInteg(t *testing.T) {
	// integ test suite
	var sts = &veniceIntegSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

func (it *veniceIntegSuite) SetUpSuite(c *C) {
	// test parameters
	it.numAgents = *numAgents

	logger := log.GetNewLogger(log.GetDefaultConfig("venice_integ"))

	// api server config
	sch := runtime.NewScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: integTestApisrvURL,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}

	// create api server
	it.apiSrv = apisrvpkg.MustGetAPIServer()
	go it.apiSrv.Run(apisrvConfig)
	time.Sleep(time.Millisecond * 100)

	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer("resolver", "localhost:0", rpckit.WithTracerEnabled(true))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)

	// populate the mock resolver with apiserver instance.
	si := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-apiserver-test",
		},
		Service: "pen-apiserver",
		Node:    "localhost",
		URL:     "localhost:8082",
	}
	m.AddServiceInstance(&si)

	// api gw config
	apigwConfig := apigw.Config{
		HTTPAddr:  integTestAPIGWURL,
		Logger:    logger,
		Resolvers: []string{resolverServer.GetListenURL()},
	}

	// create apigw
	it.apiGw = apigwpkg.MustGetAPIGateway()
	go it.apiGw.Run(apigwConfig)

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmURL, integTestApisrvURL, "", "")
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numAgents)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		// mock datapath
		dp, aerr := datapath.NewMockHalDatapath()
		c.Assert(aerr, IsNil)
		it.datapaths = append(it.datapaths, dp)

		// agent
		agent, aerr := agent.NewAgent(dp, fmt.Sprintf("/tmp/agent_%d.db", i), fmt.Sprintf("dummy-uuid-%d", i), integTestNpmURL, "")
		c.Assert(aerr, IsNil)
		it.agents = append(it.agents, agent)
	}

	// REST Client
	restcl, err := apiclient.NewRestAPIClient(integTestAPIGWURL)
	if err != nil {
		c.Fatalf("cannot create REST client. Err: %v", err)
	}
	it.restClient = restcl

	// create api server client
	l := log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest"))
	apicl, err := apiclient.NewGrpcAPIClient(integTestApisrvURL, l)
	if err != nil {
		c.Fatalf("cannot create grpc client")
	}
	it.apisrvClient = apicl
	time.Sleep(time.Millisecond * 100)
}

func (it *veniceIntegSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *veniceIntegSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
}

func (it *veniceIntegSuite) TearDownSuite(c *C) {
	// stop the agents
	for _, ag := range it.agents {
		ag.Stop()
	}
	it.agents = []*agent.Agent{}
	it.datapaths = []*datapath.MockHalDatapath{}

	// stop server and client
	it.ctrler.RPCServer.Stop()
	it.ctrler = nil
	it.apiSrv.Stop()
	it.apiGw.Stop()
}

// basic test to make sure all components come up
func (it *veniceIntegSuite) TestVeniceIntegBasic(c *C) {
	// expect a network create/delete call in data path
	for _, dp := range it.datapaths {
		dp.Netclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Netclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	// create a network using REST api
	nw, err := it.createNetwork("default", "test", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "Error creating network")

	// verify network gets created in agent
	AssertEventually(c, func() bool {
		_, cerr := it.agents[0].Netagent.FindNetwork(nw.ObjectMeta)
		return (cerr == nil)
	}, "Network not found in agent")

	// delete the network
	_, err = it.deleteNetwork("default", "test")
	AssertOk(c, err, "Error deleting network")

	// verify network is removed from agent
	AssertEventually(c, func() bool {
		_, cerr := it.agents[0].Netagent.FindNetwork(nw.ObjectMeta)
		return (cerr != nil)
	}, "Network still found in agent")
}
