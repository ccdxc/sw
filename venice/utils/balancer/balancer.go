package balancer

import (
	"fmt"
	"math/rand"
	"sync"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	defaultMonitorOfset  = time.Second * 5
	defaultMonitorJitter = time.Second * 2
)

// Balancer extends grpc.Balancer interface.
type Balancer interface {
	grpc.Balancer
	NumUpConns() int
}

// balancer implements Balancer interface.
type balancer struct {
	sync.RWMutex
	sync.WaitGroup
	service       string             // name of the service
	resolver      resolver.Interface // resolver to use
	upConns       []grpc.Address     // connections reported up by grpc
	upCh          chan struct{}      // used to wake up blocked Gets
	idx           int                // used for round robin selection of Up conns
	running       bool
	notifyCh      chan []grpc.Address // notification channel to grpc
	resetTime     time.Time
	monitorCh     chan error
	monitorExit   error
	rand          *rand.Rand
	monitorOfset  time.Duration
	monitorJitter time.Duration
}

// New creates a new balancer.
func New(resolver resolver.Interface) Balancer {
	return &balancer{
		resolver:      resolver,
		upConns:       make([]grpc.Address, 0),
		rand:          rand.New(rand.NewSource(int64(time.Now().Nanosecond()))),
		monitorCh:     make(chan error),
		monitorOfset:  defaultMonitorOfset,
		monitorJitter: defaultMonitorJitter,
	}
}

// Start registers a service with the balancer, which registers with the
// resolver to track service changes.
func (b *balancer) Start(target string, config grpc.BalancerConfig) error {
	b.Lock()
	if b.running {
		b.Unlock()
		return nil
	}
	b.upCh = make(chan struct{})
	b.notifyCh = make(chan []grpc.Address, 128)
	b.running = true
	b.service = target
	b.resolver.Register(b)
	b.resetTime = time.Now()
	b.monitorExit = nil
	b.Unlock()
	// Send the current state
	b.Add(1)
	go b.notifyServiceInstances()
	b.Add(1)
	go b.monitor()
	return nil
}

// Up handles the coming up of a connection.
func (b *balancer) Up(addr grpc.Address) func(error) {
	b.Lock()
	defer b.Unlock()
	if !b.running {
		return func(err error) {}
	}
	log.InfoLog("msg", "address UP notified", "addr", addr, "target", b.service)
	b.upConns = append(b.upConns, addr)
	// broadcast to waiting Gets.
	close(b.upCh)
	b.upCh = make(chan struct{})

	// This is the Down function that grpc will invoke when this connection breaks.
	return func(err error) {
		b.Lock()
		log.ErrorLog("msg", "address DOWN notified", "addr", addr, "target", b.service, "error", err)
		for ii := range b.upConns {
			if b.upConns[ii] == addr {
				b.upConns = append(b.upConns[:ii], b.upConns[ii+1:]...)
				if len(b.upConns) == 0 {
					b.wakeUpMonitor(nil)
				}
				break
			}
		}
		b.Unlock()
	}
}

// monitor retries connections if there are no active connections.
// The balancer could have seen non-temporary errors and stopped retrying, kickstart the balancer again.
func (b *balancer) monitor() {
	defer b.Done()
	ofset := b.monitorOfset

	for {
		err := <-b.monitorCh
		for {
			if err != nil {
				log.Errorf("monitor received close request (%s)", err)
				return
			}
			if b.monitorExit != nil {
				log.Errorf("monitor received close request (%s)", b.monitorExit)
				return
			}
			b.Lock()
			if len(b.upConns) > 0 {
				b.Unlock()
				// Break to start of of outer loop to await events on monitorCh
				break
			}
			// check if it is time to trigger the next update
			if time.Since(b.resetTime) > ofset {
				nodes := make([]grpc.Address, 0)
				if b.running {
					b.notifyCh <- nodes
				}
				b.resetTime = time.Now()
			}
			monitorCh := b.monitorCh
			b.Unlock()
			b.Add(1)
			b.notifyServiceInstances()
			// add some jitter, to prevent all balancers from getting synchronized in their retries.
			// We are not recalculating ofset from time we were woken up. We do not need to be so precise.
			waitTime := ofset + time.Duration(b.rand.Intn(int(b.monitorJitter)))
			select {
			case err = <-monitorCh:
			case <-time.After(waitTime):
			}
		}
	}
}

// wakeUpMonitor wakes up the monitor go routine. It is non-blocking.
func (b *balancer) wakeUpMonitor(err error) {
	select {
	case b.monitorCh <- err:
	default:
	}
}

// get is a helper function to determine available instances and pick
// one when more than one are available.
func (b *balancer) get() (grpc.Address, func(), error) {
	b.Lock()
	running := b.running
	numInsts := len(b.upConns)
	addr := grpc.Address{Addr: ""}
	if numInsts > 0 {
		// Round robin
		b.idx = (b.idx + 1) % len(b.upConns)
		addr = b.upConns[b.idx]
	}
	b.Unlock()
	if !running {
		return addr, nil, grpc.ErrClientConnClosing
	}
	if numInsts == 0 {
		b.wakeUpMonitor(nil)
		return addr, nil, status.Errorf(codes.Unavailable, "%s is unavailable", b.service)
	}
	return addr, func() {}, nil
}

// Get returns an address to use for the RPC. It can be blocking or non blocking.
func (b *balancer) Get(ctx context.Context, opts grpc.BalancerGetOptions) (grpc.Address, func(), error) {
	// Non blocking case - fail if no up connections.
	if !opts.BlockingWait {
		return b.get()
	}

	// Blocking case - wait until Up happens or context in canceled.
	for {
		addr, fn, err := b.get()
		if err == grpc.ErrClientConnClosing || addr.Addr != "" {
			return addr, fn, err
		}
		b.RLock()
		ch := b.upCh
		b.RUnlock()
		select {
		case <-ch:
		case <-ctx.Done():
			return grpc.Address{Addr: ""}, nil, ctx.Err()
		}
	}
}

// Notify returns the notification channel for address updates.
func (b *balancer) Notify() <-chan []grpc.Address {
	return b.notifyCh
}

// Close closes the balancer.
func (b *balancer) Close() error {
	b.Lock()
	if !b.running {
		b.Unlock()
		return nil
	}
	b.running = false
	b.resolver.Deregister(b)
	b.monitorExit = fmt.Errorf("close requested")
	b.wakeUpMonitor(b.monitorExit)
	close(b.notifyCh)
	close(b.upCh)

	b.Unlock() // unlock before calling wait so that notifyServiceInstances can proceed with execution and complete

	// wait for notifyServiceInstances to return. Only then can this balancer be reused for another Start()
	b.Wait()
	return nil
}

// notifyServiceInstances looks up instances of the service and updates the notify channel
func (b *balancer) notifyServiceInstances() {
	defer b.Done()
	// gRPC wants the whole list, not incrementals.
	siList := b.resolver.Lookup(b.service)
	nodes := make([]grpc.Address, 0)
	for ii := range siList.Items {
		nodes = append(nodes, grpc.Address{Addr: siList.Items[ii].URL})
	}
	if len(nodes) != 0 {
		b.Lock()
		if b.running {
			b.notifyCh <- nodes
		}
		b.Unlock()
	}
}

// OnNotifyResolver implements the resolver callbacks and informs grpc.
func (b *balancer) OnNotifyResolver(event types.ServiceInstanceEvent) error {
	b.Lock()
	if !b.running {
		b.Unlock()
		return nil
	}
	b.Unlock()
	b.Add(1)
	b.notifyServiceInstances()
	return nil
}

// NumUpConns returns the number of up connections.
func (b *balancer) NumUpConns() int {
	b.Lock()
	defer b.Unlock()
	return len(b.upConns)
}
