package cache

import (
	apiclient "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type options struct {
	devMode bool
}

// WithSetDevMode sets the DevMode for the client. true by default.
func WithSetDevMode(val bool) Option {
	return func(o *options) {
		o.devMode = val
	}
}

// Option is a options modifier for the client.
type Option func(opt *options)

// NewGrpcUpstream creates a cache with a gRPC Upstream
func NewGrpcUpstream(url string, logger log.Logger, opts ...Option) (apiclient.Services, error) {
	o := options{
		devMode: true,
	}

	for _, op := range opts {
		if op != nil {
			op(&o)
		}
	}
	var rpcopts []rpckit.Option
	if !o.devMode {
		rpcopts = append(rpcopts, rpckit.WithTracerEnabled(false))
		rpcopts = append(rpcopts, rpckit.WithLoggerEnabled(false))
		rpcopts = append(rpcopts, rpckit.WithStatsEnabled(false))
	}
	return apiclient.NewGrpcAPIClient(url, logger, rpcopts...)
}
