package services

import (
	"fmt"
	"sync"

	"k8s.io/client-go/pkg/api/v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
)

// resolverService is responsible for resolving services in a Pensando cluster.
type resolverService struct {
	sync.RWMutex
	k8sSvc    types.K8sService
	svcMap    map[string]types.Service
	running   bool
	observers []types.ServiceInstanceObserver
}

// NewResolverService returns a Resolver Service.
func NewResolverService(k8sSvc types.K8sService) types.ResolverService {
	return &resolverService{
		k8sSvc:    k8sSvc,
		svcMap:    make(map[string]types.Service),
		observers: make([]types.ServiceInstanceObserver, 0),
	}
}

// Start starts the resolver service.
func (r *resolverService) Start() {
	r.Lock()
	defer r.Unlock()
	if r.running {
		return
	}
	r.running = true
	r.k8sSvc.Register(r)
}

// Stop stops the resolver service.
func (r *resolverService) Stop() {
	r.Lock()
	defer r.Unlock()
	if !r.running {
		return
	}
	r.k8sSvc.UnRegister(r)
	r.running = false
}

// OnNotifyK8sPodEvent satisifies the K8s notifier interface.
func (r *resolverService) OnNotifyK8sPodEvent(e types.K8sPodEvent) error {
	sInsts := make([]types.ServiceInstance, 0)
	for _, container := range e.Pod.Spec.Containers {
		if len(container.Ports) > 0 {
			for _, port := range container.Ports {
				sInsts = append(sInsts, types.ServiceInstance{
					TypeMeta: api.TypeMeta{
						Kind: "ServiceInstance",
					},
					ObjectMeta: api.ObjectMeta{
						Name: e.Pod.Name,
					},
					Service: port.Name,
					Image:   container.Image,
					Node:    e.Pod.Status.HostIP,
					URL:     fmt.Sprintf("%s:%d", e.Pod.Status.HostIP, port.ContainerPort),
				})
			}
		} else {
			sInsts = append(sInsts, types.ServiceInstance{
				TypeMeta: api.TypeMeta{
					Kind: "ServiceInstance",
				},
				ObjectMeta: api.ObjectMeta{
					Name: e.Pod.Name,
				},
				Service: container.Name,
				Image:   container.Image,
				Node:    e.Pod.Status.HostIP,
			})
		}
	}

	var instanceRunning bool
	if e.Pod.Status.Phase == v1.PodRunning {
		instanceRunning = true

		// If any condition exists in not ConditionTrue then service is guaranteed to be good.
		for _, condition := range e.Pod.Status.Conditions {
			if condition.Status != v1.ConditionTrue {
				instanceRunning = false
			}
		}
	} else {
		instanceRunning = false
	}

	switch e.Type {
	case types.K8sPodAdded:
		fallthrough
	case types.K8sPodModified:
		if instanceRunning {
			for ii := range sInsts {
				r.addSvcInstance(&sInsts[ii])
			}
			break
		}
		fallthrough
	case types.K8sPodDeleted:
		for ii := range sInsts {
			r.delSvcInstance(&sInsts[ii])
		}
	default:
		return fmt.Errorf("Unknown event type %v", e.Type)
	}
	return nil
}

// addSvcInstance adds a service instance to a service.
func (r *resolverService) addSvcInstance(si *types.ServiceInstance) {
	r.Lock()
	svc, ok := r.svcMap[si.Service]
	if !ok {
		svc = types.Service{
			TypeMeta: api.TypeMeta{
				Kind: "Service",
			},
			ObjectMeta: api.ObjectMeta{
				Name: si.Service,
			},
			Instances: make([]*types.ServiceInstance, 0),
		}
		r.svcMap[svc.Name] = svc
	}

	for ii := range svc.Instances {
		if svc.Instances[ii].Name == si.Name {
			r.Unlock()
			return
		}
	}
	log.Infof("Adding svc instance %+v", si)
	svc.Instances = append(svc.Instances, si)
	r.svcMap[si.Service] = svc
	r.Unlock()
	r.notify(types.ServiceInstanceEvent{
		Type:     types.ServiceInstanceEvent_Added,
		Instance: si,
	})
}

// delSvcInstance deletes a service instance from a service.
func (r *resolverService) delSvcInstance(si *types.ServiceInstance) {
	r.Lock()

	svc, ok := r.svcMap[si.Service]
	if !ok {
		r.Unlock()
		return
	}

	for ii := range svc.Instances {
		if svc.Instances[ii].Name == si.Name {
			log.Infof("Deleting svc instance %+v", si)
			svc.Instances = append(svc.Instances[:ii], svc.Instances[ii+1:]...)
			r.svcMap[si.Service] = svc
			r.Unlock()
			r.notify(types.ServiceInstanceEvent{
				Type:     types.ServiceInstanceEvent_Deleted,
				Instance: si,
			})
			return
		}
	}
	r.Unlock()
}

// Get returns a Service.
func (r *resolverService) Get(name string) *types.Service {
	r.RLock()
	defer r.RUnlock()
	svc, ok := r.svcMap[name]
	if !ok {
		return nil
	}
	return &svc
}

// GetInstance returns an instance of a Service.
func (r *resolverService) GetInstance(name, instance string) *types.ServiceInstance {
	r.RLock()
	defer r.RUnlock()
	svc, ok := r.svcMap[name]
	if !ok {
		return nil
	}
	for ii := range svc.Instances {
		if svc.Instances[ii].Name == instance {
			return svc.Instances[ii]
		}
	}
	return nil
}

// List returns all Services.
func (r *resolverService) List() *types.ServiceList {
	r.RLock()
	defer r.RUnlock()
	slist := &types.ServiceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceList",
		},
	}
	for k := range r.svcMap {
		svc := r.svcMap[k]
		slist.Items = append(slist.Items, &svc)
	}
	return slist
}

// ListInstances returns all Service instances.
func (r *resolverService) ListInstances() *types.ServiceInstanceList {
	r.RLock()
	defer r.RUnlock()
	slist := &types.ServiceInstanceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstanceList",
		},
	}
	for k := range r.svcMap {
		svc := r.svcMap[k]
		for ii := range svc.Instances {
			slist.Items = append(slist.Items, svc.Instances[ii])
		}
	}
	return slist
}

func (r *resolverService) Register(o types.ServiceInstanceObserver) {
	r.Lock()
	defer r.Unlock()
	r.observers = append(r.observers, o)
}

func (r *resolverService) UnRegister(o types.ServiceInstanceObserver) {
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

// All the observers are notified of the event even if someone fails,
// returns first encountered error.
func (r *resolverService) notify(e types.ServiceInstanceEvent) error {
	r.RLock()
	defer r.RUnlock()
	var err error
	for _, o := range r.observers {
		er := o.OnNotifyServiceInstance(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}
