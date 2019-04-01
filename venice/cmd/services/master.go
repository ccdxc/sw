package services

import (
	"context"
	"fmt"
	"sync"
	"time"

	gogotypes "github.com/gogo/protobuf/types"
	k8sclient "k8s.io/client-go/kubernetes"
	k8srest "k8s.io/client-go/rest"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/ops"

	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	"github.com/pensando/sw/venice/cmd/types"
	k8stypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/elastic/curator"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	intervalPeriod     = 24 * time.Hour
	hourToTick     int = 02
	minuteToTick   int = 00
	secondToTick   int = 00
)

//DefragJobTicker defragmentation timer
type DefragJobTicker struct {
	t *time.Timer
}

// Services that run on node that wins the Leader election.
// TODO: Spread these out when it makes sense to do so.
var (
	masterServices = []string{
		globals.KubeAPIServer,
		globals.KubeScheduler,
		globals.KubeControllerManager,
	}
	// cluster status will be updated every 30 seconds or
	// when any leader event is observed or when cluster is created/updated
	clusterStatusUpdateTime = 30 * time.Second
	jt                      = NewDefragJobTicker()
)

type masterService struct {
	sync.Mutex
	sysSvc              types.SystemdService
	leaderSvc           types.LeaderService
	k8sSvc              types.K8sService
	resolverSvc         types.ResolverService
	resolverSvcObserver *resolverServiceObserver
	cfgWatcherSvc       types.CfgWatcherService
	esCuratorSvc        curator.Interface
	isLeader            bool
	enabled             bool
	configs             configs.Interface

	// this channel will be updated to indicate any change in the cluster or leader.
	// On leader/cluster event, the cluster status is updated to reflect the changes.
	// refer handleClusterEvent() and updateClusterStatus()
	updateCh chan bool

	closeCh chan bool

	leaderInstanceRPCStopChannel chan bool // Stop channel for Leader RPC server
}

// MasterOption fills the optional params
type MasterOption func(service *masterService)

//compute the NextTickDuration
func getNextTickDuration() time.Duration {
	log.Infof("getNextTickDuration")
	now := time.Now()
	nextTick := time.Date(now.Year(), now.Month(), now.Day(), hourToTick, minuteToTick, secondToTick, 0, time.Local)
	if nextTick.Before(now) {
		nextTick = nextTick.Add(intervalPeriod)
	}
	return nextTick.Sub(time.Now())
}

//NewDefragJobTicker creates a new Ticker
func NewDefragJobTicker() DefragJobTicker {
	return DefragJobTicker{time.NewTimer(getNextTickDuration())}
}

//update Ticker with new duration
func (jt DefragJobTicker) updateDefragJobTicker() {
	log.Infof("updating  DefragJobTicker")
	jt.t.Reset(getNextTickDuration())
}

// WithCfgWatcherMasterOption to pass a specifc types.CfgWatcherService implementation
func WithCfgWatcherMasterOption(cfgWatcher types.CfgWatcherService) MasterOption {
	return func(o *masterService) {
		o.cfgWatcherSvc = cfgWatcher
	}
}

// WithConfigsMasterOption to pass a specifc types.SystemdService implementation
func WithConfigsMasterOption(configs configs.Interface) MasterOption {
	return func(o *masterService) {
		o.configs = configs
	}
}

// WithLeaderSvcMasterOption to pass a specifc types.LeaderService implementation
func WithLeaderSvcMasterOption(leaderSvc types.LeaderService) MasterOption {
	return func(m *masterService) {
		m.leaderSvc = leaderSvc
	}
}

// WithSystemdSvcMasterOption to pass a specifc types.SystemdService implementation
func WithSystemdSvcMasterOption(sysSvc types.SystemdService) MasterOption {
	return func(m *masterService) {
		m.sysSvc = sysSvc
	}
}

// WithK8sSvcMasterOption to pass a specifc types.K8sService implementation
func WithK8sSvcMasterOption(k8sSvc types.K8sService) MasterOption {
	return func(m *masterService) {
		m.k8sSvc = k8sSvc
	}
}

// WithResolverSvcMasterOption to pass a specifc types.ResolverService implementation
func WithResolverSvcMasterOption(resolverSvc types.ResolverService) MasterOption {
	return func(m *masterService) {
		m.resolverSvc = resolverSvc
	}
}

// WithElasticCuratorSvcrOption to pass a specifc types.K8sService implementation
func WithElasticCuratorSvcrOption(curSvc curator.Interface) MasterOption {
	return func(m *masterService) {
		m.esCuratorSvc = curSvc
	}
}

// resolver observer that observes service instances and creates event accordingly.
type resolverServiceObserver struct{}

func (r *resolverServiceObserver) OnNotifyServiceInstance(e k8stypes.ServiceInstanceEvent) error {
	log.Infof("received pod event: %v, instance {%v}", e.Type, e.GetInstance())
	if e.GetInstance() != nil && !utils.IsEmpty(e.GetInstance().GetNode()) {
		switch e.Type {
		case k8stypes.ServiceInstanceEvent_Added:
			log.Infof("triggering event {%v} on service {%v:%v}", evtsapi.ServiceStarted, e.GetInstance().GetService(), e.GetInstance().GetNode())
			recorder.Event(evtsapi.ServiceStarted, evtsapi.SeverityLevel_INFO,
				fmt.Sprintf("Service %s started on %s", e.GetInstance().GetService(), e.GetInstance().GetNode()), nil)
		case k8stypes.ServiceInstanceEvent_Deleted:
			log.Infof("triggering event {%v} on service {%v:%v}", evtsapi.ServiceStopped, e.GetInstance().GetService(), e.GetInstance().GetNode())
			recorder.Event(evtsapi.ServiceStopped, evtsapi.SeverityLevel_WARNING,
				fmt.Sprintf("Service %s stopped on %s", e.GetInstance().GetService(), e.GetInstance().GetNode()), nil)
		}
	}
	return nil
}

// NewMasterService returns a Master Service
func NewMasterService(options ...MasterOption) types.MasterService {
	m := masterService{
		leaderSvc:           env.LeaderService,
		sysSvc:              env.SystemdService,
		cfgWatcherSvc:       env.CfgWatcherService,
		k8sSvc:              env.K8sService,
		resolverSvc:         env.ResolverService,
		resolverSvcObserver: &resolverServiceObserver{},
		configs:             configs.New(),
		updateCh:            make(chan bool),
		closeCh:             make(chan bool),
	}
	for _, o := range options {
		if o != nil {
			o(&m)
		}
	}
	if m.leaderSvc == nil {
		panic("Current implementation of Master Service needs a global Leaderservice")
	}
	if m.sysSvc == nil {
		panic("Current implementation of Master Service needs a global SystemdService")
	}
	if m.k8sSvc == nil {
		m.k8sSvc = NewK8sService(nil)
	}
	if m.resolverSvc == nil {
		m.resolverSvc = NewResolverService(m.k8sSvc)
	}

	// Initialize curator service
	if m.esCuratorSvc == nil {
		resolverClient, ok := env.ResolverClient.(resolver.Interface)
		if !ok {
			panic("Current implementation of curator service needs a resolver client")
		}
		var err error
		if m.esCuratorSvc, err = curator.NewCurator(nil, resolverClient,
			env.Logger.WithContext("submodule", "curator")); err != nil {
			log.Errorf("Error starting curator service, err: %v", err)
		}
	}

	m.leaderSvc.Register(&m)
	m.sysSvc.Register(&m)
	m.cfgWatcherSvc.SetNodeEventHandler(m.handleNodeEvent)
	m.cfgWatcherSvc.SetClusterEventHandler(m.handleClusterEvent)
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.handleSmartNICEvent)
	m.cfgWatcherSvc.SetHostEventHandler(m.handleHostEvent)

	return &m
}

// Start starts the services that run on the leader node in the
// cluster after running election. These include kubernetes master components and API Gateway, services
// that have affinity to the Virtual IP.
// TODO: Spread out kubernetes master services also?
func (m *masterService) Start() error {
	var err error
	m.sysSvc.Start()
	m.leaderSvc.Start()

	m.Lock()
	defer m.Unlock()
	if m.leaderSvc.IsLeader() {
		m.isLeader = true
	}
	m.enabled = true
	go m.updateClusterStatus()
	if m.isLeader {
		m.updateCh <- true
		err = m.startLeaderServices()
	}
	m.resolverSvc.Start()
	return err
}

// caller holds the lock
func (m *masterService) startLeaderServices() error {
	if err := m.configs.GenerateKubeMasterConfig(globals.Localhost); err != nil {
		log.Errorf("Error generating Kubernetes Master config: %v", err)
		return err
	}
	if err := m.configs.GenerateAPIServerConfig(); err != nil {
		log.Errorf("Error generating API Server config: %v", err)
		return err
	}
	for ii := range masterServices {
		if err := m.sysSvc.StartUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Error starting master service %v: %v", masterServices[ii], err)
			return err
		}
	}

	// observe pod events and record events accordingly
	m.resolverSvc.Register(m.resolverSvcObserver)
	// should only be running on leader node
	if env.ServiceRolloutClient != nil {
		env.ServiceRolloutClient.Start()
	}

	m.leaderInstanceRPCStopChannel = make(chan bool)
	go auth.RunLeaderInstanceServer(":"+env.Options.GRPCLeaderInstancePort, m.leaderInstanceRPCStopChannel)

	// Start elastic curator service
	if m.esCuratorSvc != nil {
		m.esCuratorSvc.Start()
		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.LogIndexPrefix,
			RetentionPeriod: elastic.LogIndexRetentionPeriod,
			ScanInterval:    elastic.LogIndexScanInterval,
		})

		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.EventsIndexPrefix,
			RetentionPeriod: elastic.EventsIndexRetentionPeriod,
			ScanInterval:    elastic.IndexScanInterval,
		})

		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.AuditLogsIndexPrefix,
			RetentionPeriod: elastic.AuditLogsIndexRetentionPeriod,
			ScanInterval:    elastic.IndexScanInterval,
		})
	}

	go performQuorumDefrag(true)

	return nil
}

// Stop stops the services that run on the leader node in the
// cluster.
func (m *masterService) Stop() {
	m.Lock()
	defer m.Unlock()
	m.enabled = false
	m.stopLeaderServices()
	m.k8sSvc.Stop()
	m.resolverSvc.Stop()

	// Stop elastic curator service
	m.esCuratorSvc.Stop()

	close(m.updateCh)
	<-m.closeCh
}

// caller holds the lock
func (m *masterService) stopLeaderServices() {
	go performQuorumDefrag(false)

	if m.leaderInstanceRPCStopChannel != nil {
		close(m.leaderInstanceRPCStopChannel)
		m.leaderInstanceRPCStopChannel = nil
	}

	if env.ServiceRolloutClient != nil {
		env.ServiceRolloutClient.Stop()
	}

	m.k8sSvc.Stop()
	for ii := range masterServices {
		if err := m.sysSvc.StopUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Failed to stop leader service %v with error: %v", masterServices[ii], err)
		}
	}
	m.configs.RemoveKubeMasterConfig()
	m.configs.RemoveAPIServerConfig()
	m.resolverSvc.UnRegister(m.resolverSvcObserver)
}

// AreLeaderServicesRunning returns if all the leader node services are
// running.
func (m *masterService) AreLeaderServicesRunning() bool {
	// TODO: Need systemd API for this
	return true
}
func (m *masterService) updateClusterStatus() {

	updateStatus := func() {
		if !m.isLeader {
			// Only leader should write the status
			return
		}
		ac := m.cfgWatcherSvc.APIClient()
		if ac == nil {
			log.Infof("APIClient not ready yet")
			return
		}
		cl := ac.Cluster()
		if cl == nil {
			log.Infof("APIClient retured nil Cluster client")
			return
		}
		var options api.ListWatchOptions
		hostname := m.leaderSvc.ID()
		nctx, cancel := context.WithTimeout(context.Background(), clusterStatusUpdateTime)
		defer cancel()
		clList, err := cl.List(nctx, &options)
		if err != nil {
			log.Infof("error %s getting Cluster from APIServer", err)
			return
		}

		if !m.isLeader {
			return
		}
		if len(clList) == 0 {
			log.Errorf("cluster object is nil from APIServer even though this node is leader")
			return
		}
		if clList[0].Status.Leader == hostname {
			return
		}

		clList[0].Status.Leader = hostname
		ts, err := gogotypes.TimestampProto(time.Now())
		if err != nil {
			log.Errorf("Cluster %#v status update with new leader errored %s while getting time", clList[0], err)
			return
		}
		clList[0].Status.LastLeaderTransitionTime = &api.Timestamp{Timestamp: *ts}
		_, err = cl.Update(nctx, clList[0])
		if err != nil {
			log.Errorf("Cluster %#v update on Leadership win returned %#v", clList[0], err)
		}
	}

	ticker := time.NewTicker(clusterStatusUpdateTime)
	for {
		select {
		case <-ticker.C:
			updateStatus()
		case _, ok := <-m.updateCh:
			if ok {
				updateStatus()
			} else {
				close(m.closeCh)
				return
			}
		}
	}
}

func (m *masterService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	var err error
	switch e.Evt {
	case types.LeaderEventChange:
		fallthrough
	case types.LeaderEventLost:
		m.Lock()
		defer m.Unlock()
		m.isLeader = false
		if m.enabled {
			m.stopLeaderServices()
		}

	case types.LeaderEventWon:
		m.Lock()
		defer m.Unlock()
		m.isLeader = true
		if m.enabled {
			m.updateCh <- true
			m.startLeaderServices()
		}
	}
	m.k8sSvc.Stop()
	config := &k8srest.Config{
		Host: fmt.Sprintf("%v:%v", e.Leader, globals.KubeAPIServerPort),
	}
	tlsClientConfig, err := credentials.GetKubernetesClientTLSConfig()
	if err == nil {
		config.TLSClientConfig = *tlsClientConfig
	} else {
		log.Infof("Failed to get access credentials for the Kubernetes cluster: %v", err)
		// do not return in case of failure, try to continue without tls config
		// we may authenticate in a different way or auth may not be required, for example in tests
	}
	m.k8sSvc.Start(k8sclient.NewForConfigOrDie(config), m.isLeader)
	return nil
}

func (m *masterService) OnNotifySystemdEvent(e types.SystemdEvent) error {
	found := false
	for _, n := range masterServices {
		if e.Name == n {
			found = true
			break
		}
	}
	if !found {
		return nil // service that i am not interested in
	}

	// TODO: Need leader election Restart APIs to handle failure
	return nil
}

// handleNodeEvent handles Node update
func (m *masterService) handleNodeEvent(et kvstore.WatchEventType, node *cmd.Node) {
	if !m.isLeader {
		return
	}
	switch et {
	case kvstore.Created:
		// Check if already in cluster.
		if node.Status.Phase == cmd.NodeStatus_JOINED.String() {
			return
		}
		op := ops.NewNodeJoinOp(node)
		_, err := ops.Run(op)
		if err != nil {
			log.Infof("Error %v while joining Node %v to cluster", err, node.Name)
		}
	case kvstore.Updated:
	case kvstore.Deleted:
		op := ops.NewNodeDisjoinOp(node)
		_, err := ops.Run(op)
		if err != nil {
			log.Infof("Error %v while disjoin Node %v from cluster", err, node.Name)
		}
	}
}

// handleClusterEvent handles Cluster update
func (m *masterService) handleClusterEvent(et kvstore.WatchEventType, cluster *cmd.Cluster) {
	switch et {
	case kvstore.Created:
		m.updateCh <- true
		return
	case kvstore.Updated:
		// TODO: process updates to ApprovedNIC list
		// Walk the ApprovedList to admit the NICs and remove them from
		// pendingNIC statusupdateClusterStatus
		if m.isLeader && cluster.Status.Leader != m.leaderSvc.ID() {
			m.updateCh <- true
		}
		return
	case kvstore.Deleted:
		return
	}
}

// do defragment on Quorum member
func performQuorumDefrag(start bool) {

	if start {

		for {
			<-jt.t.C
			var members, _ = env.Quorum.List()
			for _, member := range members {
				env.Quorum.Defrag(&member)
				time.Sleep(time.Minute * 5)
			}
			jt.updateDefragJobTicker()
		}
	} else {
		jt.t.Stop()
	}

}

// handleSmartNIC handles SmartNIC updates
func (m *masterService) handleSmartNICEvent(et kvstore.WatchEventType, nic *cmd.SmartNIC) {

	log.Infof("SmartNIC update: nic: %+v event: %v", *nic, et)

	var oldNIC *cmd.SmartNIC
	isLeader := env.LeaderService != nil && env.LeaderService.IsLeader()

	switch et {
	case kvstore.Created:

		err := env.StateMgr.CreateSmartNIC(nic)
		if err != nil {
			log.Errorf("Error creating smartnic {%+v}. Err: %v", nic, err)
		}

		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if isLeader && (nic.Status.AdmissionPhase == cmd.SmartNICStatus_UNKNOWN.String() || nic.Status.AdmissionPhase == "") {
			go env.NICService.InitiateNICRegistration(nic)
		}

	case kvstore.Updated:

		if isLeader && (nic.Spec.Admit == false && nic.Status.AdmissionPhase == cmd.SmartNICStatus_ADMITTED.String()) {
			log.Infof("De-admitting NIC: %+v", nic)
			// NIC has been de-admitted by user. Reset status, as it is no longer part of the cluster, and
			// change phase to pending. NIC will try to register again and if user sets admit=true it will
			// be re-admitted
			nic.Status = cmd.SmartNICStatus{
				AdmissionPhase: cmd.SmartNICStatus_PENDING.String(),
			}
			// update cache so that agent gets notified right away,
			// even if we fail to propagate the update back to ApiServer
			err := env.StateMgr.UpdateSmartNIC(nic)
			if err != nil {
				log.Errorf("Error updating smartnic {%+v} in StateMgr. Err: %v", nic, err)
			}

			// A de-admitted NIC is equivalent to a deleted NIC from Host pairing point of view
			nicUpdates, hostUpdates, err := env.StateMgr.UpdateHostPairingStatus(kvstore.Deleted, nic, nil)
			if err != nil {
				log.Errorf("Error updating NIC - Host pairing: %v", err)
			}
			// Add the NIC to nicUpdates to make sure that the Status update propagates back to ApiServer
			nicUpdates = append(nicUpdates, nic)
			m.sendNICHostUpdates(nicUpdates, hostUpdates)
			return
		}

		oldNICState, err := env.StateMgr.FindSmartNIC(nic.Tenant, nic.Name)
		if err != nil {
			log.Errorf("Error getting old SmartNIC object {%+v}. Err: %v", nic.ObjectMeta, err)
			// try to continue anyway
		} else {
			oldNIC = &(*oldNICState.SmartNIC) // make a copy before updating
		}
		err = env.StateMgr.UpdateSmartNIC(nic)
		if err != nil {
			log.Errorf("Error updating smartnic {%+v}. Err: %v", nic, err)
		}

		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if isLeader && (nic.Status.AdmissionPhase == cmd.SmartNICStatus_UNKNOWN.String() || nic.Status.AdmissionPhase == "") {
			go env.NICService.InitiateNICRegistration(nic)
		}

	case kvstore.Deleted:
		env.NICService.DeleteNicFromRetryDB(nic)
		err := env.StateMgr.DeleteSmartNIC(nic.Tenant, nic.Name)
		if err != nil {
			log.Errorf("Error deleting smartnic %s|%s. Err: %v", nic.Tenant, nic.Name, err)
		}
	}

	if isLeader {
		nicUpdates, hostUpdates, err := env.StateMgr.UpdateHostPairingStatus(et, nic, oldNIC)
		if err != nil {
			log.Errorf("Error updating NIC - Host pairing: %v", err)
		}

		m.sendNICHostUpdates(nicUpdates, hostUpdates)
	}
}

// handleHostEvent handles Host updates
func (m *masterService) handleHostEvent(et kvstore.WatchEventType, host *cmd.Host) {
	log.Infof("Host update: host: %+v event: %v", *host, et)

	var oldHost *cmd.Host
	isLeader := env.LeaderService != nil && env.LeaderService.IsLeader()

	switch et {
	case kvstore.Created:
		err := env.StateMgr.CreateHost(host)
		if err != nil {
			log.Errorf("Error creating host {%+v}. Err: %v", host, err)
		}

	case kvstore.Updated:
		oldHostState, err := env.StateMgr.FindHost(host.Tenant, host.Name)
		if err != nil {
			log.Errorf("Error getting old host {%+v}. Err: %v", host.ObjectMeta, err)
			// try to continue
		} else {
			oldHost = oldHostState.Host
		}

		err = env.StateMgr.UpdateHost(host)
		if err != nil {
			log.Errorf("Error updating host {%+v}. Err: %v", host, err)
		}

	case kvstore.Deleted:
		err := env.StateMgr.DeleteHost(host.Tenant, host.Name)
		if err != nil {
			log.Errorf("Error deleting host %s|%s. Err: %v", host.Tenant, host.Name, err)
		}
	}

	// if we are leader we also recompute the NIC/Host pairings, otherwise we will update
	// them as we receive them from ApiServer
	if isLeader {
		nicUpdates, hostUpdates, err := env.StateMgr.UpdateNICPairingStatus(et, host, oldHost)
		if err != nil {
			log.Errorf("Error updating NIC - Host pairing: %v", err)
		}

		// Go through all the NICs that have been affected by this event.
		// If we find a NIC that is no longer paired with any host,
		// see if it can be paired with one of the existing hosts
		for _, n := range nicUpdates {
			if n.Status.Host == "" {
				moreNICUpdates, moreHostUpdates, err := env.StateMgr.UpdateHostPairingStatus(kvstore.Created, n, nil)
				if err != nil {
					log.Errorf("Error updating NIC - Host pairing: %v", err)
				}
				nicUpdates = append(nicUpdates, moreNICUpdates...)
				hostUpdates = append(hostUpdates, moreHostUpdates...)
			}
		}

		m.sendNICHostUpdates(nicUpdates, hostUpdates)
	}
}

func (m *masterService) sendNICHostUpdates(nicUpdates []*cmd.SmartNIC, hostUpdates []*cmd.Host) {
	apiClient := m.cfgWatcherSvc.APIClient()

	updMap := make(map[string]bool) // map to dedup notifications for same object
	for _, upd := range nicUpdates {
		if _, dup := updMap[upd.Name]; !dup {
			// make sure to hold the lock while sending updates
			nicState, err := env.StateMgr.FindSmartNIC(upd.Tenant, upd.Name)
			if err != nil {
				log.Infof("Skipping update for deleted SmartNIC: %s", upd.Name)
				// object must have been deleted, continue
				continue
			}
			nicState.Lock()
			_, err = apiClient.SmartNIC().Update(context.Background(), upd)
			if err != nil {
				log.Errorf("Error updating smartnic {%+v} in ApiServer. Err: %v", upd, err)
			}
			log.Infof("Sent update for NIC {%+v} to ApiServer", upd)
			nicState.Unlock()
		}
	}

	updMap = make(map[string]bool) // reset the map
	for _, upd := range hostUpdates {
		if _, dup := updMap[upd.Name]; !dup {
			// make sure to hold the lock while sending updates
			hostState, err := env.StateMgr.FindHost(upd.Tenant, upd.Name)
			if err != nil {
				log.Infof("Skipping update for deleted Host: %s", upd.Name)
				// object must have been deleted, continue
				continue
			}
			hostState.Lock()
			_, err = apiClient.Host().Update(context.Background(), upd)
			if err != nil {
				log.Errorf("Error updating Host {%+v} in ApiServer. Err: %v", upd, err)
			}
			log.Infof("Sent update for Host {%+v} to ApiServer", upd)
			hostState.Unlock()
		}
	}
}
