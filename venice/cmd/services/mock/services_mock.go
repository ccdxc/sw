package mock

import (
	"context"
	"errors"
	"fmt"
	"sync"

	k8sclient "k8s.io/client-go/kubernetes"
	"k8s.io/client-go/pkg/api/v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/kvstore"
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
	started   bool
}

// NewLeaderService creates a mock leader Service
func NewLeaderService(id string) *LeaderService {
	return &LeaderService{
		id: id,
	}
}

// ID returns the leader ID.
func (l *LeaderService) ID() string {
	return l.id
}

// Start starts leader election on quorum nodes.
func (l *LeaderService) Start() error {
	if l.started == false {
		l.BecomeLeader()
	}
	l.started = true
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
func (c *Configs) GenerateKubeletConfig(nodeID, virtualIP string, p string) error {
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

// DeleteNode deletes a node
func (m *K8sService) DeleteNode(name string) error {
	return nil
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
	svcMap    map[string]protos.Service
	observers []types.ServiceInstanceObserver
}

// NewResolverService creates a mock resolver service.
func NewResolverService() *ResolverService {
	return &ResolverService{
		svcMap: make(map[string]protos.Service),
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

func (m *ResolverService) notify(e protos.ServiceInstanceEvent) error {
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
func (m *ResolverService) Get(name string) *protos.Service {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[name]
	if !ok {
		return nil
	}
	return &svc
}

// GetInstance is a mock implementation of resolving a service instance.
func (m *ResolverService) GetInstance(name, instance string) *protos.ServiceInstance {
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
func (m *ResolverService) List() *protos.ServiceList {
	m.Lock()
	defer m.Unlock()
	slist := &protos.ServiceList{
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
func (m *ResolverService) ListInstances() *protos.ServiceInstanceList {
	m.Lock()
	defer m.Unlock()
	slist := &protos.ServiceInstanceList{
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
func (m *ResolverService) AddServiceInstance(si *protos.ServiceInstance) error {
	m.Lock()
	defer m.Unlock()
	svc, ok := m.svcMap[si.Service]
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
	}
	for ii := range svc.Instances {
		if svc.Instances[ii].Name == si.Name {
			return fmt.Errorf("Svc instance %+v already exists", si)
		}
	}
	svc.Instances = append(svc.Instances, si)
	m.svcMap[svc.Name] = svc
	m.notify(protos.ServiceInstanceEvent{
		Type:     protos.ServiceInstanceEvent_Added,
		Instance: si,
	})
	return nil
}

// DeleteServiceInstance is used by test cases to delete a service instance.
func (m *ResolverService) DeleteServiceInstance(si *protos.ServiceInstance) error {
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
			m.notify(protos.ServiceInstanceEvent{
				Type:     protos.ServiceInstanceEvent_Deleted,
				Instance: si,
			})
		}
	}
	return nil
}

// Cluster mocks Dummy cluster
type Cluster struct {
	DummyCluster cmd.Cluster
}

// Create mocks cluster Create
func (c *Cluster) Create(ctx context.Context, in *cmd.Cluster) (*cmd.Cluster, error) {
	c.DummyCluster = *in
	return &c.DummyCluster, nil
}

// Update mocks cluster Update
func (c *Cluster) Update(ctx context.Context, in *cmd.Cluster) (*cmd.Cluster, error) {
	c.DummyCluster = *in
	return &c.DummyCluster, nil
}

// Get mocks cluster Get
func (c *Cluster) Get(ctx context.Context, objMeta *api.ObjectMeta) (*cmd.Cluster, error) {
	if objMeta.Name == c.DummyCluster.Name && objMeta.Tenant == c.DummyCluster.Tenant {
		return &c.DummyCluster, nil
	}
	return nil, nil
}

// Delete mocks cluster Delete
func (c *Cluster) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*cmd.Cluster, error) {
	return nil, errors.New("not implemented")
}

// List mocks cluster List
func (c *Cluster) List(ctx context.Context, options *api.ListWatchOptions) ([]*cmd.Cluster, error) {
	rv := make([]*cmd.Cluster, 1)
	rv[0] = &c.DummyCluster
	return rv, nil
}

// Watch mocks cluster Watch - not implemented till we need this
func (c *Cluster) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, errors.New("not implemented")
}

// Allowed mocks cluster Allowed
func (c *Cluster) Allowed(oper apiserver.APIOperType) bool {
	return false
}

// APIClient mocks APIClient interface
type APIClient struct {
	DummyCluster Cluster
}

// Cluster returns mock ClusterInterface
func (ma *APIClient) Cluster() cmd.CmdV1ClusterInterface {
	return &ma.DummyCluster
}

// Node return mock NodeInterface - nil till we need this functionality
func (ma *APIClient) Node() cmd.CmdV1NodeInterface {
	return nil
}

// SmartNIC returns mock SmartNICInterface - nill till we need this functionality
func (ma *APIClient) SmartNIC() cmd.CmdV1SmartNICInterface {
	return nil
}

// CfgWatcherService mocks CFGWatcher
type CfgWatcherService struct {
	DummyAPIClient APIClient
	ClusterHandler types.ClusterEventHandler
}

// Start is a dummy Start
func (c *CfgWatcherService) Start() {}

// Stop is a dummy stop
func (c *CfgWatcherService) Stop() {}

// SetNodeEventHandler sets the handler to handle events related to Node object
func (c *CfgWatcherService) SetNodeEventHandler(types.NodeEventHandler) {}

// SetClusterEventHandler sets the handler to handle events related to Cluster object
func (c *CfgWatcherService) SetClusterEventHandler(hdlr types.ClusterEventHandler) {
	c.ClusterHandler = hdlr
}

// SetSmartNICEventHandler sets the handler to handle events related to SmartNIC object
func (c *CfgWatcherService) SetSmartNICEventHandler(types.SmartNICEventHandler) {}

// APIClient returns a valid interface once the APIServer is good and
// accepting requests
func (c *CfgWatcherService) APIClient() cmd.CmdV1Interface { return &c.DummyAPIClient }

// NodeService is the interface for starting/stopping/configuring services running on all controller nodes
type NodeService struct {
	K8sAPIServerLocation string
	ElasticLocation      string
}

// Start service
func (n *NodeService) Start() error {
	return nil
}

// Stop service
func (n *NodeService) Stop() {
	return
}

// AreNodeServicesRunning returns true for now
func (n *NodeService) AreNodeServicesRunning() bool {
	return true
}

// KubeletConfig is dummy
func (n *NodeService) KubeletConfig(k8sAPIServerLocation string) {
	n.K8sAPIServerLocation = k8sAPIServerLocation
}

// FileBeatConfig is dummy
func (n *NodeService) FileBeatConfig(elasticLocation string) {
	n.ElasticLocation = elasticLocation
}

// OnNotifySystemdEvent is dummy
func (n *NodeService) OnNotifySystemdEvent(e types.SystemdEvent) error {
	return nil
}
