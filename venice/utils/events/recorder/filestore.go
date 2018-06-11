// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
)

// FIXME: address file rotations

// fileImpl represents the recorders's backup file implementation.
type fileImpl struct {
	sync.Mutex
	name         string   // name of the recorder specific events file
	backupDir    string   // stores all the rotated files that're yet to be processed
	evtsFilePath string   // events file path
	handler      *os.File // handler to the events file
}

// newFile creates a new *fileImpl
func newFile(dir, filename string) (*fileImpl, error) {
	// create directory if it does not exists
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		if mkdirErr := os.MkdirAll(dir, 0755); mkdirErr != nil {
			log.Errorf("failed to create events store %s, err: %v", dir, err)
			return nil, err
		}
	}

	// open/create events file
	evtsFilePath := filepath.Join(dir, filename)
	handler, err := os.OpenFile(evtsFilePath, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0755)
	if err != nil {
		return nil, errors.Wrap(err, "failed to open/create recorder events file")
	}

	// seek to the end
	handler.Seek(0, os.SEEK_END)

	// make backup directory which stores all the rotated files that're yet to be sent to the proxy
	backupDir := filepath.Join(dir, fmt.Sprintf("%s.bak", filename))
	if err = os.MkdirAll(backupDir, 0755); err != nil {
		return nil, err
	}

	file := &fileImpl{
		name:         filename,
		backupDir:    backupDir,
		evtsFilePath: evtsFilePath,
		handler:      handler,
	}

	return file, nil
}

// Write writes given data to the underlying events file
func (f *fileImpl) Write(data []byte) error {
	f.Lock()
	defer f.Unlock()

	if _, err := f.handler.Write(data); err != nil {
		return err
	}

	return nil
}

// Rotate rotates the file by renaming and moving it to the ../*.bak/
func (f *fileImpl) Rotate() error {
	f.Lock()
	defer f.Unlock()

	// do not rotate empty events file
	if fInfo, err := f.handler.Stat(); err != nil {
		log.Debugf("failed to get the events file stat, err: %v", err)
	} else if fInfo.Size() == 0 {
		log.Debugf("empty events file, no rotation needed")
		return nil
	}

	log.Debugf("rotating events file")

	var err error
	// rename the file; so that any incoming events will not be blocked while processing these events
	if err = os.Rename(f.evtsFilePath,
		filepath.Join(f.backupDir, fmt.Sprintf("%s.%s", f.name, time.Now().Format(time.RFC3339)))); err != nil {
		return err
	}

	// create new events file and handler
	f.handler, err = os.OpenFile(f.evtsFilePath, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0755)
	if err != nil {
		return err
	}

	return nil
}

// GetEvents iterates over all the files in ../*.bak/ and reads the events
// TODO: send events in batches to avoid piling events in memory
func (f *fileImpl) GetEvents() ([]*monitoring.Event, []string, error) {
	files, err := ioutil.ReadDir(f.backupDir)
	if err != nil {
		log.Errorf("failed to read files from backup directory, err: %v", err)
		return nil, nil, err
	}

	// get the filenames from file info
	filenames := []string{}
	for _, file := range files {
		filenames = append(filenames, file.Name())
	}

	// sort filenames to be in chronological (time sorted)
	sort.Strings(filenames)

	evts := []*monitoring.Event{}
	for i, filename := range filenames {
		fp := filepath.Join(f.backupDir, filename)
		evtsFile, err := os.Open(fp) // open file for reading
		if err != nil {
			log.Errorf("failed to open events file {%s}, err: %v", fp, err)
			return nil, nil, err
		}

		scanner := bufio.NewScanner(evtsFile)
		for scanner.Scan() {
			evtStr := strings.TrimSpace(scanner.Text())
			temp := &monitoring.Event{}
			if err := json.Unmarshal([]byte(evtStr), temp); err != nil {
				log.Debugf("failed to unmarshal the read events, err: %v", err)
				continue
			}

			evts = append(evts, temp)
		}

		// update absolute file path
		filenames[i] = fp
	}

	return evts, filenames, nil
}

// DeleteBackupFiles deletes the files identified by the given names
func (f *fileImpl) DeleteBackupFiles(filenames []string) {
	for _, fp := range filenames {
		if err := os.Remove(fp); err != nil {
			log.Errorf("failed to delete file %s, err: %v", fp, err)
		}
	}
}
