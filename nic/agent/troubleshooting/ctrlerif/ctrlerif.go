package ctrlerif

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils/events/recorder"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// TsClient is the troubleshooting facilitator client
type TsClient struct {
	sync.Mutex
	resolverClient resolver.Interface // ??
	waitGrp        sync.WaitGroup
	tsagent        types.CtrlerIntf
	tsaGrpcClient  *rpckit.RPCClient
	watchCtx       context.Context
	watchCancel    context.CancelFunc
	stopped        bool
	//debugStats     *debug.Stats
	startTime time.Time
}

const maxRetry = 3

func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewTsClient creates Troubleshooting client object
func NewTsClient(agent types.CtrlerIntf, resolverClient resolver.Interface) (*TsClient, error) {

	watchCtx, watchCancel := context.WithCancel(context.Background())

	tsClient := TsClient{
		resolverClient: resolverClient,
		tsagent:        agent,
		watchCtx:       watchCtx,
		watchCancel:    watchCancel,
		startTime:      time.Now(),
	}

	go tsClient.runTroubleShootingWatcher(tsClient.watchCtx)

	return &tsClient, nil
}

// getAgentName returns a unique name for each agent instance
func (client *TsClient) getAgentName() string {
	return "TroubleShootingAgent-" + client.tsagent.GetAgentID()
}

func (client *TsClient) runTroubleShootingWatcher(ctx context.Context) {
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for {
		rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), globals.Tsm, rpckit.WithBalancer(balancer.New(client.resolverClient)))
		if err != nil {
			log.Errorf("Error connecting to grpc server %v. Err: %v", globals.Tsm, err)
			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.tsaGrpcClient = rpcClient

		// start watch
		tsaRPCClient := tsproto.NewMirrorSessionApiClient(rpcClient.ClientConn)
		stream, err := tsaRPCClient.WatchMirrorSessions(ctx, &api.ObjectMeta{})

		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching network: Err: %v", err)
			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}

		for {
			evtList, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting trouble shooting watch. Err: %v", err)
				if client.isStopped() {
					rpcClient.Close()
					return
				}
				time.Sleep(time.Second)
				break
			}

			log.Infof("CtrlerIf: agent %s got mirror session watch eventlist: {%+v}", client.getAgentName(), evtList)

			func() {
				for _, evt := range evtList.MirrorSessionEvents {
					for iter := 0; iter < maxRetry; iter++ {
						var err error

						switch evt.EventType {
						case api.EventType_CreateEvent:
							err = client.tsagent.CreateMirrorSession(&evt.MirrorSession)
						case api.EventType_UpdateEvent:
							err = client.tsagent.UpdateMirrorSession(&evt.MirrorSession)
						case api.EventType_DeleteEvent:
							err = client.tsagent.DeleteMirrorSession(&evt.MirrorSession)
						default:
							log.Errorf("Invalid mirror event type: {%+v}", evt.EventType)
							return
						}

						if err == nil { // return on success
							return
						}

						log.Errorf("failed to apply %v %v, error: %v", evt.MirrorSession.GetName(), evt.EventType, err)
						time.Sleep(time.Second * 5)
					}

					recorder.Event(eventtypes.CONFIG_FAIL, fmt.Sprintf("Failed to %v %v %v",
						strings.Split(strings.ToLower(evt.EventType.String()), "-event")[0], evt.MirrorSession.Kind, evt.MirrorSession.Name), nil)
				}
			}()
		}
		rpcClient.Close()
	}
}

// isStopped is trouble shooting client stopped
func (client *TsClient) isStopped() bool {
	client.Lock()
	defer client.Unlock()
	return client.stopped
}

// Stop the client
func (client *TsClient) Stop() {
	client.Lock()
	defer client.Unlock()
	if client.stopped {
		return
	}
	client.watchCancel()
	client.stopped = true
}
