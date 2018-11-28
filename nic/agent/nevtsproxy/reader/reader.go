package reader

import (
	"os"
	"regexp"
	"sync"
	"time"

	"github.com/fsnotify/fsnotify"

	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// Reader watches the directory `shm.GetSharedMemoryDirectory()` and
// spins up events readers (evtReader) on any file (shm) create event.
// And, these events readers in turn starts receiving events from those shared memory files.
// Also, this reader acts as a central entity to manage all the event readers
// that were spun up.

var retryCount = 10

// Reader represents the events shared memory reader which reads events
// dir `shm.GetSharedMemoryDirectory()`
type Reader struct {
	dir            string                // dir to watch, e.g. /dev/shm/pen-events
	pollDelay      time.Duration         // poll interval
	evtReaders     map[string]*EvtReader // list of event readers that were spun up; one reader per shm file
	fileWatcher    *fsnotify.Watcher     // file watcher that watches file create events on `dir`
	evtsDispatcher events.Dispatcher     // events dispatcher to be used by the readers to dispatch events
	stop           sync.Once             // to stop the reader
}

// NewReader creates a new shm events reader
// - it creates a file watcher on the given dir to watch file(shm) create events/
func NewReader(dir string, pollDelay time.Duration, evtsDispatcher events.Dispatcher) *Reader {
	rdr := &Reader{
		dir:            dir,
		pollDelay:      pollDelay,
		evtReaders:     map[string]*EvtReader{},
		evtsDispatcher: evtsDispatcher,
	}

	return rdr
}

// Start starts the file watcher to watch shm create events
func (r *Reader) Start() error {
	var err error
	var fileWatcher *fsnotify.Watcher

	for i := 0; i < retryCount; i++ {
		if fileWatcher, err = fsnotify.NewWatcher(); err != nil {
			log.Debugf("failed to create file watcher, err: %v, retrying...", err)
			time.Sleep(1 * time.Second)
			continue
		}

		if err = fileWatcher.Add(r.dir); err != nil {
			log.Debugf("failed to add {%s} to the file watcher, err: %v, retrying...", r.dir, err)
			fileWatcher.Close()
			time.Sleep(1 * time.Second)
			continue
		}

		r.fileWatcher = fileWatcher
	}

	if r.fileWatcher != nil {
		go r.watchFileEvents()
	}

	return err
}

// Stop stops the reader by stopping the file watcher and all the events readers that were spun
func (r *Reader) Stop() {
	r.stop.Do(func() {
		r.fileWatcher.Close() // watchFileEvents will be stopped

		// close all readers
		for _, eRdr := range r.evtReaders {
			eRdr.Stop()
		}

	})
}

// GetEventReaders returns the list of readers created so far
func (r *Reader) GetEventReaders() map[string]*EvtReader {
	return r.evtReaders
}

// watchFileEvents helper function to watch file events
func (r *Reader) watchFileEvents() {
	for {
		select {
		case event, ok := <-r.fileWatcher.Events:
			if !ok { // when the watcher is closed (during Stop())
				return
			}

			// on file create event, start reader on those file that end with ".events"
			if event.Op == fsnotify.Create {
				// TODO: add retry
				fs, err := os.Stat(event.Name)
				if err != nil {
					log.Errorf("failed to get stats for %s, err: %v", event.Name, err)
					continue
				}

				ok, err := regexp.MatchString("^*.events$", fs.Name())
				if err != nil {
					log.Errorf("failed to match the file name with pattern {^*.events$} , err: %v", err)
					continue
				}

				if fs.Mode().IsRegular() && ok {
					r.startEvtsReader(event.Name)
				}
			}
		case err, ok := <-r.fileWatcher.Errors:
			if !ok { // when the watcher is closed (during Stop())
				return
			}

			log.Errorf("received error from file watcher, err: %v", err)
		}
	}
}

// startEvtsReader helper function to start events reader to reader events
// from the given shared memory file (shmPath)
func (r *Reader) startEvtsReader(shmPath string) {
	if _, ok := r.evtReaders[shmPath]; ok {
		log.Errorf("something wrong, reader already exists for %s", shmPath)
		return
	}

	eRdr, err := NewEventReader(shmPath, r.pollDelay, WithEventsDispatcher(r.evtsDispatcher))
	if err != nil {
		log.Errorf("failed to create reader for shm: %s, err: %v", shmPath, err)
		return
	}

	eRdr.Start()

	r.evtReaders[shmPath] = eRdr
	log.Infof("successfully created reader for %s", shmPath)
}
