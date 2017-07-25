package services

import (
	"fmt"
	"sync"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/cmd/env"
	configs "github.com/pensando/sw/cmd/systemd-configs"
	"github.com/pensando/sw/cmd/types"
)

// Services that run on all controller nodes
var (
	nodeServices = []string{"pen-kubelet"}
)

type nodeService struct {
	sync.Mutex
	sysSrv    types.SystemdService
	virtualIP string
	enabled   bool
	configs   configs.Interface
}

// NodeOption fills the optional params
type NodeOption func(service *nodeService)

// WithConfigsNodeOption to pass a specifc types.SystemdService implementation
func WithConfigsNodeOption(configs configs.Interface) NodeOption {
	return func(o *nodeService) {
		o.configs = configs
	}
}

// WithSystemdSvcNodeOption to pass a specifc types.SystemdService implementation
func WithSystemdSvcNodeOption(sysSrv types.SystemdService) NodeOption {
	return func(o *nodeService) {
		o.sysSrv = sysSrv
	}
}

// NewNodeService returns a NodeService
func NewNodeService(virtualIP string, options ...NodeOption) types.NodeService {
	s := nodeService{
		virtualIP: virtualIP,
		sysSrv:    env.SystemdService,
		configs:   configs.New(),
	}

	for _, o := range options {
		if o != nil {
			o(&s)
		}
	}

	s.sysSrv.Register(&s)
	return &s
}

// Start starts the services that run on all controller nodes in
// the cluster.
func (s *nodeService) Start() error {
	s.sysSrv.Start()

	s.Lock()
	defer s.Unlock()
	s.enabled = true
	if err := s.configs.GenerateKubeletConfig(s.virtualIP); err != nil {
		return err
	}

	for ii := range nodeServices {
		if err := s.sysSrv.StartUnit(fmt.Sprintf("%s.service", nodeServices[ii])); err != nil {
			return err
		}
	}
	return nil
}

// Stop stops the services that run on all controller nodes in the
// cluster.
func (s *nodeService) Stop() {
	s.Lock()
	defer s.Unlock()
	s.enabled = false
	for ii := range nodeServices {
		if err := s.sysSrv.StopUnit(fmt.Sprintf("%s.service", nodeServices[ii])); err != nil {
			log.Errorf("Failed to stop node service %v with error: %v", nodeServices[ii], err)
		}
	}
	s.configs.RemoveKubeletConfig()
}

// AreNodeServicesRunning returns if all the controller node services are
// running.
func (s *nodeService) AreNodeServicesRunning() bool {
	// TODO: Need systemd API for this
	return true
}

func (s *nodeService) OnNotifySystemdEvent(e types.SystemdEvent) error {
	found := false
	for _, n := range nodeServices {
		if e.Name == n {
			found = true
			break
		}
	}
	if !found {
		return nil // service that i am not interested in
	}
	switch e.Status {
	case types.SystemdUnitDead:
		if s.enabled {
			log.Errorf("service %v stopped", e.Name)
		}
	case types.SystemdUnitActive:
	case types.SystemdUnitInTransit:
	}
	return nil
}
