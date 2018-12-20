package ctrlerif

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/tpa/state/types"
	tpmproto "github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
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

	for client.watchCtx.Err() == nil {
		// create a grpc client
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
		time.Sleep(time.Second)
	}
}

func (w *watchChan) watchStatsPolicy(ctx context.Context, cl tpmproto.StatsPolicyApi_WatchStatsPolicyClient) {
	defer w.wg.Done()

	for {
		event, err := cl.Recv()
		if err != nil {
			if ctx.Err() != nil {
				close(w.statsChan)
				return
			}
			log.Errorf("received error in stats policy stream, error:%s", err)
			time.Sleep(time.Second)
			continue
		}
		w.statsChan <- event
	}
}

func (w *watchChan) watchFwlogPolicy(ctx context.Context, cl tpmproto.FwlogPolicyApi_WatchFwlogPolicyClient) {
	defer w.wg.Done()

	for {
		event, err := cl.Recv()
		if err != nil {
			if ctx.Err() != nil {
				close(w.fwlogChan)
				return
			}
			log.Errorf("received error in fwlog policy stream, error:%s", err)
			time.Sleep(time.Second)
			continue
		}
		w.fwlogChan <- event
	}
}

func (w *watchChan) watchFlowExpPolicy(ctx context.Context, cl tpmproto.FlowExportPolicyApi_WatchFlowExportPolicyClient) {
	defer w.wg.Done()

	for {
		event, err := cl.Recv()
		if err != nil {
			if ctx.Err() != nil {
				close(w.flowExpChan)
				return
			}
			log.Errorf("received error in flow export policy stream, error:%s", err)
			time.Sleep(time.Second)
			continue
		}
		w.flowExpChan <- event
	}
}

type watchChan struct {
	wg          sync.WaitGroup
	statsChan   chan *tpmproto.StatsPolicyEvent
	fwlogChan   chan *tpmproto.FwlogPolicyEvent
	flowExpChan chan *tpmproto.FlowExportPolicyEvent
}

func (client *TpClient) processEvents(pctx context.Context) error {
	wc := &watchChan{
		statsChan:   make(chan *tpmproto.StatsPolicyEvent),
		fwlogChan:   make(chan *tpmproto.FwlogPolicyEvent),
		flowExpChan: make(chan *tpmproto.FlowExportPolicyEvent),
	}

	ctx, cancel := context.WithCancel(pctx)
	defer func() {
		cancel()
		wc.wg.Wait()
	}()

	flowExpClient := tpmproto.NewFlowExportPolicyApiClient(client.rpcClient.ClientConn)
	flowPolicyStream, err := flowExpClient.WatchFlowExportPolicy(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Errorf("Error watching flow export policy: Err: %v", err)
		return err
	}
	wc.wg.Add(1)
	go wc.watchFlowExpPolicy(ctx, flowPolicyStream)

	statsClient := tpmproto.NewStatsPolicyApiClient(client.rpcClient.ClientConn)
	statsPolicyStream, err := statsClient.WatchStatsPolicy(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Errorf("Error watching stats policy: Err: %v", err)
		return err
	}
	wc.wg.Add(1)
	go wc.watchStatsPolicy(ctx, statsPolicyStream)

	fwlogClient := tpmproto.NewFwlogPolicyApiClient(client.rpcClient.ClientConn)
	fwlogPolicyStream, err := fwlogClient.WatchFwlogPolicy(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Errorf("Error watching fw log policy: Err: %v", err)
		return err
	}
	wc.wg.Add(1)
	go wc.watchFwlogPolicy(ctx, fwlogPolicyStream)

	for {
		select {

		case event, ok := <-wc.statsChan:
			if ok != true {
				log.Errorf("error in stats policy channel")
				break
			}
			log.Infof("received policy(%s) %+v", event.EventType, event.Policy)

		case event, ok := <-wc.fwlogChan:
			if ok != true {
				log.Errorf("error in fwlog policy channel")
				break
			}

			log.Infof("received policy(%s) %+v", event.EventType, event.Policy)
			switch event.EventType {
			case api.EventType_CreateEvent:
				client.state.CreateFwLogPolicy(ctx, event.Policy)
			case api.EventType_UpdateEvent:
				client.state.UpdateFwLogPolicy(ctx, event.Policy)
			case api.EventType_DeleteEvent:
				client.state.DeleteFwLogPolicy(ctx, event.Policy)
			}

		case event, ok := <-wc.flowExpChan:
			if ok != true {
				log.Errorf("error in flow policy channel")
				break
			}
			log.Infof("received policy(%s) %+v", event.EventType, event.Policy)

			switch event.EventType {
			case api.EventType_CreateEvent:
				client.state.CreateFlowExportPolicy(ctx, event.Policy)
			case api.EventType_UpdateEvent:
				client.state.UpdateFlowExportPolicy(ctx, event.Policy)
			case api.EventType_DeleteEvent:
				client.state.DeleteFlowExportPolicy(ctx, event.Policy)
			}

		case <-client.watchCtx.Done():
			log.Warnf("canceled telemetry policy watch")
			return nil
		}
	}
}

// Stop stops all the watchers
func (client *TpClient) Stop() {
	client.watchCancel()
	client.waitGrp.Wait()
}
