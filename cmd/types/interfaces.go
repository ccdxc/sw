package types

// LeaderService is the interface for leader election service.
type LeaderService interface {
	Start() error
	Stop()
	Leader() string
}

// IPService is the interface for managing IP addresses on interfaces.
type IPService interface {
	HasIP(string) (bool, error)
	AddSecondaryIP(string) error
	DeleteIP(string) error
}

// SystemdService is the interface for starting/stopping node/leader services.
type SystemdService interface {
	// StartNodeServices starts the services that run on all controller nodes
	// in the cluster.
	StartNodeServices(string) error

	// StopNodeServices stops the services that run on all controller nodes
	// in the cluster.
	StopNodeServices()

	// AreNodeServicesRunning returns if all the controller node services
	// are running.
	AreNodeServicesRunning() bool

	// StartLeaderServices starts the services that run on the leader node
	// in the cluster. These include kubernetes master components and API
	// Gateway, services that have affinity to the Virtual IP.
	// TODO: Spread out kubernetes master services also?
	StartLeaderServices(string) error

	// StopLeaderServices stops the services that run on the leader node in
	// the cluster.
	StopLeaderServices()

	// AreLeaderServicesRunning returns if all the leader node services are
	// running.
	AreLeaderServicesRunning() bool
}
