package types

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

// NodeService is the interface for starting/stopping services running on all controller nodes
type NodeService interface {
	SystemdEventObserver
	// Start starts the services that run on all controller nodes
	// in the cluster.
	Start() error

	// Stop stops the services that run on all controller nodes
	// in the cluster.
	Stop()

	// AreNodeServicesRunning returns if all the controller node services
	// are running.
	AreNodeServicesRunning() bool
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

// K8sService is the interface for managing controllers deployed through K8s.
type K8sService interface {
	// Start the k8s service.
	Start()

	// Stop the k8s service.
	Stop()
}
