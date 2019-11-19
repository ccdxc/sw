package services

import (
	"fmt"
	"sync"
	"time"

	gogotypes "github.com/gogo/protobuf/types"
	k8sclient "k8s.io/client-go/kubernetes"
	k8srest "k8s.io/client-go/rest"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	"github.com/pensando/sw/venice/cmd/ops"
	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	"github.com/pensando/sw/venice/cmd/types"
	k8stypes "github.com/pensando/sw/venice/cmd/types/protos"
	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
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
	ClusterStatusUpdateInterval = 30 * time.Second

	// k8sClientTimeout is the total timeout for Kubernetes ApiServer requests.
	// It covers the entire round-trip: TCP dial, TLS handshake, server response, reading response body.
	k8sClientTimeout = 10 * time.Second
)

type masterService struct {
	sync.Mutex
	nodeID               string
	sysSvc               types.SystemdService
	leaderSvc            types.LeaderService
	ntpSvc               types.NtpService
	k8sSvc               types.K8sService
	resolverSvc          types.ResolverService
	resolverSvcObserver  *resolverServiceObserver
	cfgWatcherSvc        types.CfgWatcherService
	esCuratorSvc         curator.Interface
	diagModuleSvc        module.Updater
	isLeader             bool
	enabled              bool
	configs              configs.Interface
	clusterHealthMonitor types.ClusterHealthMonitor

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
	return time.Until(nextTick)
}

//NewDefragJobTicker creates a new Ticker
func NewDefragJobTicker() DefragJobTicker {
	return DefragJobTicker{time.NewTimer(getNextTickDuration())}
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

// WithNtpSvcMasterOption to pass a specifc types.SystemdService implementation
func WithNtpSvcMasterOption(ntpSvc types.NtpService) MasterOption {
	return func(m *masterService) {
		m.ntpSvc = ntpSvc
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

// WithDiagModuleUpdaterSvcOption to pass a specific module.Updater implementation
func WithDiagModuleUpdaterSvcOption(diagModuleSvc module.Updater) MasterOption {
	return func(m *masterService) {
		m.diagModuleSvc = diagModuleSvc
	}
}

// WithClusterHealthMonitor to pass a custom cluster health monitor
func WithClusterHealthMonitor(clusterHealthMonitor types.ClusterHealthMonitor) MasterOption {
	return func(m *masterService) {
		m.clusterHealthMonitor = clusterHealthMonitor
	}
}

// UpdateNtpService updates master's NTP service
func (m *masterService) UpdateNtpService(ntpSvc types.NtpService) {
	if m.ntpSvc != nil {
		m.ntpSvc.Stop()
		m.leaderSvc.UnRegister(m.ntpSvc)
	}
	m.ntpSvc = ntpSvc
	m.ntpSvc.Start()
	m.ntpSvc.UpdateNtpConfig(m.leaderSvc.Leader())
	m.leaderSvc.Register(m.ntpSvc)
}

// resolver observer that observes service instances and creates event accordingly.
type resolverServiceObserver struct{}

func (r *resolverServiceObserver) OnNotifyServiceInstance(e k8stypes.ServiceInstanceEvent) error {
	log.Infof("received pod event: %v, instance {%v}", e.Type, e.GetInstance())
	if e.GetInstance() != nil && !utils.IsEmpty(e.GetInstance().GetNode()) {
		node := &cmd.Node{}
		node.Defaults("all")
		node.ObjectMeta.Name = e.GetInstance().GetNode()

		switch e.Type {
		case k8stypes.ServiceInstanceEvent_Added:
			log.Infof("triggering event {%v} on service {%v:%v}", eventtypes.SERVICE_STARTED, e.GetInstance().GetService(), e.GetInstance().GetNode())
			recorder.Event(eventtypes.SERVICE_STARTED,
				fmt.Sprintf("Service %s started on %s", e.GetInstance().GetService(), e.GetInstance().GetNode()), node)
		case k8stypes.ServiceInstanceEvent_Deleted:
			log.Infof("triggering event {%v} on service {%v:%v}", eventtypes.SERVICE_STOPPED, e.GetInstance().GetService(), e.GetInstance().GetNode())
			recorder.Event(eventtypes.SERVICE_STOPPED,
				fmt.Sprintf("Service %s stopped on %s", e.GetInstance().GetService(), e.GetInstance().GetNode()), node)
		}
	}
	return nil
}

// NewMasterService returns a Master Service
func NewMasterService(nodeID string, options ...MasterOption) types.MasterService {
	m := masterService{
		nodeID:              nodeID,
		leaderSvc:           env.LeaderService,
		sysSvc:              env.SystemdService,
		ntpSvc:              env.NtpService,
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

	if m.diagModuleSvc == nil {
		m.diagModuleSvc = module.GetUpdater(globals.Cmd, globals.APIServer, env.ResolverClient, env.Logger.WithContext("submodule", "diagnostics"))
	}

	m.leaderSvc.Register(&m)
	m.sysSvc.Register(&m)
	m.cfgWatcherSvc.SetNodeEventHandler(m.handleNodeEvent)
	m.cfgWatcherSvc.SetClusterEventHandler(m.handleClusterEvent)
	m.cfgWatcherSvc.SetNtpEventHandler(m.handleNtpEvent)
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.handleSmartNICEvent)
	m.cfgWatcherSvc.SetHostEventHandler(m.handleHostEvent)
	// ntp service listens for leadership changes
	if m.ntpSvc != nil {
		m.leaderSvc.Register(m.ntpSvc)
	}

	return &m
}

// Start starts the services that run on the leader node in the cluster after running election.
// These include kubernetes master components and API Gateway, services that have affinity to the Virtual IP.
// It also starts the resolver on quorum nodes and register a CMD instance pointing to the local auth API.
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

	// add local CMD service
	var localCMDSvcInstance = k8stypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "Service",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Cmd,
		},
		Service: globals.Cmd,
		Node:    globals.Localhost,
		URL:     fmt.Sprintf("%s:%s", globals.Localhost, globals.CMDGRPCAuthPort),
	}
	m.resolverSvc.AddServiceInstance(&localCMDSvcInstance)

	// add local citadel service
	var localCitadelInstance = k8stypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "Service",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.CitadelQuery,
		},
		Service: globals.CitadelQuery,
		Node:    globals.Localhost,
		URL:     fmt.Sprintf("%s:%s", globals.Localhost, globals.CitadelQueryRPCPort),
	}
	m.resolverSvc.AddServiceInstance(&localCitadelInstance)
	m.resolverSvc.Start()
	return err
}

// caller holds the lock
func (m *masterService) startLeaderServices() error {
	if err := m.configs.GenerateKubeMasterConfig(m.nodeID, globals.Localhost); err != nil {
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

	// setup diagnostics
	m.diagModuleSvc.Start()
	var observer types.K8sPodEventObserverFunc
	observer = func(e types.K8sPodEvent) error {
		if e.Pod.Status.HostIP == "" {
			// pod is not yet assigned to a node, skip creating module
			log.Debugf("pod: %v not yet assigned to node: %v", e.Pod.Name, e.Pod.Spec.NodeName)
			return nil
		}
		var diagModules []*diagapi.Module
		for _, container := range e.Pod.Spec.Containers {
			diagmod := &diagapi.Module{}
			diagmod.Defaults("all")
			diagmod.Name = fmt.Sprintf("%s-%s", e.Pod.Spec.NodeName, container.Name)
			diagmod.Status.Service = e.Pod.Name
			diagmod.Status.Module = container.Name
			diagmod.Status.Category = diagapi.ModuleStatus_Venice.String()
			diagmod.Status.Node = e.Pod.Spec.NodeName
			for _, port := range container.Ports {
				servicePort := diagapi.ServicePort{Name: port.Name, Port: port.ContainerPort}
				diagmod.Status.ServicePorts = append(diagmod.Status.ServicePorts, servicePort)
			}
			diagModules = append(diagModules, diagmod)
			log.Debugf("pod: %v, node: %v", e.Pod.Name, e.Pod.Spec.NodeName)
			log.Debugf("pod: %v, HostIP from status : %v", e.Pod.Name, e.Pod.Status.HostIP)
		}
		switch e.Type {
		case types.K8sPodAdded:
			for _, diagmod := range diagModules {
				m.diagModuleSvc.Enqueue(diagmod, module.Create)
				log.Infof("creating diagnostic module: %+v", *diagmod)
			}
		case types.K8sPodModified:
			for _, diagmod := range diagModules {
				m.diagModuleSvc.Enqueue(diagmod, module.Update)
				log.Infof("updating diagnostic module: %+v", *diagmod)
			}
		case types.K8sPodDeleted:
			for _, diagmod := range diagModules {
				m.diagModuleSvc.Enqueue(diagmod, module.Delete)
				log.Infof("deleting diagnostic module: %+v", *diagmod)
			}
		}
		return nil
	}
	m.k8sSvc.Register(observer)

	// observe pod events and record events accordingly
	m.resolverSvc.Register(m.resolverSvcObserver)
	// should only be running on leader node
	if env.ServiceRolloutClient != nil {
		env.ServiceRolloutClient.Start()
	}

	if env.NICService != nil {
		m.leaderInstanceRPCStopChannel = make(chan bool)
		go auth.RunLeaderInstanceServer(":"+env.Options.GRPCLeaderInstancePort, m.leaderInstanceRPCStopChannel)
	}

	// Start elastic curator service
	if m.esCuratorSvc != nil {
		m.esCuratorSvc.Start()
		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.LogIndexPrefix,
			RetentionPeriod: elastic.GetLogRetention(),
			ScanInterval:    elastic.LogIndexScanInterval,
		})

		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.EventsIndexPrefix,
			RetentionPeriod: elastic.GetEventRetention(),
			ScanInterval:    elastic.IndexScanInterval,
		})

		m.esCuratorSvc.Scan(&curator.Config{
			IndexName:       elastic.AuditLogsIndexPrefix,
			RetentionPeriod: elastic.GetAuditRetention(),
			ScanInterval:    elastic.IndexScanInterval,
		})
	}

	go performQuorumDefrag(true)

	if m.clusterHealthMonitor == nil {
		m.clusterHealthMonitor = NewClusterHealthMonitor(m.cfgWatcherSvc, m.k8sSvc, env.Logger.WithContext("submodule", "cluster_health_monitor"))
	}
	m.clusterHealthMonitor.Start()

	return nil
}

// Stop stops the services that run on the leader node in the
// cluster.
func (m *masterService) Stop() {
	m.Lock()
	defer m.Unlock()
	m.enabled = false
	m.diagModuleSvc.Stop()
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
	if m.clusterHealthMonitor != nil {
		m.clusterHealthMonitor.Stop()
	}
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
		clusterObj, err := m.cfgWatcherSvc.GetCluster()
		if err != nil {
			log.Errorf("Error getting Cluster object")
			return
		}
		update := false
		hostname := m.leaderSvc.ID()
		if clusterObj.Status.Leader != hostname {
			clusterObj.Status.Leader = hostname
			ts, err := gogotypes.TimestampProto(time.Now())
			if err != nil {
				log.Errorf("Cluster %#v status update with new leader errored %s while getting time", clusterObj, err)
			}
			clusterObj.Status.LastLeaderTransitionTime = &api.Timestamp{Timestamp: *ts}
			update = true
		}

		update = updateQuorumStatus(env.Quorum, clusterObj) || update
		if update {
			err := env.StateMgr.UpdateClusterStatus(clusterObj)
			if err != nil {
				log.Errorf("Cluster %#v update returned %#v", clusterObj, err)
				return
			}
			log.Infof("Quorum status updated in cluster object: %+v", clusterObj)
		}
	}

	ticker := time.NewTicker(ClusterStatusUpdateInterval)
	for {
		select {
		case <-ticker.C:
			updateStatus()
		case _, ok := <-m.updateCh:
			if ok {
				updateStatus()
			} else {
				close(m.closeCh)
				log.Infof("Stopping updateClusterStatus master service function")
				return
			}
		}
	}
}

func (m *masterService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	var err error

	// Cluster Health Monitor gets started as part of startLeaderServices()
	//	and starts using k8s immedietaly. Hence clean the references to old k8s
	//	here before calling startLeaderServices()
	//	so that cluster health monitor uses latest k8s client
	m.k8sSvc.Stop()
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
	config := &k8srest.Config{
		Host:    fmt.Sprintf("%v:%v", e.Leader, globals.KubeAPIServerPort),
		Timeout: k8sClientTimeout,
	}
	tlsClientConfig, err := credentials.GetKubernetesClientTLSConfig()
	if err == nil {
		config.TLSClientConfig = *tlsClientConfig
	} else {
		log.Infof("Failed to get access credentials for the Kubernetes cluster: %v", err)
		// do not return in case of failure, try to continue without tls config
		// we may authenticate in a different way or auth may not be required, for example in tests
	}
	// strConfig is same as config but with no timeout
	strConfig := &k8srest.Config{
		Host: fmt.Sprintf("%v:%v", e.Leader, globals.KubeAPIServerPort),
	}
	if tlsClientConfig != nil {
		strConfig.TLSClientConfig = *tlsClientConfig
	}

	m.k8sSvc.Start(k8sclient.NewForConfigOrDie(config), k8sclient.NewForConfigOrDie(strConfig), m.isLeader)
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

// isQuorumNode returns true if the node name is in the list of quorum nodes
func isQuorumNode(nodeName string) bool {
	for _, qn := range env.QuorumNodes {
		if qn == nodeName {
			return true
		}
	}
	return false
}

// handleNodeEvent handles Node update
func (m *masterService) handleNodeEvent(et kvstore.WatchEventType, node *cmd.Node) {
	// update local cache
	var err error
	switch et {
	case kvstore.Created:
		_, err = env.StateMgr.CreateNode(node)
	case kvstore.Updated:
		err = env.StateMgr.UpdateNode(node, false)
	case kvstore.Deleted:
		err = env.StateMgr.DeleteNode(node)
	}
	if err != nil {
		log.Errorf("Error updating local state, Op: %v, object: %+v", et, node)
	}

	if m.isLeader {
		switch et {
		case kvstore.Created:
			// Check if not already in cluster
			if node.Status.Phase != cmd.NodeStatus_JOINED.String() {
				// This can either be a new non-quorum node that is joining OR
				// it can be an existing quorum node that was disjoined and then rejoined.
				op := ops.NewNodeJoinOp(node)
				_, err := ops.Run(op)
				if err != nil {
					log.Errorf("Error %v while joining Node %v to cluster", err, node.Name)
				}
			}
		case kvstore.Updated:
		case kvstore.Deleted:
			cl, err := m.cfgWatcherSvc.GetCluster()
			if err != nil {
				log.Errorf("Error getting cluster object: %v", err)
				break
			}
			op := ops.NewNodeDisjoinOp(node, cl)
			_, err = ops.Run(op)
			if err != nil {
				log.Errorf("Error %v while disjoining Node %v from cluster", err, node.Name)
			}
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

// handleNtpEvent handles NTP Servers List update
func (m *masterService) handleNtpEvent(et kvstore.WatchEventType, cluster *cmd.Cluster) {
	switch et {
	case kvstore.Created:
		return
	case kvstore.Updated:
		if m.ntpSvc != nil {
			m.ntpSvc.UpdateServerList(cluster.Spec.NTPServers)
		}
		return
	case kvstore.Deleted:
		return
	}
}

// handleSmartNIC handles SmartNIC updates
func (m *masterService) handleSmartNICEvent(et kvstore.WatchEventType, evtNIC *cmd.DistributedServiceCard) {

	isLeader := env.LeaderService != nil && env.LeaderService.IsLeader()
	log.Infof("SmartNIC update: isLeader: %v, NIC: %+v event type: %v", isLeader, *evtNIC, et)

	if !isLeader {
		// Only update the local cache, overriding existing content.
		// Updates are atomic, so the kind-level lock implemented by
		// memdb is enough to protect against concurrent access.
		var err error
		switch et {
		case kvstore.Created:
			_, err = env.StateMgr.CreateSmartNIC(evtNIC, false)
		case kvstore.Updated:
			err = env.StateMgr.UpdateSmartNIC(evtNIC, false, false)
		case kvstore.Deleted:
			err = env.StateMgr.DeleteSmartNIC(evtNIC)
		}
		if err != nil {
			log.Errorf("Error updating local state, Op: %v, object: %+v", et, evtNIC)
		}
		return
	}

	// We are the leader CMD instance.
	// The high-level sequence of operations that need to happen is:
	// 1- Pick up updated Spec from ApiServer and merge it with status from local cache
	// 2- Update status based on new Spec
	// 3- Update local cache
	// 4- Write back to ApiServer

	switch et {
	case kvstore.Created:
		nicState, err := env.StateMgr.CreateSmartNIC(evtNIC, false)
		if err != nil {
			log.Errorf("Error creating smartnic {%+v}. Err: %v", evtNIC, err)
		}

		nicState.Lock()
		defer nicState.Unlock()

		nic := nicState.DistributedServiceCard
		err = env.StateMgr.UpdateHostPairingStatus(kvstore.Created, nic, nil)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing: %v", err)
		}
		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if nic.Status.AdmissionPhase == cmd.DistributedServiceCardStatus_UNKNOWN.String() || nic.Status.AdmissionPhase == "" {
			go env.NICService.InitiateNICRegistration(nic)
		}

	case kvstore.Updated:
		nicState, err := env.StateMgr.FindSmartNIC(evtNIC.Name)
		if err != nil {
			// this really should not happen... stop here if it does
			log.Errorf("Error processing update for Smartnic {%+v}: %v", evtNIC, err)
			return
		}
		nicState.Lock()
		defer nicState.Unlock()

		if evtNIC.Spec.MgmtMode == cmd.DistributedServiceCardSpec_NETWORK.String() &&
			evtNIC.Spec.Admit == false && evtNIC.Status.AdmissionPhase == cmd.DistributedServiceCardStatus_ADMITTED.String() {
			log.Infof("De-admitting NIC: %+v", evtNIC)
			// NIC has been de-admitted by user.
			// Set admission phase to PENDING and reset condtions, as the card is no longer part
			// of the cluster and will not send any update.
			evtNIC.Status.AdmissionPhase = cmd.DistributedServiceCardStatus_PENDING.String()
			evtNIC.Status.Conditions = nil

			// Override local state and Propagate changes back to API Server
			err = env.StateMgr.UpdateSmartNIC(evtNIC, true, false)
			if err != nil {
				log.Errorf("Error updating smartnic {%+v} in StateMgr. Err: %v", evtNIC, err)
			}

			// A de-admitted NIC is equivalent to a deleted NIC from Host pairing point of view
			err = env.StateMgr.UpdateHostPairingStatus(kvstore.Deleted, evtNIC, nil)
			if err != nil {
				log.Errorf("Error updating NIC-Host pairing, op: %v, NIC: %v, err: %v", kvstore.Deleted, evtNIC, err)
			}
			return
		}

		// If user has switched mode from network-managed to host-managed, we need to decommission
		// and trigger the mode change on NAPLES
		if nicState.Spec.MgmtMode == cmd.DistributedServiceCardSpec_NETWORK.String() && evtNIC.Spec.MgmtMode == cmd.DistributedServiceCardSpec_HOST.String() {
			log.Infof("Decommissioning NIC: %s", evtNIC.Name)
			// A decommissioned NIC is equivalent to a deleted NIC from Host pairing point of view
			err := env.StateMgr.UpdateHostPairingStatus(kvstore.Deleted, evtNIC, nil)
			if err != nil {
				log.Errorf("Error updating NIC-Host pairing, op: %v, NIC: %v, err: %v", kvstore.Deleted, evtNIC, err)
			}
			return
		}

		// Make a copy before proceeding
		var oldNIC cmd.DistributedServiceCard
		_, err = nicState.DistributedServiceCard.Clone(&oldNIC)
		if err != nil {
			log.Errorf("Error cloning DistributedServiceCard: %v", err)
			// try to continue
		}

		// Update current NIC state
		nic := evtNIC

		// When we receive an health update from the SmartNIC, we record the timestamp in the local cache.
		// If locally we have a more recent health condition then ApiServer, as determined by timestamps,
		// we should not override it.
		evtHealthCond := cmdutils.GetNICCondition(nic, cmd.DSCCondition_HEALTHY)
		localHealthCond := cmdutils.GetNICCondition(&oldNIC, cmd.DSCCondition_HEALTHY)
		if evtHealthCond != nil && localHealthCond != nil {
			evtLastTransitionTime, err1 := time.Parse(time.RFC3339, evtHealthCond.LastTransitionTime)
			localLastTransitionTime, err2 := time.Parse(time.RFC3339, localHealthCond.LastTransitionTime)
			if err1 == nil && err2 == nil {
				if localLastTransitionTime.After(evtLastTransitionTime) {
					cmdutils.SetNICCondition(nic, localHealthCond)
				}
			} else {
				log.Errorf("Error parsing LastTransitionTime, err1: %v, err2: %v", err1, err2)
			}
		}

		err = env.StateMgr.UpdateSmartNIC(nic, false, false)
		if err != nil {
			log.Errorf("Error updating smartnic {%+v} in StateMgr. Err: %v", evtNIC, err)
		}

		err = env.StateMgr.UpdateHostPairingStatus(kvstore.Updated, nic, &oldNIC)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing, op: %v, NIC: %v, err: %v", kvstore.Updated, nic, err)
		}

		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if nic.Spec.MgmtMode == cmd.DistributedServiceCardSpec_NETWORK.String() &&
			(nic.Status.AdmissionPhase == cmd.DistributedServiceCardStatus_UNKNOWN.String() || nic.Status.AdmissionPhase == "") {
			go env.NICService.InitiateNICRegistration(nic)
		}

	case kvstore.Deleted:
		env.NICService.DeleteNicFromRetryDB(evtNIC)
		err := env.StateMgr.DeleteSmartNIC(evtNIC)
		if err != nil {
			log.Errorf("Error deleting smartnic %s. Err: %v", evtNIC.Name, err)
		}

		env.StateMgr.UpdateHostPairingStatus(kvstore.Deleted, evtNIC, nil)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing, op: %v, NIC: %v, err: %v", kvstore.Deleted, evtNIC, err)
		}
	}
}

// handleHostEvent handles Host updates
func (m *masterService) handleHostEvent(et kvstore.WatchEventType, evtHost *cmd.Host) {

	isLeader := env.LeaderService != nil && env.LeaderService.IsLeader()
	log.Infof("Host update: isLeader: %v, evtHost: %+v event: %v", isLeader, *evtHost, et)

	if !isLeader {
		// Only update the local cache, overriding existing content.
		// Updates are atomic, so the kind-level lock implemented by
		// memdb is enough to protect against concurrent access.
		var err error
		switch et {
		case kvstore.Created:
			_, err = env.StateMgr.CreateHost(evtHost)
		case kvstore.Updated:
			err = env.StateMgr.UpdateHost(evtHost, false)
		case kvstore.Deleted:
			err = env.StateMgr.DeleteHost(evtHost)
		}
		if err != nil {
			log.Errorf("Error updating local state, Op: %v, object: %+v", et, evtHost)
		}
		return
	}

	// We are the leader CMD instance.
	// The high-level sequence of operations that need to happen is:
	// 1- Pick up updated Spec from ApiServer and merge it with status from local cache
	// 2- Update status based on new Spec
	// 3- Update local cache
	// 4- Write back to ApiServer

	switch et {
	case kvstore.Created:
		hostState, err := env.StateMgr.CreateHost(evtHost)
		if err != nil {
			log.Errorf("Error creating evtHost {%+v}. Err: %v", evtHost, err)
		}
		hostState.Lock()
		defer hostState.Unlock()

		host := hostState.Host
		err = env.StateMgr.UpdateNICPairingStatus(kvstore.Created, host, nil)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing: %v", err)
		}

	case kvstore.Updated:
		hostState, err := env.StateMgr.FindHost(evtHost.Name)
		if err != nil {
			// this really should not happen... stop here if it does
			log.Errorf("Error processing update for Host {%+v}: %v", evtHost, err)
			return
		}
		hostState.Lock()
		defer hostState.Unlock()

		// Make a copy before proceeding
		var oldHost cmd.Host
		_, err = hostState.Host.Clone(&oldHost)
		if err != nil {
			log.Errorf("Error cloning Host: %v", err)
			// try to continue
		}

		host := evtHost
		host.Status = oldHost.Status
		err = env.StateMgr.UpdateHost(evtHost, false)
		if err != nil {
			log.Errorf("Error updating evtHost {%+v}. Err: %v", evtHost, err)
		}

		err = env.StateMgr.UpdateNICPairingStatus(kvstore.Updated, host, &oldHost)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing, op: %v, Host: %v, err: %v", kvstore.Updated, host, err)
		}

	case kvstore.Deleted:
		err := env.StateMgr.DeleteHost(evtHost)
		if err != nil {
			log.Errorf("Error deleting evtHost %s. Err: %v", evtHost.Name, err)
		}

		env.StateMgr.UpdateNICPairingStatus(kvstore.Deleted, evtHost, nil)
		if err != nil {
			log.Errorf("Error updating NIC-Host pairing, op: %v, Host: %v, err: %v", kvstore.Deleted, evtHost, err)
		}
	}
}
