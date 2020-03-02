// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net"
	"os"
	"path"
	"strings"
	"time"

	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"

	"github.com/vishvananda/netlink"
	"golang.org/x/net/context"
	"gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	pencluster "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/dscagent"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/nmd"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	nmdutils "github.com/pensando/sw/nic/agent/nmd/utils"
	nmdproto "github.com/pensando/sw/nic/agent/protos/nmd"
	tmrestapi "github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	tmstate "github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/netutils"

	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/collector"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver"
	"github.com/pensando/sw/venice/citadel/data"
	httpserver "github.com/pensando/sw/venice/citadel/http"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/query"
	cmdapi "github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	cmdauth "github.com/pensando/sw/venice/cmd/grpc/server/auth"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	cmdsvc "github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/services/mock"
	cmdtypes "github.com/pensando/sw/venice/cmd/types"
	types "github.com/pensando/sw/venice/cmd/types/protos"
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
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/debug"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/nodewatcher"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/strconv"
	tutils "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// integ test suite parameters
const (
	// service URLs
	integTestNpmURL      = "localhost:9495"
	integTestNpmRESTURL  = "localhost:9496"
	integTestApisrvURL   = "localhost:8082"
	integTestTPMURL      = "localhost:9093"
	vchTestURL           = "localhost:19003"
	integTestCitadelURL  = "localhost:9094"
	integTestRolloutURL  = "localhost:9095"
	integTestRevProxyURL = "localhost:" + globals.AgentProxyPort

	smartNICServerURL       = "localhost:" + globals.CMDSmartNICRegistrationPort
	cmdAuthServer           = "localhost:" + globals.CMDGRPCAuthPort
	cmdLeaderInstanceServer = "localhost:" + globals.CMDGRPCLeaderInstancePort

	// TS Controller
	integTestTsmURL     = "localhost:9500"
	integTestTsmRestURL = "localhost:9501"

	// default valules
	numIntegTestAgents = 1
	agentDatapathKind  = "mock"
	maxConnRetry       = 30

	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("venice_integ_test"))

	// create events recorder
	evtsRecorder, _ = recorder.NewRecorder(&recorder.Config{
		Component:     "venice_integ_test",
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, logger)

	// fwUpdate variables
	fwDir = "/tmp"

	fwVer = "1.0E"

	originalPath string
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

// naples state
type naples struct {
	macAddr  string
	snicName string
	agent    *dscagent.DSCAgent
	tmAgent  *tmrestapi.RestServer
	nmd      *nmd.Agent
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
	snics               []*naples
	restClient          apiclient.Services
	apisrvClient        apiclient.Services
	vcHub               vchSuite
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
	ctx                 context.Context
	cancel              context.CancelFunc
	resolverServer      *rpckit.RPCServer
	fakehal             *fakehal.Hal
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
func (it *veniceIntegSuite) CheckNICVersionForAdmission(nicSku, nicVersion string) (string, string) {
	return "", ""
}

// getNaplesMac returns naples mac for a naples instance
func (it *veniceIntegSuite) getNaplesMac(nidx int) string {
	return fmt.Sprintf("00ae.cd01.%02d01", nidx)
}

func (it *veniceIntegSuite) createHostObjects() {
	for i := 0; i < it.config.NumHosts; i++ {
		hostName := fmt.Sprintf("host%d", i)

		host := &pencluster.Host{
			ObjectMeta: api.ObjectMeta{
				Name: hostName,
			},
			Spec: pencluster.HostSpec{
				DSCs: []pencluster.DistributedServiceCardID{
					{
						MACAddress: it.getNaplesMac(i),
					},
				},
			},
		}

		_, err := it.apisrvClient.ClusterV1().Host().Create(it.ctx, host)
		if err != nil {
			log.Fatalf("Error creating Host object %v, err: %v", host, err)
		}
	}
}

func (it *veniceIntegSuite) deleteHostObjects() {
	hosts, err := it.apisrvClient.ClusterV1().Host().List(it.ctx, &api.ListWatchOptions{})
	if err != nil {
		log.Fatalf("Couldn't get the list of host objects")
	}
	for _, h := range hosts {
		_, err := it.apisrvClient.ClusterV1().Host().Delete(it.ctx, &h.ObjectMeta)
		if err != nil {
			log.Errorf("Couldn't delete host object %+v", h)
		}
	}
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
	cmdenv.SmartNICRegRPCServer = rpcServer

	cw := cmdapi.NewCfgWatcherService(it.logger, it.apiSrvAddr)
	cmdenv.StateMgr = cache.NewStatemgr(cw, func() cmdtypes.LeaderService { return cmdenv.LeaderService })

	// create and register the RPC handler for SmartNIC service
	it.smartNICServer = smartnic.NewRPCServer(
		smartnic.HealthWatchInterval,
		smartnic.DeadInterval,
		globals.NmdRESTPort,
		cmdenv.StateMgr,
		it)
	cmdenv.NICService = it.smartNICServer

	if err != nil {
		log.Fatalf("Error creating Smart NIC server: %v", err)
	}
	grpc.RegisterSmartNICRegistrationServer(rpcServer.GrpcServer, it.smartNICServer)
	rpcServer.Start()

	cmdenv.MetricsService = cmdsvc.NewMetricsService("node", "cluster", cmdenv.ResolverClient)
	err = cmdenv.MetricsService.Start()
	if err != nil {
		log.Fatalf("Failed to start metrics service with error: %v", err)
	}

	// Start CMD config watcher
	cmdenv.Logger = it.logger
	cmdenv.QuorumNodes = []string{"localhost"}
	l := mock.NewLeaderService("testMaster")
	l.LeaderID = "testMaster"
	cmdenv.LeaderService = l
	s := cmdsvc.NewSystemdService(cmdsvc.WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cmdenv.MasterService = cmdsvc.NewMasterService(
		"testMaster",
		cmdsvc.WithLeaderSvcMasterOption(l),
		cmdsvc.WithSystemdSvcMasterOption(s),
		cmdsvc.WithConfigsMasterOption(&mock.Configs{}),
		cmdsvc.WithCfgWatcherMasterOption(cw),
		cmdsvc.WithDiagModuleUpdaterSvcOption(diagmock.GetModuleUpdater()),
		cmdsvc.WithClusterHealthMonitor(&mock.ClusterHealthMonitor{}))
	cw.Start()

	// start CMD auth server
	go cmdauth.RunAuthServer(cmdAuthServer, nil)
	go cmdauth.RunLeaderInstanceServer(cmdLeaderInstanceServer, nil)
	go it.smartNICServer.MonitorHealth()
	it.resolverServer = rpcServer
}

func (it *veniceIntegSuite) startNmd(c *check.C) {
	for i := 0; i < it.config.NumHosts; i++ {

		snic := it.snics[i]
		restURL := "localhost:0"
		dbPath := fmt.Sprintf("/tmp/nmdAg-%d.db", i)

		tutils.CreateFruJSON(it.getNaplesMac(i))

		// Create the new NMD
		nmdAg, err := nmd.NewAgent(nil,
			dbPath,
			restURL,
			integTestRevProxyURL,
			globals.NicRegIntvl*time.Second, globals.NicUpdIntvl*time.Second,
		)
		if err != nil {
			log.Fatalf("Error creating NMD. Err: %v", err)
		}
		// start NMDs rest server and
		n := nmdAg.GetNMD()

		// Switch to Managed mode
		naplesCfg := nmdproto.DistributedServiceCard{
			ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
			TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
			Spec: nmdproto.DistributedServiceCardSpec{
				Mode:        nmdproto.MgmtMode_NETWORK.String(),
				NetworkMode: nmdproto.NetworkMode_OOB.String(),
				Controllers: []string{"localhost"},
				ID:          "4242.4242.4242",
				IPConfig: &pencluster.IPConfig{
					IPAddress: "42.42.42.42/16",
					DefaultGW: "42.42.42.1",
				},
				PrimaryMAC: "4242.4242.4242",
			},
		}

		log.Infof("Naples config: %+v", naplesCfg)

		var resp nmdstate.NaplesConfigResp

		// Post the mode change config
		f3 := func() (bool, interface{}) {
			err = netutils.HTTPPost(n.GetNMDUrl(), &naplesCfg, &resp)
			if err != nil {
				log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
				return false, nil
			}
			return true, nil
		}
		tutils.AssertEventually(c, f3, "Failed to post the naples config")
		time.Sleep(time.Second * 3)
		snic.nmd = nmdAg
	}

	// verify NIC is admitted with CMD
	for i := 0; i < it.config.NumHosts; i++ {
		tutils.AssertEventually(c, func() (bool, interface{}) {
			nm := it.snics[i].nmd.GetNMD()
			// validate the mode is network
			cfg := nm.GetNaplesConfig()
			log.Infof("NaplesConfig: %v", cfg)
			if cfg.Spec.Mode != nmdproto.MgmtMode_NETWORK.String() {
				log.Errorf("Failed to switch to network mode")
				return false, nil
			}

			// Fetch smartnic object
			nic, err := nm.GetSmartNIC()
			if nic == nil || err != nil {
				log.Errorf("NIC not found in nicDB, NIC : %s ERR : %s", nic, err)
				return false, nil
			}

			// Verify NIC is admitted
			if nic.Status.AdmissionPhase != pencluster.DistributedServiceCardStatus_ADMITTED.String() {
				log.Errorf("NIC is not admitted, phase: %s", nic.Status.AdmissionPhase)
				return false, nil
			}

			// Verify Update NIC task is running
			if nm.GetUpdStatus() == false {
				log.Errorf("Update NIC is not in progress")
				return false, nil
			}

			return true, nil
		}, "Failed to verify mode is in Network Mode", string("10ms"), string("60s"))
	}
}

func (it *veniceIntegSuite) createResolver() {
	// Now create a mock resolver
	m := mock.NewResolverService()
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

	citadelQuerySi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-citadel-query-test",
		},
		Service: globals.CitadelQuery,
		Node:    "localhost",
		URL:     "localhost:" + globals.CitadelQueryRPCPort,
	}
	m.AddServiceInstance(&citadelQuerySi)

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

	nicUpdatesSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.CmdNICUpdatesSvc,
		},
		Service: globals.CmdNICUpdatesSvc,
		Node:    "localhost",
		URL:     cmdLeaderInstanceServer,
	}
	m.AddServiceInstance(&nicUpdatesSi)

	cmdAuthSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Cmd,
		},
		Service: globals.Cmd,
		Node:    "localhost",
		URL:     cmdAuthServer,
	}
	m.AddServiceInstance(&cmdAuthSi)
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
	cfg.EnableKstore = false
	cfg.EnableKstoreMeta = false
	nodeUUID := "citadel-node0"

	httpURL := "localhost:" + globals.CitadelHTTPPort
	queryURL := ":" + globals.CitadelQueryRPCPort
	collectorURL := fmt.Sprintf(":%s", globals.CollectorRPCPort)
	it.updateResolver(globals.Collector, collectorURL)

	// create the data node
	dn, err := data.NewDataNode(cfg, nodeUUID, integTestCitadelURL, "/tmp/tstore/", it.logger)
	if err != nil {
		log.Fatalf("Error creating data node. Err: %v", err)
	}

	br, err := broker.NewBroker(cfg, nodeUUID, it.logger)
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

	// start collector, use citadel
	c := collector.NewCollector(br)

	// setup an rpc server
	srv, err := rpcserver.NewCollRPCSrv(collectorURL, c)
	if err != nil {
		log.Fatalf(err.Error())
	}

	log.Infof("%s is running {%+v}", globals.Citadel, srv)

	// Creating debug instance
	dbg := debug.New(srv.Debug)

	// start the http server
	hsrv, err := httpserver.NewHTTPServer(httpURL, br, dn, dbg)
	if err != nil {
		log.Fatalf("Error creating HTTP server. Err: %v", err)
	}
	log.Infof("HTTP server is listening on %s", hsrv.GetAddr())

	qsrv, err := query.NewQueryService(queryURL, br)

	log.Infof("query server is listening on %+v", qsrv)
	time.Sleep(time.Second)
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
	it.elasticsearchAddr, it.elasticsearchDir, err = testutils.StartElasticsearch(it.config.ElasticSearchName, it.elasticsearchDir, it.signer, it.trustRoots)
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
	evtsMgr, evtsMgrURL, err := testutils.StartEvtsMgr("localhost:0", it.resolverClient, it.logger, it.esClient, nil)
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

func (it *veniceIntegSuite) startAgent(c *check.C, veniceURL string) {
	it.logger.Info("Starting Agent")
	for i := 0; i < it.config.NumHosts; i++ {

		snicMac := it.getNaplesMac(i)
		dotMac, _ := strconv.ParseMacAddr(snicMac)
		snicName := fmt.Sprintf("naples%d", i+1)

		dscAgent, err := dscagent.NewDSCAgent(logger, globals.Npm, globals.Tpm, globals.Tsm, agentTypes.DefaultAgentRestURL)
		if err != nil {
			log.Errorf("Error creating network agent. Err: %v", err)
		}

		// Handle mode change

		o := agentTypes.DistributedServiceCardStatus{
			DSCName:     dotMac,
			DSCMode:     "network_managed_inband",
			MgmtIP:      "42.42.42.42/24",
			Controllers: []string{veniceURL},
		}
		err = dscAgent.ControllerAPI.HandleVeniceCoordinates(o)
		logger.Infof("RestURL: %v", agentTypes.DefaultAgentRestURL)
		time.Sleep(time.Second * 5)

		rc := it.resolverClient

		// report node metrics
		node := &cluster.DistributedServiceCard{
			TypeMeta: api.TypeMeta{
				Kind: "DistributedServiceCard",
			},
			ObjectMeta: api.ObjectMeta{
				Name: dscAgent.InfraAPI.GetDscName(),
			},
		}

		snic := naples{
			macAddr:  snicMac,
			snicName: snicName,
			agent:    dscAgent,
		}
		it.snics = append(it.snics, &snic)

		if i == 0 { // start only 1 instance
			_, err = nodewatcher.NewNodeWatcher(it.ctx, node, 10*time.Second, it.logger)
			if err != nil {
				log.Fatalf("Error creating NodeWatcher. Err: %v", err)
			}

			tpState, err := tmstate.NewTpAgent(it.ctx, globals.AgentRESTPort)
			if err != nil {
				log.Fatalf("failed to init tmagent state, err: %v", err)
			}

			// Init the TSDB
			if err := tpState.TsdbInit(fmt.Sprintf("tmagent-%d", i), rc); err != nil {
				log.Fatalf("failed to init tsdb, err: %v", err)
			}

			tmpFd, err := ioutil.TempFile("/tmp", "palazzo-fwlogshm")
			if err != nil {
				log.Fatalf("failed to create temp file, err: %v", err)
			}

			shmPath := tmpFd.Name()
			it.tmpFiles = append(it.tmpFiles, shmPath)

			if err := tpState.FwlogInit(shmPath); err != nil {
				log.Fatal(err)
			}

			it.fwlogGen(shmPath)

			res, err := tmrestapi.NewRestServer(it.ctx, fmt.Sprintf("localhost:%d", it.config.TmAgentRestPort+i), tpState, "mock")
			if err != nil {
				log.Fatalf("Error creating tmagent rest server. Err: %v", err)
			}
			snic.tmAgent = res

			res.ReportMetrics(10*time.Second, nil)
		}
		it.logger.Info("Agent successfully started")
	}
}

func (it *veniceIntegSuite) fwlogGen(fwlogShm string) error {
	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	protKey := func() int32 {
		for k := range halproto.IPProtocol_name {
			if k != 0 {
				return k
			}
		}
		return 1 // TCP
	}

	fwActionKey := func() int32 {
		for k := range halproto.SecurityAction_name {
			if k != 0 {
				return k
			}
		}
		return 1 // ALLOW
	}

	shm, err := ipc.NewSharedMem(mSize, instCount, fwlogShm)
	if err != nil {
		return fmt.Errorf("failed to init fwlog, %s", err)
	}

	ipcList := make([]*ipc.IPC, instCount)
	for ix := 0; ix < instCount; ix++ {
		ipcList[ix] = shm.IPCInstance()
	}

	go func() {
		for it.ctx.Err() == nil {
			for _, fd := range ipcList {
				ev := &halproto.FWEvent{
					Sipv4:     uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
					Dipv4:     uint32(rand.Int31n(200) + rand.Int31n(200)<<8 + rand.Int31n(200)<<16 + rand.Int31n(200)<<24),
					Dport:     uint32(rand.Int31n(5000)),
					Sport:     uint32(rand.Int31n(5000)),
					IpProt:    halproto.IPProtocol(protKey()),
					Fwaction:  halproto.SecurityAction(fwActionKey()),
					Direction: uint32(rand.Int31n(2) + 1),
					RuleId:    uint64(rand.Int63n(5000)),
					SessionId: uint64(rand.Int63n(5000)),
				}

				if ev.IpProt == halproto.IPProtocol_IPPROTO_ICMP {
					ev.Icmpcode = uint32(rand.Int31n(16))
					ev.Icmptype = uint32(rand.Int31n(5))
					ev.Icmpid = uint32(rand.Int31n(5000))
				}
				if err := fd.Write(ev); err != nil {
					log.Errorf("failed to write fwlog, %s", err)
				}

				time.Sleep(time.Second)
			}
		}
	}()

	return nil
}

// pollTimeout returns the poll timeout value based on number of agents
func (it *veniceIntegSuite) pollTimeout() string {
	if it.config.DatapathKind == "hal" {
		// higher timeout when running on real HAL
		return fmt.Sprintf("%ds", 150+(it.config.NumHosts))
	}

	return fmt.Sprintf("%ds", 60+(it.config.NumHosts))
}

func (it *veniceIntegSuite) loggedInCtx() (context.Context, error) {
	return authntestutils.NewLoggedInContext(it.ctx, fmt.Sprintf("localhost:%s", it.config.APIGatewayPort), it.userCred)
}

func (it *veniceIntegSuite) loggedInCtxWithCred(tenant, username, password string) (context.Context, error) {
	userCred := &auth.PasswordCredential{
		Username: username,
		Password: password,
		Tenant:   tenant,
	}
	return authntestutils.NewLoggedInContext(it.ctx, fmt.Sprintf("localhost:%s", it.config.APIGatewayPort), userCred)
}

func (it *veniceIntegSuite) verifyNaplesConnected(c *check.C) {
	// verify smartnics are all admitted
	tutils.AssertEventually(c, func() (bool, interface{}) {
		ctx, err := it.loggedInCtx()
		if err != nil {
			return false, err
		}
		snicList, err := it.apisrvClient.ClusterV1().DistributedServiceCard().List(ctx, &api.ListWatchOptions{})
		if err != nil {
			return false, err
		}
		if len(snicList) < len(it.snics) {
			return false, snicList
		}
		for _, snic := range snicList {
			if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
				return false, snicList
			}
		}

		return true, nil
	}, "Smartnics are not yet admitted", "1s", it.pollTimeout())

	// verify agents are all connected
	for _, sn := range it.snics {
		ag := sn.agent
		tutils.AssertEventually(c, func() (bool, interface{}) {
			cfg := ag.InfraAPI.GetConfig()
			return cfg.IsConnectedToVenice, nil
		}, "agents are not connected to NPM", "1s", it.pollTimeout())
	}

	log.Infof("All Naples are connected")
}

func (it *veniceIntegSuite) SetUpSuite(c *check.C) {
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

	os.RemoveAll(path.Dir(globals.NetAgentDBPath))

	os.MkdirAll(path.Dir(globals.NmdBackupDBPath), 0664)

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	createNaplesOOBInterface()
	it.ctx, it.cancel = context.WithCancel(context.Background())
	// logger
	it.logger = logger
	originalPath = os.Getenv("PATH")
	os.Setenv("PATH", originalPath+":/"+fwDir)
	err := tutils.SetUpFwupdateScript(fwVer, fwDir)
	if err != nil {
		c.Fatalf("Failed to setup fwupdate script : %v", err)
	}

	globals.NaplesTrustRootsFile = "/tmp/clusterTrustRoots.pem"
	err = nmdutils.ClearNaplesTrustRoots()
	if err != nil {
		c.Fatalf("Could not remove stale trust roots file: %v", err)
	}

	// tls provider
	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		c.Fatalf("Error setting up TLS provider: %v", err)
	}

	// Override default CMD metrics send interval
	cmdsvc.MetricsSendInterval = 10 * time.Second

	// Create resolver. Resolver uses the CMD Auth server, so it will start when RunAuthServer is invoked()
	it.createResolver()
	cmdenv.ResolverService = it.mockResolver

	rc := resolver.New(&resolver.Config{Name: "venice_integ_rslvr", Servers: []string{"localhost:" + globals.CMDResolverPort}})
	it.resolverClient = rc

	// start evts proxy
	tmpDir, err := ioutil.TempDir("", "evtsprxy_venice_integ")
	c.Assert(err, check.IsNil)
	l := log.GetNewLogger(log.GetDefaultConfig("evts-prxy"))

	it.epsDir = tmpDir
	eps, err := evtsproxy.NewEventsProxy("venice_integ_test", "venice_integ_evtsprxy", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), it.resolverClient,
		5*time.Second, time.Second, &events.StoreConfig{Dir: it.epsDir}, l)
	c.Assert(err, check.IsNil)
	eps.StartDispatch()
	c.Assert(err, check.IsNil)
	it.eps = eps

	logConf := log.GetDefaultConfig("apisrv")
	l = log.GetNewLogger(logConf)
	// start API server
	it.apiSrv, it.apiSrvAddr, err = serviceutils.StartAPIServer(integTestApisrvURL, c.TestName(), l)
	c.Assert(err, check.IsNil)

	// Set feature flags to initialized
	featureflags.SetInitialized()

	// create a controller
	ctrler, err := npm.NewNetctrler(integTestNpmURL, integTestNpmRESTURL, globals.APIServer, rc, logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, check.IsNil)
	it.ctrler = ctrler

	// create a trouble shooting controller
	tsCtrler, err := tsm.NewTsCtrler(integTestTsmURL, integTestTsmRestURL, globals.APIServer, rc)
	c.Assert(err, check.IsNil)
	it.tsCtrler = tsCtrler

	// start CMD server
	it.launchCMDServer()

	// run citadel
	it.startCitadel()

	// create the rollout controller
	it.rolloutCtrler, err = rollout.NewCtrler(integTestRolloutURL, globals.APIServer, rc, evtsRecorder)
	c.Assert(err, check.IsNil)

	// start spyglass and elasticsearch
	if it.config.StartEventsAndSearch {
		it.startEventsAndSearch()
		it.disabledServices = []string{"objstore"}
	} else {
		it.disabledServices = []string{"search", "events", "objstore", "audit"}
	}

	// start API gateway
	logConf = log.GetDefaultConfig("api-gw")
	l = log.GetNewLogger(logConf)
	svcs := map[string]string{globals.APIServer: it.apiSrvAddr, globals.Spyglass: it.fdrAddr}
	it.apiGw, it.apiGwAddr, err = testutils.StartAPIGateway(fmt.Sprintf(":%s", it.config.APIGatewayPort), it.config.APIGatewaySkipAuth, svcs, it.disabledServices, []string{"localhost:" + globals.CMDResolverPort}, nil, l)
	c.Assert(err, check.IsNil)

	// REST Client
	restcl, err := apiclient.NewRestAPIClient(fmt.Sprintf("localhost:%s", it.config.APIGatewayPort))
	if err != nil {
		c.Fatalf("cannot create REST client. Err: %v", err)
	}
	it.restClient = restcl

	// create api server client
	l = log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest"))
	for i := 0; i < maxConnRetry; i++ {
		it.apisrvClient, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, l, rpckit.WithBalancer(balancer.New(rc)))
		if err == nil {
			break
		}
		time.Sleep(time.Second)
	}
	if err != nil {
		c.Fatalf("cannot create grpc client")
	}
	time.Sleep(time.Millisecond * 100)

	// Create test cluster object
	clRef := &pencluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		Spec: pencluster.ClusterSpec{
			AutoAdmitDSCs: true,
		},
	}
	_, err = it.apisrvClient.ClusterV1().Cluster().Create(it.ctx, clRef)
	if err != nil {
		fmt.Printf("Error creating Cluster object, %v", err)
		os.Exit(-1)
	}

	// create tpm
	pm, err := tpm.NewPolicyManager(integTestTPMURL, rc, "localhost:")
	c.Assert(err, check.IsNil)
	it.tpm = pm

	it.userCred = &auth.PasswordCredential{
		Username: testutils.TestLocalUser,
		Password: testutils.TestLocalPassword,
		Tenant:   testutils.TestTenant,
	}
	l = log.GetNewLogger(log.GetDefaultConfig("VeniceIntegTest-setupAuth"))
	log.Infof("setup auth")
	err = testutils.SetupAuth(integTestApisrvURL, true, nil, nil, it.userCred, l)
	c.Assert(err, check.IsNil)

	// start agents
	it.logger.Infof("Creating %d agents", it.config.NumHosts)
	it.startAgent(c, "localhost:"+globals.CMDResolverPort)

	// start NMD
	it.startNmd(c)
	time.Sleep(time.Millisecond * 100)

	it.vcHub.SetUp(c, it.config.NumHosts)

	// wait for naples to connect to Venice
	it.verifyNaplesConnected(c)
	time.Sleep(time.Second * 2)
	log.Infof("============================= Setup done ==========================")
}

func (it *veniceIntegSuite) SetUpTest(c *check.C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
	it.logger = logger.WithContext("t_name", c.TestName())

	// verify all naples are connected
	it.verifyNaplesConnected(c)
}

func (it *veniceIntegSuite) TearDownTest(c *check.C) {
	log.Infof("============================= %s completed ==========================", c.TestName())

	// Remove persisted agent db files
	for _, i := range it.tmpFiles {
		os.Remove(i)
	}
}

func (it *veniceIntegSuite) TearDownSuite(c *check.C) {
	log.Infof("============================= Tearing down Setup ==========================")

	deleteNaplesOOBInterfaces()

	tutils.DeleteFwupdateScript(fwDir)
	os.Setenv("PATH", originalPath)

	// stop delphi hub
	if it.hub != nil {
		it.hub.Stop()
	}
	// stop the agents
	for _, sn := range it.snics {
		sn.agent.Stop()
		if sn.tmAgent != nil {
			sn.tmAgent.Stop()
		}
	}
	for i, sn := range it.snics {
		// stop nmd
		if sn != nil {
			log.Info("Stopping NMD")
			sn.nmd.Stop()
		}
		dbPath := fmt.Sprintf("/tmp/nmd-%d.db", i)
		os.Remove(dbPath)
	}

	if it.epsDir != "" {
		os.RemoveAll(it.epsDir)
	}

	// remove citadel db files
	os.RemoveAll("/tmp/tstore/")

	it.epsDir = ""
	it.snics = []*naples{}

	// stop server and client
	log.Infof("Stop all Test Controllers")
	it.cancel()
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
	it.apisrvClient.Close()
	it.eps.RPCServer.Stop()

	// stop elastic
	testutils.StopElasticsearch(it.config.ElasticSearchName, it.elasticsearchDir)

	testutils.CleanupIntegTLSProvider()

	if cmdenv.CertMgr != nil {
		cmdenv.CertMgr.Close()
		cmdenv.CertMgr = nil
	}

	nmdutils.ClearNaplesTrustRoots()
	it.fakehal.Stop()
	time.Sleep(time.Millisecond * 100) // allow goroutines to cleanup and terminate gracefully
	log.Infof("============================= TearDownSuite completed ==========================")
}

func deleteNaplesOOBInterfaces() error {
	oobIntf, err := netlink.LinkByName(ipif.NaplesOOBInterface)
	if err != nil {
		log.Errorf("TearDown Failed to look up the interfaces. Err: %v", err)
		return err
	}

	return netlink.LinkDel(oobIntf)
}

func createNaplesOOBInterface() error {
	oobMAC, _ := net.ParseMAC("42:42:42:42:42:42")
	dhcpClientMock := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         ipif.NaplesOOBInterface,
			TxQLen:       1000,
			HardwareAddr: oobMAC,
		},
		PeerName: "dhcp-peer",
	}

	// Create the veth pair
	if err := netlink.LinkAdd(dhcpClientMock); err != nil {
		return err
	}
	if err := netlink.LinkSetARPOn(dhcpClientMock); err != nil {
		return err
	}

	return netlink.LinkSetUp(dhcpClientMock)
}
