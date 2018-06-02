package server

import (
	"flag"

	"google.golang.org/grpc/grpclog"

	grpcserver "github.com/pensando/sw/venice/cmd/grpc/server"
	"github.com/pensando/sw/venice/cmd/rest"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Run starts a REST and gRPC server with the provided options.
func Run(options *options.ServerRunOptions) {
	flag.Parse()

	grpclog.SetLogger(log.GetNewLogger(log.GetDefaultConfig("cmd")))
	s := rest.NewRESTServer()
	go func() {
		s.RunOnAddr(":" + options.RESTPort)
	}()

	log.Infof("%s is running", globals.Cmd)

	grpcserver.RunUnauthServer(":"+options.GRPCUnauthPort, nil)

	// Server for authenticated services is not started here because it needs a
	// certificate. It can only be started after the cluster is formed and the
	// CA is ready
}
