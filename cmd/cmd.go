package main

import (
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/server"
	"github.com/pensando/sw/cmd/server/options"
	"github.com/pensando/sw/utils/runtime"
)

func main() {
	env.Scheme = runtime.NewScheme()
	env.Scheme.AddKnownTypes(&cmd.Cluster{}, &cmd.Node{})

	server.Run(options.NewServerRunOptions())
}
