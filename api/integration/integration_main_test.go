package integration

import (
	"context"
	"fmt"
	"net"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/auth"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
	"github.com/pensando/sw/venice/utils/elastic"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apigw"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	_ "github.com/pensando/sw/venice/apigw/svc"
)

const (
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../venice/utils/certmgr/testdata/roots.pem"
	// test user
	testUser     = "test"
	testPassword = "Pensando0$"
)

type tInfo struct {
	l             log.Logger
	apiserverport string
	apigwport     string
	cache         apiintf.CacheInterface
	esServer      *esmock.ElasticServer
	certsrvurl    string
	userCred      *auth.PasswordCredential
	apisrvConfig  apiserver.Config
	gwConfig      apigw.Config
}

var tinfo tInfo

func startSpyglass() finder.Interface {

	// start mock elastic server
	tinfo.esServer = esmock.NewElasticServer(tinfo.l.WithContext("submodule", "elasticsearch-mock-server"))
	tinfo.esServer.Start()

	// create mock resolver
	rsr := mockresolver.New()
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
	// add mock elastic service to mock resolver
	rsr.AddServiceInstance(si)

	esClient, err := elastic.NewClient(tinfo.esServer.GetElasticURL(), nil, tinfo.l.WithContext("submodule", "elastic"))
	if err != nil {
		log.Errorf("failed to create Elastic client for finder, err: %v", err)
		return nil
	}
	fdr, err := finder.NewFinder(context.Background(),
		"localhost:0",
		rsr,
		nil,
		tinfo.l,
		finder.WithElasticClient(esClient),
		finder.WithModuleWatcher(diagmock.GetModuleWatcher()),
		finder.WithDiagnosticsService(diagmock.GetDiagnosticsService()))
	if err != nil {
		log.Errorf("Error creating finder: %+v", err)
		os.Exit(-1)
	}

	// start the gRPC server for search backend
	err = fdr.Start()
	if err != nil {
		log.Errorf("Failed to get start spyglass-finder: %+v", err)
		os.Exit(-1)
	}
	fmt.Printf("Spyglass-Finder gRPC endpoint: %s", fdr.GetListenURL())
	return fdr

}
func TestMain(m *testing.M) {
	l := log.WithContext("module", "CrudOpsTest")
	grpclog.SetLoggerV2(l)
	recorder.Override(mockevtsrecorder.NewRecorder("integration_main_test", l)) // mock events recorder

	// TLS is needed for ApiServer to know who is making a request (ApiGw, controller, etc)

	// instantiate a certificates server
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	if err != nil {
		log.Fatalf("Error instantiating certsrv: %v", err)
	}

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCMDBasedProvider(certSrv.GetListenURL(), svcName)
		if err != nil {
			return nil, err
		}
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)
	tinfo.certsrvurl = certSrv.GetListenURL()

	// Start the API server
	apiserverAddress := ":0"
	tinfo.l = l
	scheme := runtime.GetDefaultScheme()
	tinfo.apisrvConfig = apiserver.Config{
		GrpcServerPort: apiserverAddress,
		DebugMode:      false,
		Logger:         l,
		Version:        "v1",
		Scheme:         scheme,
		KVPoolSize:     1,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Codec:   runtime.NewJSONCodec(scheme),
			Servers: []string{"test-cluster"},
		},
		GetOverlay:       cache.GetOverlay,
		IsDryRun:         cache.IsDryRun,
		AllowMultiTenant: true,
	}

	// Start spyglass server
	fdr := startSpyglass()
	defer fdr.Stop()
	defer tinfo.esServer.Stop()

	// Start ApiServer
	trace.Init("ApiServer")
	srv := apiserverpkg.MustGetAPIServer()
	go srv.Run(tinfo.apisrvConfig)
	srv.WaitRunning()
	addr, err := srv.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		os.Exit(-1)
	}
	tinfo.cache = apiserverpkg.GetAPIServerCache()
	tinfo.apiserverport = port
	// Start the API Gateway
	tinfo.gwConfig = apigw.Config{
		HTTPAddr:  ":0",
		DebugMode: true,
		Logger:    l,
		BackendOverride: map[string]string{
			"pen-apiserver": "localhost:" + port,
			"pen-spyglass":  fdr.GetListenURL(),
		},
		SkipBackends: []string{
			"telemetry_query",
			"objstore",
			"tokenauth",
		},
		Auditor:            auditmgr.WithAuditors(auditmgr.NewLogAuditor(context.TODO(), l)),
		ModuleWatcher:      diagmock.GetModuleWatcher(),
		DiagnosticsService: diagmock.GetDiagnosticsService(),
	}
	gw := apigwpkg.MustGetAPIGateway()
	go gw.Run(tinfo.gwConfig)
	gw.WaitRunning()
	gwaddr, err := gw.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err = net.SplitHostPort(gwaddr.String())
	if err != nil {
		os.Exit(-1)
	}
	tinfo.apigwport = port

	apicl, err := client.NewGrpcUpstream("CrudOpsTest", "localhost:"+tinfo.apiserverport, tinfo.l)
	if err != nil {
		tinfo.l.Fatalf("cannot create API server client (%v)", err)
	}

	// create cluster
	testutils.MustCreateCluster(apicl)
	// create default tenant
	testutils.MustCreateTenant(apicl, globals.DefaultTenant)
	// create user
	testutils.MustCreateTestUser(apicl, testUser, testPassword, globals.DefaultTenant)
	tinfo.userCred = &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// update admin role binding
	testutils.MustUpdateRoleBinding(apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{testUser}, nil)
	// create authentication policy with local auth enabled
	testutils.MustCreateAuthenticationPolicy(apicl, &auth.Local{}, nil, nil)
	// set auth bootstrap flag to true
	testutils.MustSetAuthBootstrapFlag(apicl)

	rcode := m.Run()
	apicl.Close()
	srv.Stop()
	gw.Stop()
	os.Exit(rcode)
}
