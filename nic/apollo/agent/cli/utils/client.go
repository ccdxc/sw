package utils

import (
	"math"
	"os"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	pdsGRPCDefaultPort    = "50054"
	pdsGRPCDefaultBaseURL = "localhost"
)

// CreateNewGRPCClient creates a grpc connection to HAL
func CreateNewGRPCClient() (*grpc.ClientConn, error) {

	pdsPort := os.Getenv("HAL_GRPC_PORT")
	if pdsPort == "" {
		pdsPort = pdsGRPCDefaultPort
	}
	srvURL := pdsGRPCDefaultBaseURL + ":" + pdsPort
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(math.MaxInt32-1))
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	rpcClient, err := grpc.Dial(srvURL, grpcOpts...)

	if err != nil {
		log.Errorf("Creating gRPC Client failed. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}
