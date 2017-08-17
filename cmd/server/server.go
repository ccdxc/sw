package server

import (
	"flag"

	"github.com/pensando/sw/cmd/env"
	grpcserver "github.com/pensando/sw/cmd/grpc/server"
	"github.com/pensando/sw/cmd/rest"
	"github.com/pensando/sw/cmd/server/options"
)

// Run starts a REST and gRPC server with the provided options.
func Run(options *options.ServerRunOptions) {
	flag.Parse()

	env.Options = options

	s := rest.NewRESTServer()
	go func() {
		s.RunOnAddr(":" + options.RESTPort)
	}()
	grpcserver.RunServer(":"+options.GRPCPort, "", "", "", nil)
}
