package systemd

import (
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/coreos/go-systemd/dbus"
	"github.com/pkg/errors"
)

var (
	targetStateChangeWaitTime = 2 * time.Second
	restartServiceWaitTime    = 2 * time.Second
	watcherPollInterval       = time.Second
)

// StartTarget starts the specified systemd target
func StartTarget(name string) (err error) {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	_, err = conn.StartUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to start target")
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return errors.New("timedout when starting target")
	}
}

// StopTarget stops a systemd target
func StopTarget(name string) error {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	_, err = conn.StopUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to stop target")
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return errors.New("timedout when stopping target")
	}
}

// RestartTarget to restart a given target. Stops for a second between stop and start
func RestartTarget(name string) error {
	conn, err := dbus.New()
	if err != nil {
		return errors.Wrap(err, "unable to establish dbus connection to systemd")
	}
	defer conn.Close()

	reschan := make(chan string)
	_, err = conn.StopUnit(name, "replace", reschan)
	if err != nil {
		return errors.Wrapf(err, "unable to stop target during restart unit")
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
		return errors.Wrapf(err, "unable to start target during restart unit")
	}
	select {
	case status := <-reschan:
		if status != "done" {
			return errors.New(status)
		}
		return nil
	case <-time.After(targetStateChangeWaitTime):
		return errors.New("timedout when restarting target")
	}

}

func mismatchUnitStatus(u1, u2 *dbus.UnitStatus) bool {
	return u1.Name != u2.Name ||
		u1.Description != u2.Description ||
		u1.LoadState != u2.LoadState ||
		u1.ActiveState != u2.ActiveState ||
		u1.SubState != u2.SubState
}

func (w *Watcher) subscribeUnits(interval time.Duration) (<-chan *UnitEvent, <-chan error) {
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

// NewWatcher creates a Watcher object on which the caller can subscribe to events
func NewWatcher() (*Watcher, <-chan *UnitEvent, <-chan error) {
	w := Watcher{set: make(map[string]struct{})}
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
func (w *Watcher) Close() {
	w.stopChan <- true
	w.conn.Unsubscribe()
	w.conn.Close()
	log.Debugf("Closed watcher")
}

// Unsubscribe removes watching on a service and its dependents
func (w *Watcher) Unsubscribe(name string) {
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
func (w *Watcher) Subscribe(name string) {
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
func (w *Watcher) TargetDeps(name string) ([]string, error) {
	i, err := w.conn.GetUnitProperty(name, "Wants")
	if err != nil {
		return nil, errors.Wrapf(err, "unable to determine dependent services")
	}
	return i.Value.Value().([]string), err
}

// GetPID returns PID of specified unit. May be needed if we need to send SIGTERM etc to a unit
func (w *Watcher) GetPID(name string) (uint32, error) {
	i, err := w.conn.GetServiceProperty(name, "MainPID")
	if err != nil {
		return 0, errors.Wrapf(err, "unable to determine PID for service")
	}
	return i.Value.Value().(uint32), err
}
