package migrationinteg

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/tsdb"

	"gopkg.in/check.v1"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/agent/dscagent"
	agenttypes "github.com/pensando/sw/nic/agent/dscagent/types"
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
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

// integ test suite parameters
const (
	pensandoHosts      = 3
	nonPensandoHosts   = 0
	integTestNpmRPCURL = "localhost:9595"
	integTestTpmRPCURL = "localhost:9093"
	integTestTsmRPCURL = "localhost:9500"
	integTestRESTURL   = "localhost:9596"
	agentDatapathKind  = "mock"
	integTestApisrvURL = "localhost:8082"
)

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("migration-integ-test"))
	_      = recorder.Override(mockevtsrecorder.NewRecorder("migration-integ-test", logger))
)

type Dpagent struct {
	dscAgent *dscagent.DSCAgent
}

type migrationTestSuite struct {
	apiSrv         apiserver.Server
	apiSrvAddr     string
	apiCl          apiclient.Services
	npmCtrler      *npm.Netctrler
	tsmCtrler      *tsm.TsCtrler
	tpmCtrler      *tpm.PolicyManager
	agents         []*Dpagent
	numAgents      int
	logger         log.Logger
	resolverSrv    *rpckit.RPCServer
	resolverClient resolver.Interface
	hub            gosdk.Hub
	fakehal        *fakehal.Hal
	agent          *Dpagent
}

func TestMigrationInteg(t *testing.T) {
	var sts = &migrationTestSuite{}

	SetDefaultIntervals(time.Millisecond*500, time.Second*60)
	var _ = Suite(sts)
	TestingT(t)
}

func (it *migrationTestSuite) SetUpSuite(c *C) {
	log.Infof("====================== Setup Suite ======================")

	err := testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	it.logger = logger

	// CREATE RESOLVER SERVICE
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(false))
	c.Assert(err, IsNil)
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	it.resolverSrv = resolverServer

	rc := resolver.New(&resolver.Config{Name: globals.Npm, Servers: []string{resolverServer.GetListenURL()}})

	tsdb.Init(context.Background(), &tsdb.Opts{
		ClientName:     "npm-integ-test",
		ResolverClient: rc,
		Collector:      globals.Collector,
		DBName:         globals.DefaultTenant,
	})

	// Create API Server
	it.apiSrv, it.apiSrvAddr, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test", logger.WithContext("submodule", "pen-apiserver"))
	c.Assert(err, check.IsNil)

	// Create NPM
	featureflags.SetInitialized()
	ctrler, err := npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, rc, logger.WithContext("submodule", "pen-npm"), false)
	c.Assert(err, IsNil)
	it.npmCtrler = ctrler
	it.resolverClient = rc

	// Create TPM - needed by Netagent
	pm, err := tpm.NewPolicyManager(integTestTpmRPCURL, rc, "localhost:")
	c.Assert(err, check.IsNil)
	it.tpmCtrler = pm

	// Create TSM - needed by Netagent
	tsCtrler, err := tsm.NewTsCtrler(integTestTsmRPCURL, "localhost:", globals.APIServer, rc)
	c.Assert(err, check.IsNil)
	it.tsmCtrler = tsCtrler

	// Populate all services into Resolver
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
		URL:     integTestNpmRPCURL,
	}
	m.AddServiceInstance(&npmSi)

	tpmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-tpm-test",
		},
		Service: globals.Tpm,
		Node:    "localhost",
		URL:     integTestTpmRPCURL,
	}
	m.AddServiceInstance(&tpmSi)

	tsmSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-tsm-test",
		},
		Service: globals.Tsm,
		Node:    "localhost",
		URL:     integTestTsmRPCURL,
	}
	m.AddServiceInstance(&tsmSi)

	// Create API client
	for i := 0; i < 5; i++ {
		it.apiCl, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, logger, rpckit.WithBalancer(balancer.New(rc)))
		if err == nil {
			break
		}
	}

	if err != nil {
		c.Fatalf("cannot create api server client. Err : %v", err)
	}

	// Use the API client to push default objects like tenant
	// Since we are talking directly to API server, there is no need to initialize authentication and authorization
	err = it.createTenant("default")
	if err != nil {
		c.Fatalf("failed to create default tenant. Err : %v", err)
	}
}

func (it *migrationTestSuite) SetUpTest(c *C) {
	log.Infof("====================== Setup Test %v ======================", c.TestName())
}

func (it *migrationTestSuite) TearDownTest(c *C) {
	log.Infof("====================== TearDown Test %v ======================", c.TestName())
	err := it.deleteAllObjects()
	if err != nil {
		it.logger.Infof("Failed to delete all objects. Err : %v", err)
	}

}

func (it *migrationTestSuite) TearDownSuite(c *C) {
	log.Infof("====================== TearDown Suite ======================")
}

// UTILITIES
func (it *migrationTestSuite) restartAPIServer() error {
	it.apiSrv.Stop()
	it.apiCl.Close()
	time.Sleep(1 * time.Second)
	var err error

	it.apiSrv, _, err = serviceutils.StartAPIServer(integTestApisrvURL, "npm-integ-test", it.logger)
	if err != nil {
		log.Errorf("Failed to start API server. Err : %v", err)
		return err
	}

	for i := 0; i < 5; i++ {
		it.apiCl, err = apiclient.NewGrpcAPIClient("integ_test", globals.APIServer, it.logger, rpckit.WithBalancer(balancer.New(it.resolverClient)))
		if err == nil {
			break
		}
	}

	if err != nil {
		log.Errorf("Faield to get API client. Err : %v", err)
		return err
	}
	time.Sleep(time.Second)
	log.Infof("API server restarted.")
	return nil
}

func (it *migrationTestSuite) restartNpm() error {
	it.npmCtrler.Stop()
	log.Infof("NPM stopped")
	time.Sleep(time.Millisecond * 100)
	var err error

	featureflags.SetInitialized()
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	if err != nil {
		log.Errorf("Failed to create new NPM. Err : %v", err)
		return err
	}
	time.Sleep(time.Second)

	return nil
}

func createAgent(logger log.Logger, veniceURL, nodeUUID string) (*Dpagent, error) {
	os.RemoveAll(globals.NetAgentDBPath)
	// create new network agent
	var lis netutils.TestListenAddr
	if err := lis.GetAvailablePort(); err != nil {
		log.Errorf("Failed to find an available port. Err: %v", err)
		return nil, fmt.Errorf("failed to find an available port. Err: %v", err)
	}
	nagent, err := dscagent.NewDSCAgent(logger, globals.Npm, globals.Tpm, globals.Tsm, lis.ListenURL.String())
	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// Handle mode change
	o := agenttypes.DistributedServiceCardStatus{
		DSCName:     nodeUUID,
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		MgmtIntf:    "lo",
		Controllers: []string{veniceURL},
	}
	err = nagent.ControllerAPI.HandleVeniceCoordinates(o)
	logger.Infof("RestURL: %v", lis.ListenURL.String())
	time.Sleep(time.Second * 5)

	if err != nil {
		log.Errorf("Failed to perform mode switch. Err: %v", err)
		return nil, fmt.Errorf("failed to perform mode switch. Err: %v", err)
	}
	// create an agent instance
	ag := Dpagent{
		dscAgent: nagent,
	}

	return &ag, nil
}
