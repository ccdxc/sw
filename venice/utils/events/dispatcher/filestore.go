// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
	memcache "github.com/pensando/sw/venice/utils/cache"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	maxFileSize = 10 * 1000 * 1000 // 10mb
	maxNumFiles = 5                // up to 5 events files
	fileMode    = 0755
)

//
// File rotation
// 	- Files are rotated based on events.StoreConfig policy {maxSize and maxNumFiles}.
//	- Whenever the current file exceeds the given max size, the rotation happens. During rotation,
// 		1. New file will be create for storing the future events.
// 		2. Old files will be pruned. At any point there will only be `maxNumFiles` number of event files.
// 	- Files names are unix timestamps to make the sorting easy.
//

// File implements `PersistentStore`. Any incoming event will be written to the persistent store before it
// could be deduped and batched.
type fileImpl struct {
	storePath         string      // file store directory
	eventFiles        *eventFiles // list of event files that includes rotated + current file
	currentEventsFile *currentEventsFile
	logger            log.Logger
	maxFileSize       int64
	maxNumFiles       int
}

// currentEventsFile represents the current file acting as a persistent store for incoming events.
// Events will be written to this file before getting deduped or cached.
type currentEventsFile struct {
	sync.RWMutex
	name     string
	size     int64 // current size of the file
	*os.File       // file handler
}

// eventFiles list of available event files as per events.StoreConfig.MaxNumFiles.
// This is a sorted list with most recent file on the top.
type eventFiles struct {
	sync.RWMutex
	list []string
}

// newPersistentStore reads all the available event files in the given config.Dir and opens the most recent one or
// creates a new file if there are no event files in config.Dir.
func newPersistentStore(config *events.StoreConfig, logger log.Logger) (events.PersistentStore, error) {
	fi := &fileImpl{
		storePath:   config.Dir,
		maxFileSize: config.MaxFileSize,
		maxNumFiles: config.MaxNumFiles,
		eventFiles:  &eventFiles{},
		logger:      logger}
	if fi.maxFileSize == 0 {
		fi.maxFileSize = maxFileSize
	}
	if fi.maxNumFiles == 0 {
		fi.maxNumFiles = maxNumFiles
	}

	// create store directory if it does not exists
	if _, err := os.Stat(fi.storePath); os.IsNotExist(err) {
		if mkdirErr := os.MkdirAll(fi.storePath, fileMode); mkdirErr != nil {
			fi.logger.Errorf("failed to create events store %s, err: %v", fi.storePath, err)
			return nil, err
		}
	}

	// reload existing event files if there are any
	fi.reloadEventsFileList()

	// open/create events file
	filename := fi.getExistingOrNewFilename()
	handler, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE|os.O_APPEND, fileMode)
	if err != nil {
		return nil, errors.Wrap(err, "failed to open/create events file")
	}

	// seek to the end
	handler.Seek(0, os.SEEK_END)

	fileStat, err := os.Stat(filename)
	if err != nil {
		return nil, errors.Wrap(err, "failed to open/create events file")
	}
	fi.currentEventsFile = &currentEventsFile{name: filename, size: fileStat.Size(), File: handler}
	fi.logger.Debugf("list of event files available during persistent store init: %v", fi.eventFiles.list)
	return fi, nil
}

// Write performs rotation if needed and writes the given data to underlying events file.
// Returns a bool value indicating whether there was rotation of not.
func (f *fileImpl) Write(data []byte) (bool, error) {
	f.currentEventsFile.Lock()
	defer f.currentEventsFile.Unlock()

	dataLen := int64(len(data))
	var fileRotated bool
	if f.currentEventsFile.size+dataLen > f.maxFileSize {
		fileRotated = true
		if err := f.rotate(); err != nil {
			return false, errors.Wrap(err, "failed to rotate events file")
		}
	}

	if _, err := f.currentEventsFile.Write(data); err != nil {
		return fileRotated, errors.Wrap(err, "failed to write the given event")
	}

	f.currentEventsFile.size += dataLen
	return fileRotated, nil
}

// GetStorePath returns the Path of the events store.
func (f *fileImpl) GetStorePath() string {
	return f.storePath
}

// GetCurrentOffset returns the current offset (position) of the events file.
func (f *fileImpl) GetCurrentOffset() (*events.Offset, error) {
	f.currentEventsFile.RLock()
	defer f.currentEventsFile.RUnlock()
	bytesRead, err := f.currentEventsFile.Seek(0, os.SEEK_CUR)
	if err != nil {
		return nil, err
	}

	return &events.Offset{Filename: f.currentEventsFile.name, BytesRead: bytesRead}, nil
}

// GetEventsFromOffset returns the list of events starting from the given offset.
// offset helps to navigate the file without having to read all the file contents.
// It is possible that there could be some events files that were created after offset.Filename. So, this
// function is responsible for reading events from all those files.
func (f *fileImpl) GetEventsFromOffset(offset *events.Offset) ([]*evtsapi.Event, error) {
	f.eventFiles.Lock()
	defer f.eventFiles.Unlock()

	if utils.IsEmpty(offset.Filename) {
		return nil, fmt.Errorf("invalid offset, missing filename")
	}

	// get the list of event files created after offset.Filename
	files := f.getListOfFilesCreatedAfter(offset.Filename)

	dedupCache := memcache.New(memcache.NoExpiration, 0, nil)
	defer dedupCache.Flush() // delete all the items from the cache

	// read and dedup events from offset.Filename; it is possible that this could be deleted as part of the
	// pruning (maxNumFiles). So, ignore file not found error. In this case, the exporter will not receive those events
	// that were missed.
	if err := f.dedupEventsFromFile(offset.Filename, offset.BytesRead, dedupCache); err != nil && !os.IsNotExist(err) {
		return nil, err
	}

	// read and dedup events from files that were created after offset.Filename
	for _, file := range files {
		if err := f.dedupEventsFromFile(file, 0, dedupCache); err != nil && !os.IsNotExist(err) {
			return nil, err
		}
	}

	var evts []*evtsapi.Event
	for _, evt := range dedupCache.Items() {
		temp, ok := evt.(evtsapi.Event)
		if !ok {
			continue
		}
		evts = append(evts, &temp)
	}

	f.logger.Debugf("events from offset {%v: %v}: %v", offset.Filename, offset.BytesRead, evts)
	f.logger.Infof("total no.of.events found from offset {%v: %v}: %v", offset.Filename, offset.BytesRead, len(evts))
	return evts, nil
}

// dedupEventsFromFile reads and dedups events from `filename` and from offset `bytesRead`.
func (f *fileImpl) dedupEventsFromFile(filename string, bytesRead int64, dedupCache memcache.Cache) error {
	if _, err := os.Stat(filename); os.IsNotExist(err) {
		f.logger.Debugf("file {%s} does not exists", filename)
		return err
	}

	evtsFile, err := os.Open(filename) // open file for reading
	if err != nil {
		return errors.Wrap(err, "failed to open file")
	}

	defer evtsFile.Close()

	// move the file offset to given value; all the events before this offset will not be processed.
	_, err = evtsFile.Seek(bytesRead, os.SEEK_SET)
	if err != nil {
		return errors.Wrap(err, fmt.Sprintf("failed to seek to the given offset {%v: %v} in events file", filename, bytesRead))
	}

	// scan through each event
	scanner := bufio.NewScanner(evtsFile)
	for scanner.Scan() {
		evtStr := strings.TrimSpace(scanner.Text())
		temp := &evtsapi.Event{}
		if err := json.Unmarshal([]byte(evtStr), temp); err != nil {
			f.logger.Debugf("failed to unmarshal the read event, err: %v", err)
			continue
		}

		evt := *temp
		hashKey := events.GetEventKey(temp)
		if existingEvt, ok := dedupCache.Get(hashKey); ok {
			evt = existingEvt.(evtsapi.Event)
			// update count and timestamp
			timestamp, _ := types.TimestampProto(time.Now())
			evt.EventAttributes.Count++
			evt.ObjectMeta.ModTime.Timestamp = *timestamp
		}
		dedupCache.Add(hashKey, evt)
	}
	return nil
}

// returns the oldest filename from the available event files
func (f *fileImpl) GetOldestFilename() string {
	f.eventFiles.RLock()
	defer f.eventFiles.RUnlock()

	files := f.eventFiles.list
	sort.Strings(files)
	if len(files) > 0 {
		return files[0]
	}
	return ""
}

// GetFilename returns the current events file name
func (f *fileImpl) GetFilename() string {
	f.currentEventsFile.RLock()
	defer f.currentEventsFile.RUnlock()

	return f.currentEventsFile.name
}

// Close closes the current events file handler
func (f *fileImpl) Close() {
	f.currentEventsFile.Lock()
	defer f.currentEventsFile.Unlock()

	f.currentEventsFile.Close()
}

// the filename to be opened as part of `newPer...` returns the most recent events file or
// a new file name (which is a timestamp).
func (f *fileImpl) getExistingOrNewFilename() string {
	f.eventFiles.Lock()
	defer f.eventFiles.Unlock()

	// check the available list and return the recent one
	if len(f.eventFiles.list) == 0 {
		f.eventFiles.list = []string{f.filename()}
	}

	return f.eventFiles.list[0] // most recent
}

// reloads the eventFiles.list with the available event files from f.storePath. Also, it prunes the old files.
func (f *fileImpl) reloadEventsFileList() {
	f.eventFiles.Lock()
	defer f.eventFiles.Unlock()

	f.eventFiles.list = f.listEventFilesFromStoreInOrder()
	f.removeOldFiles()
}

// returns the sorted list of event files from f.storePath
func (f *fileImpl) listEventFilesFromStoreInOrder() []string {
	var eventFiles []string
	files, err := ioutil.ReadDir(f.storePath)
	if err != nil {
		f.logger.Errorf("failed to list event files from store {%s}, err: %v", f.storePath, err)
	}
	for _, file := range files {
		if file.IsDir() {
			continue
		}
		eventFiles = append(eventFiles, path.Join(f.storePath, file.Name()))
	}

	// since the file names are timestamps, reverse sorting it by name gives the files in the order of creation (from recent to least).
	return f.reverseSort(eventFiles)
}

// returns the list of files created the given filename (timestamp).
func (f *fileImpl) getListOfFilesCreatedAfter(filename string) []string {
	files := f.eventFiles.list
	sort.Strings(files)
	for i, file := range files {
		if file == filename {
			return files[i+1:]
		}
	}

	return files
}

// archives old events file and creates a new one
func (f *fileImpl) rotate() error {
	f.logger.Infof("rotating events file {%s}", f.currentEventsFile.name)
	if err := f.currentEventsFile.Close(); err != nil { // close the old handler
		return err
	}

	// create a new file
	filename := f.filename()
	handler, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE, fileMode)
	if err != nil {
		return err
	}

	f.currentEventsFile.File = handler
	f.currentEventsFile.name = filename
	f.currentEventsFile.size = 0

	f.eventFiles.Lock()
	f.eventFiles.list = f.reverseSort(append(f.eventFiles.list, filename))
	f.removeOldFiles()
	f.eventFiles.Unlock()

	return nil
}

// prunes old event files from f.eventFiles.list
func (f *fileImpl) removeOldFiles() {
	if len(f.eventFiles.list) > f.maxNumFiles {
		f.logger.Infof("removing stale event files: %v", f.eventFiles.list[f.maxNumFiles:]) // f.maxNumFiles ... len(f.eventFiles.list)
		for i := f.maxNumFiles; i < len(f.eventFiles.list); i++ {
			os.Remove(f.eventFiles.list[i])
		}
		f.eventFiles.list = f.eventFiles.list[:f.maxNumFiles] // 0 ... (f.maxNumFiles-1)
	}
}

// helper function to revere sort (desc.) the given list of strings
func (f *fileImpl) reverseSort(items []string) []string {
	sort.Sort(sort.Reverse(sort.StringSlice(items)))
	return items
}

// helper function to return the filename (timestamp)
func (f *fileImpl) filename() string {
	return path.Join(f.storePath, fmt.Sprintf("%d", time.Now().UnixNano()))
}
