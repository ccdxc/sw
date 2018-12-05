package watcher

import (
	"fmt"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const maxRetry = 120

// Watcher contains globals needed for creating a database on tenant creation
type Watcher struct {
	url      string
	broker   broker.Inf
	resolver resolver.Interface
}

// NewWatcher creates a citadel watcher instance
func NewWatcher(listenURL string, broker broker.Inf, resolverClient resolver.Interface) *Watcher {
	cw := &Watcher{
		url:      listenURL,
		broker:   broker,
		resolver: resolverClient,
	}
	return cw
}

// WatchTenant watches for tenant objects and creates a new database when there is new tenant
func (cw *Watcher) WatchTenant(parentCtx context.Context) {
	ctx, cancelWatch := context.WithCancel(parentCtx)
	// stop watch channel
	defer cancelWatch()
	client, err := cw.initGrpcClient(cw.url, maxRetry)
	if err != nil {
		log.Fatalf("failed to init grpc client, error: %s", err)
	}
	defer client.Close()
	log.Infof("connected to {%s}", cw.url)

	opts := api.ListWatchOptions{}
	watcher, err := client.ClusterV1().Tenant().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("failed to watch tenant, error: {%s}", err)
		return
	}
	for event := range watcher.EventChan() {
		log.Infof("received event %#v", event)
		switch event.Type {
		case kvstore.Created:
			// If DB is already created, the call will eventually nop once it propogates
			// to the datanode db create
			cw.onTenantCreate(parentCtx, event.Object.(*cluster.Tenant))
		case kvstore.Updated:
			cw.onTenantCreate(parentCtx, event.Object.(*cluster.Tenant))
		case kvstore.Deleted:
		default:
			log.Errorf("invalid tenant event, type %s  %+v", event.Type, event.Object)
		}
	}
	return
}

func (cw *Watcher) onTenantCreate(ctx context.Context, tenant *cluster.Tenant) {
	// create default db
	if err := cw.broker.CreateDatabase(ctx, tenant.GetName()); err != nil {
		log.Fatalf("failed to create db %s : %s", tenant.GetName(), err)
	}
	log.Infof("created db %s", tenant.GetName())
}

// init grpc client
func (cw *Watcher) initGrpcClient(serviceName string, retry int) (apiclient.Services, error) {
	for i := 0; i < retry; i++ {
		// create a grpc client
		l := log.GetNewLogger(log.GetDefaultConfig(globals.Citadel))
		client, apiErr := apiclient.NewGrpcAPIClient(globals.Citadel, serviceName, l, rpckit.WithBalancer(balancer.New(cw.resolver)))
		if apiErr == nil {
			return client, nil
		}
		log.Warnf("failed to connect to {%s}, error: %s, retry", serviceName, apiErr)
		time.Sleep(2 * time.Second)
	}
	return nil, fmt.Errorf("failed to connect to {%s}, exhausted all attempts(%d)", serviceName, retry)
}
