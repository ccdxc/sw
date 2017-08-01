package services

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/systemd"
)

//mock objects which are used in multiple tests

// MockIPService is used by tests for faking interface configuration.
type MockIPService struct {
	sync.Mutex
	ip    map[string]interface{}
	error bool
}

// NewMockIPService returns a Mock IPService
func NewMockIPService() *MockIPService {
	return &MockIPService{ip: make(map[string]interface{})}
}

// HasIP returns true if this IP was added before
func (i *MockIPService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return false, fmt.Errorf("Error set")
	}
	_, ok := i.ip[ip]
	return ok, nil
}

// AddSecondaryIP mocks addition of IP address
func (i *MockIPService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	_, ok := i.ip[ip]
	if ok {
		return fmt.Errorf("IP already configured")
	}
	i.ip[ip] = nil
	return nil
}

// DeleteIP deletes previously added IP
func (i *MockIPService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	delete(i.ip, ip)
	return nil
}

// SetError sets an error
func (i *MockIPService) SetError() {
	i.Lock()
	defer i.Unlock()
	i.error = true
}

// ClearError clears an error
func (i *MockIPService) ClearError() {
	i.Lock()
	defer i.Unlock()
	i.error = false
}

//MockLeaderService mocks Leader Service
type MockLeaderService struct {
	id        string
	observers []types.LeadershipObserver
	LeaderID  string
}

// NewMockLeaderService creates a mock leader Service
func NewMockLeaderService(id string) *MockLeaderService {
	return &MockLeaderService{
		id: id,
	}
}

// Start starts leader election on quorum nodes.
func (l *MockLeaderService) Start() error {
	l.BecomeLeader()
	return nil
}

// Register an Observers
func (l *MockLeaderService) Register(o types.LeadershipObserver) {
	l.observers = append(l.observers, o)
}

//UnRegister an Observer
func (l *MockLeaderService) UnRegister(o types.LeadershipObserver) {
}

// return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (l *MockLeaderService) notify(e types.LeaderEvent) error {
	var err error
	for _, o := range l.observers {
		er := o.OnNotifyLeaderEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

// Stop stops the leader election.
func (l *MockLeaderService) Stop() {
	l.processEvent("")
}

// BecomeLeader mocks becoming a leader. If the Notifiers return an error, it returns this error.
func (l *MockLeaderService) BecomeLeader() error {
	return l.processEvent(l.id)
}

// GiveupLeadership of a leader service
func (l *MockLeaderService) GiveupLeadership() error {
	return l.processEvent(l.id + "something")
}

// ChangeLeadership from some leader to some other leader
func (l *MockLeaderService) ChangeLeadership() error {
	return l.processEvent(l.id + "somethingelse")
}

// processEvent handles leader election events.
func (l *MockLeaderService) processEvent(leader string) error {
	if l.id == leader {
		e := types.LeaderEvent{Evt: types.LeaderEventWon, Leader: leader}
		if err := l.notify(e); err != nil {
			l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
			return err
		}
	} else if l.IsLeader() {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventLost, Leader: leader})
	} else {
		l.notify(types.LeaderEvent{Evt: types.LeaderEventChange, Leader: leader})
	}
	l.LeaderID = leader
	return nil
}

// IsLeader checks if this instance is the leader.
func (l *MockLeaderService) IsLeader() bool {
	return l.id == l.LeaderID
}

// Leader returns the current leader.
func (l *MockLeaderService) Leader() string {
	return l.LeaderID
}

type mockSystemdWatcher struct {
	evChan  chan *systemd.UnitEvent
	errChan chan error

	makeFail bool
}

func (w *mockSystemdWatcher) Close() {
	panic("not implemented")
}

func (w *mockSystemdWatcher) Subscribe(name string) {
	if !w.makeFail {
		w.evChan <- &systemd.UnitEvent{Name: name, Status: systemd.Active}
	} else {
		w.evChan <- &systemd.UnitEvent{Name: name, Status: systemd.Dead}
	}
}

func (w *mockSystemdWatcher) Unsubscribe(name string) {

}

func (w *mockSystemdWatcher) TargetDeps(name string) ([]string, error) {
	panic("not implemented")
}

func (w *mockSystemdWatcher) GetPID(name string) (uint32, error) {
	panic("not implemented")
}

type mockSystemdIf struct {
	m mockSystemdWatcher
}

func (s *mockSystemdIf) DaemonReload() (err error) {
	return nil
}

func (s *mockSystemdIf) StartTarget(name string) (err error) {
	return nil
}

func (s *mockSystemdIf) StopTarget(name string) error {
	return nil
}

func (s *mockSystemdIf) RestartTarget(name string) error {
	return nil
}

func (s *mockSystemdIf) NewWatcher() (systemd.Watcher, <-chan *systemd.UnitEvent, <-chan error) {
	s.m = mockSystemdWatcher{evChan: make(chan *systemd.UnitEvent, 2), errChan: make(chan error, 2)}
	return &s.m, s.m.evChan, s.m.errChan
}

type mockConfigs struct {
}

func (c *mockConfigs) GenerateKubeletConfig(virtualIP string) error {
	return nil
}

func (c *mockConfigs) RemoveKubeletConfig() {
}

func (c *mockConfigs) GenerateAPIServerConfig() error {
	return nil
}

func (c *mockConfigs) RemoveAPIServerConfig() {
}

func (c *mockConfigs) GenerateFilebeatConfig(virtualIP string) error {
	return nil
}

func (c *mockConfigs) RemoveFilebeatConfig() {
}

func (c *mockConfigs) GenerateKubeMasterConfig(virtualIP string) error {
	return nil
}

func (c *mockConfigs) RemoveKubeMasterConfig() {
}
