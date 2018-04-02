package manager

import (
	"context"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// watcher watches API server for users
type watcher struct {
	waitGrp     sync.WaitGroup     // wait group to wait on all go routines to exit
	watchCtx    context.Context    // ctx for watchers
	watchCancel context.CancelFunc // cancel for watchers
	stopFlag    syncFlag           // boolean flag to exit the API watchers
	startCond   *sync.Cond         // Condition to signal watcher to start
	name        string             // module name using the watcher
	apiServer   string             // api server address
	resolver    resolver.Interface
	cache       *memdb.Memdb
}

func (w *watcher) processUserEvent(evt *kvstore.WatchEvent, user *auth.User) {
	// update cache
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		w.cache.AddObject(user)
	case kvstore.Deleted:
		w.cache.DeleteObject(user)
	}
}

func (w *watcher) processPolicyEvent(evt *kvstore.WatchEvent, policy *auth.AuthenticationPolicy) {
	// update cache
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		w.cache.AddObject(policy)
	case kvstore.Deleted:
		w.cache.DeleteObject(policy)
	}
}

func (w *watcher) processEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *auth.User:
		user := evt.Object.(*auth.User)
		w.processUserEvent(evt, user)
	case *auth.AuthenticationPolicy:
		policy := evt.Object.(*auth.AuthenticationPolicy)
		w.processPolicyEvent(evt, policy)
	default:
		log.Errorf("watcher found object of invalid type: %+v", tp)
		return
	}
}

// initiateWatches watches api server for users
func (w *watcher) initiateWatches(apicl apiclient.Services) {
	// reset cache
	w.resetCache()
	ctx, cancel := context.WithCancel(w.watchCtx)
	defer cancel()

	var cases []reflect.SelectCase
	watchList := make(map[int]string)

	// user watcher
	opts := api.ListWatchOptions{}
	userWatcher, err := apicl.AuthV1().User().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("Failed to start user watch: Err: %v", err)
		return
	}
	defer userWatcher.Stop()
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(userWatcher.EventChan())})
	watchList[len(cases)] = "user"

	// authentication policy watcher
	policyWatcher, err := apicl.AuthV1().AuthenticationPolicy().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("Failed to start authentication policy watch: Err: %v", err)
		return
	}
	defer policyWatcher.Stop()
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(policyWatcher.EventChan())})
	watchList[len(cases)] = "authPolicy"

	// wait for events
	for {
		chosen, value, ok := reflect.Select(cases)
		if !ok {
			log.Errorf("Error receiving from [%s] watcher", watchList[chosen])
			return
		}
		event, ok := value.Interface().(*kvstore.WatchEvent)
		if !ok {
			log.Errorf("Unknown object type returned from [%s] watch: [%+v]", watchList[chosen], value.Interface())
			return
		}
		w.processEvent(event)
	}
}

func (w *watcher) runWatcher() {
	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// create logger
	config := log.GetDefaultConfig(w.name)
	l := log.GetNewLogger(config)
	b := balancer.New(w.resolver)
	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(w.name, w.apiServer, l, rpckit.WithBalancer(b))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", w.apiServer)
		} else {
			log.Infof("API client connected {%+v}", apicl)
			// handle api server watch events
			w.initiateWatches(apicl)
			apicl.Close()
		}

		// if stop flag is set, we are done
		if w.stopped() {
			log.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// Stop watcher
func (w *watcher) stop() {
	// stop the context
	w.setStopFlag()
	w.watchCancel()

	// wait for all goroutines to exit
	w.waitGrp.Wait()

	// signal that watcher has completely stopped
	w.startCond.Signal()
}

func (w *watcher) stopped() bool {
	w.stopFlag.RLock()

	defer w.stopFlag.RUnlock()
	return w.stopFlag.flag
}

func (w *watcher) setStopFlag() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// start starts the watcher. It blocks if the watcher is not in a stopped state.
func (w *watcher) start() {
	w.startCond.L.Lock()
	defer w.startCond.L.Unlock()
	// check if watcher has stopped
	for !w.stopped() {
		w.startCond.Wait()
	}
	// create context and cancel
	w.watchCtx, w.watchCancel = context.WithCancel(context.Background())
	// unset stop flag
	w.stopFlag.Lock()
	w.stopFlag.flag = false
	w.stopFlag.Unlock()
	// run watcher
	go w.runWatcher()
}

func (w *watcher) resetCache() {
	users := w.cache.ListObjects("User")
	for _, user := range users {
		w.cache.DeleteObject(user)
	}
}

// newWatcher returns a watcher for user API
func newWatcher(cache *memdb.Memdb, name, apiServer string, rslver resolver.Interface) *watcher {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	watcher := &watcher{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		startCond: sync.NewCond(&sync.Mutex{}),
		name:      name,
		apiServer: apiServer,
		resolver:  rslver,
		cache:     cache,
	}
	go watcher.runWatcher()

	return watcher
}
