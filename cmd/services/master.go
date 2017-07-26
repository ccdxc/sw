package services

import (
	"fmt"
	"sync"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/cmd/env"
	configs "github.com/pensando/sw/cmd/systemd-configs"
	"github.com/pensando/sw/cmd/types"
)

// Services that run on node that wins the Leader election.
// TODO: Spread these out when it makes sense to do so.
var (
	masterServices = []string{
		"pen-kube-apiserver",
		"pen-kube-scheduler",
		"pen-kube-controller-manager",
		"pen-apiserver",
		"pen-apigw",
		"pen-elasticsearch",
	}
)

type masterService struct {
	sync.Mutex
	sysSvc    types.SystemdService
	leaderSvc types.LeaderService
	isLeader  bool
	enabled   bool
	virtualIP string // virtualIP for services which can listed on only one VIP
	configs   configs.Interface
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
	return func(o *masterService) {
		o.leaderSvc = leaderSvc
	}
}

// WithSystemdSvcMasterOption to pass a specifc types.SystemdService implementation
func WithSystemdSvcMasterOption(sysSvc types.SystemdService) MasterOption {
	return func(o *masterService) {
		o.sysSvc = sysSvc
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
	m.leaderSvc.Register(&m)
	m.sysSvc.Register(&m)

	return &m
}

// Start starts the services that run on the leader node in the
// cluster after running election. These include kubernetes master components and API Gateway, services
// that have affinity to the Virtual IP.
// TODO: Spread out kubernetes master services also?
func (s *masterService) Start() error {
	s.sysSvc.Start()
	s.leaderSvc.Start()

	s.Lock()
	defer s.Unlock()
	if s.leaderSvc.IsLeader() {
		s.isLeader = true
	}
	s.enabled = true
	if s.isLeader {
		return s.startLeaderServices(s.virtualIP)
	}
	return nil
}

// caller holds the lock
func (s *masterService) startLeaderServices(virtualIP string) error {
	if err := s.configs.GenerateKubeMasterConfig(virtualIP); err != nil {
		return err
	}
	if err := s.configs.GenerateAPIServerConfig(); err != nil {
		return err
	}
	for ii := range masterServices {
		if err := s.sysSvc.StartUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			return err
		}
	}
	return nil
}

// Stop stops the services that run on the leader node in the
// cluster.
func (s *masterService) Stop() {
	s.Lock()
	defer s.Unlock()
	s.enabled = false
	s.stopLeaderServices()
}

// caller holds the lock
func (s *masterService) stopLeaderServices() {
	for ii := range masterServices {
		if err := s.sysSvc.StopUnit(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Failed to stop leader service %v with error: %v", masterServices[ii], err)
		}
	}
	s.configs.RemoveKubeMasterConfig()
	s.configs.RemoveAPIServerConfig()
}

// AreLeaderServicesRunning returns if all the leader node services are
// running.
func (s *masterService) AreLeaderServicesRunning() bool {
	// TODO: Need systemd API for this
	return true
}

func (s *masterService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	var err error
	switch e.Evt {
	case types.LeaderEventChange:
	case types.LeaderEventWon:
		s.Lock()
		defer s.Unlock()
		s.isLeader = true
		if s.enabled {
			s.startLeaderServices(s.virtualIP)
		}
	case types.LeaderEventLost:
		s.Lock()
		defer s.Unlock()
		s.isLeader = false
		if s.enabled {
			s.stopLeaderServices()
		}
	}
	return err
}

func (s *masterService) OnNotifySystemdEvent(e types.SystemdEvent) error {
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
