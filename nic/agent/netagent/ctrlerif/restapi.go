// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"net"
	"net/http"

	"github.com/gorilla/mux"
	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/nic/agent/netagent/state"
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

// Catchall http handler
func unknownAction(w http.ResponseWriter, r *http.Request) {
	log.Infof("Unknown REST URL %q", r.URL.Path)
	w.WriteHeader(503)
}

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

	// setup the routes
	router := mux.NewRouter()
	t := router.Methods("GET").Subrouter()
	t.HandleFunc("/api/networks/", httputils.MakeHTTPHandler(srv.networkListHandler))
	t.HandleFunc("/api/endpoints/", httputils.MakeHTTPHandler(srv.endpointListHandler))
	t.HandleFunc("/api/sgs/{*}", httputils.MakeHTTPHandler(srv.sgListHandler))

	t.HandleFunc("/api/{*}", unknownAction)

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

func (s *RestServer) networkListHandler(r *http.Request) (interface{}, error) {
	// return the list of all networks
	return s.agent.ListNetwork(), nil
}

func (s *RestServer) endpointListHandler(r *http.Request) (interface{}, error) {
	// return the list of all endpoints
	return s.agent.ListEndpoint(), nil
}

func (s *RestServer) sgListHandler(r *http.Request) (interface{}, error) {
	// return the list of all sgs
	return s.agent.ListSecurityGroup(), nil
}
