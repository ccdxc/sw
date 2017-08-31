package cache

import (
	apiclient "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/utils/log"
)

// NewGrpcUpstream creates a cache with a gRPC Upstream
func NewGrpcUpstream(url string, logger log.Logger) (apiclient.Services, error) {
	return apiclient.NewGrpcAPIClient(url, logger)
}
