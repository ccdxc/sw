package services

import (
	"sync"

	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/systemd"
)

// systemdService provides interface to start systemd units + watch + Notify on change
type systemdService struct {
	sync.Mutex
	w         systemd.Watcher
	evChan    <-chan *systemd.UnitEvent
	errChan   <-chan error
	stop      chan bool
	units     map[string]interface{}
	observers []types.SystemdEventObserver
	systemdIf systemd.Interface
}

// SystemdSvcOption fills the optional params
type SystemdSvcOption func(service *systemdService)

// WithSysIfSystemdSvcOption to pass a specifc systemd Interface
func WithSysIfSystemdSvcOption(systemdIf systemd.Interface) SystemdSvcOption {
	return func(o *systemdService) {
		o.systemdIf = systemdIf
	}
}

// NewSystemdService provides a new SystemdService
func NewSystemdService(options ...SystemdSvcOption) types.SystemdService {
	svc := systemdService{
		units:     make(map[string]interface{}),
		systemdIf: systemd.New(),
	}

	for _, o := range options {
		if o != nil {
			o(&svc)
		}
	}

	return &svc
}

func (s *systemdService) watchForEvents() {
	var err error
	var evt types.SystemdEvent

	for {
		select {
		case <-s.stop:
			return
		case e := <-s.evChan:
			switch e.Status {
			case systemd.Active:
				evt = types.SystemdEvent{Status: types.SystemdUnitActive, Name: e.Name}
			case systemd.Dead:
				s.notify(types.SystemdEvent{Status: types.SystemdUnitDead, Name: e.Name})
			case systemd.InTransit:
				s.notify(types.SystemdEvent{Status: types.SystemdUnitInTransit, Name: e.Name})
			default:
				continue
			}
			if err = s.notify(evt); err != nil {
				log.Errorf("Failed to notify %v with error: %v", evt, err)
			}
		case <-s.errChan:
			s.Lock()
			defer s.Unlock()
			s.w.Close()
			s.w, s.evChan, s.errChan = s.systemdIf.NewWatcher()
			for name := range s.units {
				s.w.Subscribe(name)
			}
		}
	}
}

// Start service
func (s *systemdService) Start() error {
	s.Lock()
	defer s.Unlock()
	if s.w != nil { // Already started.
		return nil
	}
	s.stop = make(chan bool)
	s.w, s.evChan, s.errChan = s.systemdIf.NewWatcher()
	go s.watchForEvents()
	return nil
}

// Stop service. Does NOT stop the systemd units associated with this service
func (s *systemdService) Stop() {
	s.Lock()
	defer s.Unlock()
	s.w.Close()
	close(s.stop)
	s.w = nil // conditional to indicate that service has stopped
}

// StartUnit starts a systemd unit and watch for its health
func (s *systemdService) StartUnit(name string) error {
	err := s.systemdIf.StartTarget(name)
	if err == nil {
		s.units[name] = nil
		s.w.Subscribe(name)
	}
	return err
}

// StopUnit stops a systemd unit service and stops watching for it
func (s *systemdService) StopUnit(name string) error {
	err := s.systemdIf.StopTarget(name)
	if err == nil {
		s.w.Unsubscribe(name)
		delete(s.units, name)
	}
	return err
}

// Units return the systemd units that are watched by this service
func (s *systemdService) Units() []string {
	s.Lock()
	defer s.Unlock()
	var retval []string
	for u := range s.units {
		retval = append(retval, u)
	}
	return retval
}

func (s *systemdService) Register(o types.SystemdEventObserver) {
	s.Lock()
	defer s.Unlock()
	s.observers = append(s.observers, o)
}

func (s *systemdService) UnRegister(o types.SystemdEventObserver) {
	s.Lock()
	defer s.Unlock()
	var i int
	for i = range s.observers {
		if s.observers[i] == o {
			break
		}
	}
	s.observers = append(s.observers[:i], s.observers[i+1:]...)
}

// return first encountered err of the observers.
// All the observers are notified of the event even if someone fails
func (s *systemdService) notify(e types.SystemdEvent) error {
	var err error
	for _, o := range s.observers {
		er := o.OnNotifySystemdEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}
