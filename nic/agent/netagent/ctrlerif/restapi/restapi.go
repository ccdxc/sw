// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package restapi

import (
	"expvar"
	"net"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/log"
)

// this package contains the REST API provided by the agent

// RestServer is the REST api server
type RestServer struct {
	listenURL  string           // URL where http server is listening
	agent      state.CtrlerIntf // net Agent API
	listener   net.Listener     // socket listener
	httpServer *http.Server     // HTTP server
}

type routeAddFunc func(*mux.Router, *RestServer)

// NewRestServer creates a new HTTP server servicg REST api
func NewRestServer(agent state.CtrlerIntf, listenURL string) (*RestServer, error) {
	// create server instance
	srv := RestServer{
		listenURL: listenURL,
		agent:     agent,
	}

	// if no URL was specified, just return (used during unit/integ tests)
	if listenURL == "" {
		return &srv, nil
	}

	// setup the top level routes
	router := mux.NewRouter()
	prefixRoutes := map[string]routeAddFunc{
		"/api/networks/":    addNetworkAPIRoutes,
		"/api/endpoints/":   addEndpointAPIRoutes,
		"/api/sgs/":         addSecurityGroupAPIRoutes,
		"/api/tenants/":     addTenantAPIRoutes,
		"/api/interfaces/":  addInterfaceAPIRoutes,
		"/api/namespaces/":  addNamespaceAPIRoutes,
		"/api/natpools/":    addNatPoolAPIRoutes,
		"/api/natpolicies/": addNatPolicyAPIRoutes,
	}

	for prefix, subRouter := range prefixRoutes {
		sub := router.PathPrefix(prefix).Subrouter().StrictSlash(true)
		subRouter(sub, &srv)
	}

	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("DELETE").Subrouter().Handle("/debug/vars", debug.ClearHandler())

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

	return &srv, nil
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

	return nil
}
