package rolloutclient

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"

	rolloutproto "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
)

// serviceRolloutClient implements ServiceRolloutClient interface
type serviceRolloutClient struct {
	sync.Mutex
	resolverClient        resolver.Interface
	ctx                   context.Context
	cancel                context.CancelFunc
	wg                    sync.WaitGroup
	serverURL             string
	nodeName              string
	client                rolloutproto.ServiceRolloutApiClient
	serviceRolloutHandler types.ServiceRolloutHandler
}

// NewServiceRolloutClient returns a ServiceRolloutClient struct to upgrade the current service node based on requests from rollout controller
func NewServiceRolloutClient(serverURL string, nodeName string, resolver resolver.Interface, handler types.ServiceRolloutHandler) types.ServiceRolloutClient {
	return &serviceRolloutClient{
		resolverClient:        resolver,
		serverURL:             serverURL,
		nodeName:              nodeName,
		serviceRolloutHandler: handler,
	}
}

// Start starts the rollout watcher waiting for instructions of rollout and executing them
func (v *serviceRolloutClient) Start() {
	v.ctx, v.cancel = context.WithCancel(context.Background())
	v.wg.Add(1)
	go v.run()
}

func (v *serviceRolloutClient) run() {
	defer v.wg.Done()
	v.serviceRolloutHandler.RegisterServiceStatusWriter(v)
	defer v.serviceRolloutHandler.UnregisterServiceStatusWriter(v)

	for {
		select {
		case <-v.ctx.Done():
			return
		default:
		}

		rpcClient, err := rpckit.NewRPCClient(clientName, v.serverURL,
			rpckit.WithBalancer(balancer.New(v.resolverClient)), rpckit.WithRemoteServerName(globals.Rollout))

		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)
			time.Sleep(time.Second)
			continue
		}

		v.Lock()
		v.client = rolloutproto.NewServiceRolloutApiClient(rpcClient.ClientConn)
		v.Unlock()
		meta := api.ObjectMeta{Name: v.nodeName}
		stream, err := v.client.WatchServiceRollout(v.ctx, &meta)
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching ServiceRollout: Err: %v", err)
			time.Sleep(time.Second)
			continue
		}

		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error %v receiving from watch channel. Retrying ServiceRollout watch", err)
				time.Sleep(time.Second)
				break
			}
			log.Debugf("Received %#v", evt)

			switch evt.EventType {
			case api.EventType_CreateEvent:
				v.serviceRolloutHandler.CreateServiceRollout(&evt.ServiceRollout)
			case api.EventType_UpdateEvent:
				v.serviceRolloutHandler.UpdateServiceRollout(&evt.ServiceRollout)
			case api.EventType_DeleteEvent:
				v.serviceRolloutHandler.DeleteServiceRollout(&evt.ServiceRollout)
			}
		}

		rpcClient.Close()
	}
}

func (v *serviceRolloutClient) WriteServiceStatus(ctx context.Context, s *rolloutproto.ServiceRolloutStatusUpdate) {
	var err error
	for i := numRetries; i > 0; i-- {
		v.Lock()
		if v.client != nil {
			_, err = v.client.UpdateServiceRolloutStatus(ctx, s)
		}
		v.Unlock()

		if err == nil {
			return
		}

		select {
		case <-ctx.Done():
			return
		case <-time.After(time.Second): // wait before next retry
		}

	}

	// TODO: handle this failure better
	log.Errorf("Error Writing status to Rollout controller Err: %v", err)
}

// Stop waits for completion
func (v *serviceRolloutClient) Stop() {
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
	}
	v.wg.Wait()
}
