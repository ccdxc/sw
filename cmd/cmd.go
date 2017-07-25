package main

import (
	"fmt"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/server"
	"github.com/pensando/sw/cmd/server/options"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pensando/sw/utils/systemd"
)

func main() {
	env.Scheme = runtime.NewScheme()
	env.Scheme.AddKnownTypes(&cmd.Cluster{}, &cmd.Node{})

	// We need to issue equivalent of 'systemctl daemon-reload' before anything else to make systemd read the config files
	s := systemd.New()
	err := s.DaemonReload()
	if err != nil {
		fmt.Printf("Error %v while issuing systemd.DaemonReload at startup", err)
	}

	server.Run(options.NewServerRunOptions())
}
