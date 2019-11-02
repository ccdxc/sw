package types

import (
	"context"
	"time"

	"k8s.io/api/core/v1"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	rolloutproto "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// LeaderEvtType indicates what happened regarding the leadership
type LeaderEvtType string

const (
	// LeaderEventWon is When this node is becoming a Leader
	LeaderEventWon LeaderEvtType = "LeaderEventWon"
	// LeaderEventLost is When this node is giving up leadership
	LeaderEventLost = "LeaderEventLost"
	// LeaderEventChange is when this node is not a leader and Some leader changed in the system
	LeaderEventChange = "LeaderEventChange"
)

// LeaderEvent is Notified by LeaderService when any leadership change event happens
type LeaderEvent struct {
	// Evt indicated what happened
	Evt LeaderEvtType
	// Leader is the new Leader
	Leader string
}

// LeadershipObserver is implemented by services which are interested in knowing
// about the CMD leadership status
type LeadershipObserver interface {
	// OnNotifyLeaderEvent allows an event to be "published" to interface implementations.
	OnNotifyLeaderEvent(LeaderEvent) error
}

// LeaderService is the interface for leader election service.
type LeaderService interface {
	Start() error
	Stop()
	Leader() string
	IsLeader() bool
	ID() string
	LastTransitionTime() time.Time

	// Register registers for Observer. The Observers are called in the Order of registration
	// NOTE: In the future if we need a different order to be called for Start and Stop routines, then
	// 		we will need separate Register for Start and Stop. Can be addressed when such a need arises
	Register(LeadershipObserver)
	UnRegister(LeadershipObserver)
}

// IPService is the interface for managing IP addresses on interfaces.
type IPService interface {
	HasIP(string) (bool, error)
	AddSecondaryIP(string) error
	DeleteIP(string) error
}

// VIPService is interface for managing Virtual IP in the cluster.
type VIPService interface {
	// GetAllVirtualIPs returns the list of VirtualIPs known to this service. Some virtualIP may be on this node
	// and some may be on other quorum nodes in the network
	GetAllVirtualIPs() []string

	// GetConfiguredVirtualIPs returns the VirtualIPs that are configured on this node (because this node is
	// the leader). known VirtualIPs (configured on other quorum nodes of the cluster) are not returned by this call
	GetConfiguredVirtualIPs() []string

	// AddVirtualIPs adds a set of virtualIPs to the cluster VIP service. Some of the VIP may be configured
	// on this node and some on other nodes in the cluster. This call should be made on all the quorum nodes in the
	// cluster
	AddVirtualIPs(ips ...string) error

	// DeleteVirtualIPs removes the set of virtualIPs from the cluster VIP service. Some of the VIP may be configured
	// on this node and some on other nodes in the cluster. This call should be made on all the quorum nodes in the
	// cluster
	DeleteVirtualIPs(ips ...string) error
}

// NodeService is the interface for starting/stopping/configuring services running on all controller nodes
type NodeService interface {
	SystemdEventObserver

	// InitConfigFiles initializes the config files (elastic, filebeat, etc.)
	InitConfigFiles()

	// Start starts the services that run on all controller nodes
	// in the cluster.
	Start() error

	// Stop stops the services that run on all controller nodes
	// in the cluster.
	Stop()

	// AreNodeServicesRunning returns if all the controller node services
	// are running.
	AreNodeServicesRunning() bool

	// FileBeatConfig updates the location of the Elastic servers in filebeat config file
	FileBeatConfig(elasticLocation []string) error

	// KubeletConfig updates the kubelet with the new location of k8sApi server
	KubeletConfig(k8sAPIServerLocation string) error

	// ElasticDiscoveryConfig updates the location of the Elastic instances in discovery config file
	ElasticDiscoveryConfig(elasticLocation []string) error

	// ElasticMgmtConfig configures the node's mgmt-addr that is needed by
	// elastic instance to bind and publish to its peers
	ElasticMgmtConfig() error
}

// MasterService is the interface for starting/stopping master services (that run on node winning leader election)
type MasterService interface {
	LeadershipObserver
	SystemdEventObserver

	// Start  runs leader election and starts the services if this node is the leader node that won election.
	// These services include kubernetes master components and API Server, API Gateway (for now), and any services that
	// have affinity to the Virtual IP.
	Start() error

	// Stop stops the services that run on the leader node in
	// the cluster.
	Stop()

	// AreLeaderServicesRunning returns if all the leader node services are
	// running.
	AreLeaderServicesRunning() bool

	// UpdateNtpService updates ntp service
	UpdateNtpService(service NtpService)
}

// SystemdUnitStatus indicates status of the Unit
type SystemdUnitStatus string

const (
	// SystemdUnitActive if the unit is healthy
	SystemdUnitActive SystemdUnitStatus = "Active"
	// SystemdUnitDead if the unit is dead
	SystemdUnitDead = "Dead"
	// SystemdUnitInTransit when the unit is being started or stopped
	SystemdUnitInTransit = "InTransit"
)

// SystemdEvent is Notified by LeaderService when any leadership change event happens
type SystemdEvent struct {
	// Status indicates current status
	Status SystemdUnitStatus
	// Name of the systemd unit
	Name string
}

// SystemdEventObserver is implemented by services which are interested in knowing
// about the Systemd status and events of services
type SystemdEventObserver interface {
	// OnNotifySystemdEvent allows an event to be "published" to interface implementations.
	OnNotifySystemdEvent(SystemdEvent) error
}

// SystemdService provides interface to start/stop and notify on systemd units
type SystemdService interface {
	Register(SystemdEventObserver)
	UnRegister(SystemdEventObserver)

	// Start service
	Start() error
	// Stop service. Does NOT stop the systemd units associated with this service
	Stop()

	// StartUnit starts a systemd unit and watch for its health
	StartUnit(name string) error
	// StopUnit stops a systemd unit service and stops watching for it
	StopUnit(name string) error
	// Units return the systemd units that are watched by this service
	Units() []string
}

// K8sPodEventType indicates a new event in k8s (pod creation or deletion).
type K8sPodEventType string

const (
	// K8sPodAdded is when a pod is started as part of a k8s
	K8sPodAdded K8sPodEventType = "K8sPodAdded"
	// K8sPodDeleted is when a pod is deleted from k8s
	K8sPodDeleted K8sPodEventType = "K8sPodDeleted"
	// K8sPodModified is when a pod is modified
	K8sPodModified K8sPodEventType = "K8sPodModified"
)

// K8sPodEvent is generated by K8s service on pod additions/deletions.
type K8sPodEvent struct {
	// Type contains the type of k8s pod event.
	Type K8sPodEventType

	// Pod contains information about the pod.
	Pod *v1.Pod
}

// K8sPodEventObserver is implemented by services which are interested in
// knowing about Pod creations/deletions.
type K8sPodEventObserver interface {
	// OnNotifyK8sPodEvent allows an event to be "published" to interface implementations.
	OnNotifyK8sPodEvent(K8sPodEvent) error
}

// K8sPodEventObserverFunc is a function that implements K8sPodEventObserver interface
type K8sPodEventObserverFunc func(K8sPodEvent) error

// OnNotifyK8sPodEvent implements method for K8sPodEventObserver interface
func (f K8sPodEventObserverFunc) OnNotifyK8sPodEvent(e K8sPodEvent) error {
	return f(e)
}

// K8sService is the interface for managing nodes/controllers deployed through K8s.
type K8sService interface {
	Register(K8sPodEventObserver)
	UnRegister(K8sPodEventObserver)

	// Start the k8s service.
	Start(client, strClient k8sclient.Interface, isLeader bool)

	// Delete a node from k8s cluster
	DeleteNode(name string) error

	// UpgradeServices upgrades all the deployments and daemonsets in the specified order
	UpgradeServices(order []string) error

	// GetClient returns the k8s client interface for config get/set
	GetClient() k8sclient.Interface

	// GetWatchClient returns the k8s client interface for watchers
	GetWatchClient() k8sclient.Interface

	// Stop the k8s service.
	Stop()
}

// NtpService is the interface for managing config of local NTP daemon (deployed through K8s)
// This determines which NTP server should the NTPD listen to for updates
type NtpService interface {
	// LeadershipObserver is used on Quorum nodes
	LeadershipObserver

	// Start the ntp service for maintaining config
	Start()

	// Stop the ntp service
	Stop()

	// NtpConfigFile writes the NTP config file with the specified servers.
	// No need to Start the service for just calling this function
	NtpConfigFile(servers []string)

	// UpdateNtpConfig updates NTP config
	UpdateNtpConfig(leader string)

	// UpdateServerList checks if external NTP servers list got changed and updates NTP object and config file
	UpdateServerList(servers []string)
}

// ServiceInstanceObserver is implemented by services which are interested in
// knowing about Service instance creations/deletions.
type ServiceInstanceObserver interface {
	// OnNotifyServiceInstance allows an event to be "published" to interface implementations.
	OnNotifyServiceInstance(types.ServiceInstanceEvent) error
}

// ResolverService is the interface for service resolution.
type ResolverService interface {
	Register(ServiceInstanceObserver)
	UnRegister(ServiceInstanceObserver)

	// Start the resolver service.
	Start()

	// Stop the resolver service.
	Stop()

	// Get a service by name.
	Get(name string) *types.Service

	// Get a service instance by service and instance name.
	GetInstance(name, instance string) *types.ServiceInstance

	// List all services.
	List() *types.ServiceList

	// List all service instances.
	ListInstances() *types.ServiceInstanceList

	// AddSvcInstance to add a service instance explicitly without using k8s service
	AddServiceInstance(si *types.ServiceInstance) error
	// DelSvcInstance to delete a service instance explicitly without using k8s service
	DeleteServiceInstance(si *types.ServiceInstance) error
}

// NodeEventHandler handles watch events for Node object
type NodeEventHandler func(et kvstore.WatchEventType, node *cluster.Node)

// ClusterEventHandler handles watch events for Cluster object
type ClusterEventHandler func(et kvstore.WatchEventType, cluster *cluster.Cluster)

// SmartNICEventHandler handles watch events for SmartNIC object
type SmartNICEventHandler func(et kvstore.WatchEventType, nic *cluster.DistributedServiceCard)

// HostEventHandler handles watch events for Host object
type HostEventHandler func(et kvstore.WatchEventType, nic *cluster.Host)

// CfgWatcherService watches for changes to config from API Server
type CfgWatcherService interface {
	// Start the service
	Start()
	// Stop the service
	Stop()

	// SetNodeEventHandler sets the handler to handle events related to Node object
	SetNodeEventHandler(NodeEventHandler)

	// SetClusterEventHandler sets the handler to handle events related to Cluster object
	SetClusterEventHandler(ClusterEventHandler)

	// SetNtpEventHandler sets the handler to handle events related to NTP Server List
	SetNtpEventHandler(ClusterEventHandler)

	// SetSmartNICEventHandler sets the handler to handle events related to SmartNIC object
	SetSmartNICEventHandler(SmartNICEventHandler)

	// SetHostventHandler sets the handler to handle events related to Host object
	SetHostEventHandler(HostEventHandler)

	// SetNodeService sets the node service to update 3rd party services (e.g. elastic) on node updates
	SetNodeService(NodeService)

	// SetClusterQuorumNodes sets the initial list of quorum nodes to be updated on 3rd party
	// services (e.g. elastic discovery file)
	SetClusterQuorumNodes([]string)

	// APIClient returns a valid interface once the APIServer is good and
	// accepting requests
	APIClient() cluster.ClusterV1Interface

	// GetCluster fetches the cluster object from APIServer with retries and timeouts
	GetCluster() (*cluster.Cluster, error)
}

// SmartNICService is responsible for lifecycle management of SmartNIC object
type SmartNICService interface {

	// MonitorHealth is responsible for monitoring health of SmartNIC objects
	MonitorHealth()

	// InitiateNICRegistration initiates post to naples config to switch to
	// managed mode
	InitiateNICRegistration(nic *cluster.DistributedServiceCard)

	// UpdateNicInRetryDB adds or updates a smartNIC in RetryDB that has NICs in
	// active retry for initiating NIC registration to switch to managed mode
	UpdateNicInRetryDB(nic *cluster.DistributedServiceCard)

	// DeleteNicFromRetryDB removes a smartNIC from RetryDB
	DeleteNicFromRetryDB(nic *cluster.DistributedServiceCard)
}

// ServiceTracker tracks location of a service
type ServiceTracker interface {
	// LeadershipObserver is used on Quorum nodes : Resolver is informed of the new
	//	location of Services which run on Leader.
	LeadershipObserver

	// OnNotifyResolver is used to listen to resolver updates and inform location of interested services
	OnNotifyResolver(e types.ServiceInstanceEvent) error

	// Run the service tracker. Supposed to be called with Run(resolver.Interface, NodeService)
	//	but declared this way because of complex circular dependencies
	Run(interface{}, NodeService)

	// Stop the service
	Stop()
}

// VeniceRolloutHandler is called by VeniceRolloutClient in response to watch events from RolloutController
type VeniceRolloutHandler interface {
	// CreateVeniceRollout is handler for Create events
	CreateVeniceRollout(*rolloutproto.VeniceRollout)
	// UpdateVeniceRollout is for update
	UpdateVeniceRollout(*rolloutproto.VeniceRollout)
	// DeleteVeniceRollout for delete
	DeleteVeniceRollout(*rolloutproto.VeniceRollout)

	// RegisterVeniceStatusWriter to register the writer for venice rollout status
	RegisterVeniceStatusWriter(VeniceRolloutStatusWriter)
	// UnregisterVeniceStatusWriter to unregister the writer for venice rollout status
	UnregisterVeniceStatusWriter(VeniceRolloutStatusWriter)
}

// VeniceRolloutStatusWriter is implemented by VeniceRolloutClient to write the status to RolloutController (for requests received from RolloutController)
type VeniceRolloutStatusWriter interface {
	WriteStatus(context.Context, *rolloutproto.VeniceRolloutStatusUpdate)
}

// VeniceRolloutClient is client to Rollout Controller for Venice objects
type VeniceRolloutClient interface {
	VeniceRolloutStatusWriter
	Start()
	Stop()
}

// ServiceRolloutHandler is called by ServiceRolloutClient in response to watch events from RolloutController
type ServiceRolloutHandler interface {
	// CreateServiceRollout is handler for Create events
	CreateServiceRollout(*rolloutproto.ServiceRollout)
	// UpdateServiceRollout is for update
	UpdateServiceRollout(*rolloutproto.ServiceRollout)
	// DeleteServiceRollout for delete
	DeleteServiceRollout(*rolloutproto.ServiceRollout)

	// RegisterServiceStatusWriter to register the writer for service rollout status
	RegisterServiceStatusWriter(ServiceRolloutStatusWriter)
	// UnregisterServiceStatusWriter to unregister the writer for service rollout status
	UnregisterServiceStatusWriter(ServiceRolloutStatusWriter)
}

// ServiceRolloutStatusWriter is implemented by ServiceRolloutClient to write the status to RolloutController (for requests received from RolloutController)
type ServiceRolloutStatusWriter interface {
	WriteServiceStatus(context.Context, *rolloutproto.ServiceRolloutStatusUpdate)
}

// ServiceRolloutClient is client to Rollout Controller for Service objects
type ServiceRolloutClient interface {
	ServiceRolloutStatusWriter
	Start()
	Stop()
}

// RolloutMgr is requested by VeniceRolloutClient and ServiceRolloutClient to do upgrades of either the Venice node or the Services
type RolloutMgr interface {
	VeniceRolloutHandler
	ServiceRolloutHandler
	// Start the RolloutMgr - and take any actions based on commands from RolloutController (thru VeniceRolloutClient)
	Start()
	// Stop the Rollmgr
	Stop()
}

// TokenAuthService issues authorization tokens that can be used to perform privileged operations on cluster components.
// For example, it can issue tokens to authenticate with NAPLES agents when Venice is not reachable
type TokenAuthService interface {
	GenerateNodeToken(audience []string, validityStart, validityEnd *api.Timestamp) (string, error)
}

// MetricsService is responsible for reporting node-level and cluster-level metrics
type MetricsService interface {
	// Start the metrics service
	Start() error
	// Stop the metrics service
	Stop()
	// IsRunning returns true if the serivce is running
	IsRunning() bool
	// UpdateCounters updates counters that are part of cluster metrics
	UpdateCounters(m map[string]int64)
}

// HealthClient is responsible for relaying the status of the nodes to the leader
type HealthClient interface {
	Stop()
}

// ClusterHealthMonitor responsible for cluster health update functionalities
type ClusterHealthMonitor interface {
	// Start starts the monitor and updates health
	Start()

	// Stop stops the cluster health monitor
	Stop()
}

// ClusterBootstrapService enables cluster formation using a REST API.
// API is unauthenticated and is only available when node is not part of a cluster.
type ClusterBootstrapService interface {
	// Starts starts the bootstrap API REST endpoint
	Start() error

	// Stop stops the bootstrap API REST endpoint
	Stop()
}
