package integration

import (
	"bytes"
	"net"
	"os"
	"testing"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/apigw"
	apigwpkg "github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/apiserver"
	apiserverpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pensando/sw/utils/trace"

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
	buf := &bytes.Buffer{}
	config := log.GetDefaultConfig("GrpcClientExample")
	l := log.GetNewLogger(config).SetOutput(buf)
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
	_, port, err := net.SplitHostPort(addr.String())
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
	addr, err = gw.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err = net.SplitHostPort(addr.String())
	if err != nil {
		os.Exit(-1)
	}
	tinfo.apigwport = port

	rcode := m.Run()
	srv.Stop()
	gw.Stop()
	os.Exit(rcode)
}
