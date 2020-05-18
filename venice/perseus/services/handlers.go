package services

import (
	"context"
	"fmt"
	"net"
	goruntime "runtime"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/routing"
	pdstypes "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/metaswitch/gen/agent"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/types"
	"github.com/pensando/sw/venice/utils/diagnostics"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type snicT struct {
	name   string
	phase  string
	ip     string
	uuid   string
	pushed bool
}

// ServiceHandlers holds all the servies to be supported
type ServiceHandlers struct {
	sync.Mutex
	updated        bool
	pegasusURL     string
	cfgWatcherSvc  types.CfgWatcherService
	pegasusClient  pdstypes.BGPSvcClient
	ifClient       pdstypes.IfSvcClient
	routeSvc       pdstypes.CPRouteSvcClient
	pegasusMon     msTypes.EpochSvcClient
	apiclient      apiclient.Services
	snicMap        map[string]*snicT
	naplesTemplate *network.BGPNeighbor
	ctx            context.Context
	cancelFn       context.CancelFunc
	monitorPort    net.Listener
	stallMonitor   bool
	grpcSvc        *rpckit.RPCServer
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
	m.snicMap = make(map[string]*snicT)
	m.pegasusURL = globals.Localhost + ":" + globals.PegasusGRPCPort
	m.connectToPegasus()
	m.setupLBIf()

	grpcSvc, err := rpckit.NewRPCServer(globals.Perseus, ":"+globals.PerseusGRPCPort)
	if err != nil {
		log.Fatalf("could not start the the GRPC server (%s)", err)
	}
	m.grpcSvc = grpcSvc
	routing.RegisterRoutingV1Server(grpcSvc.GrpcServer, &m)
	m.ctx, m.cancelFn = context.WithCancel(context.Background())
	grpcSvc.Start()
	m.registerDebugHandlers()

	var doneWg sync.WaitGroup
	doneWg.Add(1)
	go func() {
		doneWg.Done()
		<-m.ctx.Done()
		m.monitorPort.Close()
		log.Errorf("Closing monitor port due to failure")
	}()
	doneWg.Add(1)
	go m.monitor(&doneWg)
	doneWg.Wait()

	return &m
}

// CfgAsn is the ASN for the RR config
var CfgAsn uint32

const (
	maxInitialFail = 30
	maxRunningfail = 3
)

func (m *ServiceHandlers) monitor(wg *sync.WaitGroup) {
	wg.Done()
	initial := true
	failCount := 0
	var epoch uint32
	var err error

	log.Infof("waiting for %d seconds before enabling monitor", maxRunningfail*3)
	time.Sleep(maxRunningfail * time.Second * 3)
	log.Infof("starting monitor")
	m.monitorPort, err = net.Listen("tcp", "127.0.0.1:"+globals.PerseusMonitorPort)
	if err != nil {
		log.Fatalf("could not start listener on monitor por (%s)", err)
	}
	go func() {
		for {
			_, err := m.monitorPort.Accept()
			if err != nil {
				log.Errorf("failed to connect (%s)", err)
				if m.ctx.Err() != nil {
					log.Errorf("monitor port is closed, exiting (%s)", m.ctx.Err())
				}
			}

		}
	}()
	ticker := time.Tick(time.Second)
	for {
		select {
		case <-ticker:
			if m.stallMonitor {
				log.Infof("got monitor stall exiting")
				m.cancelFn()
				return
			}
			if initial && (failCount > maxInitialFail) || !initial && (failCount > maxRunningfail) {
				log.Errorf("failed to get epoch from pegasus initial[%v] Count[%d]", initial, failCount)
				m.cancelFn()
				return
			}
			epResp, err := m.pegasusMon.EpochGet(m.ctx, &msTypes.EpochGetRequest{})
			if err != nil {
				failCount++
				log.Errorf("Epoch get failed (%s)", err)
				continue
			}
			if epResp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				failCount++
				log.Errorf("Epoch get failed (%s)(%v)", err, epResp.ApiStatus)
				continue
			}
			failCount = 0
			if initial {
				epoch = epResp.Epoch
				initial = false
			} else {
				if epoch != epResp.Epoch {
					log.Errorf("Epoch from pegasus has changed [%v]->[%v]", epoch, epResp.Epoch)
					m.cancelFn()
					return
				}
			}
		}
	}
}

func (m *ServiceHandlers) registerDebugHandlers() {
	diagSvc := diagsvc.GetDiagnosticsService(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, log.GetDefaultInstance())
	if err := diagSvc.RegisterHandler("Debug", diagapi.DiagnosticsRequest_Stats.String(), diagsvc.NewExpVarHandler(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, log.GetDefaultInstance())); err != nil {
		log.ErrorLog("method", "registerDebugHandlers", "msg", "failed to register expvar handler", "err", err)
	}

	diagSvc.RegisterCustomAction("mon-stall", func(action string, params map[string]string) (interface{}, error) {
		m.stallMonitor = !m.stallMonitor
		return fmt.Sprintf("stall monitor [%v]", m.stallMonitor), nil
	})
	diagSvc.RegisterCustomAction("stack-trace", func(action string, params map[string]string) (interface{}, error) {
		buf := make([]byte, 1<<20)
		blen := goruntime.Stack(buf, true)
		return fmt.Sprintf("=== goroutine dump ====\n %s \n=== END ===", buf[:blen]), nil
	})
	diagnostics.RegisterService(m.grpcSvc.GrpcServer, diagSvc)
}

func (m *ServiceHandlers) configurePeers() {
	for _, nic := range m.snicMap {
		m.configurePeer(nic, false)
	}
}

func (m *ServiceHandlers) configurePeer(nic *snicT, deleteOp bool) {
	log.Infof("configurePeer: delete [%v] - snic %+v CfgAsn %d", deleteOp, nic, CfgAsn)
	if nic.phase != "admitted" || nic.ip == "" || nic.uuid == "" || CfgAsn == 0 {
		// wait for all information to be right
		// might have to handle case where nic was admitted previously
		// TODO
		log.Infof("ignoring configure peer")
		return
	}
	keepalive, holdtime := cache.getTimers()
	uid := cache.getUUID()
	ctx := context.TODO()
	if deleteOp != true {
		// call grpc api to configure ms
		peerReq := pdstypes.BGPPeerRequest{}
		peer := pdstypes.BGPPeerSpec{
			Id:           uid,
			PeerAddr:     ip2PDSType(nic.ip),
			LocalAddr:    ip2PDSType(""),
			RemoteASN:    CfgAsn,
			State:        pdstypes.AdminState_ADMIN_STATE_ENABLE,
			SendComm:     true,
			SendExtComm:  true,
			ConnectRetry: 5,
			KeepAlive:    keepalive,
			HoldTime:     holdtime,
			RRClient:     pdstypes.BGPPeerRRClient_BGP_PEER_RR_CLIENT,
		}
		if m.naplesTemplate != nil {
			peer.Password = []byte(m.naplesTemplate.Password)
			peer.TTL = m.naplesTemplate.MultiHop
			if m.naplesTemplate.Shutdown {
				peer.State = pdstypes.AdminState_ADMIN_STATE_DISABLE
			} else {
				peer.State = pdstypes.AdminState_ADMIN_STATE_ENABLE
			}
		}
		log.Infof("Add create peer [%+v]", peer)
		peerReq.Request = append(peerReq.Request, &peer)
		if nic.pushed {
			presp, err := m.pegasusClient.BGPPeerUpdate(ctx, &peerReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer update Request returned (%v)[%+v]", err, presp)
			}
		} else {
			presp, err := m.pegasusClient.BGPPeerCreate(ctx, &peerReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer create Request returned (%v)[%+v]", err, presp)
			}
			nic.pushed = true
		}

	} else {
		if nic.pushed {
			unkLocal := &pdstypes.IPAddress{
				Af: pdstypes.IPAF_IP_AF_INET,
			}
			peerDReq := pdstypes.BGPPeerDeleteRequest{}
			peer := pdstypes.BGPPeerKeyHandle{
				IdOrKey: &pdstypes.BGPPeerKeyHandle_Key{Key: &pdstypes.BGPPeerKey{PeerAddr: ip2PDSType(nic.ip), LocalAddr: unkLocal}},
			}
			log.Infof("Add Delete peer [%+v]", peer)
			peerDReq.Request = append(peerDReq.Request, &peer)
			presp, err := m.pegasusClient.BGPPeerDelete(ctx, &peerDReq)
			if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
				log.Errorf("Peer delete Request returned (%v)[%+v]", err, presp)
			} else {
				log.Infof("Peer delete request succeeded (%s)[%v]", err, presp.ApiStatus)
			}

			nic.pushed = false
		}
	}

	if !deleteOp {
		peerAfReq := pdstypes.BGPPeerAfRequest{
			Request: []*pdstypes.BGPPeerAfSpec{
				{
					Id:          uid,
					PeerAddr:    ip2PDSType(nic.ip),
					LocalAddr:   ip2PDSType(""),
					NexthopSelf: false,
					DefaultOrig: false,
					Afi:         pdstypes.BGPAfi_BGP_AFI_L2VPN,
					Safi:        pdstypes.BGPSafi_BGP_SAFI_EVPN,
				},
			},
		}
		presp, err := m.pegasusClient.BGPPeerAfCreate(ctx, &peerAfReq)
		if err != nil || presp.ApiStatus != pdstypes.ApiStatus_API_STATUS_OK {
			log.Errorf("Peer AF create Request returned (%v)[%+v]", err, presp)
		}
		nic.pushed = true
	}

	m.updated = true
}

func (m *ServiceHandlers) handleCreateUpdateSmartNICObject(evtNIC *cmd.DistributedServiceCard) {
	snic, ok := m.snicMap[evtNIC.ObjectMeta.Name]
	if !ok {
		snic = &snicT{}
	}
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
	snic, ok := m.snicMap[evtIntf.Status.DSC]
	if !ok {
		snic = &snicT{}
	}
	snic.name = evtIntf.Status.DSC
	oldip := snic.ip
	if evtIntf.Spec.IPConfig != nil {
		ip, _, err := net.ParseCIDR(evtIntf.Spec.IPConfig.IPAddress)
		if err != nil {
			log.Errorf("could not parse IP CIDR (%s)", err)
			return
		}
		if ip.IsUnspecified() {
			log.Infof("IP is unspecified [%v]", ip.String())
			snic.ip = ""
		} else {
			log.Infof("IP is [%v]", ip.String())
			snic.ip = ip.String()
		}
	} else {
		snic.ip = ""
	}
	if oldip != "" && oldip != snic.ip {
		ip := snic.ip
		snic.ip = oldip
		m.configurePeer(snic, true)
		snic.ip = ip
	}
	m.snicMap[evtIntf.Status.DSC] = snic
	m.configurePeer(snic, false)
}

func (m *ServiceHandlers) handleBGPConfigChange() {
	for _, nic := range m.snicMap {
		m.configurePeer(nic, false)
	}
}

func (m *ServiceHandlers) handleBGPConfigDelete() {
	for _, nic := range m.snicMap {
		m.configurePeer(nic, true)
	}
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

type monitorSvc struct{}

// AutoAddNeighbor creates Neighbor object
func (m *ServiceHandlers) AutoAddNeighbor(context.Context, *routing.Neighbor) (*routing.Neighbor, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoDeleteNeighbor deletes Neighbor object
func (m *ServiceHandlers) AutoDeleteNeighbor(context.Context, *routing.Neighbor) (*routing.Neighbor, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoGetNeighbor get Neighbor object
func (m *ServiceHandlers) AutoGetNeighbor(context.Context, *routing.Neighbor) (*routing.Neighbor, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoLabelNeighbor applies labels Neighbor object
func (m *ServiceHandlers) AutoLabelNeighbor(context.Context, *api.Label) (*routing.Neighbor, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoListNeighbor lists Neighbor objects
func (m *ServiceHandlers) AutoListNeighbor(context.Context, *api.ListWatchOptions) (*routing.NeighborList, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoUpdateNeighbor updates Neighbor object
func (m *ServiceHandlers) AutoUpdateNeighbor(context.Context, *routing.Neighbor) (*routing.Neighbor, error) {
	return nil, fmt.Errorf("not implemented")
}

// AutoWatchNeighbor watches Neighbor objects. Supports WebSockets or HTTP long poll
func (m *ServiceHandlers) AutoWatchNeighbor(*api.ListWatchOptions, routing.RoutingV1_AutoWatchNeighborServer) error {
	return fmt.Errorf("not implemented")
}

// AutoWatchSvcRoutingV1 is a service watcher
func (m *ServiceHandlers) AutoWatchSvcRoutingV1(*api.AggWatchOptions, routing.RoutingV1_AutoWatchSvcRoutingV1Server) error {
	return fmt.Errorf("not implemented")
}
