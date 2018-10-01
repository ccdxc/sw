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

// veniceRolloutClient implements VeniceRolloutClient interface
type veniceRolloutClient struct {
	sync.Mutex
	resolverClient       resolver.Interface
	ctx                  context.Context
	cancel               context.CancelFunc
	wg                   sync.WaitGroup
	serverURL            string
	nodeName             string
	client               rolloutproto.VeniceRolloutApiClient
	veniceRolloutHandler types.VeniceRolloutHandler
}

const clientName = "cmd"
const numRetries = 5

// NewVeniceRolloutClient returns a VeniceRolloutClient struct to upgrade the current venice node based on requests from rollout controller
func NewVeniceRolloutClient(serverURL string, nodeName string, resolver resolver.Interface, handler types.VeniceRolloutHandler) types.VeniceRolloutClient {
	return &veniceRolloutClient{
		resolverClient:       resolver,
		serverURL:            serverURL,
		nodeName:             nodeName,
		veniceRolloutHandler: handler,
	}
}

// Start starts the rollout watcher waiting for instructions of rollout and executing them
func (v *veniceRolloutClient) Start() {
	v.ctx, v.cancel = context.WithCancel(context.Background())
	v.wg.Add(1)
	go v.run()
}

func (v *veniceRolloutClient) run() {
	defer v.wg.Done()
	v.veniceRolloutHandler.RegisterVeniceStatusWriter(v)
	defer v.veniceRolloutHandler.UnregisterVeniceStatusWriter(v)

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
		v.client = rolloutproto.NewVeniceRolloutApiClient(rpcClient.ClientConn)
		v.Unlock()
		meta := api.ObjectMeta{Name: v.nodeName}
		stream, err := v.client.WatchVeniceRollout(v.ctx, &meta)
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching VeniceRollout: Err: %v", err)
			time.Sleep(time.Second)
			continue
		}

		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error %v receiving from watch channel. Retrying VeniceRollout watch", err)
				time.Sleep(time.Second)
				break
			}
			log.Debugf("Received %#v", evt)

			switch evt.EventType {
			case api.EventType_CreateEvent:
				v.veniceRolloutHandler.CreateVeniceRollout(&evt.VeniceRollout)
			case api.EventType_UpdateEvent:
				v.veniceRolloutHandler.UpdateVeniceRollout(&evt.VeniceRollout)
			case api.EventType_DeleteEvent:
				v.veniceRolloutHandler.DeleteVeniceRollout(&evt.VeniceRollout)
			}
		}

		rpcClient.Close()
	}
}

func (v *veniceRolloutClient) WriteStatus(ctx context.Context, s *rolloutproto.VeniceRolloutStatusUpdate) {
	var err error
	for i := numRetries; i > 0; i-- {
		v.Lock()
		if v.client != nil {
			_, err = v.client.UpdateVeniceRolloutStatus(ctx, s)
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
func (v *veniceRolloutClient) Stop() {
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
	}
	v.wg.Wait()
}
