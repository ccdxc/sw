// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/pkg/errors"
	"golang.org/x/net/context"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/troubleshooting"
	tshal "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/ctrler/tsm"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// integ test suite parameters
const (
	numIntegTestAgents  = 3
	integTestNpmURL     = "localhost:9495"
	integTestNpmRESTURL = "localhost:9496"
	integTestApisrvURL  = "localhost:8082"
	integTestAPIGWURL   = "localhost:9092"
	integTestTPMURL     = "localhost:9093"
	vchTestURL          = "localhost:19003"
	// TS Controller
	integTestTsmURL     = "localhost:9500"
	integTestTsmRestURL = "localhost:9501"
	agentDatapathKind   = "mock"
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(
		&evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "venice_integ_test"},
		evtsapi.GetEventTypes(), "", "/tmp")
)

// veniceIntegSuite is the state of integ test
type veniceIntegSuite struct {
	apiSrv         apiserver.Server
	apiSrvAddr     string
	apiGw          apigw.APIGateway
	certSrv        *certsrv.CertSrv
	ctrler         *npm.Netctrler
	tpm            *tpm.PolicyManager
	tsCtrler       *tsm.TsCtrler
	agents         []*netagent.Agent
	tsAgents       []*troubleshooting.Agent
	datapaths      []*datapath.Datapath
	datapathKind   datapath.Kind
	numAgents      int
	restClient     apiclient.Services
	apisrvClient   apiclient.Services
	vcHub          vchSuite
	resolverSrv    *rpckit.RPCServer
	resolverClient resolver.Interface
	userCred       *auth.PasswordCredential
	tmpFiles       []string
	tlsProviders   []*tlsproviders.CMDBasedProvider
	eps            *evtsproxy.EventsProxy
	epsDir         string
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

	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	// start certificate server
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	c.Assert(err, IsNil)
	it.certSrv = certSrv
	log.Infof("Created cert endpoint at %s", globals.CMDCertAPIPort)

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCMDBasedProvider(certSrv.GetListenURL(), svcName)
		if err != nil {
			return nil, err
		}
		it.tlsProviders = append(it.tlsProviders, p)
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)

	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(true))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	it.resolverSrv = resolverServer

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

	npmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-npm-test",
		},
		Service: globals.Npm,
		Node:    "localhost",
		URL:     integTestNpmURL,
	}
	m.AddServiceInstance(&npmSi)

	tsmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-tsm-test",
		},
		Service: globals.Tsm,
		Node:    "localhost",
		URL:     integTestTsmURL,
	}
	m.AddServiceInstance(&tsmSi)

	rc := resolver.New(&resolver.Config{Name: "venice_integ_rslvr", Servers: []string{resolverServer.GetListenURL()}})
	it.resolverClient = rc

	tmpDir, err := ioutil.TempDir("", "evtsprxy_venice_integ")
	c.Assert(err, IsNil)
	l := log.GetNewLogger(log.GetDefaultConfig("evts-prxy"))

	it.epsDir = tmpDir
	eps, err := evtsproxy.NewEventsProxy("venice_integ_evtsprxy", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "", rc,
		5*time.Second, time.Second, it.epsDir, []evtsproxy.WriterType{}, l)
	c.Assert(err, IsNil)
	it.eps = eps

	// Unique name per test for memkv
	tmpfile, err := ioutil.TempFile("", "memkv_venice_integ")
	c.Assert(err, IsNil)
	n := tmpfile.Name()
	tmpfile.Close()

	logConf := log.GetDefaultConfig("apisrv")
	logConf.Filter = log.AllowAllFilter
	l = log.GetNewLogger(logConf)
	// start API server
	it.apiSrv, it.apiSrvAddr, err = testutils.StartAPIServer(integTestApisrvURL, &store.Config{
		Type:    store.KVStoreTypeMemkv,
		Servers: []string{n},
		Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme()),
	}, l)
	c.Assert(err, IsNil)

	l = log.GetNewLogger(log.GetDefaultConfig("api-gw"))
	// start API gateway
	it.apiGw, _, err = testutils.StartAPIGateway(integTestAPIGWURL, map[string]string{globals.APIServer: it.apiSrvAddr}, []string{"search", "events"}, []string{resolverServer.GetListenURL()}, l)
	c.Assert(err, IsNil)

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmURL, integTestNpmRESTURL, globals.APIServer, "", rc)
	c.Assert(err, IsNil)
	it.ctrler = ctrler

	// create a trouble shooting controller
	tsCtrler, err := tsm.NewTsCtrler(integTestTsmURL, integTestTsmRestURL, globals.APIServer, rc)
	c.Assert(err, IsNil)
	it.tsCtrler = tsCtrler

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

		tmpfile, err2 := ioutil.TempFile("", "nicagent_db")
		c.Assert(err2, IsNil)
		n = tmpfile.Name()
		tmpfile.Close()
		it.tmpFiles = append(it.tmpFiles, n)

		// Create netagent
		agent, aerr := netagent.NewAgent(dp, n, fmt.Sprintf("dummy-uuid-%d", i), globals.Npm, rc, state.AgentMode_MANAGED)
		c.Assert(aerr, IsNil)

		tsdp, aerr := tshal.NewHalDatapath("mock")
		c.Assert(aerr, IsNil)
		//it.datapaths = append(it.datapaths, dp)

		tmpfile, err2 = ioutil.TempFile("", "tsagent_db")
		c.Assert(err2, IsNil)
		n = tmpfile.Name()
		tmpfile.Close()
		it.tmpFiles = append(it.tmpFiles, n)

		log.Infof("creating troubleshooting subagent")
		tsa, aerr := troubleshooting.NewTsAgent(tsdp, n, fmt.Sprintf("dummy-uuid-%d", i), globals.Tsm, rc, state.AgentMode_MANAGED, agent.NetworkAgent)
		c.Assert(aerr, IsNil)
		if tsa == nil {
			c.Fatalf("cannot create troubleshooting agent. Err: %v", err)
		}
		log.Infof("created troubleshooting subagent")

		// create new RestServer instance. Not started yet.
		restServer, err := restapi.NewRestServer(agent.NetworkAgent, tsa.TroubleShootingAgent, "")
		c.Assert(err, IsNil)
		if restServer == nil {
			c.Fatalf("cannot create REST server . Err: %v", err)
		}
		agent.RestServer = restServer
		it.agents = append(it.agents, agent)
		it.tsAgents = append(it.tsAgents, tsa)

	}

	// REST Client
	restcl, err := apiclient.NewRestAPIClient(integTestAPIGWURL)
	if err != nil {
		c.Fatalf("cannot create REST client. Err: %v", err)
	}
	it.restClient = restcl

	// create api server client
	l = log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest"))
	apicl, err := apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, l, rpckit.WithBalancer(balancer.New(rc)))
	if err != nil {
		c.Fatalf("cannot create grpc client")
	}
	it.apisrvClient = apicl
	time.Sleep(time.Millisecond * 100)

	// create tpm
	//rs := resolver.New(&resolver.Config{Name: globals.Tpm, Servers: []string{resolverServer.GetListenURL()}})
	pm, err := tpm.NewPolicyManager(integTestTPMURL, rc)
	c.Assert(err, IsNil)
	it.tpm = pm

	it.userCred = &auth.PasswordCredential{
		Username: testutils.TestLocalUser,
		Password: testutils.TestLocalPassword,
		Tenant:   testutils.TestTenant,
	}
	l = log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest-setupAuth"))
	err = testutils.SetupAuth(integTestApisrvURL, true, false, it.userCred, l)
	c.Assert(err, IsNil)

	it.vcHub.SetUp(c, it.numAgents)
}

func (it *veniceIntegSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *veniceIntegSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
	// Remove persisted agent db files
	for _, i := range it.tmpFiles {
		os.Remove(i)
	}
}

func (it *veniceIntegSuite) TearDownSuite(c *C) {
	// stop the agents
	for _, ag := range it.agents {
		ag.Stop()
	}
	for _, ag := range it.tsAgents {
		ag.Stop()
	}
	for _, t := range it.tlsProviders {
		t.Close()
	}
	it.tlsProviders = nil
	if it.epsDir != "" {
		os.RemoveAll(it.epsDir)
	}
	it.epsDir = ""
	it.agents = []*netagent.Agent{}
	it.tsAgents = []*troubleshooting.Agent{}
	it.datapaths = []*datapath.Datapath{}

	// stop server and client
	log.Infof("Stop all Test Controllers")
	it.tpm.Stop()
	it.ctrler.Stop()
	it.ctrler = nil
	it.tsCtrler.Stop()
	it.tsCtrler = nil
	it.apiSrv.Stop()
	it.apiSrv = nil
	it.apiGw.Stop()
	it.apiGw = nil
	it.certSrv.Stop()
	it.vcHub.TearDown()
	it.resolverClient.Stop()
	it.resolverSrv.Stop()
	it.apisrvClient.Close()
	it.eps.RPCServer.Stop()

	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		return nil, errors.New("Suite is being shutdown")
	}
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)
	log.Infof("============================= TearDownSuite completed ==========================")
}

// basic test to make sure all components come up
func (it *veniceIntegSuite) TestVeniceIntegBasic(c *C) {
	var intervals []string
	// Slightly relax AssertEventually timeouts when agent is using the hal datapath.
	if it.datapathKind == "hal" {
		intervals = []string{"30ms", "30s"}
	} else {
		intervals = []string{"10ms", "10s"}
	}

	// create a network using REST api
	nw, err := it.createNetwork("default", "default", "test", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "Error creating network")

	// verify network gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr == nil), nil
	}, "Network not found in agent", intervals...)

	// delete the network
	_, err = it.deleteNetwork("default", "test")
	AssertOk(c, err, "Error deleting network")

	// verify network is removed from agent
	AssertEventually(c, func() (bool, interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr != nil), nil
	}, "Network still found in agent", intervals...)
}

// Verify basic vchub functionality
func (it *veniceIntegSuite) TestVeniceIntegVCH(c *C) {
	// setup vchub client
	rpcClient, err := rpckit.NewRPCClient("venice_integ_test", vchTestURL, rpckit.WithRemoteServerName(globals.VCHub))
	defer rpcClient.Close()
	c.Assert(err, IsNil)

	vcHubClient := orch.NewOrchApiClient(rpcClient.ClientConn)
	// verify number of smartnics
	filter := &orch.Filter{}
	AssertEventually(c, func() (bool, interface{}) {
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

	AssertEventually(c, func() (bool, interface{}) {
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
	AssertEventually(c, func() (bool, interface{}) {
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

// test tenant watch
func (it *veniceIntegSuite) TestTenantWatch(c *C) {
	c.Skip("## skip tenant test ")
	// create watch
	client := it.apisrvClient
	ctx, cancel := context.WithCancel(context.Background())
	kvWatch, err := client.ClusterV1().Tenant().Watch(ctx, &api.ListWatchOptions{})
	defer cancel()
	AssertOk(c, err, "failed to watch tenants")
	tenChan := kvWatch.EventChan()
	defer kvWatch.Stop()

	for j := 0; j < 2; j++ {
		log.Infof("########################## tpm tenant test:%d ###################", j)
		tenantName := fmt.Sprintf("vpc-%d", j)
		// create a tenant
		_, err := it.createTenant(tenantName)
		AssertOk(c, err, fmt.Sprintf("failed to create tenant %s", tenantName))
		defer it.deleteTenant(tenantName)

		AssertEventually(c, func() (bool, interface{}) {
			_, err := it.getTenant(tenantName)
			return err == nil, err
		}, fmt.Sprintf("failed to find tenant %s ", tenantName))

		AssertEventually(c, func() (bool, interface{}) {
			select {
			case _, ok := <-tenChan:
				if !ok {
					log.Infof("###### received tenant watch %v", ok)
				}
				return ok, nil
			default:
				return false, nil
			}
		}, fmt.Sprintf("failed to receive watch event for tenant %s ", tenantName))
	}

	for j := 0; j < 2; j++ {
		log.Infof("########################## tpm tenant test:%d ###################", j)
		tenantName := fmt.Sprintf("vpc-%d", j)
		// delete a tenant
		_, err := it.deleteTenant(tenantName)
		AssertOk(c, err, fmt.Sprintf("failed to delete tenant %s", tenantName))

		AssertEventually(c, func() (bool, interface{}) {
			ten, err := it.getTenant(tenantName)
			return err != nil, ten
		}, "tenant exists after delete")

		AssertEventually(c, func() (bool, interface{}) {
			select {
			case _, ok := <-tenChan:
				return ok, nil
			default:
				return false, nil
			}
		}, fmt.Sprintf("failed to receive watch event for tenant %s ", tenantName))
	}
}

// test tpm
func (it *veniceIntegSuite) TestTelemetryPolicyMgr(c *C) {
	tenantName := fmt.Sprintf("tenant-100")
	// create a tenant
	_, err := it.createTenant(tenantName)
	AssertOk(c, err, "Error creating tenant")

	defer it.deleteTenant(tenantName)

	AssertEventually(c, func() (bool, interface{}) {
		tn, err := it.getTenant(tenantName)
		return err == nil, tn
	}, fmt.Sprintf("failed to find tenant %s ", tenantName))

	AssertEventually(c, func() (bool, interface{}) {
		sp, err := it.getStatsPolicy(tenantName)
		if err == nil {
			Assert(c, reflect.DeepEqual(sp.GetSpec(), tpm.DefaultStatsSpec),
				fmt.Sprintf("stats spec didn't match: got %+v, expectd %+v",
					sp.GetSpec(), tpm.DefaultStatsSpec))
			return true, nil
		}
		return false, nil
	}, "failed to find stats policy")

	AssertEventually(c, func() (bool, interface{}) {
		fp, err := it.getFwlogPolicy(tenantName)
		if err == nil {
			Assert(c, reflect.DeepEqual(fp.GetSpec(), tpm.DefaultFwlogSpec),
				fmt.Sprintf("fwlog spec didn't match: got %+v, expectd %+v", fp.GetSpec(), tpm.DefaultFwlogSpec))
			return true, nil
		}
		return false, nil
	}, "failed to find fwlog policy")

	_, err = it.deleteTenant(tenantName)
	AssertOk(c, err, "Error deleting tenant")

	AssertEventually(c, func() (bool, interface{}) {
		_, err := it.getStatsPolicy(tenantName)
		return err != nil, nil

	}, "failed to get stats policy")

	AssertEventually(c, func() (bool, interface{}) {
		_, err := it.getFwlogPolicy(tenantName)
		return err != nil, nil

	}, "failed to delete fwlog policy")
}
