package archive

import (
	"context"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// getter is a singleton that implements Getter interface
var gGetter *getter
var getterOnce sync.Once

type getter struct {
	sync.RWMutex
	apiServer string
	rslvr     resolver.Interface
	logger    log.Logger
	apicl     apiclient.Services
}

func (g *getter) GetArchiveRequest(ometa *api.ObjectMeta) (*monitoring.ArchiveRequest, error) {
	if g.apicl == nil {
		if err := g.refreshAPIClient(); err != nil {
			return nil, err
		}
	}
	g.RLock()
	modObj, err := g.apicl.MonitoringV1().ArchiveRequest().Get(context.Background(), ometa)
	g.RUnlock()
	if err != nil {
		// refresh API client in case underlying grpc connections have gone stale
		g.refreshAPIClient()
		return nil, err
	}
	return modObj, nil
}

func (g *getter) refreshAPIClient() error {
	defer g.Unlock()
	g.Lock()
	// close existing client
	if g.apicl != nil {
		g.apicl.Close()
	}
	var grpcOpts []rpckit.Option
	var b balancer.Balancer
	if g.rslvr != nil {
		b = balancer.New(g.rslvr)
		grpcOpts = append(grpcOpts, rpckit.WithBalancer(b))
	}
	apicl, err := apiclient.NewGrpcAPIClient(globals.APIGw, g.apiServer, g.logger, grpcOpts...)
	if err != nil {
		if b != nil {
			b.Close()
		}
		return err
	}
	g.apicl = apicl
	return nil
}

// NewGetter returns the archive.Getter implementation TODO: Can have a cache based implementation in future
func NewGetter(apiServer string, rslvr resolver.Interface, logger log.Logger) Getter {
	if gGetter != nil {
		gGetter.apiServer = apiServer
		gGetter.rslvr = rslvr
		gGetter.logger = logger
	}
	getterOnce.Do(func() {
		gGetter = &getter{
			apiServer: apiServer,
			rslvr:     rslvr,
			logger:    logger,
		}
		gGetter.refreshAPIClient()
	})
	return gGetter
}
