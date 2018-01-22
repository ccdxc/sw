// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"fmt"
	"os"
	"testing"
	"time"

	"golang.org/x/net/context"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	"github.com/pensando/sw/venice/utils/tsdb"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/generated/network/gateway"
	_ "github.com/pensando/sw/api/hooks"

	"github.com/golang/mock/gomock"

	. "github.com/pensando/sw/venice/utils/testutils"
)

// integ test suite parameters
const (
	numIntegTestAgents  = 3
	integTestNpmURL     = "localhost:9495"
	integTestNpmRESTURL = "localhost:9496"
	integTestApisrvURL  = "localhost:8082"
	integTestAPIGWURL   = "localhost:9092"
	vchTestURL          = "localhost:19003"
	agentDatapathKind   = "mock"
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

// veniceIntegSuite is the state of integ test
type veniceIntegSuite struct {
	apiSrv       apiserver.Server
	apiGw        apigw.APIGateway
	certSrv      *certsrv.CertSrv
	ctrler       *npm.Netctrler
	agents       []*netagent.Agent
	datapaths    []*datapath.HalDatapath
	datapathKind datapath.Kind
	numAgents    int
	restClient   apiclient.Services
	apisrvClient apiclient.Services
	vcHub        vchSuite
}

// test args
var numAgents = flag.Int("agents", numIntegTestAgents, "Number of agents")
var datapathKind = flag.String("datapath", agentDatapathKind, "Specify the datapath type. mock | hal")

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
	it.datapathKind = datapath.Kind(*datapathKind)

	logger := log.GetNewLogger(log.GetDefaultConfig("venice_integ"))
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	// start certificate server
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	c.Assert(err, IsNil)
	it.certSrv = certSrv
	log.Infof("Created cert endpoint at %s", globals.CMDCertAPIPort)

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCKMBasedProvider(certSrv.GetListenURL(), svcName)
		if err != nil {
			return nil, err
		}
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)

	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer("resolver", "localhost:0", rpckit.WithTracerEnabled(true))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()

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
	ctrler, err := npm.NewNetctrler(integTestNpmURL, integTestNpmRESTURL, integTestApisrvURL, "", nil)
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	log.Infof("Creating %d/%d agents", it.numAgents, *numAgents)

	// create agents
	for i := 0; i < it.numAgents; i++ {
		// mock datapath
		dp, aerr := datapath.NewHalDatapath(it.datapathKind)
		c.Assert(aerr, IsNil)
		it.datapaths = append(it.datapaths, dp)

		// set tenant create expectations for mock clients
		if it.datapathKind.String() == "mock" {
			dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		}

		// agent
		agent, aerr := netagent.NewAgent(dp, fmt.Sprintf("/tmp/agent_%d.db", i), fmt.Sprintf("dummy-uuid-%d", i), integTestNpmURL, "", nil)
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

	it.vcHub.SetUp(c, it.numAgents)
}

func (it *veniceIntegSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *veniceIntegSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
	// Remove persisted agent db files
	for i := 0; i < it.numAgents; i++ {
		// mock datapath
		os.Remove(fmt.Sprintf("/tmp/agent_%d.db", i))
	}

}

func (it *veniceIntegSuite) TearDownSuite(c *C) {
	// stop the agents
	for _, ag := range it.agents {
		ag.Stop()
	}
	it.agents = []*netagent.Agent{}
	it.datapaths = []*datapath.HalDatapath{}

	// stop server and client
	it.ctrler.Stop()
	it.ctrler = nil
	it.apiSrv.Stop()
	it.apiGw.Stop()
	it.certSrv.Stop()
	it.vcHub.TearDown()
	it.apisrvClient.Close()
}

// basic test to make sure all components come up
func (it *veniceIntegSuite) TestVeniceIntegBasic(c *C) {
	// expect a network create/delete call in data path
	if it.datapathKind.String() == "mock" {
		for _, dp := range it.datapaths {
			dp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
			dp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
			dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
		}
	}

	var intervals []string
	// Slightly relax AssertEventually timeouts when agent is using the hal datapath.
	if it.datapathKind == "hal" {
		intervals = []string{"30ms", "30s"}
	} else {
		intervals = []string{"10ms", "10s"}
	}

	// create a network using REST api
	nw, err := it.createNetwork("default", "test", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "Error creating network")

	// verify network gets created in agent
	AssertEventually(c, func() (bool, []interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr == nil), nil
	}, "Network not found in agent", intervals...)

	// delete the network
	_, err = it.deleteNetwork("default", "test")
	AssertOk(c, err, "Error deleting network")

	// verify network is removed from agent
	AssertEventually(c, func() (bool, []interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr != nil), nil
	}, "Network still found in agent", intervals...)
}

// Verify basic vchub functionality
func (it *veniceIntegSuite) TestVeniceIntegVCH(c *C) {
	// setup vchub client
	rpcClient, err := rpckit.NewRPCClient("vchTestURL", vchTestURL)
	defer rpcClient.Close()
	c.Assert(err, IsNil)

	vcHubClient := orch.NewOrchApiClient(rpcClient.ClientConn)
	// verify number of smartnics
	filter := &orch.Filter{}
	AssertEventually(c, func() (bool, []interface{}) {
		nicList, err := vcHubClient.ListSmartNICs(context.Background(), filter)
		if err == nil && len(nicList.GetItems()) == it.numAgents {
			return true, nil
		}

		return false, nil
	}, "Unable to find expected snics")

	// add a nwif and verify it is seen by client.
	addReq := &simapi.NwIFSetReq{
		WLName:    "Venice-TestVM",
		IPAddr:    "22.2.2.3",
		Vlan:      "301",
		PortGroup: "userNet101",
	}

	snicMac := it.vcHub.snics[0]
	addResp, err := it.vcHub.vcSim.CreateNwIF(snicMac, addReq)
	c.Assert(err, IsNil)

	AssertEventually(c, func() (bool, []interface{}) {
		nwifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false, nil
		}

		for _, nwif := range nwifList.GetItems() {
			s := nwif.GetStatus()
			if s.MacAddress != addResp.MacAddr || s.Network != "userNet101" || s.SmartNIC_ID != snicMac || s.WlName != "Venice-TestVM" {
				continue
			}

			return true, nil
		}

		return false, nil
	}, "Unable to find expected nwif")

	// delete and verify
	it.vcHub.vcSim.DeleteNwIF(snicMac, addResp.UUID)
	AssertEventually(c, func() (bool, []interface{}) {
		nwifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false, nil
		}

		for _, nwif := range nwifList.GetItems() {
			s := nwif.GetStatus()
			if s.MacAddress == addResp.MacAddr {
				return false, nil
			}
		}

		return true, nil
	}, "Deleted nwif still exists")

}
