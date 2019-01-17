// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/ctrler/rollout/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// length of watcher channel
const watcherQueueLen = 1000

// Watcher watches api server for changes
type Watcher struct {
	waitGrp     sync.WaitGroup     // wait group to wait on all go routines to exit
	statemgr    *statemgr.Statemgr // reference to state manager
	watchCtx    context.Context    // ctx for watchers
	watchCancel context.CancelFunc // cancel for watchers
	debugStats  *debugStats.Stats
}

// Stop watcher
func (w *Watcher) Stop() {
	w.watchCancel()
	// wait for all goroutines to exit
	w.waitGrp.Wait()
}

// NewWatcher returns a new watcher object
func NewWatcher(statemgr *statemgr.Statemgr, apisrvURL string, resolver resolver.Interface, debugStats *debugStats.Stats) (*Watcher, error) {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create a watcher
	watcher := &Watcher{
		statemgr:    statemgr,
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		debugStats:  debugStats,
	}
	// setup wait group
	watcher.waitGrp.Add(1)
	// handle api watchers
	go watcher.runApisrvWatcher(watchCtx, apisrvURL, resolver)

	return watcher, nil
}

// handleApisrvWatch handles api server watch events
func (w *Watcher) handleApisrvWatch(ctx context.Context, apicl apiclient.Services) {
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	// node watcher
	nodeWatcher, err := apicl.ClusterV1().Node().Watch(ctx, &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}) // TODO: is Spec sufficient?
	if err != nil {
		log.Errorf("Failed to start node watcher (%s)\n", err)
		return
	}
	defer nodeWatcher.Stop()

	nodes, err := apicl.ClusterV1().Node().List(ctx, &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("Failed to list nodes (%s)\n", err)
		return
	}
	for _, node := range nodes {
		w.statemgr.SetNodeState(node)
	}

	// smartNIC Watcher
	smartNICWatcher, err := apicl.ClusterV1().SmartNIC().Watch(ctx, &api.ListWatchOptions{FieldChangeSelector: []string{"Status.Conditions[0].Status", "Status.Conditions[1].Status"}})
	if err != nil {
		log.Errorf("Failed to start node watcher (%s)\n", err)
		return
	}
	defer smartNICWatcher.Stop()

	snics, err := apicl.ClusterV1().SmartNIC().List(ctx, &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("Failed to list smartNICs (%s)\n", err)
		return
	}
	for _, snic := range snics {
		w.statemgr.SetSmartNICState(snic)
	}

	// rollout watcher
	rolloutWatcher, err := apicl.RolloutV1().Rollout().Watch(ctx, &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}})
	if err != nil {
		log.Errorf("Failed to start watch (%s)\n", err)
		return
	}
	defer rolloutWatcher.Stop()

	// wait for events
	for {
		select {
		case evt, ok := <-rolloutWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from rollout watcher")
				return
			}
			w.statemgr.RolloutWatcher <- *evt
		case evt, ok := <-nodeWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from node watcher")
				return
			}
			w.statemgr.NodeWatcher <- *evt
		case evt, ok := <-smartNICWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from smartNIC watcher")
				return
			}
			w.statemgr.SmartNICWatcher <- *evt
		}
	}
}

// runApisrvWatcher run API server watcher forever
func (w *Watcher) runApisrvWatcher(ctx context.Context, apisrvURL string, resolver resolver.Interface) {
	// if we have no URL, exit
	if apisrvURL == "" {
		return
	}

	defer w.waitGrp.Done()

	// create logger
	config := log.GetDefaultConfig("RolloutApiWatcher")
	l := log.GetNewLogger(config)
	b := balancer.New(resolver)

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(globals.Rollout, apisrvURL, l, rpckit.WithBalancer(b))
		if err != nil {
			log.Warnf("Failed to connect to APIserver [%s]\n", apisrvURL)
		} else {
			log.Debugf("connected to APIserver")

			// handle api server watch events
			w.handleApisrvWatch(ctx, apicl)
			apicl.Close()
		}

		select {
		case <-w.watchCtx.Done(): // Stop() was called
			log.Infof("returning from ApisrvWatcher {%+v}", apicl)
			return
		default:
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}
