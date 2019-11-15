package apisrvpkg

import (
	"context"
	"fmt"
	"math"
	goruntime "runtime"
	"sync"

	"github.com/pkg/errors"
	"golang.org/x/time/rate"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api/cache"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	diagnostics "github.com/pensando/sw/venice/utils/diagnostics"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/safelist"
)

var (
	// The maximum number of back-to-back connections accepted by the listener
	defaultListenerConnectionBurst = 100
	// The maximum rate of incoming connetions accepted by the listener
	defaultListenerConnectionRateLimit = rate.Limit(5.0) // connections/s
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
	// kvPoolsize is the current size of the pool, only update under nextKvMutex
	kvPoolsize int
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
	// flags are runtime flags in use
	flags apiserver.Flags
	// maintModeMutex guards the maintMode and maintReason
	maintModeMutex sync.RWMutex
	// maintMode is set to true if the server is in maintenance mode
	maintMode bool
	// maintReason specifies reason for maintenance mode.
	maintReason string
	// apiChace is cache used by the server
	apiCache apiintf.CacheInterface
	// objGraph maintains a graph of all relations between objects
	apiGraph graph.Interface
	// rslvrURIs used for service discovery (for use by hooks)
	rslvrURIs []string
	// newLocalOverlayFunc creates a new local overlay. indirection is mainly for testing.
	newLocalOverlayFunc func(tenant, id, baseKey string, c apiintf.CacheInterface, apisrv apiserver.Server) (apiintf.OverlayInterface, error)
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
	a.kvPoolsize++
	return nil
}

func (a *apiSrv) getKvConn() kvstore.Interface {
	a.nextKvMutex.Lock()
	defer a.nextKvMutex.Unlock()
	if a.kvPoolsize < 1 {
		return nil
	}
	a.nextKv = (a.nextKv + 1) % a.kvPoolsize
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
	singletonAPISrv.newLocalOverlayFunc = cache.NewLocalOverlay
	singletonAPISrv.flags = apiserver.Flags{InternalParams: make(map[string]string)}
}

// MustGetAPIServer returns the singleton instance. If it is not already
//  initialized, it initializes the singleton.
func MustGetAPIServer() apiserver.Server {
	once.Do(initAPIServer)
	return &singletonAPISrv
}

// GetAPIServerCache is a utility function to retrieve the cache used by the
//  singletonAPISrv. Usage is mostly retricted to test.
func GetAPIServerCache() apiintf.CacheInterface {
	return singletonAPISrv.apiCache
}

// SetAPIServerCache force sets the Cache for the singleton
//  Usage RESTRICTED TO TEST
func SetAPIServerCache(in apiintf.CacheInterface) {
	singletonAPISrv.apiCache = in
}

// insertWatcher adds a new watcher context to the list of active Watchers
func (a *apiSrv) insertWatcher(ctx context.Context) (handle interface{}) {
	// We only need a read lock on the activeWatches safelist pointer. The list iteself is thread safe.
	defer a.RUnlock()
	a.RLock()
	handle = a.activeWatches.Insert(ctx)
	return handle
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

// GetMessage returns the registered service object give the name of the service.
func (a *apiSrv) GetMessage(svc, kind string) apiserver.Message {
	name := svc + "." + kind
	ret, ok := a.messages[name]
	if !ok {
		return nil
	}
	return ret
}

// CreateOverlay creates a new overlay on top of API server cache
func (a *apiSrv) CreateOverlay(tenant, name, base string) (apiintf.CacheInterface, error) {
	if a.apiCache != nil {
		return cache.NewOverlay(tenant, name, base, a.apiCache, a, false)
	}
	return nil, errors.New("cache not found")
}

// GetGraphDB returns the graph DB in use by the Server
func (a *apiSrv) GetGraphDB() graph.Interface {
	return a.apiGraph
}

// Run is the event loop for the API server. Registrations for all the registered services
//  are completed and a grpc listener is started to serve the registered services.
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
	a.runstate.cond.L.Lock()
	if a.runstate.running {
		a.Logger.Infof("API server is already running")
		a.runstate.cond.L.Unlock()
		a.Unlock()
		return
	}
	a.runstate.cond.L.Unlock()
	a.Logger = config.Logger
	a.version = config.Version
	a.config = config
	a.flags.AllowMultiTenant = a.config.AllowMultiTenant

	a.doneCh = make(chan error)
	if config.DebugMode {
		log.SetTraceDebug()
	}

	if config.KVPoolSize < 1 {
		a.config.KVPoolSize = apiserver.DefaultKvPoolSize
	}
	if config.BypassCache == false {
		cachecfg := cache.Config{
			APIServer:    a,
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

	a.apiGraph, err = graph.NewCayleyStore()
	if err != nil {
		a.Logger.Fatalf("could not create graph (%s)", err)
	}
	diagSvc := diagsvc.GetDiagnosticsService(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, config.Logger)
	if err := diagSvc.RegisterHandler("Debug", diagapi.DiagnosticsRequest_Stats.String(), diagsvc.NewExpVarHandler(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, config.Logger)); err != nil {
		a.Logger.ErrorLog("method", "GetDiagnosticsServiceWithDefaults", "msg", "failed to register expvar handler", "err", err)
	}
	diagSvc.RegisterCustomAction("list-watchers", func(action string, params map[string]string) (interface{}, error) {
		return a.apiCache.DebugAction(action, nil), nil
	})

	diagSvc.RegisterCustomAction("internal-params", func(action string, params map[string]string) (interface{}, error) {
		for k, v := range params {
			a.flags.InternalParams[k] = v
		}
		return a.flags, nil
	})

	diagSvc.RegisterCustomAction("stack-trace", func(action string, params map[string]string) (interface{}, error) {
		buf := make([]byte, 1<<20)
		blen := goruntime.Stack(buf, true)
		return fmt.Sprintf("=== goroutine dump ====\n %s \n=== END ===", buf[:blen]), nil
	})

	opts := []rpckit.Option{}
	if !config.DevMode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}
	if config.Logger != nil {
		opts = append(opts, rpckit.WithLogger(config.Logger))
	}
	// For ApiServer we need higher connection burst/rate because all controllers
	// open multiple connections at startup time
	opts = append(opts, rpckit.WithListenerRateLimitConfig(
		&rpckit.RateLimitConfig{
			R: defaultListenerConnectionRateLimit,
			B: defaultListenerConnectionBurst,
		}))
	// Create the GRPC connection for the server.
	var s *rpckit.RPCServer
	{
		s, err = rpckit.NewRPCServer(globals.APIServer, config.GrpcServerPort, opts...)
		if err != nil {
			panic(fmt.Sprintf("Could not start Server on port %v err(%s)", config.GrpcServerPort, err))
		}
		a.runstate.addr = s.GetListenURL()

		// Register the Diagnostics service
		diagnostics.RegisterService(s.GrpcServer, diagSvc)
	}

	a.rslvrURIs = config.Resolvers

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
		a.kvPoolsize++
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

	a.apiCache.Restore()
	a.Logger.Log("msg", "added Kvstore connections to pool", "count", poolSize, "len", len(a.kvPool))
	a.runstate.cond.L.Lock()
	a.Logger.Log("Grpc Listen Start", a.runstate.addr)
	a.runstate.running = true
	s.Start()
	a.runstate.cond.L.Unlock()
	a.runstate.cond.Broadcast()
	a.Unlock()
	recorder.Event(eventtypes.SERVICE_RUNNING, fmt.Sprintf("Service %s running on %s", globals.APIServer, utils.GetHostname()), nil)

	select {
	case donemsg := <-a.doneCh:
		config.Logger.Log("exit", "Done", "msg", donemsg)
		s.Stop()
		a.cleanup()
		close(a.doneCh)
	case donemsg := <-s.DoneCh:
		config.Logger.Log("exit", "gRPC Server", "msg", donemsg)
		a.cleanup()
	}
}

// cleanup closes and releases resources used by the api server.
func (a *apiSrv) cleanup() {
	a.runstate.cond.L.Lock()
	a.runstate.running = false
	a.runstate.cond.L.Unlock()
	a.runstate.cond.Broadcast()

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
	a.kvPoolsize = 0
	a.kvPool = []kvstore.Interface{}
	a.nextKvMutex.Unlock()
}

// reinit performs needed initialization in preparation for a restart.
func (a *apiSrv) reinit() {
	for name, svc := range singletonAPISrv.svcmap {
		svc.Reset()
		singletonAPISrv.Logger.Log("msg", "Reset complete", "backend", name)
	}
	singletonAPISrv.services = make(map[string]apiserver.Service)
	singletonAPISrv.messages = make(map[string]apiserver.Message)
	singletonAPISrv.doneCh = make(chan error)
	singletonAPISrv.activeWatches = safelist.New()
	singletonAPISrv.rslvrURIs = nil
}

// Stop sends a stop signal to the API server
func (a *apiSrv) Stop() {
	a.Logger.Log("msg", "STOP Called")
	defer a.Unlock()
	a.Lock()
	a.runstate.cond.L.Lock()
	if !a.runstate.running {
		a.Logger.Infof("API server is already stopped")
		a.runstate.cond.L.Unlock()
		return
	}
	a.runstate.cond.L.Unlock()
	a.doneCh <- errors.New("Stop called by user")
	for {
		if _, ok := <-a.doneCh; !ok {
			a.Logger.Log("msg", "closing")
			break
		}
	}

	a.reinit()
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

// GetVersion returns the native API version
func (a *apiSrv) GetVersion() string {
	if running := a.getRunState(); running {
		defer a.RUnlock()
		a.RLock()
		return a.version
	}
	return a.version
}

func (a *apiSrv) getRunState() bool {
	a.runstate.cond.L.Lock()
	defer a.runstate.cond.L.Unlock()
	return a.runstate.running
}

// RuntimeFlags returns runtime flags in use by the Server
func (a *apiSrv) RuntimeFlags() apiserver.Flags {
	return a.flags
}

func (a *apiSrv) SetRuntimeControls(ctrl apiserver.Controls) {
	defer a.maintModeMutex.Unlock()
	a.maintModeMutex.Lock()
	a.maintMode = ctrl.MaintMode
	a.maintReason = ctrl.MaintReason
}

func (a *apiSrv) getMaintState() (bool, string) {
	defer a.maintModeMutex.RUnlock()
	a.maintModeMutex.RLock()
	return a.maintMode, a.maintReason
}

// GetResolver returns the resolver initialized by API Server for use by hooks that need to do non-local work.
//  returns nil on error
func (a *apiSrv) GetResolvers() []string {
	return a.rslvrURIs
}
