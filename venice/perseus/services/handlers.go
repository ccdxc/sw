package services

import (
	"sync"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/types"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type snic struct {
	name  string
	phase string
	ip    string
}

// ServiceHandlers holds all the servies to be supported
type ServiceHandlers struct {
	sync.Mutex
	cfgWatcherSvc types.CfgWatcherService
	snicMap       map[string]snic
}

// NewServiceHandlers returns a Service Handler
func NewServiceHandlers() *ServiceHandlers {
	m := ServiceHandlers{
		cfgWatcherSvc: env.CfgWatcherService,
	}
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.HandleSmartNICEvent)
	m.cfgWatcherSvc.SetNetworkInterfaceEventHandler(m.HandleNetworkInterfaceEvent)
	m.snicMap = make(map[string]snic)
	return &m
}

func configurePeer(nic snic) {
	if nic.phase != "Admitted" || nic.ip == "" {
		// wait for all information to be right
		// might have to handle case where nic was admitted previously
		// TODO
		return
	}
	// call grpc api to configure ms
}

func (m *ServiceHandlers) handleCreateUpdateSmartNICObject(evtNIC *cmd.DistributedServiceCard) {
	snic := m.snicMap[evtNIC.ObjectMeta.Name]
	snic.name = evtNIC.ObjectMeta.Name
	snic.phase = evtNIC.Status.AdmissionPhase
	m.snicMap[evtNIC.ObjectMeta.Name] = snic
	configurePeer(snic)
}

func (m *ServiceHandlers) handleDeleteSmartNICObject(evtNIC *cmd.DistributedServiceCard) {
}

func (m *ServiceHandlers) handleCreateUpdateNetIntfObject(evtIntf *network.NetworkInterface) {
	snic := m.snicMap[evtIntf.ObjectMeta.Name]
	snic.name = evtIntf.ObjectMeta.Name
	snic.ip = evtIntf.Spec.IPConfig.IPAddress
	m.snicMap[evtIntf.ObjectMeta.Name] = snic
	configurePeer(snic)
}

func (m *ServiceHandlers) handleDeleteNetIntfObject(evtIntf *network.NetworkInterface) {
}

func isEvtTypeCreatedupdated(et kvstore.WatchEventType) bool {
	if et == "Created" || et == "Updated" {
		return true
	}
	return false
}

func isEvtTypeDeleted(et kvstore.WatchEventType) bool {
	if et == "Deleted" {
		return true
	}
	return false
}

// HandleSmartNICEvent handles SmartNIC updates
func (m *ServiceHandlers) HandleSmartNICEvent(et kvstore.WatchEventType, evtNIC *cmd.DistributedServiceCard) {
	log.Infof("HandleSmartNICEvent called: NIC: %+v event type: %v\n", *evtNIC, et)
	if isEvtTypeCreatedupdated(et) {
		m.handleCreateUpdateSmartNICObject(evtNIC)
	} else if isEvtTypeDeleted(et) {
		m.handleDeleteSmartNICObject(evtNIC)
	} else {
		log.Fatalf("unexpected event received")
	}
	return
}

// HandleNetworkInterfaceEvent handles SmartNIC updates
func (m *ServiceHandlers) HandleNetworkInterfaceEvent(et kvstore.WatchEventType, evtIntf *network.NetworkInterface) {
	log.Infof("HandleNetworkInterfaceEvent called: Intf: %+v event type: %v\n", *evtIntf, et)
	if isEvtTypeCreatedupdated(et) {
		m.handleCreateUpdateNetIntfObject(evtIntf)
	} else if isEvtTypeDeleted(et) {
		m.handleDeleteNetIntfObject(evtIntf)
	} else {
		log.Fatalf("unexpected event received")
	}
	return
}
