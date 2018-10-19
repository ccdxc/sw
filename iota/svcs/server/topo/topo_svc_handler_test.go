package topo

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"

	"github.com/pensando/sw/iota/svcs/server/cfg"

	"github.com/pensando/sw/venice/utils/netutils"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

var topoClient iota.TopologyApiClient
var cfgClient iota.ConfigMgmtApiClient
var topoServerURL string

func startTopoService() {
	var testListener netutils.TestListenAddr
	err := testListener.GetAvailablePort()
	if err != nil {
		fmt.Println("Could not get an available port")
		os.Exit(1)
	}
	topoServerURL = testListener.ListenURL.String()

	topoSvc, err := common.CreateNewGRPCServer("Topology Server", topoServerURL)
	if err != nil {
		fmt.Printf("Could not start IOTA Service. Err: %v\n", err)
		os.Exit(1)
	}
	//cfgSvc, err := common.CreateNewGRPCServer("Config Server", topoServerURL)
	//if err != nil {
	//	fmt.Printf("Could not start IOTA Service. Err: %v\n", err)
	//	os.Exit(1)
	//}

	topoHandler := NewTopologyServiceHandler()

	iota.RegisterTopologyApiServer(topoSvc.Srv, topoHandler)

	cfgHandler := cfg.NewConfigServiceHandler()

	iota.RegisterConfigMgmtApiServer(topoSvc.Srv, cfgHandler)

	topoSvc.Start()
}

func TestMain(m *testing.M) {
	go func() {
		startTopoService()
	}()

	// Ensure the service is up
	time.Sleep(time.Second * 2)

	c, err := common.CreateNewGRPCClient("topo-test-client", topoServerURL)
	if err != nil {
		fmt.Println("Could not create a GRPC Client to the IOTA Server")
		os.Exit(1)
	}

	topoClient = iota.NewTopologyApiClient(c.Client)
	cfgClient = iota.NewConfigMgmtApiClient(c.Client)

	runTests := m.Run()
	os.Exit(runTests)

}

func TestTopologyService_InitTestBed(t *testing.T) {
	t.Parallel()
	var tbMsg iota.TestBedMsg
	var naplesUUID []string
	initNodes := []*iota.TestBedNode{
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.28",
		},
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.29",
		},
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.30",
		},
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.31",
		},
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.32",
		},
		{
			Type:      iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			IpAddress: "10.8.103.33",
		},
	}

	tbMsg.Nodes = initNodes
	tbMsg.Username = "vm"
	tbMsg.Password = "vm"
	tbMsg.TestbedId = 1
	tbMsg.ApiResponse = &iota.IotaAPIResponse{}

	//tbMsg.NaplesImage = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/obj/images/naples-release-v1.tgz", os.Getenv("GOPATH"))
	//tbMsg.VeniceImage = fmt.Sprintf("%s/src/github.com/pensando/sw/bin/venice.tgz", os.Getenv("GOPATH"))

	resp, err := topoClient.InitTestBed(context.Background(), &tbMsg)
	if resp == nil || err != nil {
		t.Errorf("InitTestBed call failed. Err: %v", err)
		t.FailNow()
	}

	if len(resp.AllocatedVlans) != common.VlansPerTestBed {
		t.Errorf("Allocated VLANs did not match. Expected: %d, Actual: %d", common.VlansPerTestBed, len(resp.AllocatedVlans))
		t.FailNow()
	}

	vlans := resp.AllocatedVlans

	nodeMsg := iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}
	nodes := []*iota.Node{
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.103.28",
			NodeInfo: &iota.Node_VeniceConfig{
				VeniceConfig: &iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.1",
					VenicePeers: []*iota.VenicePeer{
						{
							HostName:  "venice-node-1",
							IpAddress: "42.42.42.1",
						},
						{
							HostName:  "venice-node-2",
							IpAddress: "42.42.42.2",
						},
						{
							HostName:  "venice-node-3",
							IpAddress: "42.42.42.3",
						},
					},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-2",
			IpAddress: "10.8.103.29",
			NodeInfo: &iota.Node_VeniceConfig{
				VeniceConfig: &iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.2",
					VenicePeers: []*iota.VenicePeer{
						{
							HostName:  "venice-node-1",
							IpAddress: "42.42.42.1",
						},
						{
							HostName:  "venice-node-2",
							IpAddress: "42.42.42.2",
						},
						{
							HostName:  "venice-node-3",
							IpAddress: "42.42.42.3",
						},
					},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-3",
			IpAddress: "10.8.103.30",
			NodeInfo: &iota.Node_VeniceConfig{
				VeniceConfig: &iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.3",
					VenicePeers: []*iota.VenicePeer{
						{
							HostName:  "venice-node-1",
							IpAddress: "42.42.42.1",
						},
						{
							HostName:  "venice-node-2",
							IpAddress: "42.42.42.2",
						},
						{
							HostName:  "venice-node-3",
							IpAddress: "42.42.42.3",
						},
					},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			Image:     "naples-release-v1.tgz",
			Name:      "naples-node-1",
			IpAddress: "10.8.103.31",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.4",
					DataIntfs:   []string{"eth2", "eth3"},
					VeniceIps:   []string{"42.42.42.1", "42.42.42.2", "42.42.42.3"},
				},
			},
		}, {
			Type:      iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			Image:     "naples-release-v1.tgz",
			Name:      "naples-node-2",
			IpAddress: "10.8.103.32",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.5",
					DataIntfs:   []string{"eth2", "eth3"},
					VeniceIps:   []string{"42.42.42.1", "42.42.42.2", "42.42.42.3"},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			Name:      "naples-node-3",
			Image:     "naples-release-v1.tgz",
			IpAddress: "10.8.103.33",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.6",
					DataIntfs:   []string{"eth2", "eth3"},
					VeniceIps:   []string{"42.42.42.1", "42.42.42.2", "42.42.42.3"},
				},
			},
		},
	}
	nodeMsg.Nodes = nodes
	nodeMsg.NodeOp = iota.Op_ADD

	addNodeResp, err := topoClient.AddNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("AddNodes call failed. Err: %v", err)
		t.FailNow()
	}

	fmt.Println("BALERION: ", addNodeResp)

	quorumNodes := []string{"venice-node-1", "venice-node-2", "venice-node-3"}
	clusterObj := &cluster.Cluster{
		TypeMeta: api.TypeMeta{
			Kind:       "Cluster",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "e2eCluster",
		},
		Spec: cluster.ClusterSpec{
			QuorumNodes:   quorumNodes,
			AutoAdmitNICs: true,
		},
	}

	b, _ := json.Marshal(clusterObj)

	clusterMsg := &iota.MakeClusterMsg{
		Endpoint:       "10.8.103.28:9001/api/v1/cluster",
		HealthEndpoint: "10.8.103.28:9000/configs/cluster/v1/tenants",
		Config:         string(b),
		ApiResponse:    &iota.IotaAPIResponse{},
	}

	_, err = cfgClient.MakeCluster(context.Background(), clusterMsg)
	if err != nil {
		t.Errorf("MakeCluster call failed. Err: %v", err)
		t.FailNow()
	}

	workloads := []*iota.Workload{
		{
			WorkloadName: "ping-app-1",
			NodeName:     "naples-node-1",
			EncapVlan:    6,
			IpAddress:    "177.75.132.4",
			MacAddress:   "06:c3:32:b7:e6:da",
			Interface:    "lif100",
		},
		{
			WorkloadName: "ping-app-2",
			NodeName:     "naples-node-2",
			EncapVlan:    6,
			IpAddress:    "170.22.196.4",
			MacAddress:   "72:ed:48:1c:e1:23",
			Interface:    "lif100",
		},
	}

	wrkloadMsg := iota.WorkloadMsg{
		WorkloadOp:  iota.Op_ADD,
		Workloads:   workloads,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	_, err = topoClient.AddWorkloads(context.Background(), &wrkloadMsg)
	if err != nil {
		t.Errorf("AddWorkloads call failed. Err: %v", err)
		t.FailNow()
	}

	var initCfgMsg iota.InitConfigMsg
	initCfgMsg.EntryPointType = iota.EntrypointType_VENICE_REST
	initCfgMsg.Endpoints = []string{"10.8.103.28:9000"}
	initCfgMsg.Vlans = vlans
	initCfgMsg.ApiResponse = &iota.IotaAPIResponse{}

	_, err = cfgClient.InitCfgService(context.Background(), &initCfgMsg)
	if err != nil {
		t.Errorf("InitCfgService call failed. Err: %v", err)
		t.FailNow()
	}

	authMsg := iota.AuthMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}
	authResp, err := cfgClient.ConfigureAuth(context.Background(), &authMsg)
	if err != nil {
		t.Errorf("ConfigureAuth call failed. Err: %v", err)
		t.FailNow()
	}

	log.Infof("Received Token: %s", authResp.AuthToken)
	fmt.Println("ANCALAGON: ", addNodeResp.Nodes)

	for _, n := range addNodeResp.Nodes {
		if n.Type == iota.PersonalityType_PERSONALITY_NAPLES_SIM {
			naplesUUID = append(naplesUUID, n.NodeUuid)
		}
	}

	genCfgMsg := iota.GenerateConfigMsg{
		NaplesUuids: naplesUUID,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	configs, err := cfgClient.GenerateConfigs(context.Background(), &genCfgMsg)
	log.Infof("Received Config Generation Response: %v", configs)
	if err != nil {
		t.Errorf("GenerateConfigs call failed. Err: %v", err)
		t.FailNow()
	}

	pushCfgResp, err := cfgClient.PushConfig(context.Background(), configs)
	log.Infof("Received Config Push Response: %v. Err: %v", pushCfgResp, err)
	if err != nil {
		t.Errorf("PushConfig call failed. Err: %v", err)
		t.FailNow()
	}

}

/*
func TestTopologyService_CleanUpTestBed(t *testing.T) {
	t.Parallel()
	var tbMsg iota.TestBedMsg

	_, err := topoClient.CleanUpTestBed(context.Background(), &tbMsg)
	if err != nil {
		t.Errorf("CleanupTestBed call failed. Err: %v", err)
		t.FailNow()
	}
}

func TestTopologyService_AddNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg
	//]string{"10.8.102.91", "10.8.102.94", "10.8.102.95", "10.8.102.90", "10.8.102.93", "10.8.102.92"}

	nodes := []*iota.Node{
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.91",
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.94",
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.95",
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Image:     "nic.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.90",
		}, {
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Image:     "nic.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.93",
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Image:     "nic.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.92",
		},
	}
	nodeMsg.Nodes = nodes
	nodeMsg.NodeOp = iota.Op_ADD

	_, err := topoClient.AddNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("AddNodes call failed. Err: %v", err)
		t.FailNow()
	}
}

func TestTopologyService_DeleteNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.DeleteNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("DeleteNodes call failed. Err: %v", err)
		t.FailNow()
	}
}

func TestTopologyService_GetNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.GetNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("GetNodes call failed. Err: %v", err)
		t.FailNow()
	}
}

func TestTopologyService_AddWorkloads(t *testing.T) {
	t.Parallel()
	var wrkldMsg iota.WorkloadMsg

	_, err := topoClient.AddWorkloads(context.Background(), &wrkldMsg)
	if err != nil {
		t.Errorf("AddWorkloads call failed. Err: %v", err)
	}
}

func TestTopologyService_DeleteWorkloads(t *testing.T) {
	t.Parallel()
	var wrkldMsg iota.WorkloadMsg

	_, err := topoClient.DeleteWorkloads(context.Background(), &wrkldMsg)
	if err != nil {
		t.Errorf("DeleteWorkloads call failed. Err: %v", err)
	}
}

func TestTopologyService_Trigger(t *testing.T) {
	t.Parallel()
	var trigMsg iota.TriggerMsg

	_, err := topoClient.Trigger(context.Background(), &trigMsg)
	if err != nil {
		t.Errorf("Trigger call failed. Err: %v", err)
	}
}

func TestTopologyService_CheckClusterHealth(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.CheckClusterHealth(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("CheckClusterHealth call failed. Err: %v", err)
	}
}
*/
