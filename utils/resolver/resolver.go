package resolver

import (
	"math/rand"
	"sync"
	"time"

	context "golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/types"
)

// Interface for a resolver client.
type Interface interface {
	// Lookup resolves a service to its instances.
	Lookup(name string) *types.ServiceInstanceList

	// Stop stops the resolver client.
	Stop()
}

// Config contains configuration to create a resolver client.
type Config struct {
	// Servers could be "CMD on localhost" for controller processes,
	// "list of virtual IPs" for NIC agent process. The string
	// format is <IP/Hostname>:<Port>.
	Servers []string

	// Options contain gRPC dial options.
	Options []grpc.DialOption
}

// resolverClient implements the resolver client functionality.
type resolverClient struct {
	sync.Mutex
	config  *Config
	ctx     context.Context
	cancel  context.CancelFunc
	svcsMap map[string]map[string]*types.ServiceInstance // service name to instance mappings.
}

// New creates a new resolver client.
func New(c *Config) (Interface, error) {
	ctx, cancel := context.WithCancel(context.Background())
	r := &resolverClient{
		config:  c,
		ctx:     ctx,
		cancel:  cancel,
		svcsMap: make(map[string]map[string]*types.ServiceInstance),
	}
	go r.runUntilCancel()
	return r, nil
}

// runUntilCancel implements the business logic of the resolver client.
func (r *resolverClient) runUntilCancel() {
	s := rand.NewSource(time.Now().UnixNano())
	for {
		// Check if cancelled.
		r.Lock()
		if r.cancel == nil {
			return
		}
		r.Unlock()

		// Pick one of the servers at random.
		i := rand.New(s).Intn(len(r.config.Servers))

		// grpc client setup
		opts := r.config.Options
		if len(opts) == 0 {
			opts = append(opts, grpc.WithInsecure())
		}
		conn, err := grpc.Dial(r.config.Servers[i], opts...)
		if err != nil {
			time.Sleep(time.Millisecond * 100)
			continue
		}
		client := types.NewServiceAPIClient(conn)

		// watch for events in a loop.
		watcher, err := client.WatchServiceInstances(r.ctx, &api.Empty{})
		if err != nil {
			time.Sleep(time.Millisecond * 100)
			continue
		}
		first := true
		for {
			el, err := watcher.Recv()
			if err != nil {
				break
			}
			// The first response on watch returns a complete list of instances. Use
			// it to replace the svcsMap.
			if first {
				svcsMap := make(map[string]map[string]*types.ServiceInstance)
				for ii := range el.Items {
					e := el.Items[ii]
					svcMap, ok := svcsMap[e.Instance.Service]
					if !ok {
						svcMap = make(map[string]*types.ServiceInstance)
						svcsMap[e.Instance.Service] = svcMap
					}
					svcMap[e.Instance.Name] = el.Items[ii].Instance
				}
				r.Lock()
				r.svcsMap = svcsMap
				r.Unlock()
				first = false
				continue
			}
			r.Lock()
			for ii := range el.Items {
				e := el.Items[ii]
				switch e.Type {
				case types.ServiceInstanceEvent_Added:
					svcMap, ok := r.svcsMap[e.Instance.Service]
					if !ok {
						svcMap = make(map[string]*types.ServiceInstance)
						r.svcsMap[e.Instance.Service] = svcMap
					}
					svcMap[e.Instance.Name] = e.Instance
				case types.ServiceInstanceEvent_Deleted:
					svcMap, ok := r.svcsMap[e.Instance.Service]
					if !ok {
						continue
					}
					delete(svcMap, e.Instance.Name)
				}
			}
			r.Unlock()
		}

		// Sleep before retrying
		time.Sleep(time.Millisecond * 100)
	}
}

// Lookup resolves a service to its instances.
func (r *resolverClient) Lookup(name string) *types.ServiceInstanceList {
	r.Lock()
	defer r.Unlock()
	result := &types.ServiceInstanceList{
		Items: make([]*types.ServiceInstance, 0),
	}
	if r.svcsMap == nil {
		return result
	}
	svcMap, ok := r.svcsMap[name]
	if !ok {
		return result
	}
	for _, v := range svcMap {
		result.Items = append(result.Items, v)
	}
	return result
}

// Stop stops the resolver client.
func (r *resolverClient) Stop() {
	r.Lock()
	defer r.Unlock()
	if r.cancel != nil {
		r.cancel()
		r.cancel = nil
	}
	r.svcsMap = nil
}
