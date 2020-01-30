package types

import (
	"context"

	"github.com/pensando/sw/venice/utils/emstore"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

// ControllerAPI implements north bound APIs for DSCAgent
type ControllerAPI interface {
	HandleVeniceCoordinates(obj DistributedServiceCardStatus) error
	Start(ctx context.Context) error
	Stop() error
}

// InfraAPI implements all infra APIs that are common across pipelines
type InfraAPI interface {
	// List returns a list of objects by kind
	List(kind string) ([][]byte, error)

	// Read returns a single object of a given kind
	Read(kind, key string) ([]byte, error)

	// Store stores a single object of a given kind
	Store(kind, key string, data []byte) error

	// Delete deletes a single object
	Delete(kind, key string) error

	// AllocateID allocates id based off an offset
	AllocateID(kind emstore.ResourceType, offset int) uint64

	// GetDscName returns the DSC Name.
	GetDscName() string

	// StoreConfig stores NetAgent config
	StoreConfig(cfg DistributedServiceCardStatus)

	// GetConfig returns NetAgent config
	GetConfig() (cfg DistributedServiceCardStatus)

	// NotifyVeniceConnection updates venice connection status from netagent
	NotifyVeniceConnection()

	Close() error
}

// PipelineAPI defines methods that all pipelines must implement.
// If the method is not applicable for a pipeline it must return nil and ErrNotImplemented
type PipelineAPI interface {
	// PipelineInit sets up the pipeline and creates default objects needed
	PipelineInit() error

	// HandleVrf handles CRUDs for VRF object
	HandleVrf(oper Operation, obj netproto.Vrf) ([]netproto.Vrf, error)

	// HandleL2Segment handles CRUDs for L2Segment object
	HandleNetwork(oper Operation, obj netproto.Network) ([]netproto.Network, error)

	// HandleEndpoint handles CRUDs for Endpoint object
	HandleEndpoint(oper Operation, obj netproto.Endpoint) ([]netproto.Endpoint, error)

	// HandleInterface handles CRUDs for Interface object
	HandleInterface(oper Operation, obj netproto.Interface) ([]netproto.Interface, error)

	// HandleTunnel handles CRUDs for Tunnel object
	HandleTunnel(oper Operation, obj netproto.Tunnel) ([]netproto.Tunnel, error)

	// HandleApp handls CRUDs for App object
	HandleApp(oper Operation, obj netproto.App) ([]netproto.App, error)

	// HandleNetworkSecurityPolicy handles CRUDs for NetworkSecurityPolicy object
	HandleNetworkSecurityPolicy(oper Operation, obj netproto.NetworkSecurityPolicy) ([]netproto.NetworkSecurityPolicy, error)

	// ValidateSecurityProfile validates SecurityProfile object and returns the attachment vrf
	ValidateSecurityProfile(profile netproto.SecurityProfile) (vrf netproto.Vrf, err error)

	// HandleSecurityProfile handles CRUDs for SecurityProfile object
	HandleSecurityProfile(oper Operation, obj netproto.SecurityProfile) ([]netproto.SecurityProfile, error)

	// HandleNetworkSecurityPolicy handles CRUDs for Mirror object
	HandleMirrorSession(oper Operation, obj netproto.MirrorSession) ([]netproto.MirrorSession, error)

	// HandleIPAMPolicy handles CRUDs for IPAM policy object
	HandleIPAMPolicy(oper Operation, obj netproto.IPAMPolicy) ([]netproto.IPAMPolicy, error)

	//HandleFlowExportPolicy handles CRUDs for NetworkSecurityPolicy object
	HandleFlowExportPolicy(oper Operation, obj netproto.FlowExportPolicy) ([]netproto.FlowExportPolicy, error)

	//HandleProfile handles CRUDs for Profile object
	HandleProfile(oper Operation, obj netproto.Profile) ([]netproto.Profile, error)

	// ReplayConfigs replays stored configs to avoid dataplane downtime on a reboot
	ReplayConfigs() error

	// PurgeConfigs deletes all stored config. This is called whenever DSC is decommissioned
	PurgeConfigs() error

	// GetWatchOptions returns aggregate watch options on per kind. Currently this is used to get only local EPs
	GetWatchOptions(ctx context.Context, kind string) api.ListWatchOptions

	// HandleCPRoutingConfig handles creation of control plane route objects
	HandleCPRoutingConfig(obj DSCStaticRoute) error
}
