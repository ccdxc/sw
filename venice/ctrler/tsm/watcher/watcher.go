// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"sync"

	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/resolver"
)

// length of watcher channel
const watcherQueueLen = 1000

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// Watcher watches api server for changes
// Create watcher for each object of interest
type Watcher struct {
	waitGrp     sync.WaitGroup     // wait group to wait on all go routines to exit
	statemgr    *statemgr.Statemgr // reference to state manager
	watchCtx    context.Context    // ctx for watchers
	watchCancel context.CancelFunc // cancel for watchers
	stopFlag    syncFlag           // boolean flag to exit the API watchers
}

// Stop watcher
func (w *Watcher) Stop() {
	// stop the context
	w.stop()
	w.watchCancel()

	// wait for all goroutines to exit
	w.waitGrp.Wait()
}

// NewWatcher returns a new watcher object
func NewWatcher(statemgr *statemgr.Statemgr, apisrvURL string, resolver resolver.Interface) (*Watcher, error) {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create a watcher
	watcher := &Watcher{
		statemgr:    statemgr,
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
	}

	// handle api watchers
	go watcher.runApisrvWatcher(watchCtx, apisrvURL, resolver)

	return watcher, nil
}

func (w *Watcher) stopped() (val bool) {
	w.stopFlag.RLock()

	defer w.stopFlag.RUnlock()
	val = w.stopFlag.flag
	return
}

func (w *Watcher) stop() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}
