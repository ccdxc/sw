package server

import (
	"fmt"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/server/cfg"
	"github.com/pensando/sw/iota/svcs/server/topo"
)

// IOTAListenURL is the default URL for IOTA Service
var IOTAListenURL = fmt.Sprintf("localhost:%d", common.IotaSvcPort)

// StartIOTAService starts IOTA Service
func StartIOTAService(stub bool) {
	serverSvc, err := common.CreateNewGRPCServer("IOTA Server", IOTAListenURL)
	if err != nil {
		log.Errorf("Could not start IOTA Service. Err: %v", err)
	}

	if stub {
		topoHandler := topo.NewStubTopologyService()
		iota.RegisterTopologyApiServer(serverSvc.Srv, topoHandler)

	} else {
		topoHandler := topo.NewTopologyServiceHandler()
		iota.RegisterTopologyApiServer(serverSvc.Srv, topoHandler)
	}
	cfgHandler := cfg.NewConfigServiceHandler()

	iota.RegisterConfigMgmtApiServer(serverSvc.Srv, cfgHandler)

	serverSvc.Start()
}
