package techsupport

import (
	"net"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
)

// RestServer has port and router for techsupport REST service
type RestServer struct {
	listenURL  string
	listener   net.Listener
	httpServer *http.Server
	tsCh       chan<- tsproto.TechSupportRequest
}

// NewRestServer creates a new techsupport server
func NewRestServer(port string, tsCh chan<- tsproto.TechSupportRequest) *RestServer {
	if port == "" {
		log.Errorf("Cannot create Server. Port is empty.")
		return nil
	}
	listenURL := ":" + port

	return &RestServer{
		listenURL: listenURL,
		tsCh:      tsCh,
	}
}

// Start starts the server on a port
func (s *RestServer) Start() {
	log.Infof("Starting Tech Support REST server on URL : " + s.listenURL)
	router := mux.NewRouter()
	router.HandleFunc("/api/v1/techsupport/collect", s.CollectTechSupport).Methods("POST")
	router.HandleFunc("/api/v1/techsupport/list", s.ListTechSupportRequests).Methods("GET")
	log.Infof("Created REST endpoints")

	listener, err := net.Listen("tcp", s.listenURL)
	if err != nil {
		log.Errorf("Failed to create listener for URL : %v. Err : %v", s.listenURL, err)
		return
	}
	s.listener = listener

	s.httpServer = &http.Server{Addr: s.listenURL, Handler: router}
	go s.httpServer.Serve(listener)

	log.Infof("Start Done!")
}

// Stop stops the REST server
func (s *RestServer) Stop() {
	log.Infof("Stopping Tech Support REST server")

	if s.httpServer != nil {
		s.httpServer.Close()
	}

	s.httpServer = nil
}
