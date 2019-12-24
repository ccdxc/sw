package services

import (
	"sync"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/persues/env"
	"github.com/pensando/sw/venice/persues/types"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// ServiceHandlers holds all the servies to be supported
type ServiceHandlers struct {
	sync.Mutex
	cfgWatcherSvc types.CfgWatcherService
}

// NewServiceHandlers returns a Service Handler
func NewServiceHandlers() *ServiceHandlers {
	m := ServiceHandlers{
		cfgWatcherSvc: env.CfgWatcherService,
	}
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.HandleSmartNICEvent)
	m.cfgWatcherSvc.SetNetworkInterfaceEventHandler(m.HandleNetworkInterfaceEvent)
	return &m
}

// HandleSmartNICEvent handles SmartNIC updates
func (m *ServiceHandlers) HandleSmartNICEvent(et kvstore.WatchEventType, evtNIC *cmd.DistributedServiceCard) {
	log.Infof("HandleSmartNICEvent called: NIC: %+v event type: %v", *evtNIC, et)
	return
}

// HandleNetworkInterfaceEvent handles SmartNIC updates
func (m *ServiceHandlers) HandleNetworkInterfaceEvent(et kvstore.WatchEventType, evtIntf *network.NetworkInterface) {
	log.Infof("HandleNetworkInterfaceEvent called: Intf: %+v event type: %v", *evtIntf, et)
	return
}
