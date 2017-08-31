package services

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/utils/log"
	k8sclient "k8s.io/client-go/kubernetes"
	k8srest "k8s.io/client-go/rest"

	"github.com/pensando/sw/cmd/env"
	configs "github.com/pensando/sw/cmd/systemd-configs"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/globals"
)

// Services that run on node that wins the Leader election.
// TODO: Spread these out when it makes sense to do so.
var (
	masterServices = []string{
		"pen-kube-apiserver",
		"pen-kube-scheduler",
		"pen-kube-controller-manager",
		"pen-apiserver",
		"pen-elasticsearch",
		"pen-npm",
		"pen-vchub",
	}
)

type masterService struct {
	sync.Mutex
	sysSvc      types.SystemdService
	leaderSvc   types.LeaderService
	k8sSvc      types.K8sService
	resolverSvc types.ResolverService
	isLeader    bool
	enabled     bool
	virtualIP   string // virtualIP for services which can listed on only one VIP
	configs     configs.Interface
}

// MasterOption fills the optional params
type MasterOption func(service *masterService)

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
func NewMasterService(virtualIP string, options ...MasterOption) types.MasterService {
	m := masterService{
		leaderSvc: env.LeaderService,
		sysSvc:    env.SystemdService,
		virtualIP: virtualIP,
		configs:   configs.New(),
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
		config := &k8srest.Config{
			Host: fmt.Sprintf("%v:%v", virtualIP, globals.KubeAPIServerPort),
		}
		m.k8sSvc = NewK8sService(k8sclient.NewForConfigOrDie(config))
	}
	if m.resolverSvc == nil {
		m.resolverSvc = NewResolverService(m.k8sSvc)
	}
	m.leaderSvc.Register(&m)
	m.sysSvc.Register(&m)

	return &m
}

// Start starts the services that run on the leader node in the
// cluster after running election. These include kubernetes master components and API Gateway, services
// that have affinity to the Virtual IP.
// TODO: Spread out kubernetes master services also?
func (m *masterService) Start() error {
	m.sysSvc.Start()
	m.leaderSvc.Start()

	m.Lock()
	defer m.Unlock()
	if m.leaderSvc.IsLeader() {
		m.isLeader = true
	}
	m.enabled = true
	if m.isLeader {
		return m.startLeaderServices(m.virtualIP)
	}
	return nil
}

// caller holds the lock
func (m *masterService) startLeaderServices(virtualIP string) error {
	if err := m.configs.GenerateKubeMasterConfig(virtualIP); err != nil {
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
	m.resolverSvc.Start()
	m.k8sSvc.Start()
	return nil
}

// Stop stops the services that run on the leader node in the
// cluster.
func (m *masterService) Stop() {
	m.Lock()
	defer m.Unlock()
	m.enabled = false
	m.stopLeaderServices()
}

// caller holds the lock
func (m *masterService) stopLeaderServices() {
	m.resolverSvc.Stop()
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

func (m *masterService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	var err error
	switch e.Evt {
	case types.LeaderEventChange:
	case types.LeaderEventWon:
		m.Lock()
		defer m.Unlock()
		m.isLeader = true
		if m.enabled {
			m.startLeaderServices(m.virtualIP)
		}
	case types.LeaderEventLost:
		m.Lock()
		defer m.Unlock()
		m.isLeader = false
		if m.enabled {
			m.stopLeaderServices()
		}
	}
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
