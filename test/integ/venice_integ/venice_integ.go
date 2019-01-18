// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"

	"golang.org/x/net/context"
	"gopkg.in/check.v1"

	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	pencluster "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	nmdproto "github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	tmrestapi "github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/tpa"
	"github.com/pensando/sw/nic/agent/troubleshooting"
	tshal "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	"github.com/pensando/sw/nic/delphi/gosdk"

	// This import is a workaround for delphi client crash
	_ "github.com/pensando/sw/nic/delphi/sdk/proto"

	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/collector"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver"
	"github.com/pensando/sw/venice/citadel/data"
	"github.com/pensando/sw/venice/citadel/http"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/citadel/watcher"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	cmdauth "github.com/pensando/sw/venice/cmd/grpc/server/auth"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/ctrler/rollout"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/ctrler/tsm"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	pcache "github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/nodewatcher"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	tutils "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// integ test suite parameters
const (
	// service URLs
	integTestNpmURL     = "localhost:9495"
	integTestNpmRESTURL = "localhost:9496"
	integTestApisrvURL  = "localhost:8082"
	integTestTPMURL     = "localhost:9093"
	vchTestURL          = "localhost:19003"
	integTestCitadelURL = "localhost:9094"
	integTestRolloutURL = "localhost:9095"

	smartNICServerURL = "localhost:9199"
	resolverURLs      = ":" + globals.CMDResolverPort
	cmdAuthServer     = "localhost:9198"

	// TS Controller
	integTestTsmURL     = "localhost:9500"
	integTestTsmRestURL = "localhost:9501"

	// default valules
	numIntegTestAgents = 3
	agentDatapathKind  = "mock"

	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

var (
	evtType = append(evtsapi.GetEventTypes(), pencluster.GetEventTypes()...)
	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "venice_integ_test"},
		EvtTypes:      evtType,
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

// SuiteConfig determines the test suite configuration
type SuiteConfig struct {
	APIGatewayPort       string // port on which API gateway listens
	APIGatewaySkipAuth   bool   // skip auth for api gw
	StartEventsAndSearch bool   // start search, events and elastic-search service
	ElasticSearchName    string // elastic search instance name
	NumHosts             int    // number of hosts to emulate (this determines number of smart-nics and number of agents)
	NumWorkloadsPerHost  int    // number of workloads on each host
	NumPolicies          int    // number of policies to create
	NumRulesPerPolicy    int    // number of rules in each policy
	NumSecurityGroups    int    // number of security groups
	DatapathKind         string // datapath kind (mock, hal, delphi)
	NetAgentRestPort     int    // REST api port for netagent
	NmdRestPort          int    // NMD REST port
	TmAgentRestPort      int    // TMAgent REST port
	TsAgentRestPort      int    // TSAgent REST port
}

// veniceIntegSuite is the state of integ test
type veniceIntegSuite struct {
	config              SuiteConfig // test suite config
	apiSrv              apiserver.Server
	apiSrvAddr          string
	disabledServices    []string
	apiGw               apigw.APIGateway
	apiGwAddr           string
	logger              log.Logger
	ctrler              *npm.Netctrler
	tpm                 *tpm.PolicyManager
	tsCtrler            *tsm.TsCtrler
	agents              []*netagent.Agent
	tsAgents            []*troubleshooting.Agent
	tmAgent             []*tmrestapi.RestServer
	nmds                []*nmd.Agent
	restClient          apiclient.Services
	apisrvClient        apiclient.Services
	vcHub               vchSuite
	resolverSrv         *rpckit.RPCServer
	resolverClient      resolver.Interface
	userCred            *auth.PasswordCredential
	tmpFiles            []string
	eps                 *evtsproxy.EventsProxy
	epsDir              string
	rpcServer           *rpckit.RPCServer
	smartNICServer      *smartnic.RPCServer
	rolloutCtrler       *rollout.Ctrler
	mockResolver        *mock.ResolverService
	esClient            elastic.ESClient       // elastic client to verify the results
	elasticsearchAddr   string                 // elastic address
	elasticsearchDir    string                 // name of the directory where Elastic credentials and logs are stored
	evtsMgr             *evtsmgr.EventsManager // events manager to write events to elastic
	evtsProxy           *evtsproxy.EventsProxy // events proxy to receive and distribute events
	proxyEventsStoreDir string                 // local events store directory
	evtsProxyURL        string
	signer              certs.CSRSigner     // function to sign CSRs for TLS
	trustRoots          []*x509.Certificate // trust roots to verify TLS certs
	fdr                 finder.Interface
	fdrAddr             string
	idr                 indexer.Interface
	dedupInterval       time.Duration // events dedup interval
	batchInterval       time.Duration // events batch interval
	pcache              pcache.Interface
	hub                 gosdk.Hub
}

// VeniceSuite is an interface implemented by venice integ test suite
type VeniceSuite interface {
	SetUpSuite(c *check.C)
	TearDownSuite(c *check.C)
}

// NewVeniceIntegSuite creates a new venice integ suite
// This is meant to be used by palazzo - venice simulator
func NewVeniceIntegSuite(cfg SuiteConfig) VeniceSuite {
	return &veniceIntegSuite{config: cfg}
}

func (it *veniceIntegSuite) APIClient() pencluster.ClusterV1Interface {
	return it.apisrvClient.ClusterV1()
}

func (it *veniceIntegSuite) launchCMDServer() {

	// create an RPC server for SmartNIC service
	rpcServer, err := rpckit.NewRPCServer("smartNIC", smartNICServerURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Fatalf("Error creating RPC-server: %v", err)
	}
	it.rpcServer = rpcServer
	cmdenv.CertMgr, err = certmgr.NewTestCertificateMgr("smartnic-test")
	if err != nil {
		log.Fatalf("Error creating CertMgr instance: %v", err)
	}
	cmdenv.UnauthRPCServer = rpcServer

	// create and register the RPC handler for SmartNIC service
	it.smartNICServer, err = smartnic.NewRPCServer(it,
		smartnic.HealthWatchInterval,
		smartnic.DeadInterval,
		globals.NmdRESTPort,
		cache.NewStatemgr())

	if err != nil {
		log.Fatalf("Error creating Smart NIC server: %v", err)
	}
	grpc.RegisterSmartNICRegistrationServer(rpcServer.GrpcServer, it.smartNICServer)
	rpcServer.Start()
	cmdenv.NICService = it.smartNICServer

	// create node watcher
	node := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "venice",
		},
	}
	nodewatcher.NewNodeWatcher(context.Background(), node, it.resolverClient, 10, it.logger)

	// start CMD auth server
	go cmdauth.RunAuthServer(cmdAuthServer, nil)
}

func (it *veniceIntegSuite) startNmd(c *check.C) {
	for i := 0; i < it.config.NumHosts; i++ {

		hostID := it.agents[i].NetworkAgent.NodeUUID
		restURL := "localhost:0"
		dbPath := fmt.Sprintf("/tmp/nmd-%d.db", i)
		hostName := fmt.Sprintf("host%d", i)

		// create a host object
		host := &pencluster.Host{
			ObjectMeta: api.ObjectMeta{
				Name: hostName,
			},
			Spec: pencluster.HostSpec{
				SmartNICs: []pencluster.SmartNICID{
					{
						MACAddress: hostID,
					},
				},
			},
		}
		_, err := it.apisrvClient.ClusterV1().Host().Create(context.Background(), host)
		if err != nil {
			log.Fatalf("Error creating Host object %v, err: %v", host, err)
		}

		// create a platform agent
		pa, err := platform.NewNaplesPlatformAgent()
		if err != nil {
			log.Fatalf("Error creating platform agent. Err: %v", err)
		}

		// create a upgrade client
		uc, err := upg.NewNaplesUpgradeClient(nil)
		if err != nil {
			log.Fatalf("Error creating Upgrade client . Err: %v", err)
		}

		// create the new NMD
		nmd, err := nmd.NewAgent(pa, uc, dbPath, hostID, hostID, smartNICServerURL,
			cmdAuthServer, restURL, "", "", "network", globals.NicRegIntvl*time.Second,
			globals.NicUpdIntvl*time.Second, it.resolverClient)
		if err != nil {
			log.Fatalf("Error creating NMD. Err: %v", err)
		}
		it.nmds = append(it.nmds, nmd)
	}

	// verify NIC is admitted with CMD
	for i := 0; i < it.config.NumHosts; i++ {
		hostID := fmt.Sprintf("44:44:44:44:%02x:%02x", i/256, i%256)
		tutils.AssertEventually(c, func() (bool, interface{}) {
			nm := it.nmds[i].GetNMD()

			// validate the mode is network
			cfg := nm.GetNaplesConfig()
			log.Infof("NaplesConfig: %v", cfg)
			if cfg.Spec.Mode != nmdproto.MgmtMode_NETWORK {
				log.Errorf("Failed to switch to network mode")
				return false, nil
			}

			// Fetch smartnic object
			nic, err := nm.GetSmartNIC()
			if nic == nil || err != nil {
				log.Errorf("NIC not found in nicDB, mac:%s", hostID)
				return false, nil
			}

			// Verify NIC is admitted
			if nic.Status.AdmissionPhase != pencluster.SmartNICStatus_ADMITTED.String() {
				log.Errorf("NIC is not admitted")
				return false, nil
			}

			// Verify Update NIC task is running
			if nm.GetUpdStatus() == false {
				log.Errorf("Update NIC is not in progress")
				return false, nil
			}

			// Verify REST server is not up
			if nm.GetRestServerStatus() == true {
				log.Errorf("REST server is still up")
				return false, nil
			}
			return true, nil
		}, "Failed to verify mode is in Network Mode", string("10ms"), string("60s"))
	}
}

func (it *veniceIntegSuite) startResolver() {
	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(true))
	if err != nil {
		log.Fatalf("Error creating resolver server. Err: %v", err)
	}
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	it.resolverSrv = resolverServer
	it.mockResolver = m

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

	tpmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-tpm-test",
		},
		Service: globals.Tpm,
		Node:    "localhost",
		URL:     integTestTPMURL,
	}
	m.AddServiceInstance(&tpmSi)

	citadelSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-citadel-test",
		},
		Service: globals.Citadel,
		Node:    "localhost",
		URL:     "localhost:" + globals.CitadelQueryRPCPort,
	}
	m.AddServiceInstance(&citadelSi)

	rolloutSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-rollout-test",
		},
		Service: globals.Rollout,
		Node:    "localhost",
		URL:     integTestRolloutURL,
	}
	m.AddServiceInstance(&rolloutSi)
}

func (it *veniceIntegSuite) updateResolver(serviceName, url string) {
	it.mockResolver.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

// startCitadel starts a single node citadel cluster
func (it *veniceIntegSuite) startCitadel() {
	// cluster config
	cfg := meta.DefaultClusterConfig()
	nodeUUID := "citadel-node0"

	httpURL := "127.0.0.1:" + globals.CitadelHTTPPort
	queryURL := ":" + globals.CitadelQueryRPCPort
	collectorURL := fmt.Sprintf(":%s", globals.CollectorRPCPort)
	it.updateResolver(globals.Collector, collectorURL)

	// create the data node
	dn, err := data.NewDataNode(cfg, nodeUUID, integTestCitadelURL, "/tmp/tstore/")
	if err != nil {
		log.Fatalf("Error creating data node. Err: %v", err)
	}

	br, err := broker.NewBroker(cfg, nodeUUID)
	if err != nil {
		log.Fatalf("Error creating broker. Err: %v", err)
	}

	// wait till cluster is ready
	for i := 0; i < 100; i++ {
		if cerr := br.ClusterCheck(); cerr == nil {
			break
		}
		time.Sleep(time.Second)
	}

	log.Infof("Datanode %+v and broker %+v are running", dn, br)

	// start the http server
	hsrv, err := httpserver.NewHTTPServer(httpURL, br, nil)
	if err != nil {
		log.Fatalf("Error creating HTTP server. Err: %v", err)
	}
	log.Infof("HTTP server is listening on %s", hsrv.GetAddr())

	// start collector, use citadel
	c := collector.NewCollector(br)

	// setup an rpc server
	srv, err := rpcserver.NewCollRPCSrv(collectorURL, c)
	if err != nil {
		log.Fatalf(err.Error())
	}

	log.Infof("%s is running {%+v}", globals.Citadel, srv)

	qsrv, err := query.NewQueryService(queryURL, br)

	log.Infof("query server is listening on %+v", qsrv)

	// watch for tenants
	watcher := watcher.NewWatcher(integTestApisrvURL, br, it.resolverClient)
	go watcher.WatchTenant(context.Background())

	if err := br.CreateDatabase(context.Background(), globals.DefaultTenant); err != nil {
		log.Fatalf(err.Error())
	}

	// wait forever
	waitCh := make(chan bool)
	<-waitCh
}

// startEventsAndSearch starts spyglass and elastic-search containers
func (it *veniceIntegSuite) startEventsAndSearch() {
	var err error

	it.dedupInterval = 10 * time.Second
	it.batchInterval = 100 * time.Millisecond

	it.signer, _, it.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		log.Fatalf("Error creating certs. Err: %v", err)
	}

	// remove old elastic container
	testutils.StopElasticsearch(it.config.ElasticSearchName, "")

	// start new elastic container
	it.elasticsearchAddr, it.elasticsearchDir, err = testutils.StartElasticsearch(it.config.ElasticSearchName, it.signer, it.trustRoots)
	if err != nil {
		log.Fatalf("Error Starting elastic search. Err: %v", err)
	}

	// add mock elastic service to mock resolver
	it.updateResolver(globals.ElasticSearch, it.elasticsearchAddr)

	// start elastic search client
	it.esClient, err = testutils.CreateElasticClient(it.elasticsearchAddr, nil, it.logger, it.signer, it.trustRoots)
	if err != nil {
		log.Fatalf("Error creating elastic client. Err: %v", err)
	}

	// start events manager
	evtsMgr, evtsMgrURL, err := testutils.StartEvtsMgr("localhost:0", it.resolverClient, it.logger, it.esClient)
	if err != nil {
		log.Fatalf("Error starting events mgr. Err: %v", err)
	}

	it.evtsMgr = evtsMgr
	it.updateResolver(globals.EvtsMgr, evtsMgrURL)

	// Create new policy cache for spyglass
	it.pcache = pcache.NewCache(it.logger)

	// start spyglass finder
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", it.resolverClient, it.pcache, it.logger, it.esClient)
	if err != nil {
		log.Fatalf("Error starting spyglass finder. Err: %v", err)
	}

	it.fdr = fdr.(finder.Interface)
	it.fdrAddr = fdrAddr

	// start spygalss indexer
	idr, _, err := testutils.StartSpyglass("indexer", it.apiSrvAddr, it.resolverClient, it.pcache, it.logger, it.esClient)
	if err != nil {
		log.Fatalf("Error starting spyglass indexer. Err: %v", err)
	}

	it.idr = idr.(indexer.Interface)
}

func (it *veniceIntegSuite) startAgent() {
	// create agents
	// Start DelphiHub
	it.hub = gosdk.NewFakeHub()
	it.hub.Start()
	for i := 0; i < it.config.NumHosts; i++ {
		// mock datapath
		datapathKind := datapath.Kind(it.config.DatapathKind)

		tmpfile, aerr := ioutil.TempFile("", "nicagent_db")
		if aerr != nil {
			log.Fatalf("Error creating tmp file. Err: %v", aerr)
		}
		n := tmpfile.Name()
		tmpfile.Close()
		it.tmpFiles = append(it.tmpFiles, n)

		// Create netagent
		rc := it.resolverClient
		agent, aerr := netagent.NewAgent(datapathKind.String(), n, globals.Npm, rc, state.AgentMode_MANAGED)
		if aerr != nil {
			log.Fatalf("Error creating netagent. Err: %v", aerr)
		}

		// TODO Remove this when nmd and delphi hub are integrated with venice_integ and npm_integ
		npmClient, err := ctrlerif.NewNpmClient(agent.NetworkAgent, globals.Npm, rc)
		if err != nil {
			log.Errorf("Error creating NPM client. Err: %v", err)
		}
		agent.NpmClient = npmClient

		tsdp, aerr := tshal.NewHalDatapath("mock")
		if aerr != nil {
			log.Fatalf("Error creating TS datapath. Err: %v", aerr)
		}

		tmpfile, aerr = ioutil.TempFile("", "tsagent_db")
		if aerr != nil {
			log.Fatalf("Error creating TS agent file. Err: %v", aerr)
		}
		n = tmpfile.Name()
		tmpfile.Close()
		it.tmpFiles = append(it.tmpFiles, n)

		log.Infof("creating troubleshooting subagent")
		tsa, aerr := troubleshooting.NewTsAgent(tsdp, fmt.Sprintf("dummy-uuid-%d", i), globals.Tsm, rc, state.AgentMode_MANAGED, agent.NetworkAgent)
		if aerr != nil {
			log.Fatalf("Error creating TS agent. Err: %v", aerr)
		}
		if tsa == nil {
			log.Fatalf("cannot create troubleshooting agent. Err: %v", aerr)
		}
		log.Infof("created troubleshooting subagent")
		tmpfile, aerr = ioutil.TempFile("", "tpagent_db")
		if aerr != nil {
			log.Fatalf("Error creating tpagent file. Err: %v", aerr)
		}
		n = tmpfile.Name()
		tmpfile.Close()
		it.tmpFiles = append(it.tmpFiles, n)

		log.Infof("creating telemetry policy agent")
		tpa, aerr := tpa.NewPolicyAgent(fmt.Sprintf("dummy-uuid-%d", i), globals.Tpm, rc, state.AgentMode_MANAGED, "mock", agent.NetworkAgent, "/tmp/tpa.sock")
		if aerr != nil {
			log.Fatalf("Error creating TPAgent. Err: %v", aerr)
		}
		if tpa == nil {
			log.Fatalf("cannot create telemetry policy agent. Err: %v", aerr)
		}
		log.Infof("created telemetry policy agent")

		// create new RestServer instance. Not started yet.
		restURL := fmt.Sprintf("localhost:%d", it.config.NetAgentRestPort+i)
		restServer, err := restapi.NewRestServer(agent.NetworkAgent, tsa.TroubleShootingAgent, tpa.TpState, restURL)
		if aerr != nil {
			log.Fatalf("Error creating agent REST server. Err: %v", aerr)
		}
		if restServer == nil {
			log.Fatalf("cannot create REST server . Err: %v", err)
		}
		agent.RestServer = restServer
		it.agents = append(it.agents, agent)
		it.tsAgents = append(it.tsAgents, tsa)

		// report node metrics
		node := &cluster.SmartNIC{
			TypeMeta: api.TypeMeta{
				Kind: "SmartNIC",
			},
			ObjectMeta: api.ObjectMeta{
				Name: agent.NetworkAgent.NodeUUID,
			},
		}

		if i == 0 { // start only 1 instance
			// Init the TSDB
			err = nodewatcher.NewNodeWatcher(context.Background(), node, it.resolverClient, 10, it.logger)
			if err != nil {
				log.Fatalf("Error creating NodeWatcher. Err: %v", err)
			}

			res, err := tmrestapi.NewRestServer(context.Background(), fmt.Sprintf("localhost:%d", it.config.TmAgentRestPort+i))
			if err != nil {
				log.Fatalf("Error creating tmagent rest server. Err: %v", err)
			}

			go res.ReportMetrics(10)
			it.tmAgent = append(it.tmAgent, res)
		}
	}
}

// pollTimeout returns the poll timeout value based on number of agents
func (it *veniceIntegSuite) pollTimeout() string {
	if it.config.DatapathKind == "hal" {
		// higher timeout when running on real HAL
		return fmt.Sprintf("%ds", 30+(it.config.NumHosts))
	}

	return fmt.Sprintf("%ds", 10+(it.config.NumHosts))
}

func (it *veniceIntegSuite) loggedInCtx() (context.Context, error) {
	return authntestutils.NewLoggedInContext(context.Background(), fmt.Sprintf("localhost:%s", it.config.APIGatewayPort), it.userCred)
}

func (it *veniceIntegSuite) SetUpSuite(c *check.C) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	// logger
	it.logger = log.GetNewLogger(log.GetDefaultConfig("venice-integ"))

	// tls provider
	err := testutils.SetupIntegTLSProvider()
	if err != nil {
		c.Fatalf("Error setting up TLS provider: %v", err)
	}

	// start resolver
	it.startResolver()
	rc := resolver.New(&resolver.Config{Name: "venice_integ_rslvr", Servers: []string{it.resolverSrv.GetListenURL()}})
	it.resolverClient = rc

	// start evts proxy
	tmpDir, err := ioutil.TempDir("", "evtsprxy_venice_integ")
	c.Assert(err, check.IsNil)
	l := log.GetNewLogger(log.GetDefaultConfig("evts-prxy"))

	it.epsDir = tmpDir
	eps, err := evtsproxy.NewEventsProxy("venice_integ_evtsprxy", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), it.resolverClient,
		5*time.Second, time.Second, it.epsDir, l)
	eps.StartDispatch()
	c.Assert(err, check.IsNil)
	it.eps = eps

	logConf := log.GetDefaultConfig("apisrv")
	l = log.GetNewLogger(logConf)
	// start API server
	it.apiSrv, it.apiSrvAddr, err = serviceutils.StartAPIServer(integTestApisrvURL, c.TestName(), l)
	c.Assert(err, check.IsNil)

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmURL, integTestNpmRESTURL, globals.APIServer, "", rc)
	c.Assert(err, check.IsNil)
	it.ctrler = ctrler

	// create a trouble shooting controller
	tsCtrler, err := tsm.NewTsCtrler(integTestTsmURL, integTestTsmRestURL, globals.APIServer, rc)
	c.Assert(err, check.IsNil)
	it.tsCtrler = tsCtrler

	// start CMD server
	it.launchCMDServer()

	// run citadel
	go it.startCitadel()

	// create the rollout controller
	it.rolloutCtrler, err = rollout.NewCtrler(integTestRolloutURL, globals.APIServer, rc)
	c.Assert(err, check.IsNil)

	// start spyglass and elasticsearch
	if it.config.StartEventsAndSearch {
		it.startEventsAndSearch()
	} else {
		it.disabledServices = []string{"search", "events"}
	}

	// start API gateway
	logConf = log.GetDefaultConfig("api-gw")
	// logConf.Filter = log.AllowAllFilter
	l = log.GetNewLogger(logConf)
	svcs := map[string]string{globals.APIServer: it.apiSrvAddr, globals.Spyglass: it.fdrAddr}
	it.apiGw, it.apiGwAddr, err = testutils.StartAPIGateway(fmt.Sprintf(":%s", it.config.APIGatewayPort), it.config.APIGatewaySkipAuth, svcs, it.disabledServices, []string{it.resolverSrv.GetListenURL()}, l)
	c.Assert(err, check.IsNil)

	// start agents
	log.Infof("Creating %d agents", it.config.NumHosts)
	it.startAgent()

	// REST Client
	restcl, err := apiclient.NewRestAPIClient(fmt.Sprintf("localhost:%s", it.config.APIGatewayPort))
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

	// Create test cluster object
	clRef := &pencluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		Spec: pencluster.ClusterSpec{
			AutoAdmitNICs: true,
		},
	}
	_, err = it.apisrvClient.ClusterV1().Cluster().Create(context.Background(), clRef)
	if err != nil {
		fmt.Printf("Error creating Cluster object, %v", err)
		os.Exit(-1)
	}

	// start NMD
	it.startNmd(c)
	time.Sleep(time.Millisecond * 100)

	// create tpm
	pm, err := tpm.NewPolicyManager(integTestTPMURL, rc)
	c.Assert(err, check.IsNil)
	it.tpm = pm

	it.userCred = &auth.PasswordCredential{
		Username: testutils.TestLocalUser,
		Password: testutils.TestLocalPassword,
		Tenant:   testutils.TestTenant,
	}
	l = log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest-setupAuth"))
	err = testutils.SetupAuth(integTestApisrvURL, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, it.userCred, l)
	c.Assert(err, check.IsNil)

	it.vcHub.SetUp(c, it.config.NumHosts)
}

func (it *veniceIntegSuite) SetUpTest(c *check.C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (it *veniceIntegSuite) TearDownTest(c *check.C) {
	log.Infof("============================= %s completed ==========================", c.TestName())
	// Remove persisted agent db files
	for _, i := range it.tmpFiles {
		os.Remove(i)
	}
}

func (it *veniceIntegSuite) TearDownSuite(c *check.C) {
	// stop delphi hub
	it.hub.Stop()
	// stop the agents
	for _, ag := range it.agents {
		ag.Stop()
	}
	for i, nmd := range it.nmds {
		// stop nmd
		nmd.Stop()
		dbPath := fmt.Sprintf("/tmp/nmd-%d.db", i)
		os.Remove(dbPath)
	}

	for _, ag := range it.tmAgent {
		ag.Stop()
	}

	for _, ag := range it.tsAgents {
		ag.Stop()
	}
	if it.epsDir != "" {
		os.RemoveAll(it.epsDir)
	}

	// remove citadel db files
	os.RemoveAll("/tmp/tstore/")

	it.epsDir = ""
	it.agents = []*netagent.Agent{}
	it.tsAgents = []*troubleshooting.Agent{}

	// stop server and client
	log.Infof("Stop all Test Controllers")
	it.tpm.Stop()
	it.ctrler.Stop()
	it.ctrler = nil
	it.tsCtrler.Stop()
	it.tsCtrler = nil
	it.apiGw.Stop()
	it.apiGw = nil
	it.vcHub.TearDown()

	// stop the CMD smartnic RPC server
	it.rpcServer.Stop()

	// stop finder and indexer
	if it.fdr != nil {
		it.fdr.Stop()
	}
	if it.idr != nil {
		it.idr.Stop()
	}

	it.apiSrv.Stop()
	it.apiSrv = nil
	it.resolverClient.Stop()
	it.resolverSrv.Stop()
	it.apisrvClient.Close()
	it.eps.RPCServer.Stop()

	// stop elastic
	testutils.StopElasticsearch(it.config.ElasticSearchName, it.elasticsearchDir)

	testutils.CleanupIntegTLSProvider()

	if cmdenv.CertMgr != nil {
		cmdenv.CertMgr.Close()
		cmdenv.CertMgr = nil
	}

	time.Sleep(time.Millisecond * 100) // allow goroutines to cleanup and terminate gracefully
	log.Infof("============================= TearDownSuite completed ==========================")
}
