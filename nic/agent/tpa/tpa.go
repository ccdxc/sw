package tpa

import (
	"os"
	"strings"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	netagent "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/tpa/ctrlerif"
	mockdatapath "github.com/pensando/sw/nic/agent/tpa/datapath"
	"github.com/pensando/sw/nic/agent/tpa/state"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// PolicyAgent contains telemetry policy state
type PolicyAgent struct {
	netAgent *netagent.Nagent
	tpCtrler *ctrlerif.TpClient
	TpState  *state.PolicyState
	mode     protos.AgentMode
	datapath string
	hal      halproto.TelemetryClient
}

// NewPolicyAgent creates a new instance of telemetry policy agent
func NewPolicyAgent(nodeUUID string, ctrlerURL string, resolverClient resolver.Interface, mode protos.AgentMode, datapath string, netAgent *netagent.Nagent) (*PolicyAgent, error) {
	log.Infof("starting policy agent node:%s ctrler:%s mode:%s", nodeUUID, ctrlerURL, mode)

	agent := &PolicyAgent{
		mode:     mode,
		datapath: datapath,
		netAgent: netAgent,
	}

	// create new HAL client
	if strings.ToLower(datapath) == "hal" {
		srvURL := "localhost:"

		halPort := os.Getenv("HAL_GRPC_PORT")
		if halPort == "" {
			srvURL += "50054"
		} else {
			srvURL += halPort
		}

		// todo: retry & reconnect
		rpcClient, err := rpckit.NewRPCClient("hal", srvURL, rpckit.WithTLSProvider(nil))
		if err != nil {
			log.Errorf("failed to create grpc client for telemetry policy, URL: %s, err:%s", srvURL, err)
			return nil, err
		}

		agent.hal = halproto.NewTelemetryClient(rpcClient.ClientConn)

	} else {
		agent.hal = mockdatapath.MockHal()
	}

	tpAgent, err := state.NewTpAgent(netAgent, agent.hal, "")
	if err != nil {
		log.Errorf("Error creating telemetry policy state, Err: %v", err)
		return nil, err
	}

	agent.TpState = tpAgent

	// start grpc client to venice
	if mode == protos.AgentMode_MANAGED {
		ctrler, err := ctrlerif.NewTpClient(nodeUUID, tpAgent, globals.Tpm, resolverClient)
		if err != nil {
			log.Errorf("Error creating telemetry policy controller client. Err: %v", err)
			return nil, err
		}

		agent.tpCtrler = ctrler
	}

	return agent, nil
}

// Stop stops the agent
func (agent *PolicyAgent) Stop() {
	if agent.mode == protos.AgentMode_MANAGED {
		agent.tpCtrler.Stop()
	}
	agent.TpState.Close()
}
