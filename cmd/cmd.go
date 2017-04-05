package main

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/server"
	"github.com/pensando/sw/cmd/server/options"
	"github.com/pensando/sw/utils/runtime"
)

func main() {
	env.Scheme = runtime.NewScheme()
	env.Scheme.AddKnownTypes(&api.Cluster{}, &api.Node{})

	server.Run(options.NewServerRunOptions())
}
