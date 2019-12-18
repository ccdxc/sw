// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package restapi

import (
	"net"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	genapi "github.com/pensando/sw/nic/agent/protos/generated/restapi/netagent"
	tpagent "github.com/pensando/sw/nic/agent/protos/generated/restapi/tpagent"
	tsagent "github.com/pensando/sw/nic/agent/protos/generated/restapi/tsagent"

	tpa "github.com/pensando/sw/nic/agent/tpa/state/types"
	troubleshooting "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/utils/log"
)

// this package contains the REST API provided by the agent

// RestServer is the REST api server
type RestServer struct {
	listenURL  string                     // URL where http server is listening
	agent      types.CtrlerIntf           // net Agent API
	TsAgent    troubleshooting.CtrlerIntf //Troubleshooting agent
	TpAgent    tpa.CtrlerIntf             // telemetry policy agent
	listener   net.Listener               // socket listener
	httpServer *http.Server               // HTTP server
}

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

// MakeErrorResponse generates error response for MakeHTTPHandler() API
func MakeErrorResponse(code int, err error) (*Response, error) {
	res := &Response{
		StatusCode: code,
	}

	if err != nil {
		res.Error = err.Error()
	}

	return res, err
}

type routeAddFunc func(*mux.Router, *RestServer)
type subrouteAddFunc func(*mux.Router)

// NewRestServer creates a new HTTP server servicg REST api
func NewRestServer(agent types.CtrlerIntf, tsAgent troubleshooting.CtrlerIntf, tpAgent tpa.CtrlerIntf, listenURL string) (*RestServer, error) {
	// create server instance
	srv := RestServer{
		listenURL: listenURL,
		agent:     agent,
		TsAgent:   tsAgent,
		TpAgent:   tpAgent,
	}

	// if no URL was specified, just return (used during unit/integ tests)
	if listenURL == "" {
		return &srv, nil
	}

	nsrv, _ := genapi.NewRestServer(agent, listenURL)
	tsasrv, _ := tsagent.NewRestServer(tsAgent, listenURL)
	tpasrv, _ := tpagent.NewRestServer(tpAgent, listenURL)

	// setup the top level routes
	router := mux.NewRouter()
	prefixRoutes := map[string]subrouteAddFunc{
		"/api/networks/":              nsrv.AddNetworkAPIRoutes,
		"/api/endpoints/":             nsrv.AddEndpointAPIRoutes,
		"/api/interfaces/":            nsrv.AddInterfaceAPIRoutes,
		"/api/security/policies/":     nsrv.AddNetworkSecurityPolicyAPIRoutes,
		"/api/security/profiles/":     nsrv.AddSecurityProfileAPIRoutes,
		"/api/tunnels/":               nsrv.AddTunnelAPIRoutes,
		"/api/system/ports":           nsrv.AddPortAPIRoutes,
		"/api/apps":                   nsrv.AddAppAPIRoutes,
		"/api/vrfs":                   nsrv.AddVrfAPIRoutes,
		"/api/mirror/sessions/":       tsasrv.AddMirrorSessionAPIRoutes,
		"/api/telemetry/flowexports/": tpasrv.AddFlowExportPolicyAPIRoutes,
		"/api/telemetry/fwlog/":       tpasrv.AddFwlogPolicyAPIRoutes,
	}

	for prefix, subRouter := range prefixRoutes {
		sub := router.PathPrefix(prefix).Subrouter().StrictSlash(true)
		subRouter(sub)
	}

	localRoutes := map[string]routeAddFunc{
		"/api/system/info":  addNaplesInfoAPIRoutes,
		"/api/system/debug": addSystemDebugRoutes,
		"/api/mapping/":     addAPIMappingRoutes,
	}

	for prefix, subRouter := range localRoutes {
		sub := router.PathPrefix(prefix).Subrouter().StrictSlash(true)
		subRouter(sub, &srv)
	}

	if tpAgent != nil { // telemetry IPFIX policy debug
		router.Methods("GET").Subrouter().Handle("/debug/tpa", httputils.MakeHTTPHandler(tpAgent.Debug))
	}

	if tsAgent != nil {
		router.Methods("GET").Subrouter().Handle("/debug/tsa", http.HandlerFunc(tsAgent.Debug))
	}

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
