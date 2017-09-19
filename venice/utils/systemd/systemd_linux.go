package systemd

import (
	"fmt"
	"time"

	"github.com/coreos/go-systemd/dbus"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pkg/errors"
)

var (
	targetStateChangeWaitTime = 10 * time.Second
	restartServiceWaitTime    = 10 * time.Second
	watcherPollInterval       = time.Second
)

type systemd struct {
}

func (s *systemd) DaemonReload() error {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()
	return conn.Reload()
}

// New returns a new systemd interface
func New() Interface {
	return &systemd{}
}

// StartTarget starts the specified systemd target
func (s *systemd) StartTarget(name string) (err error) {
	return startTarget(name)
}

// StopTarget stops target
func (s *systemd) StopTarget(name string) error {
	return stopTarget(name)
}

// RestartTarget restart target
func (s *systemd) RestartTarget(name string) error {
	return restartTarget(name)
}

// NewWatcher creates new watcher
func (s *systemd) NewWatcher() (Watcher, <-chan *UnitEvent, <-chan error) {
	return newWatcher()
}

// startTarget starts the specified systemd target
func startTarget(name string) (err error) {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	pid, err := conn.StartUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to start target(%s)", name)
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		log.Infof("Started %v with pid %v", name, pid)
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return fmt.Errorf("timedout when starting target(%s)", name)
	}
}

// stopTarget stops a systemd target
func stopTarget(name string) error {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	_, err = conn.StopUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to stop target(%s)", name)
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return fmt.Errorf("timedout when stopping target(%s)", name)
	}
}

// restartTarget to restart a given target. Stops for a second between stop and start
func restartTarget(name string) error {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	_, err = conn.StopUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to stop target(%s) during restart unit", name)
	}

	// we dont care about the status. However we wait for upto a second for response
	select {
	case <-reschan:
	case <-time.After(targetStateChangeWaitTime):
	}

	// after stop always wait . Else seeing some weird behaviors
	time.Sleep(restartServiceWaitTime)

	reschan = make(chan string)
	_, err = conn.StartUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to start target(%s) during restart unit", name)
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return fmt.Errorf("timedout when restarting target(%s)", name)
	}

}

func mismatchUnitStatus(u1, u2 *dbus.UnitStatus) bool {
	return u1.Name != u2.Name ||
		u1.Description != u2.Description ||
		u1.LoadState != u2.LoadState ||
		u1.ActiveState != u2.ActiveState ||
		u1.SubState != u2.SubState
}

func (w *sysWatcher) subscribeUnits(interval time.Duration) (<-chan *UnitEvent, <-chan error) {
	w.stopChan = make(chan bool, 0)
	w.pokeChan = make(chan bool, 0)

	// We make a small buffered channel here. Otherwise on the first timer expiry, we would be waiting sending
	// the events to statusChan and the caller might not even have started listening on the channel - hence
	// we might lose events that might have happened before the caller starts listening on channel
	statusChan := make(chan *UnitEvent, 5)
	errChan := make(chan error, 5)

	go func(poke, stop <-chan bool) {
		old := make(map[string]*dbus.UnitStatus)
		for {
			select {

			case <-time.After(interval):
			case <-poke:

			case <-stop:
				log.Debugf("Exited watcher timer")
				return
			}

			units, err := w.conn.ListUnits()
			if err == nil {
				cur := make(map[string]*dbus.UnitStatus)

				// determine the units that we are interested in
				w.RLock()
				for i := range units {
					if _, ok := w.set[units[i].Name]; !ok {
						continue
					}
					cur[units[i].Name] = &units[i]
				}
				w.RUnlock()

				// add all new or changed units
				changed := make(map[string]*dbus.UnitStatus)
				for n, u := range cur {
					if oldU, ok := old[n]; !ok || mismatchUnitStatus(oldU, u) {
						changed[n] = u
					}
					delete(old, n)
				}

				w.RLock()
				// add all deleted units
				for oldN := range old {
					if _, ok := w.set[oldN]; ok {
						changed[oldN] = nil
					}
				}
				w.RUnlock()

				old = cur

				for k, v := range changed {
					s := UnitEvent{Name: k}
					if v != nil {
						switch {
						case (v.ActiveState == "active") && (v.SubState == "active" || v.SubState == "running"):
							s.Status = Active
						case v.ActiveState == "inactive":
							s.Status = Dead
						default:
							s.Status = InTransit
						}
						s.Description = v.Description
						s.LoadState = v.LoadState
						s.ActiveState = v.ActiveState
						s.SubState = v.SubState
					} else {
						s.Status = Dead
					}
					statusChan <- &s
				}
			} else {
				errChan <- err
			}

		}
	}(w.pokeChan, w.stopChan)

	return statusChan, errChan
}

// newWatcher creates a Watcher object on which the caller can subscribe to events
func newWatcher() (Watcher, <-chan *UnitEvent, <-chan error) {
	w := sysWatcher{set: make(map[string]struct{})}
	var err error
	w.conn, err = dbus.New()
	if err != nil {
		log.Fatalf("unable to establish dbus connection to systemd. Err: %s", err.Error())
	}
	// poll every second. Constant for now
	evChan, errChan := w.subscribeUnits(watcherPollInterval)
	return &w, evChan, errChan
}

// Close the subscription
func (w *sysWatcher) Close() {
	w.stopChan <- true
	w.conn.Unsubscribe()
	w.conn.Close()
	log.Debugf("Closed watcher")
}

// Unsubscribe removes watching on a service and its dependents
func (w *sysWatcher) Unsubscribe(name string) {
	w.Lock()
	delete(w.set, name)
	w.Unlock()

	deps, err := w.TargetDeps(name)
	if err == nil {
		w.Lock()
		for _, serv := range deps {
			log.Debugf("Removing %s from listed of watched services", serv)
			delete(w.set, serv)
		}
		w.Unlock()
	}
	w.pokeChan <- true

}

// Subscribe starts watching on a service and its dependents. This is internally implemented using a poller
func (w *sysWatcher) Subscribe(name string) {
	w.Lock()
	w.set[name] = struct{}{}
	w.Unlock()

	deps, err := w.TargetDeps(name)
	if err == nil {
		w.Lock()
		for _, serv := range deps {
			log.Debugf("Adding %s to listed of watched services", serv)
			w.set[serv] = struct{}{}
		}
		w.Unlock()
	}
	// force a lookup immediately
	w.pokeChan <- true

}

// TargetDeps returns units which are dependent on a target
func (w *sysWatcher) TargetDeps(name string) ([]string, error) {
	i, err := w.conn.GetUnitProperty(name, "Wants")
	if err != nil {
		return nil, errors.Wrapf(err, "unable to determine dependent services")
	}
	return i.Value.Value().([]string), err
}

// GetPID returns PID of specified unit. May be needed if we need to send SIGTERM etc to a unit
func (w *sysWatcher) GetPID(name string) (uint32, error) {
	i, err := w.conn.GetServiceProperty(name, "MainPID")
	if err != nil {
		return 0, errors.Wrapf(err, "unable to determine PID for service")
	}
	return i.Value.Value().(uint32), err
}
