package systemd

// New is not implemented for darwin
func New() Interface {
	return nil
}

// NewWatcher crates new watcher
func NewWatcher() (*Watcher, <-chan *UnitEvent, <-chan error) {
	return nil, nil, nil
}
