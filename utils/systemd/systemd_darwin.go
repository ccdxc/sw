package systemd

// New is not implemented for darwin
func New() Interface {
	return nil
}

func startTarget(name string) (err error) {
	return nil
}
func stopTarget(name string) error {
	return nil
}
func restartTarget(name string) error {
	return nil
}

func newWatcher() (*Watcher, <-chan *UnitEvent, <-chan error) {
	return nil, nil, nil
}
