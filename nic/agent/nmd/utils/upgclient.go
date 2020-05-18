package utils

import (
	"math"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	pdsGRPCDefaultPort    = "11358" //PDS_GRPC_PORT_UPGMGR
	pdsGRPCDefaultBaseURL = "localhost"
)

// CreateUPGMGRNewGRPCClient creates a grpc connection to UPGMGR
func CreateUPGMGRNewGRPCClient() (*grpc.ClientConn, error) {

	srvURL := pdsGRPCDefaultBaseURL + ":" + pdsGRPCDefaultPort
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
