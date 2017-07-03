package systemd

import (
	"fmt"
	"sync"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/cmd/systemd/configs"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/systemd"
)

// Services that run on node that wins the election.
// TODO: Spread these out when it makes sense to do so.
var (
	masterServices = []string{"pen-kube-apiserver", "pen-kube-scheduler", "pen-kube-controller-manager"}
	nodeServices   = []string{"pen-kubelet"}
)

type systemdService struct {
	sync.Mutex
}

// NewSystemdService returns a Systemd Service
func NewSystemdService() types.SystemdService {
	return &systemdService{}
}

// StartNodeServices starts the services that run on all controller nodes in
// the cluster.
func (s *systemdService) StartNodeServices(virtualIP string) error {
	s.Lock()
	defer s.Unlock()
	if err := configs.GenerateKubeletConfig(virtualIP); err != nil {
		return err
	}
	for ii := range nodeServices {
		if err := systemd.StartTarget(fmt.Sprintf("%s.service", nodeServices[ii])); err != nil {
			return err
		}
	}
	return nil
}

// StopNodeServices stops the services that run on all controller nodes in the
// cluster.
func (s *systemdService) StopNodeServices() {
	s.Lock()
	defer s.Unlock()
	for ii := range nodeServices {
		if err := systemd.StopTarget(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Failed to stop node service %v with error: %v", masterServices[ii], err)
		}
	}
	configs.RemoveKubeletConfig()
}

// AreNodeServicesRunning returns if all the controller node services are
// running.
func (s *systemdService) AreNodeServicesRunning() bool {
	// TODO: Need systemd API for this
	return true
}

// StartLeaderServices starts the services that run on the leader node in the
// cluster. These include kubernetes master components and API Gateway, services
// that have affinity to the Virtual IP.
// TODO: Spread out kubernetes master services also?
func (s *systemdService) StartLeaderServices(virtualIP string) error {
	s.Lock()
	defer s.Unlock()
	if err := configs.GenerateKubeMasterConfig(virtualIP); err != nil {
		return err
	}
	for ii := range masterServices {
		if err := systemd.StartTarget(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			return err
		}
	}
	return nil
}

// StopLeaderServices stops the services that run on the leader node in the
// cluster.
func (s *systemdService) StopLeaderServices() {
	s.Lock()
	defer s.Unlock()
	for ii := range masterServices {
		if err := systemd.StopTarget(fmt.Sprintf("%s.service", masterServices[ii])); err != nil {
			log.Errorf("Failed to stop leader service %v with error: %v", masterServices[ii], err)
		}
	}
	configs.RemoveKubeMasterConfig()
}

// AreLeaderServicesRunning returns if all the leader node services are
// running.
func (s *systemdService) AreLeaderServicesRunning() bool {
	// TODO: Need systemd API for this
	return true
}
