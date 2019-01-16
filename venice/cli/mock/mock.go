package mock

import (
	"context"
	"net"
	"os"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/client"
	apiclient "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	"github.com/pensando/sw/venice/utils/trace"

	// import various packages natively
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
	testPassword = "pensando"
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "cli"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

// Info struct defines mock server's context
type Info struct {
	l             log.Logger
	apiserverport string
	VenicePort    string
	cache         apiintf.CacheInterface
	esServer      *esmock.ElasticServer
	certsrvurl    string
	apicl         apiclient.Services
	apisvc        apiserver.Server
	apigwsvc      apigw.APIGateway
	UserCred      *auth.PasswordCredential
}

// Start starts a mock server
func Start() *Info {
	tinfo := &Info{}

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
	l := log.WithContext("module", "cli")
	tinfo.l = l
	scheme := runtime.GetDefaultScheme()
	srvconfig := apiserver.Config{
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
		GetOverlay: cache.GetOverlay,
		IsDryRun:   cache.IsDryRun,
	}
	grpclog.SetLogger(l)

	// Start ApiServer
	trace.Init("ApiServer")
	tinfo.apisvc = apiserverpkg.MustGetAPIServer()
	go tinfo.apisvc.Run(srvconfig)
	tinfo.apisvc.WaitRunning()
	addr, err := tinfo.apisvc.GetAddr()
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
	gwconfig := apigw.Config{
		HTTPAddr:  ":0",
		DebugMode: true,
		Logger:    l,
		BackendOverride: map[string]string{
			"pen-apiserver": "localhost:" + port,
		},
		SkipBackends: []string{
			"metrics_query",
			"search",
			"events",
			"audit",
		},
		Auditor: auditmgr.WithAuditors(auditmgr.NewLogAuditor(context.TODO(), l)),
	}
	tinfo.apigwsvc = apigwpkg.MustGetAPIGateway()
	go tinfo.apigwsvc.Run(gwconfig)
	tinfo.apigwsvc.WaitRunning()
	gwaddr, err := tinfo.apigwsvc.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err = net.SplitHostPort(gwaddr.String())
	if err != nil {
		os.Exit(-1)
	}
	tinfo.VenicePort = port

	tinfo.apicl, err = client.NewGrpcUpstream("cli", "localhost:"+tinfo.apiserverport, tinfo.l)
	if err != nil {
		tinfo.l.Fatalf("cannot create API server client (%v)", err)
	}

	// create cluster
	testutils.MustCreateCluster(tinfo.apicl)
	// create default tenant
	testutils.MustCreateTenant(tinfo.apicl, globals.DefaultTenant)
	// create user
	testutils.MustCreateTestUser(tinfo.apicl, testUser, testPassword, globals.DefaultTenant)
	tinfo.UserCred = &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create admin role binding
	testutils.MustCreateRoleBinding(tinfo.apicl, "AdminRoleBinding", globals.DefaultTenant, globals.AdminRole, []string{testUser}, nil)
	// create authentication policy with local auth enabled
	testutils.MustCreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false})
	// set auth bootstrap flag to true
	testutils.MustSetAuthBootstrapFlag(tinfo.apicl)

	return tinfo
}

// Stop stops the mock server for the context
func (tinfo *Info) Stop() {
	tinfo.apicl.Close()
	tinfo.apisvc.Stop()
	tinfo.apigwsvc.Stop()
}
