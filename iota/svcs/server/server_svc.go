package server

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

// IOTAListenURL is the default URL for IOTA Service
var IOTAListenURL = fmt.Sprintf("localhost:%d", common.IotaSvcPort)

// StartIOTAService starts IOTA Service
func StartIOTAService() {
	serverSvc, err := common.CreateNewGRPCServer("IOTA Server", IOTAListenURL)
	if err != nil {
		log.Errorf("Could not start IOTA Service. Err: %v", err)
	}

	topoHandler := NewTopologyServiceHandler()
	cfgHandler := NewConfigServiceHandler()

	iota.RegisterTopologyApiServer(serverSvc.Srv, topoHandler)
	iota.RegisterConfigMgmtApiServer(serverSvc.Srv, cfgHandler)

	serverSvc.Start()
}
