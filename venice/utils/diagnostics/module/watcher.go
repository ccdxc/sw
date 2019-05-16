package module

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	watcherutils "github.com/pensando/sw/venice/utils/watcher"
)

// watcher is a singleton that implements Watcher interface
var gWatcher *watcher
var watcherOnce sync.Once

type cache struct {
	sync.RWMutex
	module diagapi.Module
}

func (c *cache) getModule() diagapi.Module {
	defer c.RUnlock()
	c.RLock()
	return c.module
}

func (c *cache) setModule(module *diagapi.Module) {
	defer c.Unlock()
	c.Lock()
	c.module = *module
}

type watcher struct {
	sync.RWMutex
	cache     cache
	name      string // module name using the watcher
	apiServer string // api server address
	resolver  resolver.Interface
	watcher   *watcherutils.Watcher
	logger    log.Logger
	stopped   bool
	cbs       []OnChangeCb
}

func (w *watcher) Stop() {
	defer w.Unlock()
	w.Lock()
	w.watcher.Stop()
	w.stopped = true
}

func (w *watcher) start(name, apiServer string, rslver resolver.Interface) {
	defer w.Unlock()
	w.Lock()
	if w.stopped {
		w.name = name
		w.apiServer = apiServer
		w.resolver = rslver
		w.watcher.Start(w.name, w.apiServer, w.resolver)
		w.stopped = false
	}
}

func (w *watcher) Start() {
	w.start(w.name, w.apiServer, w.resolver)
}

func (w *watcher) RegisterOnChangeCb(cb OnChangeCb) {
	defer w.Unlock()
	w.Lock()
	w.cbs = append(w.cbs, cb)
}

func (w *watcher) Module() diagapi.Module {
	return w.cache.getModule()
}

func (w *watcher) processEventCb(evt *kvstore.WatchEvent) error {
	defer w.RUnlock()
	w.RLock()
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		moduleObj, ok := evt.Object.(*diagapi.Module)
		if !ok {
			// unexpected error
			w.logger.ErrorLog("method", "processEventCb", "msg", fmt.Sprintf("watcher found object of invalid type: %+v", evt.Object))
			return errors.New("watcher found object of invalid type")
		}
		w.cache.setModule(moduleObj)
		for _, cb := range w.cbs {
			cb(moduleObj)
		}
	}
	return nil
}

// no-op
func (w *watcher) initiateWatchCb() {}

// GetWatcher returns a singleton implementation of Watcher watching a module object for a given process
func GetWatcher(name, apiServer string, rslver resolver.Interface, logger log.Logger, cb OnChangeCb) Watcher {
	if gWatcher != nil {
		gWatcher.start(name, apiServer, rslver)
	}
	watcherOnce.Do(func() {
		if logger == nil {
			logger = log.GetNewLogger(log.GetDefaultConfig(name))
		}
		gWatcher = &watcher{
			name:      name,
			apiServer: apiServer,
			resolver:  rslver,
			logger:    logger,
			stopped:   false,
		}
		gWatcher.cbs = append(gWatcher.cbs, cb)
		logger.InfoLog("method", "GetWatcher", "msg", "watching for module", "name", name)
		// start watcher
		gWatcher.watcher = watcherutils.NewWatcher(name, apiServer, rslver, logger, gWatcher.initiateWatchCb, gWatcher.processEventCb,
			[]rpckit.Option{rpckit.WithTLSClientIdentity(name)},
			&watcherutils.KindOptions{
				Kind:    string(diagapi.KindModule),
				Options: &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Name: name}},
			})
	})

	return gWatcher
}
