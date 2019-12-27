package test

import (
	"context"
	"net"
	"os"
	"testing"
	"time"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/auth"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/startup"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
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
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
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

func TestMain(m *testing.M) {
	l := log.WithContext("module", "CrudOpsTest")
	grpclog.SetLoggerV2(l)
	recorder.Override(mockevtsrecorder.NewRecorder("perseus_test", l)) // mock events recorder

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
		},
		SkipBackends: []string{
			"telemetry_query",
			"objstore",
			"tokenauth",
			"search",
			"events",
			"audit",
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
	testutils.MustCreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false})
	// set auth bootstrap flag to true
	testutils.MustSetAuthBootstrapFlag(apicl)

	// start perseus
	env.Options = options.NewServerRunOptions()
	env.Logger = l
	s := []string{"localhost:" + tinfo.apiserverport}

	startup.OnStart(s)
	time.Sleep(10 * time.Second)

	rcode := m.Run()
	apicl.Close()
	srv.Stop()
	gw.Stop()
	os.Exit(rcode)
}
