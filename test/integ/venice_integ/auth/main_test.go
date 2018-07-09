package auth

import (
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	_ "github.com/pensando/sw/venice/apigw/svc"
)

const (
	testUser     = "test"
	testPassword = "pensandoo0"
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
}

var tinfo tInfo

func (tInfo *tInfo) setup(kvstoreConfig *store.Config) error {
	var err error

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
