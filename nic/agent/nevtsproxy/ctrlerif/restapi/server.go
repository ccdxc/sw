// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package restapi

import (
	"net"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif"
	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/types"
	genapi "github.com/pensando/sw/nic/agent/protos/generated/restapi/nevtsproxy"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
)

// this package contains the REST server implementation provided by the agent

// RestServer is the REST api server
type RestServer struct {
	listenURL  string       // URL where http server is listening
	listener   net.Listener // socket listener
	httpServer *http.Server // HTTP server
	handler    types.CtrlerIntf
	logger     log.Logger
}

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

// NewRestServer creates a new HTTP server
func NewRestServer(listenURL string, policyMgr *policy.Manager, logger log.Logger) (*RestServer, error) {
	handler, err := ctrlerif.NewEventPolicyHandler(policyMgr, logger)
	if err != nil {
		return nil, err
	}

	// create server instance
	srv := &RestServer{
		listenURL: listenURL,
		handler:   handler,
		logger:    logger,
	}

	esrv, _ := genapi.NewRestServer(handler, listenURL)

	// setup the top level routes
	router := mux.NewRouter()
	subRouter := router.PathPrefix("/api/eventpolicies/").Subrouter().StrictSlash(true)
	esrv.AddEventPolicyAPIRoutes(subRouter)

	log.Infof("Starting server at %s", listenURL)

	// listener
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("error starting listener. err: %v", err)
		return nil, err
	}
	srv.listener = listener

	// create a http server
	srv.httpServer = &http.Server{Addr: listenURL, Handler: router}
	go srv.httpServer.Serve(listener)

	return srv, nil
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
