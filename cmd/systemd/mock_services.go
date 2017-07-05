package systemd

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/cmd/types"
)

type mockSystemdService struct {
	sync.Mutex
	nodeSvcsRunning   bool
	leaderSvcsRunning bool
}

// NewMockSystemdService returns a Mock service
func NewMockSystemdService() types.SystemdService {
	return &mockSystemdService{}
}

// StartNodeServices mocks sstart node
func (m *mockSystemdService) StartNodeServices(virtualIP string) error {
	m.Lock()
	defer m.Unlock()
	m.nodeSvcsRunning = true
	return nil
}

// StopNodeServices mocks stop node
func (m *mockSystemdService) StopNodeServices() {
	m.Lock()
	defer m.Unlock()
	m.nodeSvcsRunning = false
}

// AreNodeServicesRunning mocks service running api
func (m *mockSystemdService) AreNodeServicesRunning() bool {
	return m.nodeSvcsRunning
}

// StartLeaderServices mocks start leader
func (m *mockSystemdService) StartLeaderServices(virtualIP string) error {
	m.Lock()
	defer m.Unlock()
	if m.leaderSvcsRunning {
		return fmt.Errorf("Leader services already running")
	}
	m.leaderSvcsRunning = true
	return nil
}

// StopLeaderServices mocks leader service
func (m *mockSystemdService) StopLeaderServices() {
	m.Lock()
	defer m.Unlock()
	m.leaderSvcsRunning = false
}

// AreLeaderServicesRunning mocks leader running api
func (m *mockSystemdService) AreLeaderServicesRunning() bool {
	return m.leaderSvcsRunning
}
