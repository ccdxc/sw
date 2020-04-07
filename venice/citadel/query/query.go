package query

import (
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Server defines a query server
type Server struct {
	grpcSrv       *rpckit.RPCServer
	broker        broker.Inf
	diagSvc       diagnostics.Service
	moduleWatcher module.Watcher
}

// Option fills the optional params for Server
type Option func(*Server)

// WithDiagnosticsService passes a custom diagnostics service
func WithDiagnosticsService(diagSvc diagnostics.Service) Option {
	return func(q *Server) {
		q.diagSvc = diagSvc
		if q.grpcSrv != nil {
			diagnostics.RegisterService(q.grpcSrv.GrpcServer, diagSvc)
		}
	}
}

// WithModuleWatcher passes a module watcher
func WithModuleWatcher(moduleWatcher module.Watcher) Option {
	return func(q *Server) {
		q.moduleWatcher = moduleWatcher
	}
}

// NewQueryService creates an RPC server to handle queries from APIGW/gRPC
func NewQueryService(listenURL string, br broker.Inf, opts ...Option) (*Server, error) {
	s, err := rpckit.NewRPCServer(globals.Citadel, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Errorf("failed to start query grpc server: %v", err)
		return nil, err
	}

	srv := &Server{
		grpcSrv: s,
		broker:  br,
	}
	for _, o := range opts {
		o(srv)
	}
	telemetry_query.RegisterTelemetryV1Server(s.GrpcServer, srv)
	s.Start()

	log.Infof("citadel metrics query RPC server started at %s", listenURL)

	return srv, nil
}

// GetListenURL returns the gRPC listen URL of citadel server
func (q *Server) GetListenURL() string {
	return q.grpcSrv.GetListenURL()
}

// Stop stops the server
func (q *Server) Stop() {
	if q.moduleWatcher != nil { // stop module watcher if it has been initialized
		q.moduleWatcher.Stop()
	}
	if q.diagSvc != nil { // stop diagnostics service if it has been initialized
		q.diagSvc.Stop()
	}
	q.grpcSrv.Stop()
}
