package tpa

import (
	"os"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	netagent "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/tpa/ctrlerif"
	"github.com/pensando/sw/nic/agent/tpa/state"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// PolicyAgent contains telemetry policy state
type PolicyAgent struct {
	netAgent *netagent.Nagent
	tpCtrler *ctrlerif.TpClient
	TpState  *state.PolicyState
	datapath string
	hal      halproto.TelemetryClient
}

// NewPolicyAgent creates a new instance of telemetry policy agent
func NewPolicyAgent(netAgent *netagent.Nagent, getMgmtIPAddr func() string) (*PolicyAgent, error) {
	agent := &PolicyAgent{
		netAgent: netAgent,
	}

	// create new HAL client
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

	tpAgent, err := state.NewTpAgent(netAgent, getMgmtIPAddr)
	if err != nil {
		log.Errorf("Error creating telemetry policy state, Err: %v", err)
		return nil, err
	}

	agent.TpState = tpAgent
	return agent, nil

}

// NewTpClient creates a client to watch telemetry policy in Venice
func (agent *PolicyAgent) NewTpClient(nodeUUID string, resolverClient resolver.Interface) error {
	ctrler, err := ctrlerif.NewTpClient(nodeUUID, agent.TpState, globals.Tpm, resolverClient)
	if err != nil {
		log.Errorf("Error creating telemetry policy controller client. Err: %v", err)
		return err
	}
	agent.tpCtrler = ctrler
	return nil
}

// Stop stops the agent
func (agent *PolicyAgent) Stop() {
	if agent.tpCtrler != nil {
		agent.tpCtrler.Stop()
	}
	agent.TpState.Close()
}
