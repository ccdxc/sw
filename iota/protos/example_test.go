// +build ignore

// The above line will ensure that go doesn't treat this file as a test file. Hence skipping.
// If we copy this file as is, please remove the above build tag

package protos

import (
	"context"
	"encoding/json"
	"github.com/pensando/sw/api/generated/security"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/rpckit"
	"os"
	"testing"
)

const iotaDefaultPort  = "1337"

func TestVeniceTypeE2E_FWLog(t *testing.T) {
	var topoClient iota.TopologyApiClient
	var cfgClient iota.ConfigMgmtApiClient

	var sgPolicy security.SGPolicy

	iotaClient, err := createNewGRPCClient()
	if err != nil {
		t.Errorf("IOTA Service Unavailable. Err: %v", err)
	}

	// Instantiate IOTA Clients
	topoClient = iota.NewTopologyApiClient(iotaClient.ClientConn)
	cfgClient = iota.NewConfigMgmtApiClient(iotaClient.ClientConn)

	// Instantiate Topology
	topo, topoResp, err := setup(topoClient)
	if err != nil || topoResp.InstantiateTopoReponse.APIStatus != iota.IotaAPIResponse_API_STATUS_OK.String() {
		t.Errorf("Test case set up failed. Err: %v", err)
	}

	// Check Cluster Health Here.
	healthResp, err := topoClient.CheckClusterHealth(context.Background(), topo)
	for _, h := range healthResp.Status {
		if h.HealthCheckResponse.APIStatus != iota.IotaAPIResponse_API_STATUS_OK.String() || h.Health != iota.NodeStatus_HEALTH_OK.String(){
			t.Errorf("Testbed unhealthy. HealthCode: %v | Err: %v", h.Health, err)
		}
	}

	// Push base configs
	cfgResp, err := cfgClient.GenerateAndPushBaseConfigs(context.Background(), topo)
	if err != nil || cfgResp.GeneratedConfigResponse.APIStatus != iota.IotaAPIResponse_API_STATUS_OK.String() {
		t.Errorf("Base configs push failed. Err: %v", err)
	}
	// You can do anything with the pushed configs. or use a query api to give you a config object that matches some parameter
	pushedConfigs := cfgResp.ConfigInfo.Configs

	query := &iota.CfgQueryParameters{
		ObjectMode: iota.ConfigTopologyInfo_VENICE_REST.String(),
		ObjectKind: "SGPolicy",
		Match: []*iota.MatchSelector{
			{
				Key: "O.Tenant",
				Value: "default",
			},
			{
				Key: "SGRule.Action",
				Value: "DENY",
			},
			{
				Key: "SGRule.ToAddresses",
				Value: "any",
			},
			{
				Key: "SGRule.FromAddresses",
				Value: "10.0.0.1/30",
			},
		},
	}

	cfgObj, err := cfgClient.QueryConfig(context.Background(), query)
	if err != nil {
		// Decide how to handle the error. Fail or Skip or ignore.
	}

	err = json.Unmarshal([]byte(cfgObj.Config), sgPolicy)
	if err != nil {
		// Decide how to handle the error. Fail or Skip or ignore.
	}


	// Set up App
	pingApp := &iota.App{
		AppType: iota.App_CMD_APP.String(),
		AppConfig: "ping -c 10.0.0.1 10.0.0.2",
		AppCommand: &iota.Command{
			CommandType: iota.Command_COMMAND_FOREGROUND.String(),
		},
	}


	appResp, err := topoClient.InstantiateApps(context.Background(), pingApp)

	if err != nil || appResp.InstantiateAppResponse.APIStatus != iota.IotaAPIResponse_API_STATUS_OK.String() {
		t.Errorf("Failed to instantiate Apps. %v", err)
	}


	// Trigger App
	triggerResp, err := topoClient.TriggerApp(context.Background(), pingApp)

	// Make sure that the ping returned non zero, since we are testing deny
	if err != nil || triggerResp.ExitCode == 0 {
		t.Errorf("Ping was expected to fail with deny rule. It passed instead")
	}

	// Change policy to permit

	for idx, r := range sgPolicy.Spec.Rules {
		if r.Action == "DENY" && r.ToIPAddresses[0] == "any" && r.FromIPAddresses[0] == "10.0.0.1/30" {
			sgPolicy.Spec.Rules[idx].Action = "PERMIT"
		}
	}
	b, _ := json.Marshal(sgPolicy)

	// Push the new object
	newCfg := &iota.ConfigObject{
		Method: iota.ConfigObject_UPDATE.String(),
		ConfigEntrypoint: iota.ConfigTopologyInfo_VENICE_REST.String(),
		Config: string(b),
	}
	cfgPushResp, err := cfgClient.PushConfig(context.Background(), newCfg)

	if err != nil && cfgPushResp.ConfigPushResponse.APIStatus != iota.IotaAPIResponse_API_STATUS_OK.String() {
		t.Errorf("Failed to push updated config. Cfg: %v | Err: %v", newCfg, err)
	}

	// Trigger App again.

	triggerResp, err = topoClient.TriggerApp(context.Background(), pingApp)

	// Make sure that the ping returned non zero, since we are testing deny
	if err != nil || triggerResp.ExitCode != 0 {
		t.Errorf("Ping was expected to pass with PERMIT rule. It failed instead")
	}



}

func setup(client iota.TopologyApiClient) (*iota.ConfigTopologyInfo, *iota.InstantiateTopoResponse, error) {

	// Build Topology Object after parsing warmd.json and user spec. This can be done by harness or a language native utility
	nodes := []*iota.Node{
		{
			PersonalityType: iota.Node_PERSONALITY_VENICE.String(),
			Image: "venice.tgz",
			IPAddress: "10.10.10.1",
		},
		{
			PersonalityType: iota.Node_PERSONALITY_VENICE.String(),
			Image: "venice.tgz",
			IPAddress: "10.10.10.2",
		},
		{
			PersonalityType: iota.Node_PERSONALITY_VENICE.String(),
			Image: "venice.tgz",
			IPAddress: "10.10.10.3",

		},
		{	PersonalityType: iota.Node_PERSONALITY_NAPLES.String(),
			Image: "nic.tar",
			IPAddress: "10.10.10.4",

		},
		{	PersonalityType: iota.Node_PERSONALITY_NAPLES.String(),
			Image: "nic.tar",
			IPAddress: "10.10.10.5",

		},
		{
			PersonalityType: iota.Node_PERSONALITY_NAPLES_WITH_QEMU.String(),
			Image: "bsd_driver.tar",
			IPAddress: "10.10.10.6",
		},
	}


	topo := &iota.ConfigTopologyInfo{
		ConfigEntrypoint: iota.ConfigTopologyInfo_VENICE_REST.String(),
		RestEndpoint: []string {"10.10.10.1"},
		Nodes: nodes,
		VLANs: []uint32{10,11,12,13,14,15,16,17,18,19},
	}

	resp, err := client.InstantiateTopology(context.Background(), topo)
	return topo, resp, err
}


func createNewGRPCClient() (*rpckit.RPCClient, error) {
	iotaPort := os.Getenv("IOTA_SERVICE_PORT")
	if iotaPort == "" {
		iotaPort = iotaDefaultPort
	}
	srvURL := "localhost" + ":" + iotaPort
	return rpckit.NewRPCClient("iota", srvURL, rpckit.WithTLSProvider(nil))
}