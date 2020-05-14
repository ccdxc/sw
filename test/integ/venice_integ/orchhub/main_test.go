package orchhub

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"os/exec"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/tsdb"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/generated/orchestration"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	cmdMock "github.com/pensando/sw/venice/cmd/services/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/orchhub"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	objclient "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"
)

var (
	logger = log.WithContext("module", "orchhub_integ_test")
)

const (
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath       = "../../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath        = "../../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath      = "../../../../venice/utils/certmgr/testdata/roots.pem"
	minioURL       = "127.0.0.1:19001"
	minioHealthURL = "https://127.0.0.1:19001/minio/health/live"
)

type tInfo struct {
	l               log.Logger
	apiServer       apiserver.Server
	apiServerAddr   string
	orchHub         *orchhub.OrchCtrler
	apicl           apiclient.Services
	vcConfig        VcConfig
	eventRecorder   *mockevtsrecorder.Recorder
	resolverServer  *rpckit.RPCServer
	resolver        resolver.Interface
	objClient       objclient.Client
	resolverService *cmdMock.ResolverService
	authDir         string
}

type VcConfig struct {
	useRealVC bool
	uri       string
	user      string
	pass      string
	dcName    string
}

var tinfo tInfo

func (tInfo *tInfo) setup() error {
	var err error
	// create mock events recorder
	tInfo.eventRecorder = mockevtsrecorder.NewRecorder("orchhub_integ_test", logger)
	recorder.Override(tInfo.eventRecorder)

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	// Now create a mock resolver
	m := cmdMock.NewResolverService()
	tInfo.resolverService = m
	resolverHandler := service.NewRPCHandler(m)
	tInfo.resolverServer, err = rpckit.NewRPCServer(globals.Cmd, "localhost:0", rpckit.WithTracerEnabled(true))
	if err != nil {
		return err
	}
	types.RegisterServiceAPIServer(tInfo.resolverServer.GrpcServer, resolverHandler)
	tInfo.resolverServer.Start()

	// start API server
	trace.Init("ApiServer")
	tInfo.apiServer, tInfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", "OrchhubIntegTest", tInfo.l, []string{tInfo.resolverServer.GetListenURL()})
	if err != nil {
		return err
	}

	tInfo.updateResolver(globals.APIServer, tInfo.apiServerAddr)

	// start objstore
	err = startObjstore(tInfo)
	if err != nil {
		return err
	}

	// start OrchHub
	config := log.GetDefaultConfig("OrchhubIntegTest")
	config.Filter = log.AllowAllFilter
	config.LogToStdout = true
	l := log.GetNewLogger(config)

	url := fmt.Sprintf(":%s", globals.OrchHubAPIPort)
	opts := orchhub.Opts{
		ListenURL: url,
		Resolver:  tInfo.resolver,
		Logger:    l.WithContext("submodule", "pen-orchhub"),
		VCHubOpts: []vchub.Option{vchub.WithMockProbe},
	}
	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	ctrler, err := orchhub.NewOrchCtrler(opts)
	if err != nil || ctrler == nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}
	tInfo.orchHub = ctrler
	tInfo.updateResolver(globals.OrchHub, url)

	// grpc client
	apicl, err := apiclient.NewGrpcAPIClient("OrchhubIntegTest", tinfo.apiServerAddr, tinfo.l)
	if err != nil {
		log.Errorf("cannot create grpc client, Err: %v", err)
		return err
	}
	tinfo.apicl = apicl
	// Create default tenant
	err = createTenant()
	return err
}

func (tInfo *tInfo) teardown() {
	// Remove objStore container
	cmd := []string{
		"rm", "-f", "objstore",
	}
	_, err := exec.Command("docker", cmd...).CombinedOutput()
	if err != nil {
		tInfo.l.Errorf("Failed to terminate objStore container %s", err)
	}

	os.RemoveAll(tInfo.authDir)

	tInfo.apicl.Close()
	tInfo.orchHub.Stop()
	tInfo.apiServer.Stop()
	tInfo.resolverServer.Stop()
	testutils.CleanupIntegTLSProvider()
}

func TestMain(m *testing.M) {
	grpclog.SetLoggerV2(logger)
	config := log.GetDefaultConfig("OrchhubIntegTest")
	config.Filter = log.AllowInfoFilter
	l := log.GetNewLogger(config)
	tinfo.l = l

	tinfo.vcConfig = VcConfig{
		// useRealVC: true,
		// uri:  "barun-vc.pensando.io",
		// user: "administrator@pensando.io",
		// pass: "N0isystem$",
		uri:  "127.0.0.1:8989",
		user: "user",
		pass: "pass",
	}

	if err := tinfo.setup(); err != nil {
		log.Fatalf("failed to setup test, err: %v", err)
	}

	ret := m.Run()
	tinfo.teardown()
	os.Exit(ret)
}

func (tInfo *tInfo) updateResolver(serviceName, url string) {
	tInfo.resolverService.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		Node:    "localhost",
		URL:     url,
	})
}

func createOrchConfig(name, uri, user, pass, forceDCs string) (*orchestration.Orchestrator, error) {
	config := &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name: name,
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Orchestrator",
		},
		Spec: orchestration.OrchestratorSpec{
			Type: "vcenter",
			URI:  uri,
			Credentials: &monitoring.ExternalCred{
				AuthType:                    "username-password",
				UserName:                    user,
				Password:                    pass,
				DisableServerAuthentication: true,
			},
		},
	}
	config.Status.Defaults("all")
	if len(forceDCs) != 0 {
		config.Spec.ManageNamespaces = strings.Split(forceDCs, ",")
	}
	obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Create(context.Background(), config)
	return obj, err
}

func updateOrchConfig(name, uri, user, pass, forceDCs string) (*orchestration.Orchestrator, error) {
	configMeta := &api.ObjectMeta{
		Name: name,
		// Don't set Tenant as object is not scoped inside Tenant in proto file.
	}
	obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(context.Background(), configMeta)
	obj.Spec.URI = uri
	obj.Spec.Credentials = &monitoring.ExternalCred{
		AuthType:                    "username-password",
		UserName:                    user,
		Password:                    pass,
		DisableServerAuthentication: true,
	}
	if len(forceDCs) != 0 {
		obj.Spec.ManageNamespaces = strings.Split(forceDCs, ",")
	} else {
		obj.Spec.ManageNamespaces = []string{}
	}

	obj, err = tinfo.apicl.OrchestratorV1().Orchestrator().Update(context.Background(), obj)
	return obj, err
}

func deleteOrchConfig(name string) error {
	config := &api.ObjectMeta{
		Name: name,
		// Don't set Tenant as object is not scoped inside Tenant in proto file.
	}
	_, err := tinfo.apicl.OrchestratorV1().Orchestrator().Delete(context.Background(), config)
	return err
}

func debugSyncOrch(vcKey string) error {
	params := map[string]string{
		"key": vcKey,
	}
	_, err := tinfo.orchHub.InstanceMgr.Debug(vchub.DebugSync, params)
	return err
}

func createNetwork(name string, vlan int, orchInfo []*network.OrchestratorInfo) (*network.Network, error) {
	obj := &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Bridged.String(),
			VlanID:        uint32(vlan),
			Orchestrators: orchInfo,
		},
		Status: network.NetworkStatus{
			OperState: network.OperState_Active.String(),
		},
	}
	obj, err := tinfo.apicl.NetworkV1().Network().Create(context.Background(), obj)
	return obj, err
}

func deleteNetwork(name string) error {
	config := &api.ObjectMeta{
		Name:      name,
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	_, err := tinfo.apicl.NetworkV1().Network().Delete(context.Background(), config)
	return err
}

func deleteAllNetworks() error {
	networks, err := tinfo.apicl.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		tinfo.l.Errorf("Failed to list networks, %s", err)
		return err
	}
	for _, n := range networks {
		_, err = tinfo.apicl.NetworkV1().Network().Delete(context.Background(), n.GetObjectMeta())
		tinfo.l.Errorf("Failed to delete %s, err %s", n.Name, err)
	}
	return nil
}

func cleanup() error {
	// Delete all networks, orch config, teardown sim
	opts := &api.ListWatchOptions{}
	deleteAllNetworks()

	orchs, err := tinfo.apicl.OrchestratorV1().Orchestrator().List(context.Background(), opts)
	if err != nil {
		logger.Errorf("Failed to list orchs: %s", err)
		return err
	}
	for _, orch := range orchs {
		err := deleteOrchConfig(orch.Name)
		if err != nil {
			logger.Errorf("Deleting orch config gave err %s", err)
		}
	}

	// Give time for orchhub to process
	time.Sleep(5 * time.Second)

	return nil
}

func startObjstore(tinfo *tInfo) error {
	caKey, err := certs.ReadPrivateKey(keyPath)
	if err != nil {
		return err
	}
	caCert, err := certs.ReadCertificate(certPath)
	if err != nil {
		return err
	}
	trustRoots, err := certs.ReadCertificates(rootsPath)
	if err != nil {
		return err
	}
	tinfo.authDir, err = ioutil.TempDir("/tmp", "objstore_test")
	if err != nil {
		return err
	}
	csrSigner := func(csr *x509.CertificateRequest, opts ...certs.Option) (*x509.Certificate, error) {
		return certs.SignCSRwithCA(csr, caCert, caKey, certs.WithValidityDays(1))
	}
	err = credentials.GenVosHTTPSAuth("localhost", tinfo.authDir, csrSigner, append([]*x509.Certificate{caCert}, trustRoots[0]), trustRoots)
	if err != nil {
		return err
	}

	// start objstore
	cmd := []string{
		"run",
		"--rm",
		"-d",
		"-p",
		"19001:19001",
		"-e",
		"MINIO_ACCESS_KEY=miniokey",
		"-e",
		"MINIO_SECRET_KEY=minio0523",
		"-v",
		fmt.Sprintf("%s:/root/.minio/certs", tinfo.authDir),
		"--name",
		"objstore",
		"registry.test.pensando.io:5000/objstore:v0.2",
		"server",
		"/data",
	}
	_, err = exec.Command("docker", cmd...).CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to start objstore, %s", err)
	}

	client := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				RootCAs:    certs.NewCertPool(trustRoots),
				ServerName: globals.Vos,
			},
		},
	}

	// check health
	var s *http.Response
	for i := 0; i < 6; i++ {
		s, err = client.Get(minioHealthURL)
		if err == nil && s.StatusCode == http.StatusOK {
			s.Body.Close()
			break
		}

		time.Sleep(5 * time.Second)
	}

	if err != nil || s.StatusCode != http.StatusOK {
		return err
	}

	tinfo.updateResolver(globals.VosMinio, minioURL)

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.APIServer)
	if err != nil {
		return fmt.Errorf("[%v]error getting tls provider", err)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.APIServer)
	if err != nil {
		return fmt.Errorf("[%v]error getting tls client", err)
	}
	tlsc.ServerName = globals.Vos

	tinfo.resolver = resolver.New(&resolver.Config{Name: globals.APIServer, Servers: []string{tinfo.resolverServer.GetListenURL()}})

	oclnt, err := objclient.NewClient(globals.DefaultTenant, objstore.Buckets_snapshots.String(), tinfo.resolver, objclient.WithTLSConfig(tlsc))
	if err != nil {
		return err
	}
	tinfo.objClient = oclnt
	return nil
}

func startVCSim(uri, user, pass string) (*sim.VcSim, error) {
	u := &url.URL{
		Scheme: "https",
		Host:   uri,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, pass)
	sim, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	return sim, err
}

func createProbe(ctx context.Context, uri, user, pass string) *mock.ProbeMock {
	vcReadCh := make(chan defs.Probe2StoreMsg, 1000)
	vcEventCh := make(chan defs.Probe2StoreMsg, 1000)
	u := &url.URL{
		Scheme: "https",
		Host:   uri,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, pass)

	// Create a dummy orch config (needed for login)
	config := &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name: "TestOrch",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Orchestrator",
		},
		Spec: orchestration.OrchestratorSpec{
			Type: "vcenter",
			URI:  uri,
			Credentials: &monitoring.ExternalCred{
				AuthType:                    "username-password",
				UserName:                    user,
				Password:                    pass,
				DisableServerAuthentication: true,
			},
		},
	}
	state := defs.State{
		VcURL: u,
		VcID:  "VCProbe",
		Ctx:   ctx,
		Log:   logger.WithContext("submodule", "vcprobe"),
		Wg:    &sync.WaitGroup{},
		// Create OrchCofig with some name .. does not mapper
		OrchConfig: config,
	}
	vcp := vcprobe.NewVCProbe(vcReadCh, vcEventCh, &state)
	mockProbe := mock.NewProbeMock(vcp)
	mockProbe.Started = true
	mockProbe.Wg.Add(1)
	go mockProbe.PeriodicSessionCheck(mockProbe.Wg)

	return mockProbe
}

func createTenant() error {
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: "default",
		},
		Spec: cluster.TenantSpec{},
	}
	_, err := tinfo.apicl.ClusterV1().Tenant().Create(context.Background(), tenant)
	return err
}
