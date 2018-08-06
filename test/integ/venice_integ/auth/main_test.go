package auth

import (
	"fmt"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/utils"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
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
	certPath  = "../../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../../venice/utils/certmgr/testdata/roots.pem"
	// users
	testUser     = "test"
	testPassword = "pensandoo0"
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(
		&evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "auth_integ_test"},
		evtsapi.GetEventTypes(), "", "/tmp")
)

type tInfo struct {
	l             log.Logger
	apiServer     apiserver.Server
	apiServerAddr string
	apiGw         apigw.APIGateway
	apiGwAddr     string
	certSrv       *certsrv.CertSrv
	esServer      *esmock.ElasticServer
	mockResolver  *mockresolver.ResolverClient
	fdr           finder.Interface
	fdrAddr       string
}

var tinfo tInfo

func (tInfo *tInfo) setup(kvstoreConfig *store.Config) error {
	var err error

	// start certificate server
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	if err != nil {
		return fmt.Errorf("Error starting certificates server: %v", err)
	}
	tInfo.certSrv = certSrv

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

	// start mock elastic server
	tinfo.esServer = esmock.NewElasticServer()
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
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", tInfo.mockResolver, tInfo.l)
	if err != nil {
		return err
	}
	tInfo.fdr = fdr.(finder.Interface)
	tInfo.fdrAddr = fdrAddr

	// start API server
	trace.Init("ApiServer")
	tInfo.apiServer, tInfo.apiServerAddr, err = testutils.StartAPIServer(":0", kvstoreConfig, tInfo.l)
	if err != nil {
		return err
	}

	// start API gateway
	tInfo.apiGw, tInfo.apiGwAddr, err = testutils.StartAPIGateway(":0",
		map[string]string{globals.APIServer: tInfo.apiServerAddr, globals.Spyglass: tInfo.fdrAddr},
		[]string{}, tInfo.l)
	if err != nil {
		return err
	}

	return nil
}

func (tInfo *tInfo) teardown() {
	tinfo.esServer.Stop()
	tInfo.fdr.Stop()
	tInfo.apiServer.Stop()
	tInfo.apiGw.Stop()
	tInfo.certSrv.Stop()
}

func TestMain(m *testing.M) {
	l := log.WithContext("module", "AuthTest")
	tinfo.l = l
	tinfo.mockResolver = mockresolver.New()
	grpclog.SetLogger(l)

	if err := tinfo.setup(&store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme()),
		Servers: []string{"test-cluster"},
	}); err != nil {
		log.Fatalf("failed to setup test, err: %v", err)
	}

	defer tinfo.teardown()

	rcode := m.Run()

	os.Exit(rcode)
}
