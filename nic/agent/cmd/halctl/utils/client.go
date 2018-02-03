package utils

import (
	"os"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halGRPCDefaultPort    = "50054"
	halGRPCDefaultBaseURL = "localhost"
)

// CreateNewGRPCClient creates a grpc connection to HAL
func CreateNewGRPCClient() (*rpckit.RPCClient, error) {

	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = halGRPCDefaultPort
	}
	srvURL := halGRPCDefaultBaseURL + ":" + halPort
	// create a grpc client
	// ToDo Use TLS Provider
	rpcClient, err := rpckit.NewRPCClient("halctl", srvURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Creating gRPC Client failed. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}
