package serviceutils

import (
	"fmt"
	"net"

	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/runtime"
)

// StartAPIServer helper function to start API server
func StartAPIServer(serverAddr string, logger log.Logger) (apiserver.Server, string, error) {
	log.Info("starting API server ...")
	if utils.IsEmpty(serverAddr) {
		// find an available port
		apiServerListener := netutils.TestListenAddr{}
		if err := apiServerListener.GetAvailablePort(); err != nil {
			log.Errorf("could not find a port to run API server")
			return nil, "", err
		}
		serverAddr = apiServerListener.ListenURL.String()
	}

	// api server config
	sch := runtime.GetDefaultScheme()
	apiServerConfig := apiserver.Config{
		GrpcServerPort: serverAddr,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme()),
			Servers: []string{"test-cluster"},
		},
		GetOverlay: cache.GetOverlay,
		IsDryRun:   cache.IsDryRun,
	}

	// create api server
	apiServer := apisrvpkg.MustGetAPIServer()
	go apiServer.Run(apiServerConfig)
	apiServer.WaitRunning()

	apiServerAddr, err := apiServer.GetAddr()
	if err != nil {
		return nil, "", fmt.Errorf("failed to get API server addr, err: %v", err)
	}
	port, err := getPortFromAddr(apiServerAddr)
	if err != nil {
		return nil, "", err
	}

	localAddr := fmt.Sprintf("localhost:%s", port)
	log.Infof("API server running on %v", localAddr)
	return apiServer, localAddr, nil
}

// helper function to parse the port from given address <ip:port>
func getPortFromAddr(addr string) (string, error) {
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		return "", fmt.Errorf("failed to parse API server addr, err: %v", err)
	}

	return port, nil
}
