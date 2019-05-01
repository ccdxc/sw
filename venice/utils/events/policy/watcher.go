package policy

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/resolver"

	"github.com/pensando/sw/api"
	evtsmgrprotos "github.com/pensando/sw/nic/agent/protos/evtprotos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	// delay between retries
	retryDelay = 2 * time.Second

	pkgName = "events-policy-watcher"
)

// WOption fills the optional params for Watcher
type WOption func(*Watcher)

// WithEventsMgrURL passes a custom client for Elastic
func WithEventsMgrURL(url string) WOption {
	return func(w *Watcher) {
		w.evtsMgrURL = url
	}
}

// WithResolverClient passes a custom resolver client to connect with evtsmgr
func WithResolverClient(resolverClient resolver.Interface) WOption {
	return func(w *Watcher) {
		w.resolverClient = resolverClient
	}
}

// Watcher watches the event policies from evtsmgr
type Watcher struct {
	ctx            context.Context
	cancelFunc     context.CancelFunc
	logger         log.Logger
	rpcClient      *rpckit.RPCClient
	policyMgr      *Manager
	evtsMgrURL     string
	resolverClient resolver.Interface
	wg             sync.WaitGroup
}

// NewWatcher creates a new watcher instance with given params
func NewWatcher(policyMgr *Manager, logger log.Logger, opts ...WOption) (*Watcher, error) {
	ctx, cancelFunc := context.WithCancel(context.Background())

	w := &Watcher{
		ctx:        ctx,
		cancelFunc: cancelFunc,
		logger:     logger,
		policyMgr:  policyMgr,
	}

	for _, opt := range opts {
		if opt != nil {
			opt(w)
		}
	}

	if utils.IsEmpty(w.evtsMgrURL) {
		w.evtsMgrURL = fmt.Sprintf(":%s", globals.EvtsMgrRPCPort)
	}

	w.wg.Add(1)
	go w.start()

	return w, nil
}

// start creates the evtsmgr client to watch event policies; this function should be called in a goroutine
func (w *Watcher) start() {
	w.logger.Info("starting event policy watcher")
	defer w.wg.Done()

	var err error

	for {
		select {
		case <-w.ctx.Done():
			w.logger.Errorf("context closed; event policy watcher")
			return
		default:
			if err := w.ctx.Err(); err != nil {
				return
			}
			// policy watcher connecting to evtsmgr to watch event policies
			if w.resolverClient != nil {
				w.rpcClient, err = rpckit.NewRPCClient(pkgName, globals.EvtsMgr,
					rpckit.WithBalancer(balancer.New(w.resolverClient)), rpckit.WithRemoteServerName(globals.EvtsMgr))
			} else {
				w.rpcClient, err = rpckit.NewRPCClient("events-policy-watcher", w.evtsMgrURL,
					rpckit.WithRemoteServerName(globals.EvtsMgr))
			}

			if err != nil {
				w.logger.Errorf("failed to connect to {%s}, err: %v", globals.EvtsMgr, err)
				time.Sleep(retryDelay)
				continue
			}

			w.logger.Infof("connected to {%s}", globals.EvtsMgr)
			w.processEvents()
			w.rpcClient.Close()
			time.Sleep(retryDelay)
		}
	}
}

// Stop stops the policy watcher
func (w *Watcher) Stop() {
	w.logger.Info("stopping event policy watcher")
	w.cancelFunc() // stop the goroutine watching policies
	w.wg.Wait()
}

// process watch events
func (w *Watcher) processEvents() error {
	return w.watchEventPolicy(w.ctx)
}

// watch event policies and create required writers with the help of policy manager
func (w *Watcher) watchEventPolicy(ctx context.Context) error {
	eventPolicyClient := evtsmgrprotos.NewEventPolicyAPIClient(w.rpcClient.ClientConn)
	eventPolicyStream, err := eventPolicyClient.WatchEventPolicy(ctx, &api.ObjectMeta{Name: "events/policy/watcher.go"})
	if err != nil {
		w.logger.Errorf("error watching event policy: %v", err)
		return err
	}

	for {
		select {
		case <-ctx.Done():
			w.logger.Warnf("context closed; event policy watcher")
			return nil
		default:
			evt, err := eventPolicyStream.Recv()
			if err != nil {
				w.logger.Errorf("received error from event policy channel: %v", err)
				return err
			}

			w.logger.Debugf("got event policy watch event {%v}: %v", evt.EventType, evt.Policy)

			switch evt.EventType {
			case api.EventType_CreateEvent:
				w.policyMgr.Create(evt.Policy)
			case api.EventType_UpdateEvent:
				w.policyMgr.Update(evt.Policy)
			case api.EventType_DeleteEvent:
				w.policyMgr.Delete(evt.Policy)
			}
		}
	}
}
