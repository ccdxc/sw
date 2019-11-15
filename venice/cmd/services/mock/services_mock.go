package mock

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	v1 "k8s.io/api/core/v1"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
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
	id                 string
	observers          []types.LeadershipObserver
	LeaderID           string
	started            bool
	lastTransitionTime time.Time
}

// NewLeaderService creates a mock leader Service
func NewLeaderService(id string) *LeaderService {
	return &LeaderService{
		id: id,
	}
}

// LastTransitionTime mock impl.
func (l *LeaderService) LastTransitionTime() time.Time {
	return l.lastTransitionTime
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
	var i int
	for i = range l.observers {
		if l.observers[i] == o {
			break
		}
	}
	l.observers = append(l.observers[:i], l.observers[i+1:]...)
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
	l.started = false
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
	l.lastTransitionTime = time.Now()
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

// RestartTargetIfRunning is mock implementation of systemd daemon reload.
func (s *SystemdIf) RestartTargetIfRunning(name string) (err error) {
	return nil
}

// DaemonReload is mock implementation of systemd daemon reload.
func (s *SystemdIf) DaemonReload() (err error) {
	return nil
}

// GetUnitProperty is mock implementation of systemd daemon reload.
func (s *SystemdIf) GetUnitProperty(name string, property string) (string, error) {
	return "", nil
}

// GetServiceProperty is mock implementation of systemd daemon reload.
func (s *SystemdIf) GetServiceProperty(name string, property string) (string, error) {
	return "", nil
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
func (c *Configs) GenerateFilebeatConfig(virtualIPs []string) error {
	return nil
}

// RemoveFilebeatConfig is a mock implementation for removing Filebeat configuration.
func (c *Configs) RemoveFilebeatConfig() {
}

// GenerateElasticDiscoveryConfig is a mock implementation for Elastic-discovery config generation.
func (c *Configs) GenerateElasticDiscoveryConfig(virtualIPs []string) error {
	return nil
}

// RemoveElasticDiscoveryConfig is a mock implementation for removing Elastic-discovery configuration.
func (c *Configs) RemoveElasticDiscoveryConfig() {
}

// GenerateElasticMgmtConfig is a mock implementation for Elastic-Mgmt config generation.
func (c *Configs) GenerateElasticMgmtConfig(mgmtAddr string, quorumSize int) error {
	return nil
}

// GenerateElasticAuthConfig is a mock implementation for Elastic auth config generation.
func (c *Configs) GenerateElasticAuthConfig(nodeID string) error {
	return nil
}

// RemoveElasticMgmtConfig is a mock implementation for removing Elastic-Mgmt configuration.
func (c *Configs) RemoveElasticMgmtConfig() {
}

// RemoveElasticAuthConfig is a mock implementation for removing Elastic auth configuration.
func (c *Configs) RemoveElasticAuthConfig() {
}

// GenerateKubeMasterConfig is a mock implemenation for kube master config generation.
func (c *Configs) GenerateKubeMasterConfig(nodeID, virtualIP string) error {
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

// GetClient mock impl.
func (m *K8sService) GetClient() k8sclient.Interface {
	return nil
}

// GetWatchClient mock impl.
func (m *K8sService) GetWatchClient() k8sclient.Interface {
	return nil
}

// UpgradeServices is a mock
func (m *K8sService) UpgradeServices(order []string) error {
	return nil
}

// Start is a mock implementation of starting kubernetes service.
func (m *K8sService) Start(client, strClient k8sclient.Interface, isLeader bool) {
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
			m.observers = append(m.observers[:i], m.observers[i+1:]...)
			break
		}
	}
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
	sync.RWMutex
	DummyCluster cmd.Cluster
}

// Create mocks cluster Create
func (c *Cluster) Create(ctx context.Context, in *cmd.Cluster) (*cmd.Cluster, error) {
	c.Lock()
	defer c.Unlock()
	c.DummyCluster = *in
	return &c.DummyCluster, nil
}

// Update mocks cluster Update
func (c *Cluster) Update(ctx context.Context, in *cmd.Cluster) (*cmd.Cluster, error) {
	c.Lock()
	defer c.Unlock()
	c.DummyCluster = *in
	return &c.DummyCluster, nil
}

// UpdateStatus mocks cluster UpdateStatus
func (c *Cluster) UpdateStatus(ctx context.Context, in *cmd.Cluster) (*cmd.Cluster, error) {
	c.Lock()
	defer c.Unlock()
	c.DummyCluster = *in
	return &c.DummyCluster, nil
}

// Get mocks cluster Get
func (c *Cluster) Get(ctx context.Context, objMeta *api.ObjectMeta) (*cmd.Cluster, error) {
	c.RLock()
	defer c.RUnlock()
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
	c.RLock()
	defer c.RUnlock()
	rv := make([]*cmd.Cluster, 1)
	rv[0] = &c.DummyCluster
	return rv, nil
}

// Watch mocks cluster Watch - not implemented till we need this
func (c *Cluster) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, errors.New("not implemented")
}

// Allowed mocks cluster Allowed
func (c *Cluster) Allowed(oper apiintf.APIOperType) bool {
	return false
}

// AuthBootstrapComplete mocks setting auth bootstrap flag
func (c *Cluster) AuthBootstrapComplete(ctx context.Context, in *cmd.ClusterAuthBootstrapRequest) (*cmd.Cluster, error) {
	c.DummyCluster.Status.AuthBootstrapped = true
	return &c.DummyCluster, nil
}

// UpdateTLSConfig mocks setting TLS Config for API Gateway
func (c *Cluster) UpdateTLSConfig(ctx context.Context, in *cmd.UpdateTLSConfigRequest) (*cmd.Cluster, error) {
	return nil, errors.New("not implemented")
}

// Node mocks Dummy node
type Node struct {
	sync.RWMutex
	DummyNode   cmd.Node
	nodeWatcher *mockNodeWathcer
}

// Create mocks nodes Create
func (n *Node) Create(ctx context.Context, in *cmd.Node) (*cmd.Node, error) {
	n.Lock()
	defer n.Unlock()
	n.DummyNode = *in
	if n.nodeWatcher == nil {
		_, cancel := context.WithCancel(context.Background())
		n.nodeWatcher = &mockNodeWathcer{ch: make(chan *kvstore.WatchEvent, 100), cancel: cancel}
		n.nodeWatcher.ch <- &kvstore.WatchEvent{Type: kvstore.Created, Object: &n.DummyNode}
	}
	return &n.DummyNode, nil
}

// Update mocks nodes Update
func (n *Node) Update(ctx context.Context, in *cmd.Node) (*cmd.Node, error) {
	n.Lock()
	defer n.Unlock()
	n.DummyNode = *in
	n.nodeWatcher.ch <- &kvstore.WatchEvent{Type: kvstore.Updated, Object: &n.DummyNode}
	return &n.DummyNode, nil
}

// UpdateStatus mocks nodes UpdateStatus
func (n *Node) UpdateStatus(ctx context.Context, in *cmd.Node) (*cmd.Node, error) {
	n.Lock()
	defer n.Unlock()
	n.DummyNode = *in
	n.nodeWatcher.ch <- &kvstore.WatchEvent{Type: kvstore.Updated, Object: &n.DummyNode}
	return &n.DummyNode, nil
}

// Get mocks nodes Get
func (n *Node) Get(ctx context.Context, objMeta *api.ObjectMeta) (*cmd.Node, error) {
	n.RLock()
	defer n.RUnlock()
	if objMeta.Name == n.DummyNode.Name && objMeta.Tenant == n.DummyNode.Tenant {
		return &n.DummyNode, nil
	}
	return nil, nil
}

// Delete mocks nodes Delete
func (n *Node) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*cmd.Node, error) {
	return nil, errors.New("not implemented")
}

// List mocks nodes List
func (n *Node) List(ctx context.Context, options *api.ListWatchOptions) ([]*cmd.Node, error) {
	n.RLock()
	defer n.RUnlock()
	rv := make([]*cmd.Node, 1)
	rv[0] = &n.DummyNode
	return rv, nil
}

// Watch mocks nodes Watch - not implemented till we need this
func (n *Node) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	n.Lock()
	defer n.Unlock()
	if n.nodeWatcher == nil {
		_, cancel := context.WithCancel(context.Background())
		n.nodeWatcher = &mockNodeWathcer{ch: make(chan *kvstore.WatchEvent, 100), cancel: cancel}
		n.nodeWatcher.ch <- &kvstore.WatchEvent{Type: kvstore.Created, Object: &n.DummyNode}
	}
	return n.nodeWatcher, nil
}

// mock node watcher
type mockNodeWathcer struct {
	ch     chan *kvstore.WatchEvent
	cancel context.CancelFunc
}

// EventChan returns the event channel
func (m *mockNodeWathcer) EventChan() <-chan *kvstore.WatchEvent {
	return m.ch
}

// Stop stops the watcher
func (m *mockNodeWathcer) Stop() {
	m.cancel()
}

// Allowed mocks node Allowed
func (n *Node) Allowed(oper apiintf.APIOperType) bool {
	return false
}

// APIClient mocks APIClient interface
type APIClient struct {
	DummyCluster Cluster
	DummyNode    Node
}

// Cluster returns mock ClusterInterface
func (ma *APIClient) Cluster() cmd.ClusterV1ClusterInterface {
	return &ma.DummyCluster
}

// Node return mock NodeInterface - nil till we need this functionality
func (ma *APIClient) Node() cmd.ClusterV1NodeInterface {
	return &ma.DummyNode
}

// Host return mock HostInterface - nil till we need this functionality
func (ma *APIClient) Host() cmd.ClusterV1HostInterface {
	return nil
}

// Version return mock VersionInterface - nil till we need this functionality
func (ma *APIClient) Version() cmd.ClusterV1VersionInterface {
	return nil
}

// DistributedServiceCard returns mock DistributedServiceCardInterface - nill till we need this functionality
func (ma *APIClient) DistributedServiceCard() cmd.ClusterV1DistributedServiceCardInterface {
	return nil
}

// Tenant returns mock Tenant - nill till we need this functionality
func (ma *APIClient) Tenant() cmd.ClusterV1TenantInterface {
	return nil
}

// ConfigurationSnapshot returns mock Tenant - nill till we need this functionality
func (ma *APIClient) ConfigurationSnapshot() cmd.ClusterV1ConfigurationSnapshotInterface {
	return nil
}

// SnapshotRestore returns mock Tenant - nill till we need this functionality
func (ma *APIClient) SnapshotRestore() cmd.ClusterV1SnapshotRestoreInterface {
	return nil
}

// Watch implements a watch on the service - nill till we need this functionality
func (ma *APIClient) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}

// CfgWatcherService mocks CFGWatcher
type CfgWatcherService struct {
	DummyAPIClient APIClient
	ClusterHandler types.ClusterEventHandler
	APIClientSvc   apiclient.Services
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

// SetHostEventHandler sets the handler to handle events related to Host object
func (c *CfgWatcherService) SetHostEventHandler(types.HostEventHandler) {}

// SetNodeService sets the node service to update 3rd party services (e.g. elastic) on node updates
func (c *CfgWatcherService) SetNodeService(types.NodeService) {}

// SetClusterQuorumNodes sets the initial list of quorum nodes to be updated on 3rd party
// services (e.g. elastic discovery file)
func (c *CfgWatcherService) SetClusterQuorumNodes([]string) {}

// SetNtpEventHandler sets the handler to handle events related to Cluster object
func (c *CfgWatcherService) SetNtpEventHandler(types.ClusterEventHandler) {}

// APIClient returns a valid interface once the APIServer is good and
// accepting requests
func (c *CfgWatcherService) APIClient() cmd.ClusterV1Interface {
	if c.APIClientSvc != nil {
		return c.APIClientSvc.ClusterV1()
	}

	return &c.DummyAPIClient
}

// GetCluster returns the Cluster object
func (c *CfgWatcherService) GetCluster() (*cmd.Cluster, error) {
	cl := c.APIClient()
	return cl.Cluster().Get(context.TODO(), &api.ObjectMeta{})
}

// NodeService is the interface for starting/stopping/configuring services running on all controller nodes
type NodeService struct {
	K8sAPIServerLocation string
	ElasticLocations     []string
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
func (n *NodeService) KubeletConfig(k8sAPIServerLocation string) error {
	n.K8sAPIServerLocation = k8sAPIServerLocation
	return nil
}

// FileBeatConfig is dummy
func (n *NodeService) FileBeatConfig(elasticLocations []string) error {
	n.ElasticLocations = elasticLocations
	return nil
}

// ElasticDiscoveryConfig is dummy
func (n *NodeService) ElasticDiscoveryConfig(elasticLocations []string) error {
	n.ElasticLocations = elasticLocations
	return nil
}

// ElasticMgmtConfig is dummy
func (n *NodeService) ElasticMgmtConfig() error {
	return nil
}

// OnNotifySystemdEvent is dummy
func (n *NodeService) OnNotifySystemdEvent(e types.SystemdEvent) error {
	return nil
}

// InitConfigFiles is dummy
func (n *NodeService) InitConfigFiles() {
}

// ClusterHealthMonitor mock cluster health monitor
type ClusterHealthMonitor struct {
}

// Start mock impl
func (c *ClusterHealthMonitor) Start() {}

// Stop mock impl
func (c *ClusterHealthMonitor) Stop() {}
