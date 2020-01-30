package ctrlerif

import (
	"context"
	"fmt"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils/events/recorder"

	"github.com/pensando/sw/api"
	tpmproto "github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/nic/agent/tpa/state/types"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// TpClient is the telemetry policy client
type TpClient struct {
	sync.Mutex
	agentName      string
	srvURL         string
	resolverClient resolver.Interface
	waitGrp        sync.WaitGroup
	state          types.CtrlerIntf
	rpcClient      *rpckit.RPCClient
	watchCtx       context.Context
	watchCancel    context.CancelFunc
}

var syncInterval = time.Minute * 5

const maxRetry = 3

// NewTpClient creates telemetry policy client object
func NewTpClient(name string, state types.CtrlerIntf, srvURL string, resolverClient resolver.Interface) (*TpClient, error) {
	watchCtx, watchCancel := context.WithCancel(context.Background())
	tpClient := TpClient{
		agentName:      name,
		srvURL:         srvURL,
		resolverClient: resolverClient,
		state:          state,
		watchCtx:       watchCtx,
		watchCancel:    watchCancel,
	}

	go tpClient.runWatcher(tpClient.watchCtx)

	return &tpClient, nil
}

// watch telemetry/fwlog/flow-export policies
func (client *TpClient) runWatcher(ctx context.Context) {
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for ctx.Err() == nil {
		// create a grpc client
		if client.resolverClient != nil {
			rpcClient, err := rpckit.NewRPCClient(client.agentName, client.srvURL, rpckit.WithBalancer(balancer.New(client.resolverClient)))
			if err != nil {
				log.Errorf("Error connecting to %s, Err: %v", client.srvURL, err)
				time.Sleep(time.Second)
				continue
			}
			log.Infof("grpc client connected to %v", client.srvURL)

			client.rpcClient = rpcClient
			client.processEvents(ctx)

			client.rpcClient.Close()
		}
		time.Sleep(time.Second)
	}
}

func (w *watchChan) watchFwlogPolicy(ctx context.Context, cl tpmproto.FwlogPolicyApiV1_WatchFwlogPolicyClient) {
	defer func() {
		close(w.fwlogChan)
		w.wg.Done()
	}()

	for ctx.Err() == nil {
		event, err := cl.Recv()
		if err != nil {
			log.Errorf("stop watching fwlog policy, error:%s", err)
			return
		}
		select {
		case w.fwlogChan <- event:
		case <-ctx.Done():
		}
	}
}

func (w *watchChan) watchFlowExpPolicy(ctx context.Context, cl netproto.FlowExportPolicyApiV1_WatchFlowExportPolicyClient) {
	defer func() {
		close(w.flowExpChan)
		w.wg.Done()
	}()

	for ctx.Err() == nil {
		event, err := cl.Recv()
		if err != nil {
			log.Errorf("stop watching flow export policy, error:%s", err)
			return
		}
		select {
		case w.flowExpChan <- event:
		case <-ctx.Done():
		}

	}
}

const watchlen = 100

type watchChan struct {
	wg          sync.WaitGroup
	fwlogChan   chan *tpmproto.FwlogPolicyEvent
	flowExpChan chan *netproto.FlowExportPolicyEventList
}

func (client *TpClient) processEvents(pctx context.Context) error {
	wc := &watchChan{
		fwlogChan:   make(chan *tpmproto.FwlogPolicyEvent, watchlen),
		flowExpChan: make(chan *netproto.FlowExportPolicyEventList, watchlen),
	}

	ctx, cancel := context.WithCancel(pctx)
	defer func() {
		cancel()
		wc.wg.Wait()
	}()

	fwlogClient := tpmproto.NewFwlogPolicyApiV1Client(client.rpcClient.ClientConn)
	fwlogPolicyStream, err := fwlogClient.WatchFwlogPolicy(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Errorf("Error watching fwlog policy: Err: %v", err)
		return err
	}
	wc.wg.Add(1)
	go wc.watchFwlogPolicy(ctx, fwlogPolicyStream)

	flowExpClient := netproto.NewFlowExportPolicyApiV1Client(client.rpcClient.ClientConn)
	flowPolicyStream, err := flowExpClient.WatchFlowExportPolicy(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Errorf("Error watching flow export policy: Err: %v", err)
		return err
	}
	wc.wg.Add(1)
	go wc.watchFlowExpPolicy(ctx, flowPolicyStream)

	for {
		select {

		case event, ok := <-wc.fwlogChan:
			if !ok {
				log.Errorf("fwlog policy channel closed")
				return nil
			}

			log.Infof("received policy(%s) %+v", event.EventType, event.Policy)

			func() {
				var err error
				for iter := 0; iter < maxRetry; iter++ {
					switch event.EventType {
					case api.EventType_CreateEvent:
						if err = client.state.CreateFwlogPolicy(ctx, event.Policy); err != nil {
							if strings.Contains(err.Error(), "already exists") {
								err = client.state.UpdateFwlogPolicy(ctx, event.Policy)
							}
						}
					case api.EventType_UpdateEvent:
						if err = client.state.UpdateFwlogPolicy(ctx, event.Policy); err != nil {
							if strings.Contains(err.Error(), "doesn't exist") {
								err = client.state.CreateFwlogPolicy(ctx, event.Policy)
							}
						}
					case api.EventType_DeleteEvent:
						if err = client.state.DeleteFwlogPolicy(ctx, event.Policy); err != nil {
							if strings.Contains(err.Error(), "doesn't exist") {
								err = nil
							}
						}
					}

					if err == nil {
						return
					}

					log.Errorf("[%v] fwlog policy failed, err: %v", event.EventType, err)
					time.Sleep(time.Second * 5)
				}
				recorder.Event(eventtypes.CONFIG_FAIL, fmt.Sprintf("Failed to %v %v %v, error: %v",
					strings.Split(strings.ToLower(event.EventType.String()), "-event")[0], event.Policy.Kind,
					event.Policy.Name, err), event.Policy)
			}()

		case event, ok := <-wc.flowExpChan:
			if !ok {
				log.Errorf("flow policy channel closed")
				return nil
			}
			log.Infof("received policy(%s) %+v", event.FlowExportPolicyEvents[0].EventType, event.FlowExportPolicyEvents[0].Policy)

			func() {
				var err error
				for iter := 0; iter < maxRetry; iter++ {
					switch event.FlowExportPolicyEvents[0].EventType {

					case api.EventType_CreateEvent:
						//if err = client.state.CreateFlowExportPolicy(ctx, event.Policy); err != nil {
						//	if strings.Contains(err.Error(), "already exists") {
						//		err = client.state.UpdateFlowExportPolicy(ctx, event.Policy)
						//	}
						//}
					case api.EventType_UpdateEvent:
						//if err = client.state.UpdateFlowExportPolicy(ctx, event.Policy); err != nil {
						//	if strings.Contains(err.Error(), "doesn't exist") {
						//		err = client.state.CreateFlowExportPolicy(ctx, event.Policy)
						//	}
						//
						//}
					case api.EventType_DeleteEvent:
						//if err = client.state.DeleteFlowExportPolicy(ctx, event.Policy); err != nil {
						//	if strings.Contains(err.Error(), "doesn't exist") {
						//		err = nil // ignore
						//	}
						//}
					}

					if err == nil {
						return
					}

					log.Errorf("[%v] flow export policy failed, err: %v", event.FlowExportPolicyEvents[0].EventType, err)
					time.Sleep(time.Second * 5)
				}
				recorder.Event(eventtypes.CONFIG_FAIL, fmt.Sprintf("Failed to %v %v %v, error: %v",
					strings.Split(strings.ToLower(event.FlowExportPolicyEvents[0].EventType.String()), "-event")[0], event.FlowExportPolicyEvents[0].Policy.Kind,
					event.FlowExportPolicyEvents[0].Policy.Name, err), event.FlowExportPolicyEvents[0].Policy)
			}()

			// periodic sync
		case <-time.After(syncInterval):
			// flow export
			flowEvent, err := flowExpClient.ListFlowExportPolicy(ctx, &api.ObjectMeta{})
			if err == nil {
				ctrlFlowExp := map[string]*netproto.FlowExportPolicy{}
				for _, exp := range flowEvent.FlowExportPolicyEvents {
					ctrlFlowExp[exp.Policy.GetKey()] = exp.Policy
				}

				// read policy from agent
				//agFlowExp, err := client.state.ListFlowExportPolicy(ctx)
				if err == nil {
					//for _, pol := range agFlowExp {
					//	if _, ok := ctrlFlowExp[pol.GetKey()]; !ok {
					//		log.Infof("sync deleting flowExport policy %v", pol.GetKey())
					//		if err := client.state.DeleteFlowExportPolicy(ctx, pol); err != nil {
					//			log.Errorf("failed to delete %v, err: %v", pol.GetKey(), err)
					//		}
					//	}
					//}
				}
			}

			// fwlog
			fwEvent, err := fwlogClient.ListFwlogPolicy(ctx, &api.ObjectMeta{})
			if err == nil {
				ctrlFw := map[string]*tpmproto.FwlogPolicy{}
				for _, fw := range fwEvent.EventList {
					ctrlFw[fw.Policy.GetKey()] = fw.Policy
				}

				// read policy from agent
				agFw, err := client.state.ListFwlogPolicy(ctx)
				if err == nil {
					for _, pol := range agFw {
						if _, ok := ctrlFw[pol.GetKey()]; !ok {
							log.Infof("sync deleting fwlog policy %v", pol.GetKey())
							if err := client.state.DeleteFwlogPolicy(ctx, pol); err != nil {
								log.Errorf("failed to delete %v, err: %v", pol.GetKey(), err)
							}
						}
					}
				}
			}

		case <-pctx.Done():
			log.Warnf("canceled telemetry policy watch while there are %d events in flowExpChan, %d in fwlogChan ",
				len(wc.flowExpChan), len(wc.fwlogChan))
			return nil
		}
	}
}

// Stop stops all the watchers
func (client *TpClient) Stop() {
	client.watchCancel()
	client.waitGrp.Wait()
}
