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
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/ops"

	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

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
)

type masterService struct {
	sync.Mutex
	sysSvc        types.SystemdService
	leaderSvc     types.LeaderService
	k8sSvc        types.K8sService
	resolverSvc   types.ResolverService
	cfgWatcherSvc types.CfgWatcherService
	isLeader      bool
	enabled       bool
	configs       configs.Interface

	// this channel will be updated to indicate any change in the cluster or leader.
	// On leader/cluster event, the cluster status is updated to reflect the changes.
	// refer handleClusterEvent() and updateClusterStatus()
	updateCh chan bool

	closeCh chan bool
}

// MasterOption fills the optional params
type MasterOption func(service *masterService)

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

// NewMasterService returns a Master Service
func NewMasterService(options ...MasterOption) types.MasterService {
	m := masterService{
		leaderSvc:     env.LeaderService,
		sysSvc:        env.SystemdService,
		cfgWatcherSvc: env.CfgWatcherService,
		k8sSvc:        env.K8sService,
		resolverSvc:   env.ResolverService,
		configs:       configs.New(),
		updateCh:      make(chan bool),
		closeCh:       make(chan bool),
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
		m.k8sSvc = NewK8sService()
	}
	if m.resolverSvc == nil {
		m.resolverSvc = NewResolverService(m.k8sSvc)
	}
	m.leaderSvc.Register(&m)
	m.sysSvc.Register(&m)
	m.cfgWatcherSvc.SetNodeEventHandler(m.handleNodeEvent)
	m.cfgWatcherSvc.SetClusterEventHandler(m.handleClusterEvent)
	m.cfgWatcherSvc.SetSmartNICEventHandler(m.handleSmartNICEvent)

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
	if err := m.configs.GenerateKubeMasterConfig("localhost"); err != nil {
		return err
	}
	if err := m.configs.GenerateAPIServerConfig(); err != nil {
		return err
	}
	for ii := range masterServices {
		if err := m.sysSvc.StartUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			return err
		}
	}
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
	close(m.updateCh)
	<-m.closeCh
}

// caller holds the lock
func (m *masterService) stopLeaderServices() {
	m.k8sSvc.Stop()
	for ii := range masterServices {
		if err := m.sysSvc.StopUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Failed to stop leader service %v with error: %v", masterServices[ii], err)
		}
	}
	m.configs.RemoveKubeMasterConfig()
	m.configs.RemoveAPIServerConfig()
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
	case types.LeaderEventWon:
		m.Lock()
		defer m.Unlock()
		m.isLeader = true
		if m.enabled {
			m.updateCh <- true
			m.startLeaderServices()
		}
	case types.LeaderEventLost:
		m.Lock()
		defer m.Unlock()
		m.isLeader = false
		if m.enabled {
			m.stopLeaderServices()
		}
	}
	m.k8sSvc.Stop()
	config := &k8srest.Config{
		Host: fmt.Sprintf("%v:%v", e.Leader, globals.KubeAPIServerPort),
	}
	m.k8sSvc.Start(k8sclient.NewForConfigOrDie(config), m.isLeader)
	return err
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

// handleSmartNIC handles SmartNIC updates
func (m *masterService) handleSmartNICEvent(et kvstore.WatchEventType, nic *cmd.SmartNIC) {

	log.Debugf("SmartNIC update: nic: %+v event: %v", *nic, et)

	switch et {
	case kvstore.Created:

		err := env.StateMgr.CreateSmartNIC(nic)
		if err != nil {
			log.Fatalf("Error creating smartnic {%+v}. Err: %v", nic, err)
		}

		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if nic.Spec.Phase == cmd.SmartNICSpec_UNKNOWN.String() || nic.Spec.Phase == "" {
			go env.NICService.InitiateNICRegistration(nic)
		}

		return

	case kvstore.Updated:

		err := env.StateMgr.UpdateSmartNIC(nic)
		if err != nil {
			log.Errorf("Error updating smartnic {%+v}. Err: %v", nic, err)
		}

		// Initiate NIC registration only in cases where Phase is unknown or empty
		// For Naples initiated case, the phase will be set to REGISTERING initially
		if nic.Spec.Phase == cmd.SmartNICSpec_UNKNOWN.String() || nic.Spec.Phase == "" {
			go env.NICService.InitiateNICRegistration(nic)
		}

		return

	case kvstore.Deleted:
		env.NICService.DeleteNicFromRetryDB(nic)
		err := env.StateMgr.DeleteSmartNIC(nic.Tenant, nic.Name)
		if err != nil {
			log.Errorf("Error deleting smartnic %s|%s. Err: %v", nic.Tenant, nic.Name, err)
		}
		return
	}
}
