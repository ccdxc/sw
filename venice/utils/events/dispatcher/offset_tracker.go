// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"fmt"
	"os"
	"path"
	"strconv"
	"strings"
	"sync"

	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// offsetTrackerImpl implements `OffsetTracker` interface. The writers can use this tracker to
// update offset once they're done processing list of events. Each writer gets a unique file
// to track the offset. The file gets updated with the new offset whenever the writer calls `UpdateOffset`.
type offsetTrackerImpl struct {
	sync.Mutex
	name           string // name of tracker; same as the writer
	offsetFilepath string // offset file path
}

// newOffsetTracker create a new offset tracker by creating a file with the given name.
func newOffsetTracker(offsetDirPath, name string) (events.OffsetTracker, error) {
	// check if the directory exists; if not, create
	if _, err := os.Stat(offsetDirPath); os.IsNotExist(err) {
		log.Infof("creating offset folder %s", offsetDirPath)
		if mkdirErr := os.Mkdir(offsetDirPath, 0755); mkdirErr != nil {
			log.Errorf("failed to create offset folder %s, err: %v", offsetDirPath, err)
			return nil, err
		}
	}

	offsetTracker := &offsetTrackerImpl{
		name:           name,
		offsetFilepath: path.Join(offsetDirPath, name),
	}

	if err := offsetTracker.createIfNotExists(); err != nil {
		return nil, err
	}

	return offsetTracker, nil
}

// createIfNotExists create the file only if it not exists.
func (o *offsetTrackerImpl) createIfNotExists() error {
	o.Lock()
	defer o.Unlock()

	// check if the file exists
	if _, err := os.Stat(o.offsetFilepath); err == nil {
		log.Debugf("offset file {%s} exists already", o.offsetFilepath)
		return nil
	}

	handler, err := os.OpenFile(o.offsetFilepath, os.O_WRONLY|os.O_CREATE, 0755)
	if err != nil {
		log.Errorf("failed to open/create offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	defer handler.Close()

	// write offset value 0 to the file
	if _, err = handler.Write([]byte("0")); err != nil {
		log.Errorf("failed to write `0` to the created offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	return nil
}

// UpdateOffset updates the offset to given value
func (o *offsetTrackerImpl) UpdateOffset(newOffset int64) error {
	o.Lock()
	defer o.Unlock()

	fh, err := os.OpenFile(o.offsetFilepath, os.O_RDWR, 0) // open file in read write mode
	if err != nil {
		log.Errorf("failed to open the offset tracker file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	defer fh.Close() // close the file

	// write new offset to the file
	newData := []byte(fmt.Sprintf("%d", newOffset))
	if _, err := fh.WriteAt(newData, 0); err != nil {
		log.Errorf("could not write to offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	// truncate the file to discard any old entries
	if err := fh.Truncate(int64(len(newData))); err != nil {
		log.Errorf("could not truncate the file after writing new data to offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	return nil
}

// GetOffset reads the offset from the file and returns it
func (o *offsetTrackerImpl) GetOffset() (int64, error) {
	o.Lock()
	defer o.Unlock()

	fh, err := os.Open(o.offsetFilepath) // open file for reading
	if err != nil {
		log.Errorf("failed to open the offset tracker file {%s}, err: %v", o.offsetFilepath, err)
		return -1, err
	}

	defer fh.Close() // close the file

	fInfo, err := fh.Stat()
	if err != nil {
		log.Errorf("failed to get file info {%s}, err: %v", o.offsetFilepath, err)
		return -1, err
	}

	data := make([]byte, fInfo.Size())
	if _, err = fh.Read(data); err != nil {
		log.Errorf("failed to read from offset tracker file, err: %v", err)
		return -1, err
	}

	dataStr := strings.TrimSpace(string(data))
	offset, err := strconv.ParseInt(dataStr, 10, 64)
	if err != nil {
		log.Errorf("failed to parse offset from the file contents {%s}, err: %v", dataStr, err)
		return -1, err
	}

	return offset, nil
}
