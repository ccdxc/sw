package service

import (
	"testing"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	testURL = "localhost:9001"
)

// createServiceAPIServerClient creates rpc server and client
func createServiceAPIServerClient(t *testing.T) (*mock.ResolverService, *rpckit.RPCServer, types.ServiceAPIClient) {
	// create RPC Server.
	rpcServer, err := rpckit.NewRPCServer("testService", testURL)
	if err != nil {
		t.Fatalf("Failed to create rpc server, error: %v", err)
	}

	m := setupMockResolver()
	types.RegisterServiceAPIServer(rpcServer.GrpcServer, NewRPCHandler(m))

	// create RPC Client.
	rpcClient, err := rpckit.NewRPCClient("testService", testURL)
	if err != nil {
		t.Fatalf("Failed to create rpc client, error: %v", err)
	}
	return m, rpcServer, types.NewServiceAPIClient(rpcClient.ClientConn)
}

// setupMockResolver sets up a mock resolver.
func setupMockResolver() *mock.ResolverService {
	m := mock.NewResolverService()
	si1 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "inst1",
		},
		Service: "svc1",
	}
	si2 := si1
	si2.Name = "inst2"
	si3 := si1
	si3.Name = "inst3"
	si3.Service = "svc2"

	m.AddServiceInstance(&si1)
	m.AddServiceInstance(&si2)
	m.AddServiceInstance(&si3)

	return m
}

// TestServiceRPC tests service APIs.
func TestServiceRPC(t *testing.T) {
	m, rpcServer, rpcClient := createServiceAPIServerClient(t)
	defer rpcServer.Stop()

	// Get, List tests.
	sList, err := rpcClient.ListServices(context.Background(), &api.Empty{})
	if err != nil {
		t.Fatalf("Service List failed with error: %v", err)
	}
	if len(sList.Items) != 2 {
		t.Fatalf("Incorrect service list, expected 2 items, found %v", len(sList.Items))
	}

	svc, err := rpcClient.GetService(context.Background(), &api.ObjectMeta{Name: "svc1"})
	if err != nil {
		t.Fatalf("Service Get failed with error: %v", err)
	}
	if svc.Name != "svc1" || len(svc.Instances) != 2 {
		t.Fatalf("Service Get returned incorrect data")
	}

	si, err := rpcClient.GetServiceInstance(context.Background(), &types.ServiceInstanceReq{Service: "svc1", Instance: "inst1"})
	if err != nil {
		t.Fatalf("Service Instance Get failed with error: %v", err)
	}
	if si.Name != "inst1" || si.Service != "svc1" {
		t.Fatalf("Service Instance Get returned incorrect data")
	}

	// Watch tests.
	ctx, cancel := context.WithCancel(context.Background())
	watcher, err := rpcClient.WatchServiceInstances(ctx, &api.Empty{})
	if err != nil {
		t.Fatalf("Failed to watch service instances, error: %v", err)
	}
	evCh := make(chan *types.ServiceInstanceEvent)
	fn := func(evCh chan *types.ServiceInstanceEvent, expected int) {
		if expected < 1 {
			return
		}
		count := 0
		for {
			siList, err := watcher.Recv()
			if err != nil {
				t.Fatalf("Service instance watcher failed, error: %v", err)
			}
			for ii := range siList.Items {
				evCh <- siList.Items[ii]
				count++
				if count == expected {
					return
				}
			}
		}
	}

	siList, err := rpcClient.ListServiceInstances(context.Background(), &api.Empty{})
	if err != nil {
		t.Fatalf("Service instance list failed with error: %v", err)
	}
	// Run until 2 future events occur
	go fn(evCh, len(siList.Items)+2)
	evCount := 0
	// First get existing service instances
	for {
		select {
		case e := <-evCh:
			if e.Type != types.ServiceInstanceEvent_Added {
				t.Fatalf("Did not get added event, got %v", e.Type)
			}
			evCount++
		case <-time.After(time.Second):
			t.Fatalf("Timed out waiting for service instance list")
		}
		if evCount == len(siList.Items) {
			break
		}
	}

	// Add a service instance.
	si4 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "inst4",
		},
		Service: "svc3",
	}

	m.AddServiceInstance(&si4)
	select {
	case e := <-evCh:
		if e.Type != types.ServiceInstanceEvent_Added {
			t.Fatalf("Did not get added event, got %v", e.Type)
		}
		if e.Instance.Name != si4.Name {
			t.Fatalf("Watch expected name %v, got %v", si4.Name, e.Instance.Name)
		}
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for service instance addition")
	}

	// Delete a service instance.
	m.DeleteServiceInstance(&si4)
	select {
	case e := <-evCh:
		if e.Type != types.ServiceInstanceEvent_Deleted {
			t.Fatalf("Did not get deleted event, got %v", e.Type)
		}
		if e.Instance.Name != si4.Name {
			t.Fatalf("Watch expected name %v, got %v", si4.Name, e.Instance.Name)
		}
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for service instance deletion")
	}

	// Cancel the watch.
	cancel()
	select {
	case e := <-evCh:
		t.Fatalf("Received event %v on closed watch", e)
	case <-time.After(time.Millisecond * 100):
	}
}
