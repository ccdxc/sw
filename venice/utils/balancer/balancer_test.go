package balancer

import (
	"fmt"
	"sync"
	"testing"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
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
	downFn1 := b.Up(grpc.Address{Addr: "node1:8888"})
	addr, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	AssertOk(t, err, fmt.Sprintf("Failed to get with error: %v", err))
	AssertEquals(t, "node1:8888", addr.Addr, fmt.Sprintf("Expected to get node1:8888, got %v", addr.Addr))

	// Mark instance2 up.
	downFn2 := b.Up(grpc.Address{Addr: "node2:8888"})
	addr, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	AssertOk(t, err, fmt.Sprintf("Failed to get with error: %v", err))
	AssertOneOf(t, addr.Addr, []string{"node1:8888", "node2:8888"})

	// Mark instance2 down.
	downFn2(fmt.Errorf("Test down"))
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

	// bring down all conns and try get
	br := b.(*balancer)
	br.resetTime = time.Now().Add(-10 * time.Second)
	downFn1(fmt.Errorf("Test down"))
	_, _, err = b.Get(context.Background(), grpc.BalancerGetOptions{})
	addrs := [][]grpc.Address{}
WaitLoop:
	for {
		select {
		case n := <-notifyCh:
			addrs = append(addrs, n)

		case <-time.After(time.Second):
			if len(addrs) == 3 {
				break WaitLoop
			}
			t.Fatalf("Timed out waiting for resolver notification [%v]", addrs)
		}
	}

	Assert(t, len(addrs) == 3, "expecting 3 notifications got [%d]", len(addrs))
	Assert(t, len(addrs[0]) == 0, "expecting first notification to be empty")
	Assert(t, len(addrs[1]) == 1, "expecting second notification to have 1 entry")
	Assert(t, len(addrs[2]) == 1, "expecting third notification to have 1 entry")
	AssertEquals(t, "node1:8888", addrs[1][0].Addr, fmt.Sprintf("Expected node1:8888, got %v", addrs[1][0].Addr))
	b.Close()
	b1.Close()
}

func TestMonitor(t *testing.T) {
	rc := mock.New()
	b := New(rc).(*balancer)
	b.running = true
	b.service = "testService"
	b.notifyCh = make(chan []grpc.Address, 128)
	b.resetTime = time.Now()
	b.monitorJitter = time.Millisecond * 10
	b.monitorOfset = time.Millisecond * 100
	b.Add(1)
	go b.monitor()
	// With no up connections notify monitorr and expect notification with 0 nodes, and periodically
	var ts time.Time
	var d time.Duration
	got := false
	select {
	case <-b.notifyCh:
		got = true
	case <-time.After(200 * time.Millisecond):
	}
	Assert(t, got == false, "should not have seen any notification")

	b.wakeUpMonitor(nil)
	time.Sleep(250 * time.Millisecond)
	Assert(t, len(b.notifyCh) > 2, "should have seen atleast 2 notifications")

	// Wakeup of the monitor does not reset the timer
	b.Lock()
	time.Sleep(time.Millisecond * 200)
loop0:
	for {
		select {
		case <-b.notifyCh:
		default:
			break loop0
		}
	}
	b.resetTime = time.Now()
	b.Unlock()
	go func() {
		for i := 0; i < 5; i++ {
			time.Sleep(20 * time.Millisecond)
			b.wakeUpMonitor(nil)
		}
	}()
	set := false

	tick := time.Tick(time.Millisecond * 300)
loop1:
	for {
		select {
		case <-b.notifyCh:
			if !set {
				ts = time.Now()
				set = true
				break
			}
			d = time.Since(ts)
			Assert(t, d > time.Millisecond*100, "too little time between notifications [%v]", d)
		case <-tick:
			break loop1
		}
	}

	// With no up connections but with resolver returning some node, expect notification
	si := types.ServiceInstance{
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
	rc.AddServiceInstance(&si)
	b.wakeUpMonitor(nil)
loop2:
	for {
		select {
		case <-time.After(100 * time.Millisecond):
			t.Fatalf("Timed out receiving event")
		case n := <-b.notifyCh:
			if len(n) == 1 && n[0].Addr == url {
				break loop2
			}
		}
	}

	// Make sure we are giving atleast ofset time for grpc to establish connection.
loop3:
	for {
		select {
		case n := <-b.notifyCh:
			if len(n) == 0 && set {
				ts = time.Now()
				break loop3
			}
			if len(n) > 0 && !set {
				d = time.Since(ts)
				set = true
			}
		case <-time.After(200 * time.Millisecond):
			t.Fatal("timeout waiting for notification")
		}
	}
	Assert(t, d > time.Millisecond*100, "did not give enough time to grpc to establish connection [%v]", d)

	// UP connection and ensure that we do not see any notifications from monitor
	//  -- drain all notifications and hold lock to make sure no new timers fire.
	b.Lock()
	tick = time.Tick(time.Millisecond * 200)
loop4:
	for {
		select {
		case e := <-b.notifyCh:
			log.Printf("got [%v]", e)
		case <-tick:
			break loop4
		}
	}
	b.upConns = []grpc.Address{grpc.Address{Addr: url}}
	b.wakeUpMonitor(nil)
	b.Unlock()

	count := 0
loop5:
	for {
		select {
		case <-b.notifyCh:
			count++
			time.Sleep(time.Second)
		case <-time.After(200 * time.Millisecond):
			break loop5
		}
	}
	Assert(t, count < 2, "should not have seen more than 1 notification [%d]", count)

	// close and ensure monitor exits
	b.wakeUpMonitor(fmt.Errorf("test end"))
	b.Wait()

	// test close when monitor is not yet running
	var closeWg sync.WaitGroup
	closefn := func() {
		b.Close()
		closeWg.Done()
	}
	monitorFn := func() {
		b.monitor()
		closeWg.Done()
	}

	b.monitorExit = nil
	b.running = true
	b.upCh = make(chan struct{})
	b.notifyCh = make(chan []grpc.Address, 128)
	closeWg.Add(1)
	b.Add(1) // for the monitor
	go closefn()
	time.Sleep(time.Millisecond * 10)
	closeWg.Add(1)
	go monitorFn()
	closeWg.Wait()
}
