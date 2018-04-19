package auth

import (
	"context"
	"fmt"
	"net"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
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
	apiserverport string
	apigwport     string
	esServer      *esmock.ElasticServer
}

var tinfo tInfo

func startSpyglass() finder.Interface {

	// start mock elastic server
	tinfo.esServer = esmock.NewElasticServer()
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

	fdr, err := finder.NewFinder(context.Background(),
		"localhost:0",
		rsr,
		tinfo.l)
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
	// Start the API server
	apiserverAddress := ":0"
	l := log.WithContext("module", "AuthTest")
	tinfo.l = l
	scheme := runtime.NewScheme()
	srvconfig := apiserver.Config{
		GrpcServerPort: apiserverAddress,
		DebugMode:      false,
		Logger:         l,
		Version:        "v1",
		Scheme:         scheme,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Codec:   runtime.NewJSONCodec(scheme),
			Servers: []string{"test-cluster"},
		},
		KVPoolSize: 1,
	}
	grpclog.SetLogger(l)

	// Start spyglass server
	fdr := startSpyglass()
	defer fdr.Stop()
	defer tinfo.esServer.Stop()

	trace.Init("ApiServer")
	srv := apiserverpkg.MustGetAPIServer()
	go srv.Run(srvconfig)
	srv.WaitRunning()
	defer srv.Stop()
	addr, err := srv.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		os.Exit(-1)
	}

	tinfo.apiserverport = port
	// Start the API Gateway
	gwconfig := apigw.Config{
		HTTPAddr:  ":0",
		DebugMode: true,
		Logger:    l,
		BackendOverride: map[string]string{
			"pen-apiserver": "localhost:" + port,
			"pen-spyglass":  fdr.GetListenURL(),
		},
	}
	gw := apigwpkg.MustGetAPIGateway()
	go gw.Run(gwconfig)
	gw.WaitRunning()
	defer gw.Stop()
	gwaddr, err := gw.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err = net.SplitHostPort(gwaddr.String())
	if err != nil {
		os.Exit(-1)
	}
	tinfo.apigwport = port

	rcode := m.Run()

	os.Exit(rcode)
}
