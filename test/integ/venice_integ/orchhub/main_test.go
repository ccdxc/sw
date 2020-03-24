package orchhub

import (
	"context"
	"fmt"
	"net/url"
	"os"
	"sync"
	"testing"

	"github.com/pensando/sw/venice/utils/tsdb"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
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
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"
)

var (
	logger = log.WithContext("module", "orchhub_integ_test")

	// create mock events recorder
	eventRecorder = mockevtsrecorder.NewRecorder("orchhub_integ_test", logger)
	_             = recorder.Override(eventRecorder)
)

type tInfo struct {
	l             log.Logger
	apiServer     apiserver.Server
	apiServerAddr string
	orchHub       *orchhub.OrchCtrler
	mockResolver  *mockresolver.ResolverClient
	apicl         apiclient.Services
	vcConfig      VcConfig
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

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	// start API server
	trace.Init("ApiServer")
	tInfo.apiServer, tInfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", "OrchhubIntegTest", tInfo.l)
	if err != nil {
		return err
	}

	tInfo.updateResolver(globals.APIServer, tInfo.apiServerAddr)

	// start OrchHub
	config := log.GetDefaultConfig("OrchhubIntegTest")
	config.Filter = log.AllowAllFilter
	l := log.GetNewLogger(config)

	url := fmt.Sprintf(":%s", globals.OrchHubAPIPort)
	opts := orchhub.Opts{
		ListenURL: url,
		Resolver:  tInfo.mockResolver,
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
	tInfo.apicl.Close()
	tInfo.orchHub.Stop()
	tInfo.apiServer.Stop()
	testutils.CleanupIntegTLSProvider()
}

func TestMain(m *testing.M) {
	grpclog.SetLoggerV2(logger)
	config := log.GetDefaultConfig("OrchhubIntegTest")
	config.Filter = log.AllowInfoFilter
	l := log.GetNewLogger(config)
	tinfo.l = l
	tinfo.mockResolver = mockresolver.New()

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
	tInfo.mockResolver.AddServiceInstance(&types.ServiceInstance{
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
				AuthType: "username-password",
				UserName: user,
				Password: pass,
			},
		},
	}
	config.Status.Defaults("all")
	if len(forceDCs) != 0 {
		config.Labels = map[string]string{
			"force-dc-names": forceDCs,
		}
	}
	obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Create(context.Background(), config)
	return obj, err
}

func updateOrchConfig(name, uri, user, pass string) (*orchestration.Orchestrator, error) {
	configMeta := &api.ObjectMeta{
		Name: name,
		// Don't set Tenant as object is not scoped inside Tenant in proto file.
	}
	obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(context.Background(), configMeta)
	obj.Spec.URI = uri
	obj.Spec.Credentials = &monitoring.ExternalCred{
		AuthType: "username-password",
		UserName: user,
		Password: pass,
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
		Status: network.NetworkStatus{},
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

func cleanup() error {
	// Delete all networks, orch config, teardown sim
	opts := &api.ListWatchOptions{}
	nws, err := tinfo.apicl.NetworkV1().Network().List(context.Background(), opts)
	if err == nil {
		logger.Errorf("%s", err)
		return err
	}
	for _, nw := range nws {
		err := deleteNetwork(nw.Name)
		if err == nil {
			logger.Errorf("%s", err)
			return err
		}
	}

	orchs, err := tinfo.apicl.OrchestratorV1().Orchestrator().List(context.Background(), opts)
	if err == nil {
		logger.Errorf("%s", err)
		return err
	}
	for _, orch := range orchs {
		err := deleteOrchConfig(orch.Name)
		if err == nil {
			logger.Errorf("%s", err)
			return err
		}
	}

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

	state := defs.State{
		VcURL: u,
		VcID:  "VCProbe",
		Ctx:   ctx,
		Log:   logger.WithContext("submodule", "vcprobe"),
		Wg:    &sync.WaitGroup{},
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
