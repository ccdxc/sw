package watcher

import (
	"context"
	"fmt"
	"sync"

	"github.com/pensando/sw/venice/utils/resolver"
)

const watcherQueueLen = 1000

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// Watcher is the struct which watches for update to vc config objects
type Watcher struct {
	waitGrp     sync.WaitGroup
	watchCtx    context.Context
	watchCancel context.CancelFunc
	stopFlag    syncFlag
}

// Stop stops the watcher
func (w *Watcher) Stop() {
	w.stop()
	w.watchCancel()
	w.waitGrp.Wait()
}

func (w *Watcher) stopped() bool {
	w.stopFlag.RLock()
	defer w.stopFlag.RUnlock()
	return w.stopFlag.flag
}

func (w *Watcher) stop() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// NewWatcher creates a new watcher
func NewWatcher(apisrvURL string, resolver resolver.Interface) (*Watcher, error) {

	if len(apisrvURL) == 0 {
		return nil, fmt.Errorf("API server URL is empty")
	}

	watchCtx, watchCancel := context.WithCancel(context.Background())

	watcher := &Watcher{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
	}

	go watcher.runApisrvWatcher(watchCtx, apisrvURL, resolver)
	return watcher, nil
}
