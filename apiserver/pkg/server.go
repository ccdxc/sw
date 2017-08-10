package apisrvpkg

import (
	"context"
	"errors"
	"fmt"
	"net"
	"sync"

	"google.golang.org/grpc"

	apiserver "github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
)

// apiSrv is the container type for the Api Server.
type apiSrv struct {
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
	// kv is the kv store interface where the API server performs all KV
	//  store operations.
	kv kvstore.Interface
	// doneCh is a error chan used to signal the apiSrv about async errors and exit
	doneCh chan error
	// runstate is set when the API server is ready to serve requests
	runstate struct {
		cond    *sync.Cond
		running bool
		addr    net.Addr
	}
}

// singletonAPISrv is the singleton instance of the API server. This is
//  initialized exactly once and is guarded by the once.
var singletonAPISrv apiSrv
var once sync.Once

// initAPIServer performs all needed initializations.
func initAPIServer() {
	singletonAPISrv.svcmap = make(map[string]apiserver.ServiceBackend)
	singletonAPISrv.services = make(map[string]apiserver.Service)
	singletonAPISrv.messages = make(map[string]apiserver.Message)
	singletonAPISrv.hookregs = make(map[string]apiserver.ServiceHookCb)
	singletonAPISrv.doneCh = make(chan error)
	singletonAPISrv.runstate.cond = &sync.Cond{L: &sync.Mutex{}}

}

// MustGetAPIServer returns the singleton instance. If it is not already
//  initialized, it initializes the singleton.
func MustGetAPIServer() apiserver.Server {
	once.Do(initAPIServer)
	return &singletonAPISrv
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
	a.services[name] = svc
}

// RegisterHookCb registers a callback to register hooks for a service. One callback can be registered per
//  service.
func (a *apiSrv) RegisterHooksCb(name string, fn apiserver.ServiceHookCb) {
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
	var cancel context.CancelFunc
	{
		ctx = context.Background()
		ctx, cancel = context.WithCancel(ctx)
		defer cancel()
	}
	a.Logger = config.Logger
	a.version = config.Version

	if config.DebugMode {
		log.SetTraceDebug()
	}

	ln, err := net.Listen("tcp", config.GrpcServerPort)
	if err != nil {
		panic(fmt.Sprintf("could not start a listener on port %v", config.GrpcServerPort))
	}
	a.runstate.addr = ln.Addr()
	a.Logger.Log("msg", "Started Listener", "Port", a.runstate.addr)

	// Create the GRPC connection for the server.
	var s *grpc.Server
	{
		s = grpc.NewServer()
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

	// Connect to the KV Store
	a.kv, err = store.New(config.Kvstore)
	if err != nil {
		panic(fmt.Sprintf("Could not connect to KV Store (%s)", err))
	}

	go func() {
		a.runstate.cond.L.Lock()
		a.Logger.Log("Grpc Listen Start", a.runstate.addr)
		a.runstate.running = true
		a.runstate.cond.L.Unlock()
		a.runstate.cond.Broadcast()

		a.doneCh <- s.Serve(ln)
		close(a.doneCh)
	}()
	donemsg := <-a.doneCh
	s.Stop()
	config.Logger.Log("exit", donemsg)
}

func (a *apiSrv) Stop() {
	a.doneCh <- errors.New("Stop called by user")
	for {
		if _, ok := <-a.doneCh; !ok {
			a.Logger.Log("msg", "closing")
			break
		}
	}
}

func (a *apiSrv) WaitRunning() {
	a.runstate.cond.L.Lock()
	for !a.runstate.running {
		a.runstate.cond.Wait()
	}
	a.runstate.cond.L.Unlock()
}

func (a *apiSrv) GetAddr() (net.Addr, error) {
	a.runstate.cond.L.Lock()
	defer a.runstate.cond.L.Unlock()
	if a.runstate.running {
		return a.runstate.addr, nil
	}
	return nil, fmt.Errorf("not running")
}
