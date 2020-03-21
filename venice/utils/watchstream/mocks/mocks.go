package mocks

import (
	"context"
	"sync"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/watchstream"
)

// FakeWatchPrefixes implements a mock interface
type FakeWatchPrefixes struct {
	sync.Mutex
	Qmap                     map[string]*FakeWatchEventQ
	Adds, Dels, Gets, Getexs uint64
	Addfn                    func(path string) watchstream.WatchEventQ
	Delfn                    func(path string) watchstream.WatchEventQ
	Getfn                    func(path string) []watchstream.WatchEventQ
	Getexfn                  func(path string) watchstream.WatchEventQ
}

// Add implements a mock interface
func (f *FakeWatchPrefixes) Add(path, peer string) watchstream.WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.Adds++
	if f.Addfn != nil {
		return f.Addfn(path)
	}
	return nil
}

// Del implements a mock interface
func (f *FakeWatchPrefixes) Del(path, peer string) watchstream.WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.Dels++
	if f.Delfn != nil {
		return f.Delfn(path)
	}
	return nil
}

// Get implements a mock interface
func (f *FakeWatchPrefixes) Get(path string) []watchstream.WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.Gets++
	if f.Getfn != nil {
		return f.Getfn(path)
	}
	return nil
}

// GetExact implements a mock interface
func (f *FakeWatchPrefixes) GetExact(path string) watchstream.WatchEventQ {
	defer f.Unlock()
	f.Lock()
	f.Getexs++
	if f.Getexfn != nil {
		return f.Getexfn(path)
	}
	return nil
}

// Stats implements a mock interface
func (f *FakeWatchPrefixes) Stats() map[string][]watchstream.WatchQueueStat {
	return nil
}

// FakeWatchEventQ implements a mock interface
type FakeWatchEventQ struct {
	sync.Mutex
	Enqueues, Dequeues, Stops uint64
	DqCh                      chan error
}

// Enqueue implements a mock interface
func (f *FakeWatchEventQ) Enqueue(evType kvstore.WatchEventType, obj, prev runtime.Object) error {
	defer f.Unlock()
	f.Lock()
	f.Enqueues++
	return nil
}

// Dequeue implements a mock interface
func (f *FakeWatchEventQ) Dequeue(ctx context.Context,
	fromver uint64, cb apiintf.EventHandlerFn, cleanupfn func(), opts *api.ListWatchOptions) {
	defer f.Unlock()
	f.Lock()
	close(f.DqCh)
	f.Dequeues++
}

// Stop implements a mock interface
func (f *FakeWatchEventQ) Stop() bool {
	defer f.Unlock()
	f.Lock()
	f.Stops++
	return false
}
