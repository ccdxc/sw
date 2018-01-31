package runtime

import (
	"sync"
)

// BG implements handshake utilities for running a method in background.
// To use, just embed BG inline in your struct and use the methods
// provided here
type BG struct {
	wg     sync.WaitGroup
	stopCh chan bool
}

// Start is invoked just before invoking your go func. It performs
// necessary initialization. Must be called once before invoking go ...
// Called from outside
func (bg *BG) Start() {
	bg.stopCh = make(chan bool)
	bg.wg.Add(1)
}

// StopSync stops the go routine and waits until it exits.
// Called from outside
func (bg *BG) StopSync() {
	close(bg.stopCh)
	bg.wg.Wait()
}

// StopAsync stops the go routine and returns
// Called from outside
func (bg *BG) StopAsync() {
	close(bg.stopCh)
}

// StopCh returns a channel to listen to for a stop signal
// Called from inside the go func
func (bg *BG) StopCh() <-chan bool {
	return bg.stopCh
}

// Done indicates the go routine is exiting
// Called right at the start from inside the go func via defer
func (bg *BG) Done() {
	bg.wg.Done()
}
