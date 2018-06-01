package apisrvpkg

import (
	"context"
	"fmt"
	"math"
	"sync"

	"github.com/pkg/errors"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api/cache"
	apiserver "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/safelist"
)

// apiSrv is the container type for the Api Server.
type apiSrv struct {
	sync.RWMutex
	// svcmap is the set of service modules registered.
	svcmap map[string]apiserver.ServiceBackend
	// services is the set of services registered with the API server.
	services map[string]apiserver.Service
	// messages is the set of messages serviced by the API server.
	messages map[string]apiserver.Message
	// hookregs is a collection of hooks registration callbacks.
	hookregs map[string]apiserver.ServiceHookCb
	// version is the native version of the API server. Can be changed
	//  on the command line when starting the API server.
	version string
	// Logger is used by the API server for all logging.
	Logger log.Logger
	// kvPool is a pool of kv store interfaces which the API server uses for all KV
	//  store operations.
	kvPool []kvstore.Interface
	// nextKv tracks the next pool item to be used from the KV Pool
	nextKv int
	// nextKvMutex protects nextKv
	nextKvMutex sync.Mutex
	// activeWatches is a slice of active KVs used by Watches.
	activeWatches *safelist.SafeList
	// doneCh is a error chan used to signal the apiSrv about async errors and exit
	doneCh chan error
	// runstate is set when the API server is ready to serve requests
	runstate struct {
		cond    *sync.Cond
		running bool
		addr    string
	}
	// config is the passed in config at Run.
	config apiserver.Config
	// apiChace is cache used by the server
	apiCache cache.Interface
}

// addKvConnToPool is adds connections to the pool if there is space in the pool.
func (a *apiSrv) addKvConnToPool() error {
	a.nextKvMutex.Lock()
	defer a.nextKvMutex.Unlock()
	cfg := a.config.Kvstore
	cfg.GrpcOptions = append(cfg.GrpcOptions, grpc.WithMaxMsgSize(math.MaxInt32))
	k, err := store.New(cfg)
	if err != nil {
		errors.Wrap(err, "could not create KV conn pool")
		return err
	}
	a.kvPool = append(a.kvPool, k)
	return nil
}

func (a *apiSrv) getKvConn() kvstore.Interface {
	a.nextKvMutex.Lock()
	defer a.nextKvMutex.Unlock()
	a.nextKv = (a.nextKv + 1) % a.config.KVPoolSize
	return a.kvPool[a.nextKv]
}

// singletonAPISrv is the singleton instance of the API server. This is
//  initialized exactly once and is guarded by the once.
var singletonAPISrv apiSrv
var once sync.Once

// initAPIServer performs all needed initializations.
func initAPIServer() {
	// This happens during init(). No lock is needed here
	singletonAPISrv.svcmap = make(map[string]apiserver.ServiceBackend)
	singletonAPISrv.services = make(map[string]apiserver.Service)
	singletonAPISrv.messages = make(map[string]apiserver.Message)
	singletonAPISrv.hookregs = make(map[string]apiserver.ServiceHookCb)
	singletonAPISrv.doneCh = make(chan error)
	singletonAPISrv.runstate.cond = &sync.Cond{L: &sync.Mutex{}}
	singletonAPISrv.activeWatches = safelist.New()
}

// reinitAPIServer performs needed initialization on reinit.
func reinitAPIServer() {
	defer singletonAPISrv.Unlock()
	singletonAPISrv.Lock()
	singletonAPISrv.services = make(map[string]apiserver.Service)
	singletonAPISrv.messages = make(map[string]apiserver.Message)
	singletonAPISrv.hookregs = make(map[string]apiserver.ServiceHookCb)
	singletonAPISrv.doneCh = make(chan error)
	singletonAPISrv.activeWatches = safelist.New()
}

// MustGetAPIServer returns the singleton instance. If it is not already
//  initialized, it initializes the singleton.
func MustGetAPIServer() apiserver.Server {
	once.Do(initAPIServer)
	return &singletonAPISrv
}

// GetAPIServerCache is a utility function to retrieve the cache used by the
//  singletonAPISrv. Usage is mostly retricted to test.
func GetAPIServerCache() cache.Interface {
	return singletonAPISrv.apiCache
}

// insertWatcher adds a new watcher context to the list of active Watchers
func (a *apiSrv) insertWatcher(ctx context.Context) (handle interface{}) {
	// We only need a read lock on the activeWatches safelist pointer. The list iteself is thread safe.
	defer a.RUnlock()
	a.RLock()
	handle = a.activeWatches.Insert(ctx)
	return
}

// removeWatcher removes a active Watcher from the list of active Watchers
func (a *apiSrv) removeWatcher(handle interface{}) {
	// We only need a read lock on the activeWatches safelist pointer. The list iteself is thread safe.
	defer a.RUnlock()
	a.RLock()
	a.activeWatches.Remove(handle)
}

// Register is the registration entrypoint used by the service backends/Modules. Each registration
//  could cover more than one service. Duplicate registration indicates a problem and not allowed.
// Registration is driven by the generated code.
func (a *apiSrv) Register(name string, svc apiserver.ServiceBackend) (apiserver.ServiceBackend, error) {
	if _, ok := a.svcmap[name]; ok {
		// Duplicate registration of the same service.
		panic(fmt.Sprintf("Duplicate registration for %s", name))
	}
	a.svcmap[name] = svc
	return a.svcmap[name], nil
}

// RegisterMessages is used by the service backends/modules to register all messages defined. This
//  usually happens once per package/Protofile. The registration is driven by auto-generated code.
func (a *apiSrv) RegisterMessages(svc string, msgs map[string]apiserver.Message) {
	for k, v := range msgs {
		a.messages[k] = v
	}
}

// RegisterService is used by the service backends/modules to register a defined service. The
//  registration is driven by auto-generated code.
func (a *apiSrv) RegisterService(name string, svc apiserver.Service) {
	if _, ok := a.services[name]; ok {
		panic(fmt.Sprintf("Duplicate service registration for %s", name))
	}
	a.services[name] = svc
}

// RegisterHookCb registers a callback to register hooks for a service. One callback can be registered per
//  service.
func (a *apiSrv) RegisterHooksCb(name string, fn apiserver.ServiceHookCb) {
	if _, ok := a.hookregs[name]; ok {
		panic(fmt.Sprintf("Duplicate hooks registration for %s", name))
	}
	a.hookregs[name] = fn
}

// GetService returns the registered service object give the name of the service.
func (a *apiSrv) GetService(name string) apiserver.Service {
	return a.services[name]
}

// Run is the eventloop for the API server. Registrations for all the registered services
//  are completed and a grpc listerner is started to serve the registered services.
func (a *apiSrv) Run(config apiserver.Config) {
	var ctx context.Context
	var err error
	var cancel context.CancelFunc
	{
		ctx = context.Background()
		ctx, cancel = context.WithCancel(ctx)
		defer cancel()
	}
	a.Lock()
	a.Logger = config.Logger
	a.version = config.Version
	a.config = config

	a.doneCh = make(chan error)
	if config.DebugMode {
		log.SetTraceDebug()
	}

	if config.KVPoolSize < 1 {
		a.config.KVPoolSize = apiserver.DefaultKvPoolSize
	}
	if config.BypassCache == false {
		cachecfg := cache.Config{
			Config:       config.Kvstore,
			NumKvClients: a.config.KVPoolSize,
			Logger:       config.Logger,
		}
		a.apiCache, err = cache.CreateNewCache(cachecfg)
		if err != nil {
			panic(fmt.Sprintf("failed to create cache (%s)", err))
		}
		// override PoolSize to 1
		a.config.KVPoolSize = 1
	}
	poolSize := a.config.KVPoolSize
	opts := []rpckit.Option{}
	if !config.DevMode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}
	// Create the GRPC connection for the server.
	var s *rpckit.RPCServer
	{
		s, err = rpckit.NewRPCServer(globals.APIServer, config.GrpcServerPort, opts...)
		if err != nil {
			panic(fmt.Sprintf("Could not start Server on port %v err(%s)", config.GrpcServerPort, err))
		}
		a.runstate.addr = s.GetListenURL()
	}

	// Let all the services complete registration.
	for name, svc := range a.svcmap {
		err = svc.CompleteRegistration(ctx, a.Logger, s, config.Scheme)
		if err != nil {
			panic(fmt.Sprintf("Failed to complete registration of %v (%v)", name, err))
		}
		a.Logger.Log("msg", "Registration complete", "backend", name)
	}

	// Callback all registration hooks that have been registered.
	for name, fn := range a.hookregs {
		if s, ok := a.services[name]; ok {
			a.Logger.Log("service", name, "msg", "calling hooks registration function")
			fn(s, a.Logger)
		} else {
			a.Logger.ErrorLog("Service", name, "msg", "service not known")
		}
	}

	if a.apiCache != nil {
		// connect to the cache provided. The cache will in turn connect to the KV store backend.
		a.nextKvMutex.Lock()
		a.kvPool = append(a.kvPool, a.apiCache)
		a.nextKvMutex.Unlock()
		err := a.apiCache.Start()
		if err != nil {
			panic(fmt.Sprintf("failed to start cache (%s)", err))
		}
	} else {
		// Connect to the KV Store
		for i := 0; i < poolSize; i++ {
			if err = a.addKvConnToPool(); err != nil {
				panic(fmt.Sprintf("could not create KV conn pool entry %d (%s)", i, err))
			}
		}
	}
	a.Logger.Log("msg", "added Kvstore connections to pool", "count", poolSize, "len", len(a.kvPool))
	a.Unlock()
	a.runstate.cond.L.Lock()
	a.Logger.Log("Grpc Listen Start", a.runstate.addr)
	s.Start()
	a.runstate.running = true
	a.runstate.cond.L.Unlock()
	a.runstate.cond.Broadcast()

	select {
	case donemsg := <-a.doneCh:
		config.Logger.Log("exit", "Done", "msg", donemsg)
		s.Stop()
		close(a.doneCh)
	case donemsg := <-s.DoneCh:
		config.Logger.Log("exit", "gRPC Server", "msg", donemsg)
	}
}

func (a *apiSrv) Stop() {
	a.Logger.Log("msg", "STOP Called")
	a.runstate.cond.L.Lock()
	a.runstate.running = false
	a.runstate.cond.L.Unlock()
	a.doneCh <- errors.New("Stop called by user")
	for {
		if _, ok := <-a.doneCh; !ok {
			a.Logger.Log("msg", "closing")
			break
		}
	}
	// Cleanup any remaining Watchers
	fn := func(i interface{}) {
		ctx := i.(context.Context)
		_, cancel := context.WithCancel(ctx)
		cancel()
	}
	a.activeWatches.RemoveAll(fn)

	a.nextKvMutex.Lock()
	for i := range a.kvPool {
		a.kvPool[i].Close()
	}
	a.kvPool = []kvstore.Interface{}
	a.nextKvMutex.Unlock()
	// Let all the services cleanup.
	for name, svc := range a.svcmap {
		svc.Reset()
		a.Logger.Log("msg", "Reset complete", "backend", name)
	}
	reinitAPIServer()
}

func (a *apiSrv) WaitRunning() {
	a.runstate.cond.L.Lock()
	for !a.runstate.running {
		a.runstate.cond.Wait()
	}
	a.runstate.cond.L.Unlock()
}

func (a *apiSrv) GetAddr() (string, error) {
	a.runstate.cond.L.Lock()
	defer a.runstate.cond.L.Unlock()
	if a.runstate.running {
		return a.runstate.addr, nil
	}
	return "", fmt.Errorf("not running")
}

func (a *apiSrv) getRunState() bool {
	a.runstate.cond.L.Lock()
	defer a.runstate.cond.L.Unlock()
	return a.runstate.running
}
