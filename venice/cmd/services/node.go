package services

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/venice/cmd/env"
	configs "github.com/pensando/sw/venice/cmd/systemd-configs"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Services that run on all controller nodes
var (
	nodeServices = []string{"pen-kubelet"}
)

type nodeService struct {
	sync.Mutex
	sysSvc               types.SystemdService
	enabled              bool
	configs              configs.Interface
	nodeID               string
	k8sAPIServerLocation string
	elasticLocation      string
}

// NodeOption fills the optional params
type NodeOption func(service *nodeService)

// WithConfigsNodeOption to pass a specific types.SystemdService implementation
func WithConfigsNodeOption(configs configs.Interface) NodeOption {
	return func(o *nodeService) {
		o.configs = configs
	}
}

// WithSystemdSvcNodeOption to pass a specific types.SystemdService implementation
func WithSystemdSvcNodeOption(sysSvc types.SystemdService) NodeOption {
	return func(o *nodeService) {
		o.sysSvc = sysSvc
	}
}

// NewNodeService returns a NodeService
func NewNodeService(nodeID string, options ...NodeOption) types.NodeService {
	s := nodeService{
		sysSvc:  env.SystemdService,
		configs: configs.New(),
		nodeID:  nodeID,
	}

	for _, o := range options {
		if o != nil {
			o(&s)
		}
	}

	if s.sysSvc != nil {
		s.sysSvc.Register(&s)
	}

	return &s
}

// Start starts the services that run on all controller nodes in
// the cluster.
func (s *nodeService) Start() error {
	if s.sysSvc == nil {
		s.sysSvc = env.SystemdService
		s.sysSvc.Register(s)
	}

	s.sysSvc.Start()

	s.Lock()
	defer s.Unlock()
	if !s.enabled {
		s.enabled = true
	}

	for ii := range nodeServices {
		if err := s.sysSvc.StartUnit(fmt.Sprintf("%s.service", nodeServices[ii])); err != nil {
			return err
		}
	}
	return nil
}

// InitConfigFiles initializes the config files with default values.
func (s *nodeService) InitConfigFiles() {
	s.Lock()
	defer s.Unlock()
	if !s.enabled {
		s.enabled = true
	}

	s.configs.GenerateFilebeatConfig([]string{})
	s.configs.GenerateElasticAuthConfig(s.nodeID)
	s.configs.GenerateElasticDiscoveryConfig([]string{})
	s.configs.GenerateElasticMgmtConfig("", len(env.QuorumNodes))
	s.configs.GenerateElasticCuratorConfig([]string{})
}

// Stop stops the services that run on all controller nodes in the
// cluster.
func (s *nodeService) Stop() {
	s.Lock()
	defer s.Unlock()
	s.enabled = false
	for ii := range nodeServices {
		if err := s.sysSvc.StopUnit(fmt.Sprintf("%s.service", nodeServices[ii])); err != nil {
			log.Errorf("Failed to stop node service %v with error: %v", nodeServices[ii], err)
		}
	}
	s.configs.RemoveKubeletConfig()
	s.configs.RemoveFilebeatConfig()
	s.configs.RemoveElasticDiscoveryConfig()
	s.configs.RemoveElasticMgmtConfig()
	s.configs.RemoveElasticAuthConfig()
	s.configs.RemoveElasticCuratorConfig()
}

func (s *nodeService) ElasticCuratorConfig(elasticLocations []string) error {
	if !s.enabled {
		log.Warnf("Skipping %s config generation, node not enabled", globals.ElasticSearchCurator)
		return nil
	}

	if err := s.configs.GenerateElasticCuratorConfig(elasticLocations); err != nil {
		log.Errorf("Failed to generate %s config with error: %v", globals.ElasticSearchCurator, err)
		return err
	}

	return nil
}

// FileBeatConfig with the location of the Elastic servers
func (s *nodeService) FileBeatConfig(elasticLocations []string) error {

	if !s.enabled {
		log.Warnf("Skipping filebeat config generation, node not enabled")
		return nil
	}

	if err := s.configs.GenerateFilebeatConfig(elasticLocations); err != nil {
		log.Errorf("Failed to generate filebeat config with error: %v", err)
		return err
	}

	if s.sysSvc != nil {
		if err := s.sysSvc.StartUnit("pen-filebeatconfig.service"); err != nil {
			log.Errorf("Failed to reconfigure filebeat service with new config.  error: %v", err)
			return err
		}
	}

	return nil
}

// ElasticDiscoveryConfig updates the location of the Elastic servers
func (s *nodeService) ElasticDiscoveryConfig(elasticLocations []string) error {

	if !s.enabled {
		log.Warnf("Skipping elastic-discovery config generation, node not enabled")
		return nil
	}

	if err := s.configs.GenerateElasticDiscoveryConfig(elasticLocations); err != nil {
		log.Errorf("Failed to generate elastic-discovery config with error: %v", err)
		return err
	}

	return nil
}

// ElasticMgmtConfig configures the mgmtAddr needed for elastic instance
func (s *nodeService) ElasticMgmtConfig() error {

	if !s.enabled {
		log.Warnf("Skipping elastic-mgmt config generation, node not enabled")
		return nil
	}

	if err := s.configs.GenerateElasticMgmtConfig(s.nodeID, len(env.QuorumNodes)); err != nil {
		log.Errorf("Failed to generate elastic-mgmt config with error: %v", err)
		return err
	}

	return nil
}

// KubectlConfig updates the kubelet with the new location of k8sAPI server
func (s *nodeService) KubeletConfig(k8sAPIServerLocation string) error {
	if s.k8sAPIServerLocation == k8sAPIServerLocation {
		return nil
	}
	if err := s.configs.GenerateKubeletConfig(s.nodeID, k8sAPIServerLocation, globals.KubeAPIServerPort); err != nil {
		log.Errorf("Failed to generate kubelet config with error: %v", err)
		return err
	}
	if !s.enabled {
		return nil
	}
	if err := s.sysSvc.StartUnit(fmt.Sprintf("pen-kubeletconfig.service")); err != nil {
		log.Errorf("Failed to reconfigure kubelet service with new config.  error: %v", err)
		return err
	}

	return nil
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
