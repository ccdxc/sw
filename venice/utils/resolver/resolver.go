package resolver

import (
	"fmt"
	"math/rand"
	"strings"
	"sync"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	rpcRetryInterval = 500 * time.Millisecond
)

// Observer is an interface implemented by observers of resolver.
type Observer interface {
	// OnNotifyResolver handles an event published by the resolver
	// Observers should be able to handle receiving this event even after Deregister is called
	//	(Possible if Deregister happens in the middle of notification)
	OnNotifyResolver(types.ServiceInstanceEvent) error
}

// Interface for a resolver client.
type Interface interface {
	// Lookup resolves a service to its instances.
	Lookup(name string) *types.ServiceInstanceList

	// GetURLs gets the URLs for a service.
	GetURLs(name string) []string

	// Stop stops the resolver client.
	Stop()

	// Register an observer.
	Register(Observer)

	// Deregister an observer.
	Deregister(Observer)

	// Update list of servers providing the resolver service
	UpdateServers(servers []string)
}

// Config contains configuration to create a resolver client.
type Config struct {
	// Name contains the name of the client (used for stats).
	Name string

	// Servers could be "CMD on localhost" for controller processes,
	// "list of virtual IPs" for NIC agent process. The string
	// format is <IP/Hostname>:<Port>.
	Servers []string

	// Options contain gRPC dial options.
	Options []rpckit.Option
}

// resolverClient implements the resolver client functionality.
type resolverClient struct {
	sync.Mutex
	config    Config
	ctx       context.Context
	cancel    context.CancelFunc
	svcsMap   map[string]map[string]*types.ServiceInstance // service name to instance mappings.
	observers []Observer
}

// New creates a new resolver client.
func New(c *Config) Interface {
	if c.Name == "" {
		return nil
	}
	ctx, cancel := context.WithCancel(context.Background())

	md := metadata.Pairs("cname", c.Name)
	ctx = metadata.NewOutgoingContext(ctx, md)

	r := &resolverClient{
		config:    *c,
		ctx:       ctx,
		cancel:    cancel,
		svcsMap:   make(map[string]map[string]*types.ServiceInstance),
		observers: make([]Observer, 0),
	}
	go r.runUntilCancel()
	return r
}

func (r *resolverClient) String() string {
	var b strings.Builder
	fmt.Fprintf(&b, "name:%s servers:%s ", r.config.Name, strings.Join(r.config.Servers, ","))
	fmt.Fprintf(&b, "Services:")
	for srv, s := range r.svcsMap {

		instStr := []string{}
		for instName, inst := range s {
			instStr = append(instStr, fmt.Sprintf("inst:%s,image:%s,node:%s,url:%s", instName, inst.Image, inst.Node, inst.URL))
		}
		fmt.Fprintf(&b, "%s:{ %s } ", srv, strings.Join(instStr, " "))
	}

	return b.String()
}

// runUntilCancel implements the business logic of the resolver client.
func (r *resolverClient) runUntilCancel() {
	s := rand.NewSource(time.Now().UnixNano())
	var rpcClient *rpckit.RPCClient
	var err error
	defer func() {
		if rpcClient != nil {
			rpcClient.Close()
			rpcClient = nil
		}
	}()
	for {
		// Check if cancelled.
		r.Lock()
		if r.cancel == nil {
			r.Unlock()
			return
		}
		servers := r.config.Servers
		r.Unlock()

		if len(servers) == 0 {
			time.Sleep(2 * time.Second)
			continue
		}

		// Pick one of the servers at random.
		i := rand.New(s).Intn(len(servers))

		if rpcClient != nil {
			rpcClient.Close()
			rpcClient = nil
		}

		// grpc client setup
		// By default resolver server uses TLS and is hosted by CMD.
		// If client passes different options, they override
		rpcOptions := []rpckit.Option{rpckit.WithRemoteServerName(globals.Cmd)}
		rpcOptions = append(rpcOptions, r.config.Options...)
		rpcClient, err = rpckit.NewRPCClient(r.config.Name, servers[i], rpcOptions...)
		if err != nil {
			time.Sleep(rpcRetryInterval)
			continue
		}
		client := types.NewServiceAPIClient(rpcClient.ClientConn)

		// watch for events in a loop.
		watcher, err := client.WatchServiceInstances(r.ctx, &api.Empty{})
		if err != nil {
			time.Sleep(rpcRetryInterval)
			continue
		}
		log.Infof("Resolver connected to server %v", servers[i])
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
				log.Infof("Resolver :%s", r)
				r.Unlock()
				first = false
				for ii := range el.Items {
					r.notify(*el.Items[ii])
				}
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
			for ii := range el.Items {
				r.notify(*el.Items[ii])
			}
		}

		// Sleep before retrying
		time.Sleep(rpcRetryInterval)
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

// GetURLs gets the URLs for a service.
func (r *resolverClient) GetURLs(name string) []string {
	r.Lock()
	defer r.Unlock()
	result := []string{}
	if r.svcsMap == nil {
		return result
	}
	svcMap, ok := r.svcsMap[name]
	if !ok {
		return result
	}
	for _, v := range svcMap {
		if v.URL != "" {
			result = append(result, v.URL)
		}
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

// Register an observer.
func (r *resolverClient) Register(o Observer) {
	r.Lock()
	defer r.Unlock()
	r.observers = append(r.observers, o)
}

// Deregister an observer.
func (r *resolverClient) Deregister(o Observer) {
	r.Lock()
	defer r.Unlock()
	var i int
	for i = range r.observers {
		if r.observers[i] == o {
			r.observers = append(r.observers[:i], r.observers[i+1:]...)
			return
		}
	}
}

// notify all observers, return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (r *resolverClient) notify(e types.ServiceInstanceEvent) error {
	var err error
	r.Lock()
	observers := make([]Observer, len(r.observers))
	copy(observers, r.observers)
	r.Unlock()
	for _, o := range observers {
		er := o.OnNotifyResolver(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

func (r *resolverClient) UpdateServers(servers []string) {
	log.Infof("Resolver servers updated to %v", servers)

	s := append([]string{}, servers...)
	r.Lock()
	r.config.Servers = s
	r.Unlock()
}
