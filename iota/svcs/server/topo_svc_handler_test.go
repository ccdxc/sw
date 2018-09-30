package server

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

var topoClient iota.TopologyApiClient
var cfgClient iota.ConfigMgmtApiClient

func TestMain(m *testing.M) {
	go func() {
		StartIOTAService()
	}()

	// Ensure the service is up
	time.Sleep(time.Second * 2)

	c, err := common.CreateNewGRPCClient("test-client", IOTAListenURL)
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

	_, err := topoClient.InitTestBed(context.Background(), &tbMsg)
	if err != nil {
		t.Errorf("InitTestBed call failed. Err: %v", err)
	}
}

func TestTopologyService_CleanUpTestBed(t *testing.T) {
	t.Parallel()
	var tbMsg iota.TestBedMsg

	_, err := topoClient.CleanUpTestBed(context.Background(), &tbMsg)
	if err != nil {
		t.Errorf("CleanupTestBed call failed. Err: %v", err)
	}
}

func TestTopologyService_AddNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.AddNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("AddNodes call failed. Err: %v", err)
	}
}

func TestTopologyService_DeleteNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.DeleteNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("DeleteNodes call failed. Err: %v", err)
	}
}

func TestTopologyService_GetNodes(t *testing.T) {
	t.Parallel()
	var nodeMsg iota.NodeMsg

	_, err := topoClient.GetNodes(context.Background(), &nodeMsg)
	if err != nil {
		t.Errorf("GetNodes call failed. Err: %v", err)
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
