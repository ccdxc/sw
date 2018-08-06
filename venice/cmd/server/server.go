package server

import (
	"flag"
	"fmt"

	"google.golang.org/grpc/grpclog"

	evtsapi "github.com/pensando/sw/api/generated/events"
	grpcserver "github.com/pensando/sw/venice/cmd/grpc/server"
	"github.com/pensando/sw/venice/cmd/rest"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
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
	recorder.Event(evtsapi.ServiceRunning, evtsapi.SeverityLevel_INFO, fmt.Sprintf("Service %s running on %s", globals.Cmd, utils.GetHostname()), nil)

	grpcserver.RunUnauthServer(":"+options.GRPCUnauthPort, nil)

	// Server for authenticated services is not started here because it needs a
	// certificate. It can only be started after the cluster is formed and the
	// CA is ready
}
