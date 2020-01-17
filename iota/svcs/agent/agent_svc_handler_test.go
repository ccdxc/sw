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
	Workload "github.com/pensando/sw/iota/svcs/agent/workload"
	"github.com/pensando/sw/iota/svcs/common"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

const (
	IotaWorkloadImage = "registry.test.pensando.io:5000/pensando/iota/centos:1.1"
	naplesSimName     = "naples-sim"
	naplesUserName    = "root"
	naplesPassword    = "root"
)

var agentClient iota.IotaAgentApiClient
var stubNaplesContainer *Utils.Container
var controlIntf string

func stubRunCmd(cmdArgs []string, timeout int, background bool, shell bool,
	env []string) (int, string, error) {
	return 0, "", nil
}

func stubGetContainer(name string,
	registry string, containerID string, mountDir string, privileged bool) (*Utils.Container, error) {
	var err error
	stubNaplesContainer, err = Utils.NewContainer(naplesSimName, "alpine", "", "", privileged)
	if err != nil {
		log.Fatal("container create failed")
	}
	return stubNaplesContainer, nil
}

func RestartIOTAAgent() {
	stubMode := false
	StopIOTAAgent()
	time.Sleep(1 * time.Second)
	go StartIOTAAgent(&stubMode)
	time.Sleep(1 * time.Second)
	c, err := common.CreateNewGRPCClient("test-client", IOTAAgentListenURL, 0)
	if err != nil {
		fmt.Println("Could not create a GRPC Client to the IOTA Agent Server")
		os.Exit(1)
	}
	agentClient = iota.NewIotaAgentApiClient(c.Client)
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

	os.Mkdir(common.DstIotaAgentDir, 0777)
	os.Mkdir(common.DstIotaEntitiesDir, 0777)
	os.Mkdir(common.ImageArtificatsDirectory, 0777)

	c, err := common.CreateNewGRPCClient("test-client", IOTAAgentListenURL, 0)
	if err != nil {
		fmt.Println("Could not create a GRPC Client to the IOTA Agent Server")
		os.Exit(1)
	}

	agentClient = iota.NewIotaAgentApiClient(c.Client)

	Utils.RunCmd([]string{"mkdir", "-p", common.DstIotaAgentDir}, 0, false, false, nil)
	Utils.RunCmd([]string{"cp", common.NicFinderConf, common.DstIotaAgentDir}, 0, false, false, nil)
	runTests := m.Run()
	os.Exit(runTests)

}

func TestAgentService_Node_Multi_Naples_Add_Delete(t *testing.T) {
	/*
		iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM, Name: "naples-multi"}
		iotaNode.NodeInfo = &iota.Node_NaplesMultiSimConfig{NaplesMultiSimConfig: &iota.NaplesMultiSimConfig{
			Gateway: "172.17.0.1", Parent: "eth0", Network: "172.17.0.0/24", NumInstances: 1,
			VeniceIps: []string{"10.1.1.3/24"}}}

		nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
		if err != nil {
			t.Errorf("Delete Node call failed. Err: %v", err)
		}

		TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

		resp, err := agentClient.AddNode(context.Background(), iotaNode)
		if err != nil {
			t.Errorf("Add Node call failed. Err: %v", err)
		}

		TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

		iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM, Name: "naples-multi"}
		resp, err = agentClient.AddNode(context.Background(), iotaNode)
		if err != nil {
			t.Errorf("Add Node call failed. Err: %v", err)
		}

		TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

		iotaNodeHealth := &iota.NodeHealth{NodeName: "naples-multi"}
		if err != nil {
			t.Errorf("Check health call failed. Err: %v", err)
		}
		iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
		if err != nil {
			t.Errorf("Check health call failed. Err: %v", err)
		}
		TestUtils.Assert(t, iotaNodeHealth.GetHealthCode() != iota.NodeHealth_HEALTH_OK, "Node health not ok!")

		iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM, Name: "naples-multi"}
		nodeResp, err = agentClient.DeleteNode(context.Background(), iotaNode)
		if err != nil {
			t.Errorf("Delete Node call failed. Err: %v", err)
		}

		TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
	*/
}

func TestAgentService_Node_Naples_Add_Delete(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{
		NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
			VeniceIps: []string{"10.1.1.3/24"}}}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}

	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{
		NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
			VeniceIps: []string{"10.1.1.3/24"}}}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
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
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
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
	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}

	workload.WorkloadName = "test-workload"
	workload.NodeName = "venice"

	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	workload.WorkloadName = "test-workload"
	workload.NodeName = "venice"
	workloadMsg = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete workload success!")

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
	startNaplesNode()
	defer stopNaplesNode()

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}

	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "eth1",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	time.Sleep(5 * time.Second)
	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.Entities = []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{Name: "naples-host", Type: iota.EntityType_ENTITY_TYPE_HOST}}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: controlIntf,
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
		VeniceIps: []string{"10.1.1.3/24"}, NaplesIpAddress: "127.0.0.1", NaplesUsername: naplesUserName, NaplesPassword: naplesPassword}}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
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
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	//Workload.WorkloadDir = "/tmp"

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}

	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)

	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.GetWorkloads()[0].GetWorkloadStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")
	TestUtils.Assert(t, workloadResp.GetWorkloads()[0].GetInterface() == workload.ParentInterface+"_500", "Interface match")
	TestUtils.Assert(t, workloadResp.GetWorkloads()[0].GetMacAddress() == workload.MacAddress, "Mac match")

	//Try to add again.
	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err1 = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

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
	workload.ParentInterface = "test"
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
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload succeded!")

	triggerMsg := iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "test-workload",
			Command: "ls"}}}

	triggeResp, err := agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "test-workload",
			Command: "pwd"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: naplesSimName, EntityName: "naples-host",
			Command: "ping -c 10 127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
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
			Command: "ping 127.0.0.1", ForegroundTimeout: 5}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == true, "Command timed out")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetStdout() != "", "Command stdout")

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
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == false, "Command not timed out")

	triggeResp.TriggerOp = iota.TriggerOp_TERMINATE_ALL_CMDS
	triggeResp, err = agentClient.Trigger(context.Background(), triggeResp)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == false, "Command not timed out")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err1 = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: "venice"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Node_Mellanox_Add_Delete(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_ThirdPartyNicConfig{ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{
		NicType: "mellanox"}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_ThirdPartyNicConfig{ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{
		NicType: "mellanox"}}
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

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}
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
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	//Workload.WorkloadDir = "/tmp"
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_ThirdPartyNicConfig{ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{
		NicType: "mellanox"}}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkloads(context.Background(), workloadMsg)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err1 = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func startNaplesNode() {
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "naplesnode", "sickp/alpine-sshd:7.5-r2"}
	if retCode, stdout, err := Utils.RunCmd(startCmd, 0, false, false, nil); err != nil || retCode != 0 {
		log.Fatalf("Could not start docker container %v %v ", stdout, err)
	}

	time.Sleep(2 * time.Second)
	mkdir := []string{"docker", "exec", "naplesnode", "mkdir", "/mnt"}
	exitCode, stdout, _ := Utils.RunCmd(mkdir, 0, false, true, nil)
	fmt.Println("Exit code stodu ", exitCode, stdout)
	touchCmd := []string{"docker", "exec", "naplesnode", "touch", "/mnt/sysuuid"}
	exitCode, stdout, _ = Utils.RunCmd(touchCmd, 0, false, true, nil)
	fmt.Println("Exit code stodu ", exitCode, stdout)
	createUUID := []string{"docker", "exec", "naplesnode", "sh", "-c", "'", "echo", "{ \\\"mac-address\\\" : \\\"11:33:44:55:66:77\\\" }", ">", "/mnt/sysuuid", "'"}
	exitCode, stdout, _ = Utils.RunCmd(createUUID, 0, false, true, nil)
	fmt.Println("Exit code stodu ", exitCode, stdout)

}

func stopNaplesNode() {
	stopCmd := []string{"docker", "stop", "naplesnode"}
	Utils.RunCmd(stopCmd, 0, false, false, nil)
}

func TestAgentService_Naples_Hw_Workload_Add_Delete(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER

	startNaplesNode()
	defer stopNaplesNode()

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)

	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.Entities = []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{
		Name: "naples", Type: iota.EntityType_ENTITY_TYPE_NAPLES}}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{NaplesUsername: naplesUserName, NaplesPassword: naplesPassword,
		NaplesIpAddress: "127.0.0.1", NicType: "pensando", Name: naplesSimName}}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")
	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	//Try to add again.
	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err1 = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

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
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_BARE_METAL

	startNaplesNode()
	defer stopNaplesNode()

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Add workload success!")

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.Entities = []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{
		Name: "naples", Type: iota.EntityType_ENTITY_TYPE_NAPLES}}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{NaplesUsername: naplesUserName, NaplesPassword: naplesPassword, NaplesIpAddress: "127.0.0.1",
		NicType: "pensando", Name: naplesSimName}}}}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadResp, err = agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	//Try to add again.
	workloadResp, _ = agentClient.AddWorkloads(context.Background(), workloadMsg)

	fmt.Println(workloadResp)

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

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
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.IpPrefix = "1.1.1.1/24"
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_BARE_METAL

	startNaplesNode()
	defer stopNaplesNode()

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	iotaNode.Entities = []*iota.Entity{&iota.Entity{Name: naplesSimName, Type: iota.EntityType_ENTITY_TYPE_NAPLES}, &iota.Entity{
		Name: "naples", Type: iota.EntityType_ENTITY_TYPE_NAPLES}}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{NaplesUsername: naplesUserName, NaplesPassword: naplesPassword, NaplesIpAddress: "127.0.0.1",
		NicType: "pensando", Name: naplesSimName}}}}
	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, err := agentClient.AddWorkloads(context.Background(), workloadMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload success!")

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

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", EntityName: "",
			Mode:    iota.CommandMode_COMMAND_BACKGROUND,
			Command: "ping  127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetHandle() != "", "Handle set")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == false, "Handle set")

	time.Sleep(3 * time.Second)
	triggeResp.TriggerOp = iota.TriggerOp_TERMINATE_ALL_CMDS
	triggeResp, err = agentClient.Trigger(context.Background(), triggeResp)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetHandle() != "", "Handle not set")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == false, "Handle set")

	triggerMsg = iota.TriggerMsg{TriggerOp: iota.TriggerOp_EXEC_CMDS,
		Commands: []*iota.Command{&iota.Command{NodeName: "naples", EntityName: "",
			ForegroundTimeout: 5,
			Command:           "ping  127.0.0.1"}}}

	triggeResp, err = agentClient.Trigger(context.Background(), &triggerMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}

	TestUtils.Assert(t, triggeResp.GetApiResponse().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Trigger msg failed!")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetHandle() == "", "Handle set")
	TestUtils.Assert(t, triggeResp.GetCommands()[0].GetTimedOut() == true, "Handle set")

	workloadDelMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{&workload}}
	workloadDelResp, err1 := agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err1 != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete workload failed!")

	workloadDelResp, err1 = agentClient.DeleteWorkloads(context.Background(), workloadDelMsg)
	if err != nil {
		t.Errorf("Add Workload call failed. Err: %v", err)
	}

	TestUtils.Assert(t, workloadDelResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK, "Delete workload success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES, Name: "naples"}
	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")
}

func TestAgentService_Node_Naples_reload_invalid(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples-junk"}

	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err := agentClient.ReloadNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK, "Reload node succeded")
}

func TestAgentService_Node_Naples_Add_Save(t *testing.T) {
	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
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
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	nodeResp, err = agentClient.SaveNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Save Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Save node faild!")

	resp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")

	RestartIOTAAgent()

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err = agentClient.ReloadNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Reload node failed")

	resp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

}

func TestAgentService_Node_Naples_Add_Save_with_workloads(t *testing.T) {
	var workload iota.Workload

	workload.WorkloadName = "test-workload"
	workload.NodeName = "naples"
	workload.ParentInterface = "test"
	workload.MacAddress = "aa:bb:cc:dd:ee:ff"
	workload.EncapVlan = 500
	workload.PinnedPort = 1
	workload.UplinkVlan = 100
	workload.WorkloadImage = IotaWorkloadImage
	workload.WorkloadType = iota.WorkloadType_WORKLOAD_TYPE_CONTAINER
	workload.IpPrefix = "1.1.1.1/24"
	hntapCfgTempFile = "test/hntap-cfg.json"

	iotaNode := &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}}}

	nodeResp, err := agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Delete Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Delete node success!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{ControlIntf: "",
		DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24",
		VeniceIps: []string{"10.1.1.3/24"}}}}}

	resp, err := agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add node failed")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err = agentClient.AddNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_BAD_REQUEST, "Duplicate add node success!")

	iotaNodeHealth := &iota.NodeHealth{NodeName: "naples"}
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}

	workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{&workload}}
	workloadResp, _ := agentClient.AddWorkloads(context.Background(), workloadMsg)

	TestUtils.Assert(t, workloadResp.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK, "Add workload failed!")

	iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
	if err != nil {
		t.Errorf("Check health call failed. Err: %v", err)
	}
	TestUtils.Assert(t, iotaNodeHealth.GetHealthCode() != iota.NodeHealth_HEALTH_OK, "Node health not ok!")

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	nodeResp, err = agentClient.SaveNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Save Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Save node faild!")

	resp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, nodeResp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Delete node faild!")

	RestartIOTAAgent()

	iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES_SIM, Name: "naples"}
	iotaNode.NodeInfo = &iota.Node_NaplesConfigs{NaplesConfigs: &iota.NaplesConfigs{Configs: []*iota.NaplesConfig{
		&iota.NaplesConfig{ControlIntf: "",
			DataIntfs: []string{"eth2"}, ControlIp: "10.1.1.2/24", NicType: "pensando", Name: naplesSimName,
			VeniceIps: []string{"10.1.1.3/24"}}}}}
	resp, err = agentClient.ReloadNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

	TestUtils.Assert(t, resp.GetNodeStatus().ApiStatus == iota.APIResponseType_API_STATUS_OK, "Reload node failed")

	resp, err = agentClient.DeleteNode(context.Background(), iotaNode)
	if err != nil {
		t.Errorf("Add Node call failed. Err: %v", err)
	}

}

func init() {
	Workload.RunArpCmd = func(app Workload.Workload, ip string, intf string) error {
		return nil
	}

	controlIntf = Utils.GetIntfsMatchingPrefix("e")[0]
	naplesHwUUIDFile = "/mnt/sysuuid"
	Workload.ContainerPrivileged = false

	hostIntfCmd = func(osType string, nicType, hint string) []string {
		return []string{"date"}
	}

	naplesIPCmd = func(osType string, nicType, hint string) []string {
		return []string{"date"}
	}

	naplesMgmtIntfCmd = func(osType string, nicType, hint string) []string {
		return []string{"date"}
	}

	common.NaplesMnicIP = "127.0.0.1"

}
