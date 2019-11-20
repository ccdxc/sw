package mock

import (
	"context"
	"net"
	"os"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/globals"
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

	// import various packages natively
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apigw"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	_ "github.com/pensando/sw/venice/apigw/svc"

	// added on top
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	//"github.com/pensando/sw/api/generated/workload"
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

var (
	logger = log.WithContext("module", "cli")

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("mock_cli", logger))
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
	tinfo.l = logger
	scheme := runtime.GetDefaultScheme()
	srvconfig := apiserver.Config{
		GrpcServerPort: apiserverAddress,
		DebugMode:      false,
		Logger:         logger,
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
	grpclog.SetLoggerV2(logger)

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
		Logger:    logger,
		BackendOverride: map[string]string{
			"pen-apiserver": "localhost:" + port,
		},
		SkipBackends: []string{
			"telemetry_query",
			"search",
			"events",
			"audit",
			"objstore",
			"tokenauth",
		},
		Auditor:            auditmgr.WithAuditors(auditmgr.NewLogAuditor(context.TODO(), logger)),
		ModuleWatcher:      diagmock.GetModuleWatcher(),
		DiagnosticsService: diagmock.GetDiagnosticsService(),
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
	// update admin role binding
	testutils.MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{testUser}, nil)
	// create authentication policy with local auth enabled
	testutils.MustCreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false})
	// set auth bootstrap flag to true
	testutils.MustSetAuthBootstrapFlag(tinfo.apicl)

	tinfo.createMiscObjects()

	return tinfo
}

// Stop stops the mock server for the context
func (tinfo *Info) Stop() {
	tinfo.apicl.Close()
	tinfo.apisvc.Stop()
	tinfo.apigwsvc.Stop()
}

// create miscObjects create objects in the mock backend because POST is not allowed on certain objects
func (tinfo *Info) createMiscObjects() {

	// SmartNICs
	for i := 0; i < 5; i++ {
		// smartNic params
		snic := cluster.DistributedServiceCard{
			TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
			ObjectMeta: api.ObjectMeta{
				Name: fmt.Sprintf("snic-host-%d", i),
			},
			Spec: cluster.DistributedServiceCardSpec{
				MgmtMode:    "NETWORK",
				NetworkMode: "OOB",
			},
			Status: cluster.DistributedServiceCardStatus{
				AdmissionPhase: "ADMITTED",
				PrimaryMAC:     fmt.Sprintf("00ae.dd%x%x.%x%x%x%x", i, i, i, i, i, i),
			},
		}

		_, err := tinfo.apicl.ClusterV1().DistributedServiceCard().Create(context.Background(), &snic)
		if err != nil {
			tinfo.l.Fatalf("unable to create smartnic object: err %s,\n%+v", err, snic)
		}
	}

	/*
		// Endpoints
		for i := 0; i < 10; i++ {
			ep := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("ep-%d", i),
					Tenant:    "default",
					Namespace: "default",
				},
				Spec: workload.EndpointSpec{},
				Status: workload.EndpointStatus{
					//WorkloadName:       fmt.Sprintf("workload-%d", i),
					Network:            "subnet-31-31",
					HomingHostName:     fmt.Sprintf("esx-lab2-1%d", i),
					IPv4Address:        fmt.Sprintf("31.31.0.%d", i),
					IPv4Gateway:        "31.31.0.254",
					MacAddress:         fmt.Sprintf("00ae.cd%x%x.%x%x%x%x", i, i, i, i, i, i),
					EndpointState:      "up",
					SecurityGroups:     []string{"core-svcs", "lab-22"},
					MicroSegmentVlan:   111,
					WorkloadAttributes: map[string]string{"os": "linux", "version": "rhel7.5"},
				},
			}
			_, err := tinfo.apicl.WorkloadV1().Endpoint().Create(context.Background(), &ep)
			if err != nil {
				tinfo.l.Fatalf("unable to create endpoint object: err %s,\n%+v", err, ep)
			}
		}
	*/
}
