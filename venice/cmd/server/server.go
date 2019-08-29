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

	s := rest.NewDebugRESTServer()
	debugURL := globals.Localhost + ":" + globals.CMDDebugPort
	go func() {
		s.RunOnAddr(debugURL)
	}()
	log.Infof("Debug endpoint started at %s", debugURL)

	clusterMgmtURL := ":" + options.ClusterMgmtPort
	localCertsURL := globals.Localhost + ":" + options.UnauthCertAPIPort
	nicRegURL := ":" + options.SmartNICRegistrationPort

	go grpcserver.RunUnauthServer(clusterMgmtURL, localCertsURL, nicRegURL, nil)
	log.Infof("gRPC Endpoints: ClusterMgmt API %s, Local Certs API %s, SmartNIC registration: %s",
		clusterMgmtURL, localCertsURL, nicRegURL)

	// Server for authenticated services is not started here because it needs a
	// certificate. It can only be started after the cluster is formed and the
	// CA is ready

	log.Infof("%s is running", globals.Cmd)
	recorder.Event(eventtypes.SERVICE_RUNNING,
		fmt.Sprintf("Service %s running on %s", globals.Cmd, utils.GetHostname()), nil)

}
