package query

import (
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Server defines a query server
type Server struct {
	grpcSrv *rpckit.RPCServer
	broker  broker.Inf
}

// NewQueryService creates an RPC server to handle queries from APIGW/gRPC
func NewQueryService(listenURL string, br broker.Inf) (*Server, error) {
	s, err := rpckit.NewRPCServer(globals.Citadel, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Errorf("failed to start query grpc server: %v", err)
		return nil, err
	}

	srv := &Server{
		grpcSrv: s,
		broker:  br,
	}

	telemetry_query.RegisterTelemetryV1Server(s.GrpcServer, srv)
	s.Start()

	log.Infof("citadel metrics query RPC server started at %s", listenURL)

	return srv, nil
}

// Stop stops the server
func (q *Server) Stop() {
	q.grpcSrv.Stop()
}
