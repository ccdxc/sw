package auth

import (
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"

	. "github.com/pensando/sw/test/utils"
)

const (
	// users
	testUser     = "test"
	testPassword = TestLocalPassword
	testTenant   = "testtenant"
)

var (
	logger = log.WithContext("module", "auth_integ_test")

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("auth_integ_test", logger))
)

type tInfo struct {
	l             log.Logger
	apiServer     apiserver.Server
	apiServerAddr string
	apiGw         apigw.APIGateway
	apiGwAddr     string
	esServer      *esmock.ElasticServer
	mockResolver  *mockresolver.ResolverClient
	fdr           finder.Interface
	fdrAddr       string
	ldapAddr      string
	openLDAPAddr  string
	referralAddr  string
	restcl        apiclient.Services
	apicl         apiclient.Services
}

var tinfo tInfo

func (tInfo *tInfo) setup() error {
	var err error

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	// start mock elastic server
	tinfo.esServer = esmock.NewElasticServer(tInfo.l.WithContext("submodule", "elasticsearch-mock-server"))
	tinfo.esServer.Start()
	si := &types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     tinfo.esServer.GetElasticURL(),
	}
	tInfo.mockResolver.AddServiceInstance(si) // add mock elastic service to mock resolver

	// start spyglass finder
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", tInfo.mockResolver, nil, tInfo.l, nil)
	if err != nil {
		return err
	}
	tInfo.fdr = fdr.(finder.Interface)
	tInfo.fdrAddr = fdrAddr

	// start API server
	trace.Init("ApiServer")
	tInfo.apiServer, tInfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", "AuthIntegTest", tInfo.l)
	if err != nil {
		return err
	}

	// start API gateway
	tInfo.apiGw, tInfo.apiGwAddr, err = testutils.StartAPIGateway(":0", false,
		map[string]string{globals.APIServer: tInfo.apiServerAddr, globals.Spyglass: tInfo.fdrAddr},
		[]string{"telemetry_query", "objstore", "tokenauth"},
		[]string{}, nil, tInfo.l)
	if err != nil {
		return err
	}

	// start ldap server
	if err := setupOpenLdap(); err != nil {
		log.Errorf("cannot start openldap server, err: %v", err)
		return err
	}
	// REST Client
	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		log.Errorf("cannot create REST client, Err: %v", err)
		return err
	}
	tinfo.restcl = restcl
	// grpc client
	apicl, err := apiclient.NewGrpcAPIClient("AuthIntegTest", tinfo.apiServerAddr, tinfo.l)
	if err != nil {
		log.Errorf("cannot create grpc client, Err: %v", err)
		return err
	}
	tinfo.apicl = apicl

	return nil
}

func (tInfo *tInfo) teardown() {
	tinfo.apicl.Close()
	tinfo.restcl.Close()
	tinfo.esServer.Stop()
	tInfo.fdr.Stop()
	tInfo.apiServer.Stop()
	tInfo.apiGw.Stop()
	testutils.CleanupIntegTLSProvider()
	shutdownOpenLdap()
}

func TestMain(m *testing.M) {
	grpclog.SetLoggerV2(logger)
	config := log.GetDefaultConfig("AuthIntegTest")
	config.Filter = log.AllowAllFilter
	l := log.GetNewLogger(config)
	tinfo.l = l
	tinfo.mockResolver = mockresolver.New()

	if err := tinfo.setup(); err != nil {
		log.Fatalf("failed to setup test, err: %v", err)
	}

	ret := m.Run()
	tinfo.teardown()
	os.Exit(ret)
}
