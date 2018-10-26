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

const (
	IotaWorkloadImage = "registry.test.pensando.io:5000/pensando/iota/centos:1.0"
	naplesSimName     = "naples-sim"
)

var agentClient iota.IotaAgentApiClient
var stubNaplesContainer *Utils.Container
var controlIntf string

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
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: controlIntf,
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

	/* Create fake naples entity for ssh testing */
	naplesUserName := "root"
	naplesPassword := "root"
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "naplesnode", "sickp/alpine-sshd:7.5-r2"}
	if retCode, stdout, err := Utils.RunCmd(startCmd, 0, false, false, nil); err != nil || retCode != 0 {
		t.Errorf("Could not start docker container %v %v ", stdout, err)
	}
	stopCmd := []string{"docker", "stop", "naplesnode"}
	defer Utils.RunCmd(stopCmd, 0, false, false, nil)

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: "eth1",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	time.Sleep(5 * time.Second)
	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.Entities = []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{Name: "naples-host", Type: iota.EntityType_ENTITY_TYPE_HOST}}
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: controlIntf,
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}, NaplesIpAddress: "127.0.0.1", NaplesUsername: naplesUserName, NaplesPassword: naplesPassword}}

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

	triggerMsg := iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: naplesSimName,
			Command: "ls"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: naplesSimName,
			Command: "ping -c 10 127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: naplesSimName,
			Mode:    iota.CommandMode_COMMAND_BACKGROUND,
			Command: "watch -d df"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetHandle() != "", "handle not set")

	time.Sleep(3 * time.Second)
	triggeResp.TriggerOp = iota.TriggerOp_TERMINATE_ALL_CMDS
	triggeResp, err = agentClient.Trigger(context.Background(), triggeResp)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

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
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
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
	workload.NodeName = naplesSimName
	workload.Interface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER

	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "naplesnode", "sickp/alpine-sshd:7.5-r2"}
	if retCode, stdout, err := Utils.RunCmd(startCmd, 0, false, false, nil); err != nil || retCode != 0 {
		t.Errorf("Could not start docker container %v %v ", stdout, err)
	}
	stopCmd := []string{"docker", "stop", "naplesnode"}
	defer Utils.RunCmd(stopCmd, 0, false, false, nil)

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: naplesSimName,
		Entities: []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{Name: "naples-host", Type: iota.EntityType_ENTITY_TYPE_HOST}}}
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
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "test-workload",
			Command: "ls"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "test-workload",
			Command: "ping -c 10 127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: naplesSimName,
			Command: "ls -lrt"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "test-workload",
			Mode:    iota.CommandMode_COMMAND_BACKGROUND,
			Command: "echo HELLO WORLD && sleep 300"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")

	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetHandle() != "", "handle not set")

	triggeResp.TriggerOp = iota.TriggerOp_TERMINATE_ALL_CMDS
	triggeResp, err = agentClient.Trigger(context.Background(), triggeResp)
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
	iotaNode.NodeInfo = &iota.Node_NaplesConfig{NaplesConfig: &iota.NaplesConfig{ControlIntf: controlIntf,
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
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
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
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
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
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_BARE_METAL
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
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_BARE_METAL

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
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", EntityName: "",
			Command: "ls"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", EntityName: "",
			Command: "ping -c 10 127.0.0.1"}}}

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

	controlIntf = Utils.GetIntfsMatchingPrefix("e")[0]
}
