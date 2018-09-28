
// The above line will ensure that go doesn't treat this file as a test file. Hence skipping.
// If we copy this file as is, please remove the above build tag

package protos

import (
	"context"
	"encoding/json"
	"fmt"
	"github.com/pensando/sw/api/generated/security"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/rpckit"
	"os"
	"testing"
)

const iotaDefaultPort  = "1337"

func TestVeniceTypeE2E_FW_PERMIT_AND_DENY(t *testing.T) {
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
	topoResp, allocatedVlans, err := setup(topoClient)
	if err != nil || topoResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Test case set up failed. Err: %v", err)
	}

	// Check Cluster Health Here.
	healthResp, err := topoClient.CheckClusterHealth(context.Background(), topoResp)
	if healthResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Failed to check health of the cluster. Err: %v", healthResp.ApiResponse)
	}
	for _, h := range healthResp.Health {
		if h.HealthCode != iota.NodeHealth_HEALTH_OK {
			t.Errorf("Testbed unhealthy. HealthCode: %v | Node: %v", h.HealthCode, h.NodeName)
		}
	}

	// Push base configs
	cfgMsg := &iota.InitConfigMsg{
		EntryPointType: iota.InitConfigMsg_VENICE_REST,
		Endpoints: []string{"10.10.10.1"},
		Vlans: allocatedVlans,
	}
	cfgInitResp, err := cfgClient.InitCfgService(context.Background(), cfgMsg)
	if err != nil || cfgInitResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Config service Init failed. API Status: %v , Err: %v", cfgInitResp.ApiResponse.ApiStatus, err)
	}

	genCfgMsg := &iota.GenerateConfigMsg{

	}

	cfgGenResp, err := cfgClient.GenerateConfigs(context.Background(), genCfgMsg)
	// You can do anything with the generated configs. Edit the same and ask to be pushed, or push the generated base configs as is.
	// Or you can choose to build the cfgGenResp to be pushed.
	//generatedConfigs := cfgGenResp.GeneratedConfigs

	pushCfgResp, err := cfgClient.PushConfig(context.Background(), cfgGenResp)

	if err != nil {
		t.Errorf("Config service Push failed. API Status: %v , Err: %v", pushCfgResp.ApiResponse.ApiStatus, err)
	}


	queryMsg := &iota.ConfigQueryMsg{
		Kind: "SGPolicy",
			Selectors: []*iota.MatchSelector{
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
	cfgObj, err := cfgClient.QueryConfig(context.Background(), queryMsg)
	if err != nil {
		// Decide how to handle the error. Fail or Skip or ignore.
	}



	// Set up App
	workloads := []*iota.Workload{
		{
			WorkloadName: "ping-app",
			NodeName: "node-2",
		},
	}
	wrkLd := &iota.WorkloadMsg{
		WorkloadOp: iota.Op_ADD,
		Workloads: workloads,
	}

	appResp, err := topoClient.AddWorkloads(context.Background(), wrkLd)

	if err != nil || appResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Failed to instantiate Apps. %v", err)
	}
	epIP := appResp.Workloads[0].IpAddress

	pingCmd := fmt.Sprintf("ping -c 5 %v", epIP)
	trigMsg := &iota.TriggerMsg{
		TriggerOp: iota.TriggerMsg_EXEC_CMDS,
		CommandReq: []*iota.CommandReq{
			{
				Mode: iota.CommandReq_COMMAND_FOREGROUND,
				CommandOrPkt: pingCmd,
				WorkloadName: "ping-app",
			},
		},
	}

	// Trigger App
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)

	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Failed to trigger a ping. API Status: %v | Err: %v", triggerResp.ApiResponse.ApiStatus, err)
	}

	if triggerResp.CommandResp[0].ExitCode == 0 {
		t.Errorf("Ping trigger failed. Expected the ping to fail, but it passed instead. %v", triggerResp.CommandResp[0].ExitCode)
	}


	// Change the rule to permit and push

	for idx, sgp := range cfgObj.GeneratedConfigs {
		err = json.Unmarshal([]byte(sgp.Config), sgPolicy)
		if err != nil {
			// Decide how to handle the error. Fail or Skip or ignore.
		}
		// Handle updates for the SGPolicy here
		sgPolicy.Spec.Rules[0].Action = "PERMIT"

		b, _  := json.Marshal(sgPolicy)
		sgp.Config = string(b)
		cfgObj.GeneratedConfigs[idx] = sgp
	}

	pushCfgResp, err = cfgClient.PushConfig(context.Background(), cfgGenResp)

	if err != nil {
		t.Errorf("Config service Push failed. API Status: %v , Err: %v", pushCfgResp.ApiResponse.ApiStatus, err)
	}

	// Trigger App
	triggerResp, err = topoClient.Trigger(context.Background(), trigMsg)

	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.IotaAPIResponse_API_STATUS_OK {
		t.Errorf("Failed to trigger a ping. API Status: %v | Err: %v", triggerResp.ApiResponse.ApiStatus, err)
	}

	if triggerResp.CommandResp[0].ExitCode != 0 {
		t.Errorf("Ping trigger failed. Expected the ping to pass on a permit rule, but it failed instead. %v", triggerResp.CommandResp[0].ExitCode)
	}
}

func setup(client iota.TopologyApiClient) (*iota.NodeMsg, []uint32, error) {

	// Allocate VLANs
	testBedMsg := &iota.TestBedMsg{
		SwitchPortId: 1,
		NaplesImage: "sw/nic/nic.tar",
		VeniceImage: "venice.tar",
		DriverSources: "platform/bsd.tar",
	}

	resp, err := client.InitTestBed(context.Background(), testBedMsg)

	if err != nil {
		return nil, nil, err
	}
	allocatedVLans := resp.AllocatedVlans
	// Build Topology Object after parsing warmd.json and user spec. This can be done by harness or a language native utility
	nodes := &iota.NodeMsg{
		NodeOp: iota.Op_ADD,
		Nodes: []*iota.Node{
			{
				Type: iota.Node_PERSONALITY_VENICE,
				Image: "venice.tgz",
				IpAddress: "10.10.10.1",
			},
			{
				Type: iota.Node_PERSONALITY_VENICE,
				Image: "venice.tgz",
				IpAddress: "10.10.10.2",
			},
			{
				Type: iota.Node_PERSONALITY_VENICE,
				Image: "venice.tgz",
				IpAddress: "10.10.10.3",

			},
			{	Type: iota.Node_PERSONALITY_NAPLES,
				Image: "nic.tar",
				IpAddress: "10.10.10.4",

			},
			{	Type: iota.Node_PERSONALITY_NAPLES,
				Image: "nic.tar",
				IpAddress: "10.10.10.5",

			},
			{
				Type: iota.Node_PERSONALITY_NAPLES_WITH_QEMU,
				Image: "bsd_driver.tar",
				IpAddress: "10.10.10.6",
			},

		},
		MakeCluster: true,
	}

	addNodeResp, err := client.AddNodes(context.Background(), nodes)
	return addNodeResp, allocatedVLans, err
}


func createNewGRPCClient() (*rpckit.RPCClient, error) {
	iotaPort := os.Getenv("IOTA_SERVICE_PORT")
	if iotaPort == "" {
		iotaPort = iotaDefaultPort
	}
	srvURL := "localhost" + ":" + iotaPort
	return rpckit.NewRPCClient("iota", srvURL, rpckit.WithTLSProvider(nil))
}