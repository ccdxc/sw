package types

import (
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// SmartNICEventHandler handles watch events for SmartNIC object
type SmartNICEventHandler func(et kvstore.WatchEventType, nic *cluster.DistributedServiceCard)

// NetworkInterfaceEventHandler handles watch events for Network Interface object
type NetworkInterfaceEventHandler func(et kvstore.WatchEventType, nwintf *network.NetworkInterface)

// CfgWatcherService watches for changes to config from API Server
type CfgWatcherService interface {
	// Start the service
	Start()
	// Stop the service
	Stop()

	// SetSmartNICEventHandler sets the handler to handle events related to SmartNIC object
	SetSmartNICEventHandler(SmartNICEventHandler)

	// SetNetworkInterfaceEventHandler sets the handler to handle events related to network interface object
	SetNetworkInterfaceEventHandler(NetworkInterfaceEventHandler)

	// APIClient returns a valid interface once the APIServer is good and
	// accepting requests
	APIClient() cluster.ClusterV1Interface
}
