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

// offsetTrackerImpl implements `OffsetTracker` interface. The exporters can use this tracker to
// update offset (filename, number of bytes read) once they're done processing list of events. Each writer gets
// a unique file to track the offset. The file gets updated with the new offset whenever the writer calls `UpdateOffset`.
type offsetTrackerImpl struct {
	sync.Mutex
	name           string // name of tracker; same as the writer
	offsetFilepath string // offset file Path
	logger         log.Logger
}

// newOffsetTracker create a new offset tracker by creating a file with the given name.
func newOffsetTracker(offsetDirPath, name string, logger log.Logger) (events.OffsetTracker, error) {
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
		logger:         logger,
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
		o.logger.Debugf("offset file {%s} exists already", o.offsetFilepath)
		return nil
	}

	handler, err := os.OpenFile(o.offsetFilepath, os.O_CREATE, 0755)
	if err != nil {
		o.logger.Errorf("failed to create offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}
	handler.Close()
	return nil
}

// UpdateOffset updates the offset to given value
func (o *offsetTrackerImpl) UpdateOffset(offset *events.Offset) error {
	o.Lock()
	defer o.Unlock()

	fh, err := os.OpenFile(o.offsetFilepath, os.O_RDWR, 0) // open file in read write mode
	if err != nil {
		o.logger.Errorf("failed to open the offset tracker file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	defer fh.Close() // close the file

	// write new offset to the file
	newData := []byte(fmt.Sprintf("%s %d", offset.Filename, offset.BytesRead))
	if _, err := fh.WriteAt(newData, 0); err != nil {
		o.logger.Errorf("could not write to offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	// truncate the file to discard any old entries
	if err := fh.Truncate(int64(len(newData))); err != nil {
		o.logger.Errorf("could not truncate the file after writing new data to offset file {%s}, err: %v", o.offsetFilepath, err)
		return err
	}

	return nil
}

// GetOffset reads the offset from the file and returns it
func (o *offsetTrackerImpl) GetOffset() (*events.Offset, error) {
	o.Lock()
	defer o.Unlock()
	offset := &events.Offset{}

	fh, err := os.Open(o.offsetFilepath) // open file for reading
	if err != nil {
		o.logger.Errorf("failed to open the offset tracker file {%s}, err: %v", o.offsetFilepath, err)
		return nil, err
	}

	defer fh.Close() // close the file

	fInfo, err := fh.Stat()
	if err != nil {
		o.logger.Errorf("failed to get file info {%s}, err: %v", o.offsetFilepath, err)
		return nil, err
	}
	if fInfo.Size() == 0 { // empty file
		return offset, nil
	}

	data := make([]byte, fInfo.Size())
	if _, err = fh.Read(data); err != nil {
		o.logger.Errorf("failed to read from offset tracker file, err: %v", err)
		return nil, err
	}

	dataStr := strings.TrimSpace(string(data))
	temp := strings.Split(dataStr, " ") // split by spaces (filename offset)
	if len(temp) == 2 {
		offset.Filename = temp[0]
		offset.BytesRead, err = strconv.ParseInt(temp[1], 10, 64)
		if err != nil {
			o.logger.Errorf("failed to parse offset from the file contents {%s}, err: %v", dataStr, err)
			return nil, err
		}

		o.logger.Debugf("last processed offset for %s: {%v}", o.name, offset)
	}

	return offset, nil
}

// Deletes the underlying file tracking the offset value.
// when user deletes the event policy, the offset file associated with the exporter should be deleted as well.
func (o *offsetTrackerImpl) Delete() error {
	return os.Remove(o.offsetFilepath)
}
