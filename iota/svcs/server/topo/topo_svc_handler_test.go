package topo

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/netutils"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

var topoClient iota.TopologyApiClient
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

	topoHandler := NewTopologyServiceHandler()

	iota.RegisterTopologyApiServer(topoSvc.Srv, topoHandler)

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

	runTests := m.Run()
	os.Exit(runTests)

}

func TestTopologyService_InitTestBed(t *testing.T) {
	t.Parallel()
	var tbMsg iota.TestBedMsg

	tbMsg.IpAddress = []string{"10.8.102.123", "10.8.102.124", "10.8.102.125", "10.8.102.126", "10.8.102.127", "10.8.102.128"}
	//tbMsg.IpAddress = []string{"10.8.102.91"}
	tbMsg.SwitchPortId = 1
	tbMsg.User = "vm"
	tbMsg.Passwd = "vm"

	tbMsg.NaplesImage = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/obj/images/naples-release-v1.tgz", os.Getenv("GOPATH"))
	tbMsg.VeniceImage = fmt.Sprintf("%s/src/github.com/pensando/sw/bin/venice.tgz", os.Getenv("GOPATH"))

	resp, err := topoClient.InitTestBed(context.Background(), &tbMsg)
	if resp == nil || err != nil {
		t.Errorf("InitTestBed call failed. Err: %v", err)
		t.FailNow()
	}

	if len(resp.AllocatedVlans) != common.VlansPerTestBed {
		t.Errorf("Allocated VLANs did not match. Expected: %d, Actual: %d", common.VlansPerTestBed, len(resp.AllocatedVlans))
		t.FailNow()
	}

	nodeMsg := iota.NodeMsg{}
	nodes := []*iota.Node{
		{
			Type:      iota.PersonalityType_PERSONALITY_VENICE,
			Image:     "venice.tgz",
			Name:      "venice-node-1",
			IpAddress: "10.8.102.123",
			NodeInfo: &iota.Node_VeniceConfig{
				VeniceConfig: &iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.1",
					VenicePeers: []*iota.VenicePeer{
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
			IpAddress: "10.8.102.124",
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
			IpAddress: "10.8.102.125",
			NodeInfo: &iota.Node_VeniceConfig{
				VeniceConfig: &iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.3",
					VenicePeers: []*iota.VenicePeer{
						{
							HostName:  "venice-node-2",
							IpAddress: "42.42.42.1",
						},
						{
							HostName:  "venice-node-1",
							IpAddress: "42.42.42.2",
						},
					},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Image:     "naples-release-v1.tgz",
			Name:      "naples-node-1",
			IpAddress: "10.8.102.126",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.4",
					DataIntfs:   []string{"eth2"},
					VeniceIps:   []string{"10.8.102.123", "10.8.102.124", "10.8.102.125"},
				},
			},
		}, {
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Image:     "naples-release-v1.tgz",
			Name:      "naples-node-2",
			IpAddress: "10.8.102.127",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.5",
					DataIntfs:   []string{"eth2"},
					VeniceIps:   []string{"10.8.102.123", "10.8.102.124", "10.8.102.125"},
				},
			},
		},
		{
			Type:      iota.PersonalityType_PERSONALITY_NAPLES,
			Name:      "naples-node-3",
			Image:     "naples-release-v1.tgz",
			IpAddress: "10.8.102.128",
			NodeInfo: &iota.Node_NaplesConfig{
				NaplesConfig: &iota.NaplesConfig{
					ControlIntf: "eth1",
					ControlIp:   "42.42.42.6",
					DataIntfs:   []string{"eth2"},
					VeniceIps:   []string{"10.8.102.123", "10.8.102.124", "10.8.102.125"},
				},
			},
		},
	}
	nodeMsg.Nodes = nodes
	nodeMsg.NodeOp = iota.Op_ADD

	_, err = topoClient.AddNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("AddNodes call failed. Err: %v", err)
		t.FailNow()
	}

	workloads := []*iota.Workload{
		{
			WorkloadName: "ping-app-1",
			NodeName:     "naples-node-1",
			EncapVlan:    6,
			IpAddress:    "177.75.132.4",
			MacAddress:   "06:c3:32:b7:e6:da",
		},
		{
			WorkloadName: "ping-app-2",
			NodeName:     "naples-node-2",
			EncapVlan:    6,
			IpAddress:    "170.22.196.4",
			MacAddress:   "72:ed:48:1c:e1:23",
		},
	}

	wrkloadMsg := iota.WorkloadMsg{
		WorkloadOp: iota.Op_ADD,
		Workloads:  workloads,
	}

	_, err = topoClient.AddWorkloads(context.Background(), &wrkloadMsg)
	if err != nil {
		t.Errorf("AddWorkloads call failed. Err: %v", err)
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
