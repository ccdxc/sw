// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"bufio"
	"encoding/json"
	"os"
	"path"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// FIXME: address file rotations

// File implements `PersistentStore`. Any incoming event will be written to the persistent store before it
// could be deduped and batched.
// NOTE: the caller will need to ensure thread safety.
type fileImpl struct {
	storePath      string   // file store directory
	eventsFilePath string   // events file path
	handler        *os.File // handler to the events file
}

// newPersistentStore creates a new events file in append only mode.
func newPersistentStore(storePath string) (events.PersistentStore, error) {
	// create store directory if it does not exists
	if _, err := os.Stat(storePath); os.IsNotExist(err) {
		if mkdirErr := os.MkdirAll(storePath, 0755); mkdirErr != nil {
			log.Errorf("failed to create events store %s, err: %v", storePath, err)
			return nil, err
		}
	}

	filepath := path.Join(storePath, "events")

	// open/create events file
	handler, err := os.OpenFile(filepath, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0755)
	if err != nil {
		return nil, errors.Wrap(err, "failed to open/create events file")
	}

	// seek to the end
	handler.Seek(0, os.SEEK_END)

	file := &fileImpl{
		storePath:      storePath,
		eventsFilePath: filepath,
		handler:        handler,
	}

	return file, nil
}

// Write writes given data to the underlying events file
func (f *fileImpl) Write(data []byte) error {
	if _, err := f.handler.Write(data); err != nil {
		return errors.Wrap(err, "failed to write the given event")
	}

	return nil
}

// GetStorePath returns the path of the events store
func (f *fileImpl) GetStorePath() string {
	return f.storePath
}

// GetCurrentOffset returns the current offset (position) of the events file
func (f *fileImpl) GetCurrentOffset() (int64, error) {
	return f.handler.Seek(0, os.SEEK_CUR)
}

// GetEventsFromOffset returns the list of events starting from the given offset.
// offset helps to navigate the file without having to read all the file contents.
func (f *fileImpl) GetEventsFromOffset(offset int64) ([]*monitoring.Event, error) {
	evtsFile, err := os.Open(f.eventsFilePath) // open file for reading
	if err != nil {
		log.Errorf("failed to open events file {%s}, err: %v", f.eventsFilePath, err)
		return nil, err
	}

	defer evtsFile.Close()

	// move the file offset to given value; all the events before this offset will not be processed.
	_, err = evtsFile.Seek(offset, os.SEEK_SET)
	if err != nil {
		log.Debugf("failed to seek to the given offset {%v} in events file, err: %v", offset, err)
		return nil, err
	}

	// scan through each event
	evts := []*monitoring.Event{}
	scanner := bufio.NewScanner(evtsFile)
	for scanner.Scan() {
		evtStr := strings.TrimSpace(scanner.Text())
		temp := &monitoring.Event{}
		if err := json.Unmarshal([]byte(evtStr), temp); err != nil {
			log.Debugf("failed to unmarshal the read event, err: %v", err)
			continue
		}

		evts = append(evts, temp)
	}

	log.Debugf("events from offset {%v}: %v", offset, evts)
	log.Infof("total no.of.events found from offset {%v}: %v", offset, len(evts))
	return evts, nil
}

// Close closes the events file handler
func (f *fileImpl) Close() {
	f.handler.Close()
}
