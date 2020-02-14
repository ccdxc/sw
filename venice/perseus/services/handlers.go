package services

import (
	"context"
	"sync"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	pegasusClient "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/types"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type snic struct {
	name   string
	phase  string
	ip     string
	uuid   string
	pushed bool
}

// ServiceHandlers holds all the servies to be supported
type ServiceHandlers struct {
	sync.Mutex
	updated       bool
	pegasusURL    string
	cfgWatcherSvc types.CfgWatcherService
	pegasusClient pegasusClient.BGPSvcClient
	ifClient      pdstypes.IfSvcClient
	routeSvc      pegasusClient.CPRouteSvcClient
	apiclient     apiclient.Services
	snicMap       map[string]snic
}

// NewServiceHandlers returns a Service Handler
func NewServiceHandlers() *ServiceHandlers {
	m := ServiceHandlers{
		cfgWatcherSvc: env.CfgWatcherService,
	}
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.HandleSmartNICEvent)
	m.cfgWatcherSvc.SetNetworkInterfaceEventHandler(m.HandleNetworkInterfaceEvent)
	m.cfgWatcherSvc.SetRoutingConfigEventHandler(m.HandleRoutingConfigEvent)
	m.cfgWatcherSvc.SetNodeConfigEventHandler(m.HandleNodeConfigEvent)
	m.snicMap = make(map[string]snic)
	m.pegasusURL = globals.Localhost + ":" + globals.PegasusGRPCPort
	m.connectToPegasus()
	m.setupLBIf()
	return &m
}

// CfgAsn is the ASN for the RR config
var CfgAsn uint32

func (m *ServiceHandlers) configurePeers() {
	for _, nic := range m.snicMap {
		if nic.pushed == false {
			m.configurePeer(nic, false)
		}
	}
}

func (m *ServiceHandlers) configurePeer(nic snic, deleteOp bool) {
	if nic.phase != "admitted" || nic.ip == "" || nic.uuid == "" || CfgAsn == 0 {
		// wait for all information to be right
		// might have to handle case where nic was admitted previously
		// TODO
		return
	}
	uid, err := uuid.FromString(nic.uuid)
	if err != nil {
		log.Errorf("failed to parse UUID (%v)", err)
		return
	}
	// call grpc api to configure ms
	peerReq := pegasusClient.BGPPeerRequest{}
	peer := pegasusClient.BGPPeerSpec{
		Id:           uid.Bytes(),
		PeerAddr:     ip2PDSType(nic.ip),
		LocalAddr:    ip2PDSType(""),
		RemoteASN:    CfgAsn,
		SendComm:     true,
		SendExtComm:  true,
		ConnectRetry: 5,
		RRClient:     pegasusClient.BGPPeerRRClient_BGP_PEER_RR_CLIENT,
	}
	log.Infof("Add create peer [%+v]", peer)
	peerReq.Request = append(peerReq.Request, &peer)

	ctx := context.TODO()
	if deleteOp != true {
		presp, err := m.pegasusClient.BGPPeerCreate(ctx, &peerReq)
		if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("Peer create Request returned (%v)[%+v]", err, presp)
		}
		nic.pushed = true
	} else {
		presp, err := m.pegasusClient.BGPPeerDelete(ctx, &peerReq)
		if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("Peer delete Request returned (%v)[%+v]", err, presp)
		}
		nic.pushed = false
	}
	m.updated = true
	pReq = peerReq
	once.Do(m.pollStatus)
}

func (m *ServiceHandlers) handleCreateUpdateSmartNICObject(evtNIC *cmd.DistributedServiceCard) {
	snic := m.snicMap[evtNIC.ObjectMeta.Name]
	snic.name = evtNIC.ObjectMeta.Name
	snic.phase = evtNIC.Status.AdmissionPhase
	snic.uuid = evtNIC.UUID
	snic.pushed = false
	m.snicMap[evtNIC.ObjectMeta.Name] = snic
	m.configurePeer(snic, false)
}

func (m *ServiceHandlers) handleDeleteSmartNICObject(evtNIC *cmd.DistributedServiceCard) {
}

func (m *ServiceHandlers) handleCreateUpdateNetIntfObject(evtIntf *network.NetworkInterface) {
	snic := m.snicMap[evtIntf.Status.DSC]
	snic.name = evtIntf.Status.DSC
	snic.pushed = false
	if evtIntf.Spec.IPConfig != nil {
		snic.ip = evtIntf.Spec.IPConfig.IPAddress
	}
	m.snicMap[evtIntf.Status.DSC] = snic
	m.configurePeer(snic, false)
}

func (m *ServiceHandlers) handleDeleteNetIntfObject(evtIntf *network.NetworkInterface) {
	snic := m.snicMap[evtIntf.Status.DSC]
	if snic.pushed == true {
		m.configurePeer(snic, true)
	}
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
