package apigwpkg

import (
	"context"
	"errors"
	"fmt"
	"net"
	"net/http"
	"net/http/pprof"
	"strings"
	"sync"

	"google.golang.org/grpc"

	opentracing "github.com/opentracing/opentracing-go"
	otext "github.com/opentracing/opentracing-go/ext"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type apiGw struct {
	svcmap  map[string]apigw.APIGatewayService
	svcname map[string]string
	logger  log.Logger
	doneCh  chan error
	// runstate is set when the API server is ready to serve requests
	runstate struct {
		cond    *sync.Cond
		running bool
		addr    net.Addr
	}
	apiSrvOverride string
}

// Singleton API Gateway Object with init gaurded by the Once.
var sinletonAPIGw apiGw
var once sync.Once

// MustGetAPIGateway gets the singleton API Gateway Object. Initialize if not already done.
func MustGetAPIGateway() apigw.APIGateway {
	once.Do(initAPIGw)
	return &sinletonAPIGw
}

// initAPIGw funciton is called exactly once. (Guarded by the once)
// All inititializaion for the apiGw object goes here.
func initAPIGw() {
	sinletonAPIGw.svcmap = make(map[string]apigw.APIGatewayService)
	sinletonAPIGw.svcname = make(map[string]string)
	sinletonAPIGw.doneCh = make(chan error)
	sinletonAPIGw.runstate.cond = &sync.Cond{L: &sync.Mutex{}}
}

// Register a service with the APi Gateway service. Duplicate registrations
// are not allowed and will cause a panic. Each service is expected to serve
// non overlapping API path prefixes.
func (a *apiGw) Register(name, path string, svc apigw.APIGatewayService) apigw.APIGatewayService {
	if svc == nil {
		panic(fmt.Sprintf("Invalid service registration for %s", name))
	}
	if _, ok := a.svcmap[name]; ok {
		// Detected duplicate Registration. Cannot do much, this is probably
		// happening during Init. Panic!!
		panic(fmt.Sprintf("Duplicate registration for %s", name))
	}
	a.svcmap[name] = svc
	a.svcname[name] = name
	return a.svcmap[name]
}

// GetService returns registered Service, nil if not found.
func (a *apiGw) GetService(name string) apigw.APIGatewayService {
	return a.svcmap[name]
}

// extractHdrInfo transfers request headers set here to grpc header fields
// and subsequently used by the API server.
func (a *apiGw) extractHdrInfo(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// All URIs are of the form /<version>/<service>/<resource>
		p := strings.SplitN(r.URL.Path, "/", 3)
		if len(p) > 1 && p[1] != "" {
			r.Header.Set(apigw.GrpcMDRequestVersion, p[1])
		} else {
			a.logger.Errorf("Could not find Version (%s)", r.URL.Path)
		}
		if len(p) > 2 {
			r.Header.Set(apigw.GrpcMDRequestURI, "/"+p[2])
		}
		r.Header.Set(apigw.GrpcMDRequestMethod, r.Method)
		next.ServeHTTP(w, r)
	})
}

// preflightHandler sets common headers needed. Used to respond to preflight handler checks
//  from clients.
func (a *apiGw) preflightHandler(w http.ResponseWriter, r *http.Request) {
	headers := []string{"Content-Type", "Accept"}
	w.Header().Set("Access-Control-Allow-Headers", strings.Join(headers, ","))
	methods := []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"}
	w.Header().Set("Access-Control-Allow-Methods", strings.Join(methods, ","))
	a.logger.DebugLog("msg", "preflight request", "URL", r.URL.Path)
	return
}

func (a *apiGw) tracerMiddleware(h http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		operationName := "http"

		span := opentracing.StartSpan(operationName)
		otext.HTTPMethod.Set(span, r.Method)
		otext.HTTPUrl.Set(span, r.URL.String())
		ctx := opentracing.ContextWithSpan(r.Context(), span)
		req := r.WithContext(ctx)

		h.ServeHTTP(w, req)
		span.Finish()
		return
	})
}

// allowCORS enables CORS and returns other preflight check headers.
func (a *apiGw) allowCORS(h http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if origin := r.Header.Get("Origin"); origin != "" {
			w.Header().Set("Access-Control-Allow-Origin", origin)
			if r.Method == "OPTIONS" && r.Header.Get("Access-Control-Request-Method") != "" {
				// TODO(sanjayt): This is treating the OPTIONS request as a *. Make it more granular
				//  to resource level.
				a.preflightHandler(w, r)
				return
			}
		}
		h.ServeHTTP(w, r)
	})
}

// Run starts the "eventloop" for the gateway. Completes registration for all the services registered
// and then starts servicing requests.
func (a *apiGw) Run(config apigw.Config) {
	var ctx context.Context
	var cancel context.CancelFunc
	{
		ctx = context.Background()
		ctx, cancel = context.WithCancel(ctx)
		defer cancel()
	}

	a.logger = config.Logger
	a.apiSrvOverride = config.APIServerOverride

	// Http Connection
	m := http.NewServeMux()

	// Create the GRPC connection for the server.
	s := grpc.NewServer()

	ln, err := net.Listen("tcp", config.HTTPAddr)
	if err != nil {
		panic(fmt.Sprintf("could not start a listener on port %v", config.HTTPAddr))
	}
	a.runstate.addr = ln.Addr()
	a.logger.Log("msg", "Started Listener", "Port", a.runstate.addr)
	var rslvr resolver.Interface
	{
		if len(config.Resolvers) > 0 {
			rslvr = resolver.New(&resolver.Config{Name: "apigw", Servers: config.Resolvers})
		}
	}
	a.logger.Infof("Resolving via %v", config.Resolvers)
	// Let all the services complete registration. All services served by this
	// gateway should have registered themselves via their init().
	for name, svc := range a.svcmap {
		config.Logger.Log("Svc", name, "msg", "RegisterComplete")
		err := svc.CompleteRegistration(ctx, config.Logger, s, m, rslvr)
		if err != nil {
			panic(fmt.Sprintf("Failed to complete registration of %v (%v)", name, err))
		}
	}

	// Now RUN!
	if config.DebugMode {
		m.Handle("/_debug/pprof/", http.HandlerFunc(pprof.Index))
		m.Handle("/_debug/pprof/cmdline", http.HandlerFunc(pprof.Cmdline))
		m.Handle("/_debug/pprof/profile", http.HandlerFunc(pprof.Profile))
		m.Handle("/_debug/pprof/symbol", http.HandlerFunc(pprof.Symbol))
		m.Handle("/_debug/pprof/trace", http.HandlerFunc(pprof.Trace))
		// Enable tracedumping with SIGQUIT or ^\
		// Will dump stacktrace for all go routines
		log.SetTraceDebug()
	}

	go func() {
		a.runstate.cond.L.Lock()
		a.logger.InfoLog("msg", "Http Listen Start", "address", config.HTTPAddr)
		a.runstate.running = true
		a.runstate.cond.Broadcast()
		a.runstate.cond.L.Unlock()

		a.doneCh <- http.Serve(ln, a.extractHdrInfo(a.allowCORS(a.tracerMiddleware(m))))
	}()

	a.logger.Infof("exit", <-a.doneCh)
}

func (a *apiGw) Stop() {
	a.doneCh <- errors.New("User called stop")
}

// WaitRunning blocks till the API gateway is completely initialized
func (a *apiGw) WaitRunning() {
	a.runstate.cond.L.Lock()
	for !a.runstate.running {
		a.runstate.cond.Wait()
	}
	a.runstate.cond.L.Unlock()
}

// GetAddr returns the address at which the API gateway is listening
//   returns error if the API gateway is not initialized
func (a *apiGw) GetAddr() (net.Addr, error) {
	a.runstate.cond.L.Lock()
	defer a.runstate.cond.L.Unlock()
	if a.runstate.running {
		return a.runstate.addr, nil
	}
	return nil, fmt.Errorf("not running")
}

// GetApiServerAddr gets the API gateway address to connect to
func (a *apiGw) GetAPIServerAddr(addr string) string {
	// Currently this is only used to override the API server address.
	//  returns the override address if set or return the original address
	if a.apiSrvOverride != "" {
		return a.apiSrvOverride
	}
	return addr
}
