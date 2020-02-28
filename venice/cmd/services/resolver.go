package services

import (
	"fmt"
	"sort"
	"sync"
	"time"

	"k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
)

// resolverService is responsible for resolving services in a Pensando cluster.
type resolverService struct {
	sync.RWMutex
	k8sSvc    types.K8sService
	svcMap    map[string]protos.Service
	running   bool
	observers []types.ServiceInstanceObserver

	// BEGIN WORKAROUND
	// TODO: remove workaround for following Kubernetes issues when move to a version with proper fixes:
	// https://github.com/kubernetes/kubernetes/issues/80968
	// https://github.com/kubernetes/kubernetes/issues/82346
	pendingInstances     map[string][]protos.ServiceInstance // instances that we will need to be added later due to readiness probe
	pendingInstancesCh   chan bool
	pendingInstancesLock sync.Mutex
}

var (
	// time since creation of a pod with readiness probes after which we declare it good even if Ready = false
	pendingInstancesGracePeriod = 60 * time.Second
	pendingInstancesCheckPeriod = 5 * time.Second
)

// END WORKAROUND

// NewResolverService returns a Resolver Service.
func NewResolverService(k8sSvc types.K8sService) types.ResolverService {
	r := &resolverService{
		k8sSvc:    k8sSvc,
		svcMap:    make(map[string]protos.Service),
		observers: make([]types.ServiceInstanceObserver, 0),
		// BEGIN WORKAROUND
		pendingInstances:   make(map[string][]protos.ServiceInstance),
		pendingInstancesCh: make(chan bool),
		// END WORKAROUND
	}
	return r
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
	go r.monitorPendingInstances(r.pendingInstancesCh)
}

// Stop stops the resolver service.
func (r *resolverService) Stop() {
	r.Lock()
	defer r.Unlock()
	if !r.running {
		return
	}
	r.pendingInstancesCh <- true
	r.k8sSvc.UnRegister(r)
	r.running = false
}

// OnNotifyK8sPodEvent satisifies the K8s notifier interface.
func (r *resolverService) OnNotifyK8sPodEvent(e types.K8sPodEvent) error {
	if isCronJob(e.Pod) {
		log.Infof("ignoring pod event, pod(%s) belongs to a cron job", e.Pod.Name)
		return nil
	}
	sInsts := make([]protos.ServiceInstance, 0)
	for _, container := range e.Pod.Spec.Containers {
		if len(container.Ports) > 0 {
			for _, port := range container.Ports {
				sInsts = append(sInsts, protos.ServiceInstance{
					TypeMeta: api.TypeMeta{
						Kind: "ServiceInstance",
					},
					ObjectMeta: api.ObjectMeta{
						Name: e.Pod.Name,
					},
					Service: port.Name,
					Image:   container.Image,
					Node:    e.Pod.Spec.NodeName,
					URL:     fmt.Sprintf("%s:%d", e.Pod.Status.HostIP, port.ContainerPort),
				})
			}
		} else {
			sInsts = append(sInsts, protos.ServiceInstance{
				TypeMeta: api.TypeMeta{
					Kind: "ServiceInstance",
				},
				ObjectMeta: api.ObjectMeta{
					Name: e.Pod.Name,
				},
				Service: container.Name,
				Image:   container.Image,
				Node:    e.Pod.Spec.NodeName,
			})
		}
	}

	instanceRunning := isPodRunning(e.Pod)

	switch e.Type {
	case types.K8sPodAdded:
		fallthrough
	case types.K8sPodModified:
		if instanceRunning {
			// BEGIN WORKAROUND
			if podHasReadinessProbe(e.Pod) && getPodReadyCond(e.Pod) == false {
				// since we are ignoring the Ready flag (see comment in isPodRunning)
				// if this pod has readiness probe and is still not ready, it needs
				// to be added asynchronously
				r.pendingInstancesLock.Lock()
				r.pendingInstances[e.Pod.Name] = sInsts
				log.Infof("Added instances for pod %s to pending map %+v", e.Pod.Name, r.pendingInstances)
				r.pendingInstancesLock.Unlock()
				break
			}
			// END WORKAROUND
			for ii := range sInsts {
				r.addSvcInstance(&sInsts[ii])
			}
			// BEGIN WORKAROUND
			r.pendingInstancesLock.Lock()
			delete(r.pendingInstances, e.Pod.Name)
			r.pendingInstancesLock.Unlock()
			// END WORKAROUND
			break
		}
		fallthrough
	case types.K8sPodDeleted:
		// BEGIN WORKAROUND
		r.pendingInstancesLock.Lock()
		delete(r.pendingInstances, e.Pod.Name)
		r.pendingInstancesLock.Unlock()
		// END WORKAROUND
		for ii := range sInsts {
			r.delSvcInstance(&sInsts[ii])
		}
	default:
		return fmt.Errorf("Unknown event type %v", e.Type)
	}
	return nil
}

func (r *resolverService) AddServiceInstance(si *protos.ServiceInstance) error {
	r.addSvcInstance(si)
	return nil
}

// addSvcInstance adds a service instance to a service.
func (r *resolverService) addSvcInstance(si *protos.ServiceInstance) {
	r.Lock()
	svc, ok := r.svcMap[si.Service]
	if !ok {
		svc = protos.Service{
			TypeMeta: api.TypeMeta{
				Kind: "Service",
			},
			ObjectMeta: api.ObjectMeta{
				Name: si.Service,
			},
			Instances: make([]*protos.ServiceInstance, 0),
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
	r.notify(protos.ServiceInstanceEvent{
		Type:     protos.ServiceInstanceEvent_Added,
		Instance: si,
	})
}

func (r *resolverService) DeleteServiceInstance(si *protos.ServiceInstance) error {
	r.delSvcInstance(si)
	return nil
}

// delSvcInstance deletes a service instance from a service.
func (r *resolverService) delSvcInstance(si *protos.ServiceInstance) {
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
			r.notify(protos.ServiceInstanceEvent{
				Type:     protos.ServiceInstanceEvent_Deleted,
				Instance: si,
			})
			return
		}
	}
	r.Unlock()
}

// Get returns a Service.
func (r *resolverService) Get(name string) *protos.Service {
	r.RLock()
	defer r.RUnlock()
	svc, ok := r.svcMap[name]
	if !ok {
		return nil
	}
	return &svc
}

// GetInstance returns an instance of a Service.
func (r *resolverService) GetInstance(name, instance string) *protos.ServiceInstance {
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
func (r *resolverService) List() *protos.ServiceList {
	r.RLock()
	defer r.RUnlock()
	slist := &protos.ServiceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceList",
		},
	}
	for k := range r.svcMap {
		svc := r.svcMap[k]
		// return sorted list so that results from multiple invocations of this RPC can be compared using reflect.DeepEqual
		sort.Slice(svc.Instances, func(i, j int) bool { return svc.Instances[i].Name < svc.Instances[j].Name })
		slist.Items = append(slist.Items, &svc)
	}
	return slist
}

// ListInstances returns all Service instances.
func (r *resolverService) ListInstances() *protos.ServiceInstanceList {
	r.RLock()
	defer r.RUnlock()
	slist := &protos.ServiceInstanceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstanceList",
		},
	}
	for k := range r.svcMap {
		svc := r.svcMap[k]
		for ii := range svc.Instances {
			slist.Items = append(slist.Items, svc.Instances[ii])
		}
		// return sorted list so that results from multiple invocations of this RPC can be compared using reflect.DeepEqual
		sort.Slice(slist.Items, func(i, j int) bool { return slist.Items[i].Name < slist.Items[j].Name })
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
			r.observers = append(r.observers[:i], r.observers[i+1:]...)
			break
		}
	}
}

// All the observers are notified of the event even if someone fails,
// returns first encountered error.
func (r *resolverService) notify(e protos.ServiceInstanceEvent) error {
	r.RLock()
	defer r.RUnlock()
	var err error
	for _, o := range r.observers {
		log.Infof("Calling observer: %+v  with serviceInstanceEvent: %v", o, e)
		er := o.OnNotifyServiceInstance(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// BEGIN WORKAROUND
func podHasReadinessProbe(pod *v1.Pod) bool {
	for _, cont := range pod.Spec.Containers {
		if cont.ReadinessProbe != nil {
			return true
		}
	}
	return false
}

func getPodReadyCond(pod *v1.Pod) bool {
	for _, condition := range pod.Status.Conditions {
		if condition.Type == v1.PodReady {
			return condition.Status == v1.ConditionTrue
		}
	}
	return false
}

func (r *resolverService) canProgramPendingInstance(podName string) bool {
	client := r.k8sSvc.GetClient()
	pod, err := client.CoreV1().Pods(defaultNS).Get(podName, metav1.GetOptions{})
	if err != nil {
		log.Errorf("Error getting pod from Kubernetes: %v", err)
		return false
	}
	log.Infof("canProgramPendingInstance fetched pod %s from Kubernetes: %+v", pod.Name, pod)
	if getPodReadyCond(pod) == true || time.Since(pod.ObjectMeta.GetCreationTimestamp().Time) >= pendingInstancesGracePeriod {
		return true
	}
	return false
}

func (r *resolverService) monitorPendingInstances(done chan bool) {
	ticker := time.NewTicker(pendingInstancesCheckPeriod)
	defer ticker.Stop()
	log.Infof("Starting pending instances monitor")
	for {
		select {
		case <-ticker.C:
			r.pendingInstancesLock.Lock()
			for podName, insts := range r.pendingInstances {
				if r.canProgramPendingInstance(podName) {
					for ii := range insts {
						r.addSvcInstance(&insts[ii])
					}
					log.Infof("Programmed pending instances for pod %s: %+v", podName, insts)
					delete(r.pendingInstances, podName)
				}
			}
			if len(r.pendingInstances) > 0 {
				log.Infof("Pending instances after iteration: %+v", r.pendingInstances)
			}
			r.pendingInstancesLock.Unlock()
		case <-done:
			// clear pending services map
			r.pendingInstances = make(map[string][]protos.ServiceInstance)
			log.Infof("Stopping PendingInstances monitor")
			return
		}
	}
}

// END WORKAROUND
