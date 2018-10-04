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

	//tbMsg.IpAddress = []string{"10.8.102.91", "10.8.102.94", "10.8.102.95", "10.8.102.90", "10.8.102.93", "10.8.102.92"}
	//tbMsg.IpAddress = []string{"10.8.102.91"}
	tbMsg.SwitchPortId = 1
	tbMsg.User = "vm"
	tbMsg.Pass = "vm"

	resp, err := topoClient.InitTestBed(context.Background(), &tbMsg)
	if resp == nil || err != nil {
		t.Errorf("InitTestBed call failed. Err: %v", err)
		t.FailNow()
	}

	if len(resp.AllocatedVlans) != common.VlansPerTestBed {
		t.Errorf("Allocated VLANs did not match. Expected: %d, Actual: %d", common.VlansPerTestBed, len(resp.AllocatedVlans))
		t.FailNow()
	}
}

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
