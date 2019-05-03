package server

import (
	"flag"
	"fmt"

	"github.com/pensando/sw/events/generated/eventtypes"
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

	s := rest.NewRESTServer()
	go func() {
		s.RunOnAddr(":" + options.RESTPort)
	}()

	log.Infof("%s is running", globals.Cmd)
	recorder.Event(eventtypes.SERVICE_RUNNING,
		fmt.Sprintf("Service %s running on %s", globals.Cmd, utils.GetHostname()), nil)

	clusterMgmtURL := ":" + options.ClusterMgmtPort
	localCertsURL := globals.Localhost + ":" + options.UnauthCertAPIPort
	nicRegURL := ":" + options.SmartNICRegistrationPort

	grpcserver.RunUnauthServer(clusterMgmtURL, localCertsURL, nicRegURL, nil)

	// Server for authenticated services is not started here because it needs a
	// certificate. It can only be started after the cluster is formed and the
	// CA is ready
}
