package ctrlerif

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils/events/recorder"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
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

var syncInterval = time.Minute * 5

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
		log.Infof("connected to %v", globals.Tsm)

		// start watch
		tsaRPCClient := netproto.NewMirrorSessionApiV1Client(rpcClient.ClientConn)
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

		mc := make(chan *netproto.MirrorSessionEventList)
		go func() {
			defer close(mc)
			for {
				evtList, err := stream.Recv()
				if err != nil {
					log.Errorf("Error receiving from watch channel. Exiting trouble shooting watch. Err: %v", err)
					if rpcClient != nil {
						rpcClient.Close()
					}
					return
				}
				mc <- evtList
			}
		}()

	loop:
		for {
			select {
			case evtList, ok := <-mc:
				if !ok { // channel closed
					log.Errorf("exit processing mirror policy")
					time.Sleep(time.Second)
					break loop
				}
				log.Infof("CtrlerIf: agent %s got %v mirror sessions", client.getAgentName(), len(evtList.MirrorSessionEvents))

				for _, evt := range evtList.MirrorSessionEvents {
					func() {
						log.Infof("CtrlerIf: agent %s got mirror session : {%+v}", client.getAgentName(), evt)
						for iter := 0; iter < maxRetry; iter++ {
							var err error

							switch evt.EventType {
							case api.EventType_CreateEvent:
								if err = client.tsagent.CreateMirrorSession(evt.MirrorSession); err != nil {
									if strings.Contains(err.Error(), "already exists") {
										err = client.tsagent.UpdateMirrorSession(evt.MirrorSession)
									}
								}
							case api.EventType_UpdateEvent:
								if err = client.tsagent.UpdateMirrorSession(evt.MirrorSession); err != nil {
									if strings.Contains(err.Error(), "does not exist") {
										err = client.tsagent.CreateMirrorSession(evt.MirrorSession)
									}
								}
							case api.EventType_DeleteEvent:
								if err = client.tsagent.DeleteMirrorSession(evt.MirrorSession); err != nil {
									if strings.Contains(err.Error(), "does not exist") {
										err = nil
									}
								}
							default:
								log.Errorf("Invalid mirror event type: {%+v}", evt.EventType)
								return
							}

							if err != nil {
								// Raise a config event
							}

							if err == nil { // return on success
								return
							}

							log.Errorf("failed to apply %v %v, error: %v", evt.MirrorSession.GetName(), evt.EventType, err)
							time.Sleep(time.Second * 5)
						}

						recorder.Event(eventtypes.CONFIG_FAIL, fmt.Sprintf("Failed to %v %v %v",
							strings.Split(strings.ToLower(evt.EventType.String()), "-event")[0], evt.MirrorSession.Kind, evt.MirrorSession.Name), evt.MirrorSession)
					}()
				}

			case <-time.After(syncInterval):
				eventList, err := tsaRPCClient.ListMirrorSessions(ctx, &api.ObjectMeta{})
				if err == nil {
					ctrlMs := map[string]*netproto.MirrorSession{}
					for i := range eventList.MirrorSessionEvents {
						ev := eventList.MirrorSessionEvents[i]
						ctrlMs[ev.MirrorSession.GetKey()] = ev.MirrorSession
					}

					// read policy from agent
					agMs := client.tsagent.ListMirrorSession()
					for _, pol := range agMs {
						if _, ok := ctrlMs[pol.GetKey()]; !ok {
							log.Infof("sync deleting mirror session %v", pol.GetKey())
							if err := client.tsagent.DeleteMirrorSession(pol); err != nil {
								log.Errorf("failed to delete %v, err: %v", pol.GetKey(), err)
							}
						}
					}
				}
			}
		}
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
