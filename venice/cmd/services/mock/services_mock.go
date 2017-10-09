package mock

import (
	"fmt"
	"sync"

	"k8s.io/api/core/v1"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/systemd"
)

//mock objects which are used in multiple tests

// IPService is used by tests for faking interface configuration.
type IPService struct {
	sync.Mutex
	ip    map[string]interface{}
	error bool
}

// NewIPService returns a Mock IPService
func NewIPService() *IPService {
	return &IPService{ip: make(map[string]interface{})}
}

// HasIP returns true if this IP was added before
func (i *IPService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return false, fmt.Errorf("Error set")
	}
	_, ok := i.ip[ip]
	return ok, nil
}

// AddSecondaryIP mocks addition of IP address
func (i *IPService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	_, ok := i.ip[ip]
	if ok {
		return fmt.Errorf("IP already configured")
	}
	i.ip[ip] = nil
	return nil
}

// DeleteIP deletes previously added IP
func (i *IPService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	delete(i.ip, ip)
	return nil
}

// SetError sets an error
func (i *IPService) SetError() {
	i.Lock()
	defer i.Unlock()
	i.error = true
}

// ClearError clears an error
func (i *IPService) ClearError() {
	i.Lock()
	defer i.Unlock()
	i.error = false
}

//LeaderService mocks Leader Service
type LeaderService struct {
	id        string
	observers []types.LeadershipObserver
	LeaderID  string
}

// NewLeaderService creates a mock leader Service
func NewLeaderService(id string) *LeaderService {
	return &LeaderService{
		id: id,
	}
}

// Start starts leader election on quorum nodes.
func (l *LeaderService) Start() error {
	l.BecomeLeader()
	return nil
}

// Register an Observers
func (l *LeaderService) Register(o types.LeadershipObserver) {
	l.observers = append(l.observers, o)
}

//UnRegister an Observer
func (l *LeaderService) UnRegister(o types.LeadershipObserver) {
}

// return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (l *LeaderService) notify(e types.LeaderEvent) error {
	var err error
	for _, o := range l.observers {
		er := o.OnNotifyLeaderEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// Stop stops the leader election.
func (l *LeaderService) Stop() {
	l.processEvent("")
}

// BecomeLeader mocks becoming a leader. If the Notifiers return an error, it returns this error.
func (l *LeaderService) BecomeLeader() error {
	return l.processEvent(l.id)
}

// GiveupLeadership of a leader service
func (l *LeaderService) GiveupLeadership() error {
	return l.processEvent(l.id + "something")
}

// ChangeLeadership from some leader to some other leader
func (l *LeaderService) ChangeLeadership() error {
	return l.processEvent(l.id + "somethingelse")
}

// processEvent handles leader election events.
func (l *LeaderService) processEvent(leader string) error {
	if l.id == leader {
		e := types.LeaderEvent{Evt: types.LeaderEventWon, Leader: leader}
		if err := l.notify(e); err != nil {
			l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
			return err
		}
	} else if l.IsLeader() {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
	} else {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventChange, Leader: leader})
	}
	l.LeaderID = leader
	return nil
}

// IsLeader checks if this instance is the leader.
func (l *LeaderService) IsLeader() bool {
	return l.id == l.LeaderID
}

// Leader returns the current leader.
func (l *LeaderService) Leader() string {
	return l.LeaderID
}

type mockSystemdWatcher struct {
	evChan  chan *systemd.UnitEvent
	errChan chan error

	makeFail bool
}

func (w *mockSystemdWatcher) Close() {
	panic("not implemented")
}

func (w *mockSystemdWatcher) Subscribe(name string) {
	if !w.makeFail {
		w.evChan <- &systemd.UnitEvent{Name: name, Status: systemd.Active}
	} else {
		w.evChan <- &systemd.UnitEvent{Name: name, Status: systemd.Dead}
	}
}

func (w *mockSystemdWatcher) Unsubscribe(name string) {

}

func (w *mockSystemdWatcher) TargetDeps(name string) ([]string, error) {
	panic("not implemented")
}

func (w *mockSystemdWatcher) GetPID(name string) (uint32, error) {
	panic("not implemented")
}

// SystemdIf mocks systemd interface.
type SystemdIf struct {
	m mockSystemdWatcher
}

// DaemonReload is mock implementation of systemd daemon reload.
func (s *SystemdIf) DaemonReload() (err error) {
	return nil
}

// StartTarget is mock implementation of starting a target.
func (s *SystemdIf) StartTarget(name string) (err error) {
	return nil
}

// StopTarget is mock implementation of stopping a target.
func (s *SystemdIf) StopTarget(name string) error {
	return nil
}

// RestartTarget is mock implementation of restarting a target.
func (s *SystemdIf) RestartTarget(name string) error {
	return nil
}

// NewWatcher returns a systemd watcher.
func (s *SystemdIf) NewWatcher() (systemd.Watcher, <-chan *systemd.UnitEvent, <-chan error) {
	s.m = mockSystemdWatcher{evChan: make(chan *systemd.UnitEvent, 2), errChan: make(chan error, 2)}
	return &s.m, s.m.evChan, s.m.errChan
}

// Configs mocks generation of configuration files.
type Configs struct {
}

// GenerateKubeletConfig is a mock implementation of kubelet config generation.
func (c *Configs) GenerateKubeletConfig(virtualIP string) error {
	return nil
}

// RemoveKubeletConfig is a mock implementation for removing kubelet configuration.
func (c *Configs) RemoveKubeletConfig() {
}

// GenerateAPIServerConfig is a mock implementation for API Server config generation.
func (c *Configs) GenerateAPIServerConfig() error {
	return nil
}

// RemoveAPIServerConfig is a mock implementation for removing API Server configuration.
func (c *Configs) RemoveAPIServerConfig() {
}

// GenerateFilebeatConfig is a mock implementation for Filebeat config generation.
func (c *Configs) GenerateFilebeatConfig(virtualIP string) error {
	return nil
}

// RemoveFilebeatConfig is a mock implementation for removing Filebeat configuration.
func (c *Configs) RemoveFilebeatConfig() {
}

// GenerateKubeMasterConfig is a mock implemenation for kube master config generation.
func (c *Configs) GenerateKubeMasterConfig(virtualIP string) error {
	return nil
}

// RemoveKubeMasterConfig is a mock implementation for removing kube master configuration.
func (c *Configs) RemoveKubeMasterConfig() {
}

// K8sService is a mock implementation of kubernetes service to deploy and watch modules.
type K8sService struct {
	sync.Mutex
	observers []types.K8sPodEventObserver
}

// Start is a mock implementation of starting kubernetes service.
func (m *K8sService) Start(client k8sclient.Interface, isLeader bool) {
}

// Stop is a mock implementation of stopping kubernetes service.
func (m *K8sService) Stop() {
}

// Register is a mock implementation of registering an observer.
func (m *K8sService) Register(o types.K8sPodEventObserver) {
	m.Lock()
	defer m.Unlock()
	m.observers = append(m.observers, o)
}

// UnRegister is a mock implementation of unregistering an observer.
func (m *K8sService) UnRegister(o types.K8sPodEventObserver) {
	m.Lock()
	defer m.Unlock()
	var i int
	for i = range m.observers {
		if m.observers[i] == o {
			break
		}
	}
	m.observers = append(m.observers[:i], m.observers[i+1:]...)
}

func (m *K8sService) notify(e types.K8sPodEvent) error {
	var err error
	m.Lock()
	defer m.Unlock()
	for _, o := range m.observers {
		er := o.OnNotifyK8sPodEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// AddPod is used by test cases to simulate a pod creation.
func (m *K8sService) AddPod(pod *v1.Pod) {
	m.notify(types.K8sPodEvent{
		Type: types.K8sPodAdded,
		Pod:  pod,
	})
}

// ModifyPod is used by test cases to simulate pod modification.
func (m *K8sService) ModifyPod(pod *v1.Pod) {
	m.notify(types.K8sPodEvent{
		Type: types.K8sPodModified,
		Pod:  pod,
	})
}

// DeletePod is used by test cases to simulate pod deletion.
func (m *K8sService) DeletePod(pod *v1.Pod) {
	m.notify(types.K8sPodEvent{
		Type: types.K8sPodDeleted,
		Pod:  pod,
	})
}

// ResolverService is a mock implementation of service discovery.
type ResolverService struct {
	sync.Mutex
	svcMap    map[string]types.Service
	observers []types.ServiceInstanceObserver
}

// NewResolverService creates a mock resolver service.
func NewResolverService() *ResolverService {
	return &ResolverService{
		svcMap: make(map[string]types.Service),
	}
}

// Register is a mock implementation of registering an observer.
func (m *ResolverService) Register(o types.ServiceInstanceObserver) {
	m.Lock()
	defer m.Unlock()
	m.observers = append(m.observers, o)
}

// UnRegister is a mock implementation of unregistering an observer.
func (m *ResolverService) UnRegister(o types.ServiceInstanceObserver) {
	m.Lock()
	defer m.Unlock()
	var i int
	for i = range m.observers {
		if m.observers[i] == o {
			break
		}
	}
	m.observers = append(m.observers[:i], m.observers[i+1:]...)
}

func (m *ResolverService) notify(e types.ServiceInstanceEvent) error {
	var err error
	for _, o := range m.observers {
		er := o.OnNotifyServiceInstance(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// Start is a mock implementation of starting the resolver.
func (m *ResolverService) Start() {
}

// Stop is a mock implementation of stopping the resolver.
func (m *ResolverService) Stop() {
}

// Get is a mock implementation of resolving a service.
func (m *ResolverService) Get(name string) *types.Service {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[name]
	if !ok {
		return nil
	}
	return &svc
}

// GetInstance is a mock implementation of resolving a service instance.
func (m *ResolverService) GetInstance(name, instance string) *types.ServiceInstance {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[name]
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

// List is a mock implementation of listing all services.
func (m *ResolverService) List() *types.ServiceList {
	m.Lock()
	defer m.Unlock()
	slist := &types.ServiceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceList",
		},
	}
	for k := range m.svcMap {
		svc := m.svcMap[k]
		slist.Items = append(slist.Items, &svc)
	}
	return slist
}

// ListInstances returns all Service instances.
func (m *ResolverService) ListInstances() *types.ServiceInstanceList {
	m.Lock()
	defer m.Unlock()
	slist := &types.ServiceInstanceList{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstanceList",
		},
	}
	for k := range m.svcMap {
		svc := m.svcMap[k]
		for ii := range svc.Instances {
			slist.Items = append(slist.Items, svc.Instances[ii])
		}
	}
	return slist
}

// AddServiceInstance is used by test cases to add a service instance.
func (m *ResolverService) AddServiceInstance(si *types.ServiceInstance) error {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[si.Service]
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
	}
	for ii := range svc.Instances {
		if svc.Instances[ii].Name == si.Name {
			return fmt.Errorf("Svc instance %+v already exists", si)
		}
	}
	svc.Instances = append(svc.Instances, si)
	m.svcMap[svc.Name] = svc
	m.notify(types.ServiceInstanceEvent{
		Type:     types.ServiceInstanceEvent_Added,
		Instance: si,
	})
	return nil
}

// DeleteServiceInstance is used by test cases to delete a service instance.
func (m *ResolverService) DeleteServiceInstance(si *types.ServiceInstance) error {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[si.Service]
	if !ok {
		return fmt.Errorf("Not found %v", si.Name)
	}
	for ii := range svc.Instances {
		if svc.Instances[ii].Name == si.Name {
			svc.Instances = append(svc.Instances[:ii], svc.Instances[ii+1:]...)
			m.svcMap[svc.Name] = svc
			m.notify(types.ServiceInstanceEvent{
				Type:     types.ServiceInstanceEvent_Deleted,
				Instance: si,
			})
		}
	}
	return nil
}
