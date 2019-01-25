package watcher

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// KindOptions defines API Server kind and its ListWatchOptions
type KindOptions struct {
	Kind    string
	Options *api.ListWatchOptions
}

// InitiateWatchCb is a callback called before creating watches. It can be used to clear state in watch event processor like clear cache.
type InitiateWatchCb func()

// ProcessEventCb is a callback called when a watch event is received
type ProcessEventCb func(event *kvstore.WatchEvent)

type syncFlag struct {
	sync.RWMutex
	flag bool
}

func (f *syncFlag) set() {
	defer f.Unlock()
	f.Lock()
	f.flag = true
}

func (f *syncFlag) unSet() {
	defer f.Unlock()
	f.Lock()
	f.flag = false
}

func (f *syncFlag) isSet() bool {
	defer f.RUnlock()
	f.RLock()
	return f.flag
}

// Watcher watches API server
type Watcher struct {
	waitGrp          sync.WaitGroup     // wait group to wait on all go routines to exit
	watchCtx         context.Context    // ctx for watchers
	watchCancel      context.CancelFunc // cancel for watchers
	stopFlag         syncFlag           // boolean flag to exit the API watchers
	kinds            []*KindOptions     // api server objects to watch
	logger           log.Logger
	processEventCb   ProcessEventCb
	inititateWatchCb InitiateWatchCb
}

// initiateWatches watches on api server objects
func (w *Watcher) initiateWatches(apicl apiclient.Services) {
	// reset cache
	w.inititateWatchCb()
	ctx, cancel := context.WithCancel(w.watchCtx)
	defer cancel()

	var cases []reflect.SelectCase
	watchList := make(map[int]string)

	// tenant Watcher
	for _, kind := range w.kinds {
		watcher, err := w.watch(ctx, apicl, kind)
		if err != nil {
			w.logger.ErrorLog("method", "initiateWatches", "msg", fmt.Sprintf("failed to start watcher for %s", kind), "error", err)
			return
		}
		defer watcher.Stop()
		cases = append(cases, reflect.SelectCase{
			Dir:  reflect.SelectRecv,
			Chan: reflect.ValueOf(watcher.EventChan())})
		watchList[len(cases)] = kind.Kind
	}

	// wait for events
	for {
		chosen, value, ok := reflect.Select(cases)
		if !ok {
			w.logger.ErrorLog("method", "initiateWatches", "msg", fmt.Sprintf("error receiving from [%s] watcher", watchList[chosen]))
			return
		}
		event, ok := value.Interface().(*kvstore.WatchEvent)
		if !ok {
			w.logger.ErrorLog("method", "initiateWatches", "msg", fmt.Sprintf("unknown object type returned from [%s] watch: [%+v]", watchList[chosen], value.Interface()))
			return
		}
		w.processEventCb(event)
	}
}

func (w *Watcher) runWatcher(name, apiSrvURL string, rslver resolver.Interface) {
	defer w.waitGrp.Done()

	b := balancer.New(rslver)
	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(name, apiSrvURL, w.logger, rpckit.WithBalancer(b))
		if err != nil {
			w.logger.WarnLog("method", "runWatcher", "msg", fmt.Sprintf("failed to connect to gRPC server [%s]", apiSrvURL))
		} else {
			w.logger.InfoLog("method", "runWatcher", "msg", fmt.Sprintf("API client connected {%+v}", apicl))
			// handle api server watch events
			w.initiateWatches(apicl)
			apicl.Close()
		}

		// if stop flag is set, we are done
		if w.stopped() {
			w.logger.InfoLog("method", "runWatcher", "msg", "Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// Stop Watcher
func (w *Watcher) Stop() {
	// stop the context
	w.setStopFlag()
	w.watchCancel()

	// wait for all goroutines to exit
	w.waitGrp.Wait()
}

// Start re-starts the Watcher. It should be only be called after Stop() has been called.
func (w *Watcher) Start(name, apiServerURL string, rslver resolver.Interface) {
	// create context and cancel
	w.watchCtx, w.watchCancel = context.WithCancel(context.Background())
	// unset stop flag
	w.stopFlag.unSet()
	// setup wait group
	w.waitGrp.Add(1)
	go w.runWatcher(name, apiServerURL, rslver)
}

func (w *Watcher) stopped() bool {
	return w.stopFlag.isSet()
}

func (w *Watcher) setStopFlag() {
	w.stopFlag.set()
}

// NewWatcher returns a Watcher for API server objects
func NewWatcher(name, apiServer string, rslver resolver.Interface, l log.Logger, initiateWatchCb InitiateWatchCb, processEventCb ProcessEventCb, kinds ...*KindOptions) *Watcher {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	watcher := &Watcher{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		logger:           l,
		kinds:            kinds,
		inititateWatchCb: initiateWatchCb,
		processEventCb:   processEventCb,
	}
	// setup wait group
	watcher.waitGrp.Add(1)
	go watcher.runWatcher(name, apiServer, rslver)

	return watcher
}

func (w *Watcher) watch(ctx context.Context, apicl apiclient.Services, kind *KindOptions) (kvstore.Watcher, error) {
	var watcher kvstore.Watcher
	var err error
	switch kind.Kind {
	case string(cluster.KindCluster):
		if kind.Options == nil {
			kind.Options = &api.ListWatchOptions{}
		}
		// cluster watcher
		watcher, err = apicl.ClusterV1().Cluster().Watch(ctx, kind.Options)
	default:
		return nil, fmt.Errorf("unsupported kind: %s", kind)
	}
	return watcher, err
}
