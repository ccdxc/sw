package techsupport

import (
	"net"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/protos/tsproto"
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/utils/log"
)

// RestServer has port and router for techsupport and diagnostics REST service
type RestServer struct {
	techsupportURL        string
	techsupportListener   net.Listener
	techsupportHTTPServer *http.Server
	tsCh                  chan<- tsproto.TechSupportRequest
	diagnosticsURL        string
	diagnosticsListener   net.Listener
	diagnosticsHTTPServer *http.Server
	cfg                   *tsconfig.TechSupportConfig
}

// NewRestServer creates a new techsupport server
func NewRestServer(techsupportURL string, diagnosticsURL string, tsCh chan<- tsproto.TechSupportRequest, cfg *tsconfig.TechSupportConfig) *RestServer {
	if techsupportURL == "" {
		log.Errorf("Cannot create Server. techsupportURL is empty.")
		return nil
	}

	return &RestServer{
		techsupportURL: techsupportURL,
		tsCh:           tsCh,
		diagnosticsURL: diagnosticsURL,
		cfg:            cfg,
	}
}

// Start starts the server on a port
func (s *RestServer) Start() {
	log.Infof("Starting Tech Support REST server on URL : " + s.techsupportURL)
	router := mux.NewRouter()
	router.HandleFunc("/api/techsupport/", s.CollectTechSupport).Methods("POST")
	log.Infof("Created Tech Support REST endpoints")

	techsupportListener, err := net.Listen("tcp", s.techsupportURL)
	if err != nil {
		log.Errorf("Failed to create techsupportListener for URL : %v. Err : %v", s.techsupportURL, err)
		return
	}
	s.techsupportListener = techsupportListener

	s.techsupportHTTPServer = &http.Server{Addr: s.techsupportURL, Handler: router}
	go s.techsupportHTTPServer.Serve(techsupportListener)
	log.Info("Started Tech Support REST server")

	if len(s.diagnosticsURL) > 0 {
		log.Infof("Creating Diagnostics endpoint")
		diagnosticsRouter := mux.NewRouter()
		diagnosticsRouter.HandleFunc("/api/diagnostics/", s.HandleDiagnostics).Methods("POST")
		diagnosticsListener, err := net.Listen("tcp", s.diagnosticsURL)
		if err != nil || diagnosticsListener == nil {
			log.Errorf("Failed to create diagnostics listener. Err : %v", err)
			return
		}
		s.diagnosticsListener = diagnosticsListener

		s.diagnosticsHTTPServer = &http.Server{Addr: s.diagnosticsURL, Handler: diagnosticsRouter}
		go s.diagnosticsHTTPServer.Serve(s.diagnosticsListener)
		log.Info("Started Diagnostics REST server")
	}

}

// Stop stops the REST server
func (s *RestServer) Stop() {
	log.Info("Stopping Tech Support REST server")

	if s.techsupportHTTPServer != nil {
		s.techsupportHTTPServer.Close()
	}

	s.techsupportHTTPServer = nil
	log.Info("Techsupport REST server stopped")

	log.Info("Stopping Diagnostics REST server")
	if s.diagnosticsHTTPServer != nil {
		s.diagnosticsHTTPServer.Close()
	}
	s.diagnosticsHTTPServer = nil
	log.Info("Diagnostics REST server stopped")
}
