package cache

import (
	"google.golang.org/grpc"

	apiclient "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/utils/log"
)

// NewGrpcUpstream creates a cache with a gRPC Upstream
func NewGrpcUpstream(url string, logger log.Logger, opts ...grpc.DialOption) (apiclient.Services, error) {
	return apiclient.NewGrpcAPIClient(url, logger, opts...)
}
