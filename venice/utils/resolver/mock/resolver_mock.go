package mock

import (
	"fmt"
	"sync"

	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/resolver"
)

// ResolverClient is a mock implementation of resolver client.
type ResolverClient struct {
	sync.Mutex
	svcsMap   map[string]map[string]*protos.ServiceInstance
	observers []resolver.Observer
}

// New creates a new mock resolver client.
func New() *ResolverClient {
	r := &ResolverClient{
		svcsMap:   make(map[string]map[string]*protos.ServiceInstance),
		observers: make([]resolver.Observer, 0),
	}
	return r
}

// Register an observer.
func (r *ResolverClient) Register(o resolver.Observer) {
	r.Lock()
	defer r.Unlock()
	r.observers = append(r.observers, o)
}

// Deregister an observer.
func (r *ResolverClient) Deregister(o resolver.Observer) {
	r.Lock()
	defer r.Unlock()
	var i int
	for i = range r.observers {
		if r.observers[i] == o {
			break
		}
	}
	r.observers = append(r.observers[:i], r.observers[i+1:]...)
}

// notify all observers, return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (r *ResolverClient) notify(e protos.ServiceInstanceEvent) error {
	var err error
	for _, o := range r.observers {
		er := o.OnNotifyResolver(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// Stop is a mock implementation of stopping the resolver client.
func (r *ResolverClient) Stop() {
}

// Lookup resolves a service to its instances.
func (r *ResolverClient) Lookup(name string) *protos.ServiceInstanceList {
	r.Lock()
	defer r.Unlock()
	result := &protos.ServiceInstanceList{
		Items: make([]*protos.ServiceInstance, 0),
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
func (r *ResolverClient) GetURLs(name string) []string {
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

// AddServiceInstance is used by test cases to add a service instance.
func (r *ResolverClient) AddServiceInstance(si *protos.ServiceInstance) error {
	r.Lock()
	svc, ok := r.svcsMap[si.Service]
	if !ok {
		svc = make(map[string]*protos.ServiceInstance)
	}
	svc[si.Name] = si
	r.svcsMap[si.Service] = svc
	r.Unlock()
	r.notify(protos.ServiceInstanceEvent{
		Type:     protos.ServiceInstanceEvent_Added,
		Instance: si,
	})
	return nil
}

// DeleteServiceInstance is used by test cases to delete a service instance.
func (r *ResolverClient) DeleteServiceInstance(si *protos.ServiceInstance) error {
	r.Lock()
	svc, ok := r.svcsMap[si.Service]
	if !ok {
		return fmt.Errorf("Not found %v", si.Name)
	}
	delete(svc, si.Name)
	r.svcsMap[si.Service] = svc
	r.Unlock()
	r.notify(protos.ServiceInstanceEvent{
		Type:     protos.ServiceInstanceEvent_Deleted,
		Instance: si,
	})
	return nil
}
