package apigwpkg

import (
	"context"
	"fmt"
	"net/http"
	"net/http/pprof"
	"strings"
	"sync"

	"google.golang.org/grpc"

	"github.com/pensando/sw/apigw"
	"github.com/pensando/sw/utils/log"
)

type apiGw struct {
	svcmap  map[string]apigw.ApiGwService
	svcname map[string]string
	logger  log.Logger
	doneCh  chan error
}

// Singleton API Gateway Object with init gaurded by the Once.
var sinletonApiGw apiGw
var once sync.Once

// MustGetApiGateway gets the singleton API Gateway Object. Initialize if not already done.
func MustGetApiGateway() apigw.ApiGateway {
	once.Do(initApiGw)
	return &sinletonApiGw
}

// initApiGw funciton is called exactly once. (Guarded by the once)
// All inititializaion for the apiGw object goes here.
func initApiGw() {
	sinletonApiGw.svcmap = make(map[string]apigw.ApiGwService)
	sinletonApiGw.svcname = make(map[string]string)
	sinletonApiGw.doneCh = make(chan error)
}

// Register a service with the APi Gateway service. Duplicate registrations
// are not allowed and will cause a panic. Each service is expected to serve
// non overlapping API path prefixes.
func (a *apiGw) Register(name, path string, svc apigw.ApiGwService) apigw.ApiGwService {
	fmt.Printf("Register for %v\n", name)
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
func (a *apiGw) GetService(name string) apigw.ApiGwService {
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
			a.logger.Errorf("Could not find Version")
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

	// Http Connection
	m := http.NewServeMux()

	// Create the GRPC connection for the server.
	s := grpc.NewServer()

	// Let all the services complete registration. All services serveed by this
	// gateway should have registered themselves via their init().
	for name, svc := range a.svcmap {
		config.Logger.Log("Svc", name, "msg", "RegisterComplete")
		err := svc.CompleteRegistration(ctx, config.Logger, s, m)
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
		a.logger.InfoLog("msg", "Http Listen Start", "address", config.HttpAddr)
		a.doneCh <- http.ListenAndServe(config.HttpAddr, a.extractHdrInfo(a.allowCORS(m)))
	}()

	a.logger.Infof("exit", <-a.doneCh)
}
