package agent

import (
	"context"
	"fmt"
	"log"
	"os"
	"testing"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	"github.com/pensando/sw/iota/svcs/common"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

var agentClient iota.IotaAgentApiClient
var stubNaplesContainer *Utils.Container

func stubRunCmd(cmdArgs []string, timeout int, background bool, shell bool,
	env []string) (int, string, error) {
	return 0, "", nil
}

func stubGetContainer(name string,
	registry string, containerID string, mountDir string) (*Utils.Container, error) {
	var err error
	stubNaplesContainer, err = Utils.NewContainer(naplesSimName, "alpine", "", "")
	if err != nil {
		log.Fatal("container create failed")
	}
	return stubNaplesContainer, nil
}

func TestMain(m *testing.M) {
	go func() {
		stubMode := false
		Utils.Run = stubRunCmd
		Utils.GetContainer = stubGetContainer
		StartIOTAAgent(&stubMode)
	}()

	// Ensure the service is up
	time.Sleep(time.Second * 2)

	c, err := common.CreateNewGRPCClient("test-client", IOTAAgentListenURL)
	if err != nil {
		fmt.Println("Could not create a GRPC Client to the IOTA Agent Server")
		os.Exit(1)
	}

	agentClient = iota.NewIotaAgentApiClient(c.Client)

	Utils.RunCmd([]string{"mkdir", "-p", common.DstIotaAgentDir}, 0, false, false, nil)
	runTests := m.Run()
	os.Exit(runTests)

}

func TestAgentService_Node_Naples_Add_Delete(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "eth1",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "en0",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	resp, err = agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

	iotaNodeHealth := &iota.NodeHealth{NodeName: "naples"}
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	TestUtils.Assert(t, iotaNodeHealth.GetHealthCode() != iota.NodeHealth_HEALTH_OK, "Node health not ok!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	nodeResp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Node_Venice_Add_Delete(t *testing.T) {

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	iotaNode.NodeInfo = &iota.Node_VeniceConfig{VeniceConfig: &iota.VeniceConfig{ControlIntf: "lo",
		ControlIp: "10.1.1.3/24", VenicePeers: []*iota.VenicePeer{&iota.VenicePeer{HostName: "test", IpAddress: "1.2.3.4/24"},
			&iota.VenicePeer{HostName: "Test", IpAddress: "1.2.3.4/24"}}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "venice"
	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	workload.WorkloadName = "test-workload"
	workload.NodeName = "venice"
	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	resp, err = agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	nodeResp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Node_Naples_Hw_Add_Delete(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "eth1",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "en0",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	resp, err = agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

	iotaNodeHealth := &iota.NodeHealth{NodeName: "naples"}
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	TestUtils.Assert(t, iotaNodeHealth.GetHealthCode() == iota.NodeHealth_HEALTH_OK, "Node health not ok!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	nodeResp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Workload_Add_Delete(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workloadDir = "/tmp"
	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkload(context.Background(), &workload)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Workload_Trigger(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	triggerMsg := iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", WorkloadName: "test-workload", Command: "lsas"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", WorkloadName: "test-workload", Command: "ping -c 10 127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Node_Mellanox_Add_Delete(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "en0",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}
	resp, err = agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

	iotaNodeHealth := &iota.NodeHealth{NodeName: "naples"}
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	TestUtils.Assert(t, iotaNodeHealth.GetHealthCode() == iota.NodeHealth_HEALTH_OK, "Node health not ok!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}
	nodeResp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Mellanox_Workload_Add_Delete(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workloadDir = "/tmp"
	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkload(context.Background(), &workload)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_MELLANOX, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Naples_Hw_Workload_Add_Delete(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workloadDir = "/tmp"
	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkload(context.Background(), &workload)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Naples_Hw_baremetal_Workload_Add_Delete(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = ""
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workloadDir = "/tmp"
	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkload(context.Background(), &workload)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_baremetal_Workload_Trigger(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = ""
	workload.NodeName = "naples"
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err := agentClient.AddWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	triggerMsg := iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", WorkloadName: "", Command: "lsas"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", WorkloadName: "", Command: "ping -c 10 127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadResp, err = agentClient.DeleteWorkload(context.Background(), &workload)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloadStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func init() {
	runArpCmd = func(app workload, ip string, intf string) error {
		return nil
	}
}
