package integration

import (
	"net"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks"
)

type tInfo struct {
	l             log.Logger
	apiserverport string
	apigwport     string
}

var tinfo tInfo

func TestMain(m *testing.M) {
	// Start the API server
	apiserverAddress := ":0"
	l := log.WithContext("module", "CrudOpsTest")
	tinfo.l = l
	scheme := runtime.NewScheme()
	srvconfig := apiserver.Config{
		GrpcServerPort: apiserverAddress,
		DebugMode:      false,
		Logger:         l,
		Version:        "v1",
		Scheme:         scheme,
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(scheme),
		},
	}
	grpclog.SetLogger(l)
	trace.Init("ApiServer")
	srv := apiserverpkg.MustGetAPIServer()
	go srv.Run(srvconfig)
	srv.WaitRunning()
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
		HTTPAddr:          ":0",
		DebugMode:         true,
		Logger:            l,
		APIServerOverride: "localhost:" + port,
	}
	gw := apigwpkg.MustGetAPIGateway()
	go gw.Run(gwconfig)
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

	rcode := m.Run()
	srv.Stop()
	gw.Stop()
	os.Exit(rcode)
}
