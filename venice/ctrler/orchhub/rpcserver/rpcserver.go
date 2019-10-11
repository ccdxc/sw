package rpcserver

import (
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// OrchServer defines a grpc server
type OrchServer struct {
	name          string
	server        *rpckit.RPCServer
	listenURL     string
	diagSvc       diagnostics.Service
	moduleWatcher module.Watcher
	errCh         chan error
}

// Option fills the optional params for Server
type Option func(*OrchServer)

// WithDiagnosticsService passes a custom diagnostics service
func WithDiagnosticsService(diagSvc diagnostics.Service) Option {
	return func(q *OrchServer) {
		q.diagSvc = diagSvc
		if q.server != nil {
			diagnostics.RegisterService(q.server.GrpcServer, diagSvc)
		}
	}
}

// WithModuleWatcher passes a module watcher
func WithModuleWatcher(moduleWatcher module.Watcher) Option {
	return func(q *OrchServer) {
		q.moduleWatcher = moduleWatcher
	}
}

// NewOrchServer creates and starts a grpc server
func NewOrchServer(listenURL string, opts ...Option) (*OrchServer, error) {
	name := "OrchHub-API"
	server, err := rpckit.NewRPCServer(globals.OrchHub, listenURL)
	if err != nil {
		log.Infof("failed to listen: %v", err)
		return nil, err
	}
	orchServer := &OrchServer{
		name:      name,
		server:    server,
		listenURL: listenURL,
	}
	for _, o := range opts {
		o(orchServer)
	}
	orchServer.errCh = server.DoneCh
	orchServer.server.Start()
	log.Infof("OrchHub server started at %s", listenURL)
	return orchServer, nil
}

// ErrOut returns a channel that gives an error indication
func (s *OrchServer) ErrOut() <-chan error {
	return s.errCh
}

// StopServer stops the server
func (s *OrchServer) StopServer() {
	if s.moduleWatcher != nil { // stop module watcher if it has been initialized
		s.moduleWatcher.Stop()
	}
	if s.diagSvc != nil { // stop diagnostics service if it has been initialized
		s.diagSvc.Stop()
	}
	if s.server != nil {
		s.server.Stop()
		s.server = nil
		log.Infof("OrchHub grpc server at %s stopped", s.listenURL)
	}
}
