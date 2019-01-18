// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package restapi

import (
	"context"
	"expvar"
	"fmt"
	"net"
	"net/http"
	"net/http/pprof"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/ntranslate"

	"github.com/pensando/sw/api"

	tsdb "github.com/pensando/sw/venice/utils/ntsdb"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/log"
)

// this package contains the REST API provided by the agent

// RestServer is the REST api server
type RestServer struct {
	ctx               context.Context
	cancel            context.CancelFunc
	waitGrp           sync.WaitGroup
	listenURL         string                   // URL where http server is listening
	listener          net.Listener             // socket listener
	httpServer        *http.Server             // HTTP server
	keyTranslator     *ntranslate.Translator   // key to objMeta translator
	prefixRoutes      map[string]routeAddFunc  // REST API route add functions
	getPointsFuncList map[string]getPointsFunc // Get metrics points
}

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

type routeAddFunc func(*mux.Router, *RestServer)
type getPointsFunc func() ([]*tsdb.Point, error)

// NewRestServer creates a new HTTP server servicg REST api
func NewRestServer(ctx context.Context, listenURL string) (*RestServer, error) {

	ctx, cancel := context.WithCancel(ctx)

	// create server instance
	srv := RestServer{
		listenURL: listenURL,
		ctx:       ctx,
		cancel:    cancel,
	}

	// if no URL was specified, just return (used during unit/integ tests)
	if listenURL == "" {
		return &srv, nil
	}

	tstr := ntranslate.MustGetTranslator()
	if tstr == nil {
		return nil, fmt.Errorf("failed to get key to objMeta translator")
	}

	srv.keyTranslator = tstr

	// setup the top level routes
	router := mux.NewRouter()

	// update functions from auto-generated code
	srv.registerAPIRoutes()
	srv.registerListMetrics()

	for prefix, subRouter := range srv.prefixRoutes {
		sub := router.PathPrefix(prefix).Subrouter().StrictSlash(true)
		subRouter(sub, &srv)
	}
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/", pprof.Index)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/profile", pprof.Profile)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/trace", pprof.Trace)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)

	log.Infof("Starting server at %s", listenURL)

	// listener
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("Error starting listener. Err: %v", err)
		return nil, err
	}
	srv.listener = listener

	// create a http server
	srv.httpServer = &http.Server{Addr: listenURL, Handler: router}
	go srv.httpServer.Serve(listener)
	srv.waitGrp.Add(1)
	return &srv, nil
}

// getTagsFromMeta returns tags to store in Venice TSDB
func (s *RestServer) getTagsFromMeta(meta *api.ObjectMeta) map[string]string {
	return map[string]string{
		"tenant":    meta.Tenant,
		"namespace": meta.Namespace,
		"name":      meta.Name,
	}
}

// ReportMetrics sends metrics to tsdb
func (s *RestServer) ReportMetrics(frequency int) {
	defer s.waitGrp.Done()

	tsdbObj := map[string]tsdb.Obj{}

	// create tsdb objects
	for kind := range s.getPointsFuncList {
		obj, err := tsdb.NewObj(kind, nil, nil, nil)
		if err != nil {
			log.Errorf("failed to create tsdb object for kind: %s", kind)
			continue
		}

		if obj == nil {
			log.Errorf("found invalid tsdb object for kind: %s", kind)
			continue
		}

		tsdbObj[kind] = obj
	}

	for {
		select {
		case <-time.After(time.Duration(frequency) * time.Second):
			ts := time.Now()

			for kind, obj := range tsdbObj {
				mi, err := s.getPointsFuncList[kind]()
				if err != nil {
					log.Errorf("failed to get %s metrics, %s", kind, err)
					continue
				}

				// skip empty entry
				if len(mi) == 0 {
					continue
				}

				if err := obj.Points(mi, ts); err != nil {
					log.Errorf("failed to add <%s> metrics to tsdb, %s", kind, err)
					continue
				}

			}

		case <-s.ctx.Done():
			log.Infof("stopped reporting metrics")
			return
		}
	}
}

// GetListenURL returns the listen URL of the http server
func (s *RestServer) GetListenURL() string {
	if s.listener != nil {
		return s.listener.Addr().String()
	}

	return ""
}

// Stop stops the http server
func (s *RestServer) Stop() error {
	if s.httpServer != nil {
		s.httpServer.Close()
	}

	s.cancel()
	s.waitGrp.Wait()
	return nil
}
