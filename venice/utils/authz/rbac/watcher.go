package rbac

import (
	"context"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// permissionCache represents in-memory cache where this watcher saves user roles and role bindings
type permissionCache interface {
	addRole(role *auth.Role)
	deleteRole(role *auth.Role)
	getRole(name, tenant string) (auth.Role, bool)

	addRoleBinding(binding *auth.RoleBinding)
	deleteRoleBinding(binding *auth.RoleBinding)
	getRoleBinding(name, tenant string) (auth.RoleBinding, bool)

	initializeCacheForTenant(tenant string)
	deleteCacheForTenant(tenant string)
	reset()
}

type syncFlag struct {
	sync.RWMutex
	flag bool
}

// watcher watches API server for roles and role bindings
type watcher struct {
	waitGrp     sync.WaitGroup     // wait group to wait on all go routines to exit
	watchCtx    context.Context    // ctx for watchers
	watchCancel context.CancelFunc // cancel for watchers
	stopFlag    syncFlag           // boolean flag to exit the API watchers
	cache       permissionCache
}

func (w *watcher) processTenantEvent(evt *kvstore.WatchEvent, tenant *cluster.Tenant) {
	// update cache
	switch evt.Type {
	case kvstore.Created:
		// create role cache for tenant
		w.cache.initializeCacheForTenant(tenant.GetName())
		log.Infof("Initialized RBAC cache for Tenant [%#v]", tenant.ObjectMeta)
	case kvstore.Updated:
		// do nothing
	case kvstore.Deleted:
		w.cache.deleteCacheForTenant(tenant.GetName())
		log.Infof("Deleted RBAC cache for Tenant [%#v]", tenant.ObjectMeta)
	}
}

func (w *watcher) processRoleEvent(evt *kvstore.WatchEvent, role *auth.Role) {
	// update cache
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		w.cache.addRole(role)
		log.Infof("Updated Role [%#v] in RBAC cache", role.ObjectMeta)
	case kvstore.Deleted:
		w.cache.deleteRole(role)
		log.Infof("Deleted Role [%#v] in RBAC cache", role.ObjectMeta)
	}
}

func (w *watcher) processRoleBindingEvent(evt *kvstore.WatchEvent, roleBinding *auth.RoleBinding) {
	// update cache
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		w.cache.addRoleBinding(roleBinding)
		log.Infof("Updated RoleBinding [%#v] in RBAC cache", roleBinding.ObjectMeta)
	case kvstore.Deleted:
		w.cache.deleteRoleBinding(roleBinding)
		log.Infof("Deleted RoleBinding [%#v] in RBAC cache", roleBinding.ObjectMeta)
	}
}

func (w *watcher) processEvent(evt *kvstore.WatchEvent) {
	switch tp := evt.Object.(type) {
	case *cluster.Tenant:
		tenant := evt.Object.(*cluster.Tenant)
		w.processTenantEvent(evt, tenant)
	case *auth.Role:
		role := evt.Object.(*auth.Role)
		w.processRoleEvent(evt, role)
	case *auth.RoleBinding:
		roleBinding := evt.Object.(*auth.RoleBinding)
		w.processRoleBindingEvent(evt, roleBinding)
	default:
		log.Errorf("watcher found object of invalid type: %+v", tp)
		return
	}
}

// initiateWatches watches api server for tenants, roles and role bindings
func (w *watcher) initiateWatches(apicl apiclient.Services) {
	// reset cache
	w.cache.reset()
	ctx, cancel := context.WithCancel(w.watchCtx)
	defer cancel()

	var cases []reflect.SelectCase
	watchList := make(map[int]string)

	// tenant watcher
	opts := api.ListWatchOptions{}
	tenantWatcher, err := apicl.ClusterV1().Tenant().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("Failed to start tenant watch: Err: %v", err)
		return
	}
	defer tenantWatcher.Stop()
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(tenantWatcher.EventChan())})
	watchList[len(cases)] = "tenant"

	// role watcher
	roleWatcher, err := apicl.AuthV1().Role().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("Failed to start role watch: Err: %v", err)
		return
	}
	defer roleWatcher.Stop()
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(roleWatcher.EventChan())})
	watchList[len(cases)] = "role"

	// roleBinding watcher
	roleBindingWatcher, err := apicl.AuthV1().RoleBinding().Watch(ctx, &opts)
	if err != nil {
		log.Errorf("Failed to start role binding watch: Err: %v", err)
		return
	}
	defer roleBindingWatcher.Stop()
	cases = append(cases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(roleBindingWatcher.EventChan())})
	watchList[len(cases)] = "role-binding"

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
		go w.processEvent(event)
	}
}

func (w *watcher) runWatcher(name, apiSrvURL string, rslver resolver.Interface) {
	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// create logger
	config := log.GetDefaultConfig("AuthzApiWatcher")
	l := log.GetNewLogger(config)
	// loop forever
	for {
		b := balancer.New(rslver)
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(name, apiSrvURL, l, rpckit.WithBalancer(b))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", apiSrvURL)
		} else {
			log.Infof("API client connected {%+v}", apicl)
			// handle api server watch events
			w.initiateWatches(apicl)
			apicl.Close()
		}
		b.Close()
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
}

// start starts the watcher. It blocks if the watcher is not in a stopped state.
func (w *watcher) start(name, apiServerURL string, rslver resolver.Interface) {
	// create context and cancel
	w.watchCtx, w.watchCancel = context.WithCancel(context.Background())
	// unset stop flag
	w.stopFlag.Lock()
	w.stopFlag.flag = false
	w.stopFlag.Unlock()
	go w.runWatcher(name, apiServerURL, rslver)
}

func (w *watcher) stopped() (val bool) {
	w.stopFlag.RLock()

	defer w.stopFlag.RUnlock()
	val = w.stopFlag.flag
	return
}

func (w *watcher) setStopFlag() {
	w.stopFlag.Lock()
	w.stopFlag.flag = true
	w.stopFlag.Unlock()
}

// newWatcher returns a watcher for roles and role bindings API
func newWatcher(cache permissionCache, name, apiServer string, rslver resolver.Interface) *watcher {
	// create context and cancel
	watchCtx, watchCancel := context.WithCancel(context.Background())

	watcher := &watcher{
		watchCtx:    watchCtx,
		watchCancel: watchCancel,
		stopFlag: syncFlag{
			flag: false,
		},
		cache: cache,
	}
	go watcher.runWatcher(name, apiServer, rslver)

	return watcher
}
