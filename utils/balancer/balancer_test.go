package balancer

import (
	"fmt"
	"testing"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/utils/resolver/mock"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/types"
	. "github.com/pensando/sw/utils/testutils"
)

const (
	url = "node1:8888"
)

func TestBalancer(t *testing.T) {
	rc := mock.New()
	b := New(rc)
	if _, _, err := b.Get(context.Background(), grpc.BalancerGetOptions{}); err != grpc.ErrClientConnClosing {
		t.Fatalf("Did not get conn closing error before Start, got %v", err)
	}
	if err := b.Close(); err != nil {
		t.Fatalf("Close failed before Start with error: %v", err)
	}
	if err := b.Start("testService", grpc.BalancerConfig{}); err != nil {
		t.Fatalf("Start failed with error: %v", err)
	}
	notifyCh := b.Notify()
	select {
	case addrs := <-b.Notify():
		t.Fatalf("Got entry when there should be none %v", addrs)
	case <-time.After(time.Millisecond * 300):
		t.Logf("Timed out waiting for resolver notification as expected")
	}
	si1 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "inst1",
		},
		Service: "testService",
		Node:    "node1",
		URL:     url,
	}
	// Add instance1.
	rc.AddServiceInstance(&si1)
	select {
	case addrs := <-notifyCh:
		AssertEquals(t, len(addrs), 1, fmt.Sprintf("Expected 1 addr, got %v", len(addrs)))
		AssertEquals(t, url, addrs[0].Addr, fmt.Sprintf("Expected %v, got %v", url, addrs[0].Addr))
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for resolver notification")
	}

	// Create a new Balancer
	b1 := New(rc)
	if err := b1.Start("testService", grpc.BalancerConfig{}); err != nil {
		t.Fatalf("Start failed with error: %v", err)
	}
	// Start should publish existing state to channel. Give some time for go routine to kick in.
	select {
	case addrs := <-b1.Notify():
		AssertEquals(t, 1, len(addrs), fmt.Sprintf("Expected 1 addr, got %v", len(addrs)))
		AssertEquals(t, url, addrs[0].Addr, fmt.Sprintf("Expected %v, got %v", url, addrs[0].Addr))
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for resolver notification")
	}

	si2 := si1
	si2.Name = "inst2"
	si2.Node = "node2"
	// Add instance2.
	rc.AddServiceInstance(&si2)
	select {
	case addrs := <-notifyCh:
		AssertEquals(t, 2, len(addrs), fmt.Sprintf("Expected 2 addrs, got %v", len(addrs)))
		for ii := range addrs {
			AssertOneOf(t, addrs[ii].Addr, []string{"node1:8888", "node2:8888"})
		}
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for resolver notification")
	}
	select {
	case addrs := <-b1.Notify():
		AssertEquals(t, 2, len(addrs), fmt.Sprintf("Expected 2 addrs, got %v", len(addrs)))
		for ii := range addrs {
			AssertOneOf(t, addrs[ii].Addr, []string{"node1:8888", "node2:8888"})
		}
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for resolver notification for second balancer")
	}
	// Both instances are still down.
	addr, _, err := b.Get(context.Background(), grpc.BalancerGetOptions{})
	Assert(t, (err != nil), "Did not get unavailable error")
	Assert(t, (addr.Addr == ""), fmt.Sprintf("Expected empty Get, got %v", addr.Addr))

	// Mark instance1 up.
	b.Up(grpc.Address{Addr: "node1:8888"})
	addr, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	AssertOk(t, err, fmt.Sprintf("Failed to get with error: %v", err))
	AssertEquals(t, "node1:8888", addr.Addr, fmt.Sprintf("Expected to get node1:8888, got %v", addr.Addr))

	// Mark instance2 up.
	downFn := b.Up(grpc.Address{Addr: "node2:8888"})
	addr, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	AssertOk(t, err, fmt.Sprintf("Failed to get with error: %v", err))
	AssertOneOf(t, addr.Addr, []string{"node1:8888", "node2:8888"})

	// Mark instance2 down.
	downFn(fmt.Errorf("Test down"))
	addr, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	AssertOk(t, err, fmt.Sprintf("Failed to get with error: %v", err))
	AssertEquals(t, "node1:8888", addr.Addr, fmt.Sprintf("Expected to get node1:8888, got %v", addr.Addr))

	// Delete instance2.
	rc.DeleteServiceInstance(&si2)
	select {
	case addrs := <-notifyCh:
		AssertEquals(t, len(addrs), 1, fmt.Sprintf("Expected 1 addr, got %v", len(addrs)))
		AssertEquals(t, "node1:8888", addrs[0].Addr, fmt.Sprintf("Expected node1:8888, got %v", addrs[0].Addr))
	case <-time.After(time.Second):
		t.Fatalf("Timed out waiting for resolver notification")
	}
	b.Close()
}
