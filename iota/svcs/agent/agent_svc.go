package agent

import (
	"fmt"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

// IOTAAgentListenURL is the default URL for IOTA Agent
var IOTAAgentListenURL = fmt.Sprintf(":%d", common.IotaAgentPort)

// StartIOTAAgent starts IOTA Agent
func StartIOTAAgent(stubMode *bool) {
	agentSvc, err := common.CreateNewGRPCServer("IOTA Agent", IOTAAgentListenURL)
	if err != nil {
		log.Errorf("Could not start IOTA Agent. Err: %v", err)
	}
	// Change this to NewAgentService when ready to integrate
	if !*stubMode {
		agentHandler := NewAgentService()
		iota.RegisterIotaAgentApiServer(agentSvc.Srv, agentHandler)
	} else {
		agentHandler := NewAgentStubService()
		iota.RegisterIotaAgentApiServer(agentSvc.Srv, agentHandler)
	}
	agentSvc.Start()
}
