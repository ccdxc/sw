package resolver

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	url1 = "10.1.1.1:5001"
	url2 = "10.1.1.2:5001"
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
		URL:     url1,
	}
	si2 := si1
	si2.Name = "inst2"
	si2.URL = url2

	m.AddServiceInstance(&si1)
	m.AddServiceInstance(&si2)

	return m
}

// createFakeResolver creates a fake gRPC resolver server.
func createFakeResolver(t *testing.T) (*rpckit.RPCServer, *mock.ResolverService) {
	// create RPC Server.
	rpcServer, err := rpckit.NewRPCServer("fakeResolverService", "localhost:0")
	if err != nil {
		t.Fatalf("Failed to create fake resolver, error: %v", err)
	}
	m := setupMockResolver()
	types.RegisterServiceAPIServer(rpcServer.GrpcServer, service.NewRPCHandler(m))
	rpcServer.Start()
	return rpcServer, m
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
		Name:    "test",
		Servers: []string{server.GetListenURL()},
	}
	client := New(config)
	mo := &mockServiceInstanceObserver{}
	client.Register(mo)
	c2 := func() (bool, []interface{}) {
		instances := client.Lookup("svc1")
		if len(instances.Items) != 2 {
			return false, nil
		}
		for ii := range instances.Items {
			name := instances.Items[ii].Name
			if name != "inst1" && name != "inst2" {
				return false, nil
			}
		}
		return true, nil
	}
	AssertEventually(t, c2, "Failed to resolve svc1")
	urls := client.GetURLs("svc1")
	for ii := range urls {
		AssertOneOf(t, urls[ii], []string{url1, url2})
	}

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
	c3 := func() (bool, []interface{}) {
		instances := client.Lookup("svc1")
		if len(instances.Items) != 3 {
			return false, nil
		}
		for ii := range instances.Items {
			name := instances.Items[ii].Name
			if name != "inst1" && name != "inst2" && name != "inst3" {
				return false, nil
			}
		}
		return true, nil
	}
	AssertEventually(t, c3, "Failed to resolve svc1 after adding an instance")

	m.DeleteServiceInstance(&si3)
	AssertEventually(t, c2, "Failed to resolve svc1 after deleting an instance")

	instances := client.Lookup("NonExistentSvc")
	if len(instances.Items) != 0 {
		t.Fatalf("Found non-zero instances for non-existant service")
	}

	AssertEventually(t, func() (bool, []interface{}) {
		if mo.addedCount == 3 && mo.deletedCount == 1 {
			return true, nil
		}
		return false, nil
	}, "Failed to find added and deleted instances")

	client.Stop()
}
