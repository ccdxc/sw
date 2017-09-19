package systemd

import (
	"sync"

	"github.com/coreos/go-systemd/dbus"
)

// Watcher watches for events from systemd
type sysWatcher struct {
	sync.RWMutex
	conn     *dbus.Conn          // dbus connection reference
	set      map[string]struct{} // sevices for which we are interested in
	stopChan chan bool           // to return from the go routine
	pokeChan chan bool           // to immediately update cache after subscriber update
}

// Interface provided by systemd package
type Interface interface {
	DaemonReload() error
	StartTarget(name string) (err error)
	StopTarget(name string) error
	RestartTarget(name string) error
	NewWatcher() (Watcher, <-chan *UnitEvent, <-chan error)
}

// Watcher interface provides a channel for watching on systemd bus for interesting services
type Watcher interface {
	Close()
	Subscribe(name string)
	Unsubscribe(name string)
	TargetDeps(name string) ([]string, error)
	GetPID(name string) (uint32, error)
}

// UnitStatus is current state of the Unit/Target
type UnitStatus int

const (
	// Active Unit or Target
	Active UnitStatus = iota
	// InTransit Unit
	InTransit
	// Dead or Not-Running
	Dead
)

// UnitEvent represents the current event given unit.
type UnitEvent struct {
	Name   string
	Status UnitStatus

	// When Event is Removed, the following fields are nil
	Description string // The human readable description string
	LoadState   string // The load state (i.e. whether the unit file has been loaded successfully)
	ActiveState string // The active state (i.e. whether the unit is currently started or not)
	SubState    string // The sub state (a more fine-grained version of the active state that is specific to the unit type, which the active state is not)

}
