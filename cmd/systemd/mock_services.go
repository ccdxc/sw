package systemd

import (
	"sync"

	"github.com/pensando/sw/cmd/types"
)

type mockSystemdService struct {
	sync.Mutex
	nodeSvcsRunning   bool
	leaderSvcsRunning bool
}

func NewMockSystemdService() types.SystemdService {
	return &mockSystemdService{}
}

func (m *mockSystemdService) StartNodeServices(virtualIP string) error {
	m.Lock()
	defer m.Unlock()
	m.nodeSvcsRunning = true
	return nil
}

func (m *mockSystemdService) StopNodeServices() {
	m.Lock()
	defer m.Unlock()
	m.nodeSvcsRunning = false
}

func (m *mockSystemdService) AreNodeServicesRunning() bool {
	return m.nodeSvcsRunning
}

func (m *mockSystemdService) StartLeaderServices(virtualIP string) error {
	m.Lock()
	defer m.Unlock()
	m.leaderSvcsRunning = true
	return nil
}

func (m *mockSystemdService) StopLeaderServices() {
	m.Lock()
	defer m.Unlock()
	m.leaderSvcsRunning = false
}

func (m *mockSystemdService) AreLeaderServicesRunning() bool {
	return m.leaderSvcsRunning
}
