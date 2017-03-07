package systemd

func StartTarget(name string) (err error) {
	return nil
}
func StopTarget(name string) error {
	return nil
}
func RestartTarget(name string) error {
	return nil
}
func NewWatcher() (*Watcher, <-chan *UnitStatus, <-chan error) {
	return nil, nil, nil
}
func (w *Watcher) Close() {
}
func (w *Watcher) Unsubscribe(name string) {
}
func (w *Watcher) Subscribe(name string) {
}
func (w *Watcher) TargetDeps(name string) ([]string, error) {
	return nil, nil
}
func (w *Watcher) GetPID(name string) (uint32, error) {
	return 0, nil
}
