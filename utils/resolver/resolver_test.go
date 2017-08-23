package resolver

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/grpc/service"
	"github.com/pensando/sw/cmd/services/mock"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/rpckit"
	"github.com/pensando/sw/utils/testutils"
)

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

	m.AddServiceInstance(&si1)
	m.AddServiceInstance(&si2)

	return m
}

// createFakeResolver creates a fake gRPC resolver server.
func createFakeResolver(t *testing.T) (*rpckit.RPCServer, *mock.ResolverService) {
	startPort := 9001
	for {
		testURL := fmt.Sprintf("localhost:%d", startPort)
		// create RPC Server.
		rpcServer, err := rpckit.NewRPCServer("fakeResolverService", testURL)
		if err == nil {
			m := setupMockResolver()
			types.RegisterServiceAPIServer(rpcServer.GrpcServer, service.NewRPCHandler(m))
			return rpcServer, m
		}
		startPort++
		if startPort > 15001 {
			t.Fatalf("Failed to find a free server port")
		}
	}
}

type mockServiceInstanceObserver struct {
	addedCount   int
	deletedCount int
}

func (m *mockServiceInstanceObserver) OnNotifyResolver(e types.ServiceInstanceEvent) error {
	switch e.Type {
	case types.ServiceInstanceEvent_Added:
		m.addedCount++
	case types.ServiceInstanceEvent_Deleted:
		m.deletedCount++
	}
	return nil
}

func TestResolverClient(t *testing.T) {
	server, m := createFakeResolver(t)
	config := &Config{
		Servers: []string{server.GetListenURL()},
	}
	client, err := New(config)
	if err != nil {
		t.Fatalf("Failed to create client, error: %v", err)
	}
	mo := &mockServiceInstanceObserver{}
	client.Register(mo)
	c2 := func() bool {
		instances := client.Lookup("svc1")
		if len(instances.Items) != 2 {
			return false
		}
		for ii := range instances.Items {
			name := instances.Items[ii].Name
			if name != "inst1" && name != "inst2" {
				return false
			}
		}
		return true
	}
	testutils.AssertEventually(t, c2, "Failed to resolve svc1")

	si3 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "inst3",
		},
		Service: "svc1",
	}
	m.AddServiceInstance(&si3)
	c3 := func() bool {
		instances := client.Lookup("svc1")
		if len(instances.Items) != 3 {
			return false
		}
		for ii := range instances.Items {
			name := instances.Items[ii].Name
			if name != "inst1" && name != "inst2" && name != "inst3" {
				return false
			}
		}
		return true
	}
	testutils.AssertEventually(t, c3, "Failed to resolve svc1 after adding an instance")

	m.DeleteServiceInstance(&si3)
	testutils.AssertEventually(t, c2, "Failed to resolve svc1 after deleting an instance")

	instances := client.Lookup("NonExistentSvc")
	if len(instances.Items) != 0 {
		t.Fatalf("Found non-zero instances for non-existant service")
	}

	testutils.AssertEventually(t, func() bool {
		if mo.addedCount == 3 && mo.deletedCount == 1 {
			return true
		}
		return false
	}, "Failed to find added and deleted instances")

	client.Stop()
}
