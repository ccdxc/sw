package server

import (
	"flag"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/grpc"
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
	grpc.RunServer(":"+options.GRPCPort, "", "", "", nil)
}
