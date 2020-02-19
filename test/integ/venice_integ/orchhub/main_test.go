package orchhub

import (
	"context"
	"fmt"
	"net/url"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/orchestration"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/orchhub"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
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
	url := fmt.Sprintf(":%s", globals.OrchHubAPIPort)
	opts := orchhub.Opts{
		ListenURL: url,
		Resolver:  tInfo.mockResolver,
		Logger:    tInfo.l.WithContext("submodule", "pen-orchhub"),
	}
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

	return nil
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

func createOrchConfig(name, uri, user, pass string) (*orchestration.Orchestrator, error) {
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
