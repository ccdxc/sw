package balancer

import (
	"fmt"
	"sync"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/resolver"
)

// balancer implements grpc.Balancer interface.
type balancer struct {
	sync.RWMutex
	service  string             // name of the service
	resolver resolver.Interface // resolver to use
	upConns  []grpc.Address     // connections reported up by grpc
	upCh     chan struct{}      // used to wake up blocked Gets
	idx      int                // used for round robin selection of Up conns
	running  bool
	notifyCh chan []grpc.Address // notification channel to grpc
}

// New creates a new balancer.
func New(resolver resolver.Interface) grpc.Balancer {
	return &balancer{
		resolver: resolver,
		upConns:  make([]grpc.Address, 0),
		upCh:     make(chan struct{}),
		notifyCh: make(chan []grpc.Address, 1),
	}
}

// Start registers a service with the balancer, which registers with the
// resolver to track service changes.
func (b *balancer) Start(target string, config grpc.BalancerConfig) error {
	b.Lock()
	defer b.Unlock()
	if b.running {
		return nil
	}
	b.running = true
	b.service = target
	b.resolver.Register(b)
	return nil
}

// Up handles the coming up of a connection.
func (b *balancer) Up(addr grpc.Address) func(error) {
	b.Lock()
	defer b.Unlock()
	if !b.running {
		return func(err error) {}
	}
	b.upConns = append(b.upConns, addr)
	// broadcast to waiting Gets.
	close(b.upCh)
	b.upCh = make(chan struct{})

	// This is the Down function that grpc will invoke when this connection breaks.
	return func(err error) {
		b.Lock()
		// TODO: log error?
		for ii := range b.upConns {
			if b.upConns[ii] == addr {
				b.upConns = append(b.upConns[:ii], b.upConns[ii+1:]...)
				break
			}
		}
		b.Unlock()
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
	defer b.Unlock()
	if !b.running {
		return nil
	}
	b.running = false
	close(b.notifyCh)
	close(b.upCh)
	b.resolver.Deregister(b)
	return nil
}

// OnNotifyResolver implements the resolver callbacks and informs grpc.
func (b *balancer) OnNotifyResolver(event types.ServiceInstanceEvent) error {
	b.Lock()
	defer b.Unlock()
	if !b.running {
		return nil
	}
	// gRPC wants the whole list, not incrementals.
	siList := b.resolver.Lookup(b.service)
	nodes := make([]grpc.Address, 0)
	for ii := range siList.Items {
		nodes = append(nodes, grpc.Address{Addr: fmt.Sprintf("%s:%d", siList.Items[ii].Node, siList.Items[ii].Port)})
	}
	if len(nodes) != 0 {
		b.notifyCh <- nodes
	}
	return nil
}
