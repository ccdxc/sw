package watcher

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// processEvent is aborted after this timeout and watches re-established
	defProcessEventTimeout = time.Second
)

// KindOptions defines API Server kind and its ListWatchOptions
type KindOptions struct {
	Kind    string
	Options *api.ListWatchOptions
}

// InitiateWatchCb is a callback called before creating watches. It can be used to clear state in watch event processor like clear cache.
type InitiateWatchCb func()

// ProcessEventCb is a callback called when a watch event is received
type ProcessEventCb func(event *kvstore.WatchEvent) error

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
	module              string             // module using this watcher
	waitGrp             sync.WaitGroup     // wait group to wait on all go routines to exit
	watchCtx            context.Context    // ctx for watchers
	watchCancel         context.CancelFunc // cancel for watchers
	stopFlag            syncFlag           // boolean flag to exit the API watchers
	kinds               []*KindOptions     // api server objects to watch
	logger              log.Logger
	processEventCb      ProcessEventCb
	processEventTimeout time.Duration // processEvent is aborted after this timeout
	inititateWatchCb    InitiateWatchCb
	rpcOptions          []rpckit.Option
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
			w.logger.ErrorLog("method", "initiateWatches", "msg", fmt.Sprintf("failed to start watcher for %s", kind), "err", err)
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
		if err := w.processEvent(event); err != nil {
			w.logger.ErrorLog("method", "initiateWatches", "msg", fmt.Sprintf("error processing event from [%s] watcher", watchList[chosen]))
			return
		}
	}
}

func (w *Watcher) runWatcher(name, apiSrvURL string, rslver resolver.Interface) {
	defer w.waitGrp.Done()

	// loop forever
	for {
		b := balancer.New(rslver)
		grpcOpts := []rpckit.Option{rpckit.WithBalancer(b)}
		if w.rpcOptions != nil {
			grpcOpts = append(grpcOpts, w.rpcOptions...)
		}
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(name, apiSrvURL, w.logger, grpcOpts...)
		if err != nil {
			w.logger.WarnLog("method", "runWatcher", "msg", fmt.Sprintf("failed to connect to gRPC server [%s]", apiSrvURL))
		} else {
			w.logger.InfoLog("method", "runWatcher", "msg", fmt.Sprintf("API client connected {%+v}", apicl))
			// handle api server watch events
			w.initiateWatches(apicl)
			apicl.Close()
		}
		b.Close()
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
	w.module = name
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
func NewWatcher(name, apiServer string, rslver resolver.Interface, l log.Logger, initiateWatchCb InitiateWatchCb, processEventCb ProcessEventCb, rpcOptions []rpckit.Option, kinds ...*KindOptions) *Watcher {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	watcher := &Watcher{
		module:      name,
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		logger:              l,
		kinds:               kinds,
		inititateWatchCb:    initiateWatchCb,
		processEventCb:      processEventCb,
		processEventTimeout: defProcessEventTimeout,
		rpcOptions:          rpcOptions,
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
	case string(auth.KindUser):
		if kind.Options == nil {
			kind.Options = &api.ListWatchOptions{}
		}
		// user watcher
		watcher, err = apicl.AuthV1().User().Watch(ctx, kind.Options)
	case string(auth.KindAuthenticationPolicy):
		if kind.Options == nil {
			kind.Options = &api.ListWatchOptions{}
		}
		// authentication policy watcher
		watcher, err = apicl.AuthV1().AuthenticationPolicy().Watch(ctx, kind.Options)
	case string(diagnostics.KindModule):
		if kind.Options == nil {
			kind.Options = &api.ListWatchOptions{}
		}
		// module watcher
		watcher, err = apicl.DiagnosticsV1().Module().Watch(ctx, kind.Options)
	case string(monitoring.KindArchiveRequest):
		if kind.Options == nil {
			kind.Options = &api.ListWatchOptions{}
		}
		// archive request watcher
		watcher, err = apicl.MonitoringV1().ArchiveRequest().Watch(ctx, kind.Options)
	default:
		return nil, fmt.Errorf("unsupported kind: %s", kind)
	}
	return watcher, err
}

func (w *Watcher) processEvent(event *kvstore.WatchEvent) error {
	ctx, cancel := context.WithTimeout(w.watchCtx, w.processEventTimeout)
	defer cancel()
	c := make(chan error, 1)
	go func() {
		c <- w.processEventCb(event)
		return
	}()
	select {
	case <-ctx.Done():
		obj, _ := runtime.GetObjectMeta(event.Object)
		w.logger.ErrorLog("method", "processEvent",
			"msg", fmt.Sprintf("watch event processing timed out for watch event obj kind [%s], objmeta [%v]", event.Object.GetObjectKind(), obj),
			"err", ctx.Err())
		recorder.Event(eventtypes.SERVICE_UNRESPONSIVE,
			fmt.Sprintf("module [%s] timed out processing watch events, please check system resources", w.module), event.Object)
		return ctx.Err()
	case err := <-c:
		return err
	}
}
