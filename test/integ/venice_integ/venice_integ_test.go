// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"testing"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/agent"
	"github.com/pensando/sw/agent/netagent/datapath"
	"github.com/pensando/sw/apigw"
	apigwpkg "github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/ctrler/npm"
	"github.com/pensando/sw/utils/kvstore/store"
	plog "github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"

	. "gopkg.in/check.v1"
)

// integ test suite parameters
const (
	numIntegTestAgents = 3
	integTestNpmURL    = "localhost:9595"
	integTestApisrvURL = "localhost:9082"
	integTestAPIGWURL  = "localhost:9092"
)

// veniceIntegSuite is the state of integ test
type veniceIntegSuite struct {
	apiSrv    apiserver.Server
	apiGw     apigw.APIGateway
	ctrler    *npm.Netctrler
	agents    []*agent.Agent
	datapaths []*datapath.MockHalDatapath
	numAgents int
}

// test args
var numAgents = flag.Int("agents", numIntegTestAgents, "Number of agents")

// integ test suite
var sts = &veniceIntegSuite{}

var _ = Suite(sts)

// Hook up gocheck into the "go test" runner.
func TestInteg(t *testing.T) {
	TestingT(t)
}

func (it *veniceIntegSuite) SetUpSuite(c *C) {
	// test parameters
	it.numAgents = *numAgents

	logger := plog.GetNewLogger(plog.GetDefaultConfig("venice_integ"))

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
	time.Sleep(time.Millisecond * 10)

	// api gw config
	apigwConfig := apigw.Config{
		HTTPAddr: integTestAPIGWURL,
		Logger:   logger,
	}

	// create apigw
	it.apiGw = apigwpkg.MustGetAPIGateway()
	go it.apiGw.Run(apigwConfig)

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmURL)
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numAgents)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		// mock datapath
		dp, err := datapath.NewMockHalDatapath()
		c.Assert(err, IsNil)
		it.datapaths = append(it.datapaths, dp)

		// agent
		agent, err := agent.NewAgent(dp, integTestNpmURL)
		c.Assert(err, IsNil)
		it.agents = append(it.agents, agent)
	}

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

	// stop server and client
	it.ctrler.RPCServer.Stop()
}

// basic test to make sure all components come up
func (it *veniceIntegSuite) TestVeniceIntegBasic(c *C) {
	// if we reached here, everything must be up
	time.Sleep(time.Millisecond * 10)
}
