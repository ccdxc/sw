package recorder

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

func setup(t *testing.T) (string, *fileImpl) {
	tmpDir, err := ioutil.TempDir("", t.Name())
	AssertOk(t, err, "failed to create a temp dir, err: %v", err)

	eventsFile, err := newFile(tmpDir, t.Name()+CreateAlphabetString(5))
	AssertOk(t, err, "failed to create events file, err: %v", err)

	return tmpDir, eventsFile
}

// tests the recorder events file operations (read, write, rotate, etc.)
func TestFileStore(t *testing.T) {
	tmpDir, eventsFile := setup(t)
	defer os.RemoveAll(tmpDir)

	// rotate a empty events file
	err := eventsFile.Rotate()
	AssertOk(t, err, "rotate failed on empty events file, err: %v", err)

	// write event to the file
	evt := policygen.CreateEventObj(globals.DefaultTenant, globals.DefaultNamespace, "evt1", "DUMMY",
		"INFO", "test event -1")
	bEvt, err := json.Marshal(evt)
	eventsFile.Write(append(bEvt, '\n'))

	// rotate the file
	err = eventsFile.Rotate()
	AssertOk(t, err, "failed to rotate the events file, err: %v", err)

	// read events from the backed up files (from above rotation)
	evts, filePaths, err := eventsFile.GetEvents()
	AssertOk(t, err, "failed to get events from the .bak/* files, err: %v", err)
	Assert(t, len(filePaths) == 1, "expected only one evts file, got: %v", len(filePaths))
	Assert(t, len(evts) == 1, "expected only one event, got: %v", len(evts))
	Assert(t, reflect.DeepEqual(evt, evts[0]), "expected: %v, got: %v", evt, evts[0])
}

// tests the file rotate functionality
func TestFileStoreRotate(t *testing.T) {
	tmpDir, eventsFile := setup(t)

	// write event to the file
	evt := policygen.CreateEventObj(globals.DefaultTenant, globals.DefaultNamespace, "evt1", "DUMMY",
		"INFO", "test event -1")
	bEvt, err := json.Marshal(evt)
	eventsFile.Write(append(bEvt, '\n'))

	// remove the directory containing events file
	os.RemoveAll(tmpDir)

	err = eventsFile.Rotate()
	Assert(t, strings.Contains(err.Error(), "no such file or directory"), "expected {no such file or directory} error")
}

// tests the read functionality
func TestFileStoreGetEvents(t *testing.T) {
	tmpDir, eventsFile := setup(t)

	// write event to the file
	evt := policygen.CreateEventObj(globals.DefaultTenant, globals.DefaultNamespace, "evt1", "DUMMY",
		"INFO", "test event -1")
	bEvt, err := json.Marshal(evt)
	eventsFile.Write(append(bEvt, '\n'))

	err = eventsFile.Rotate()
	AssertOk(t, err, "failed to rotate events file, err: %v", err)
	// remove the directory containing events file (.bak folder)
	os.RemoveAll(tmpDir)

	evts, filepaths, err := eventsFile.GetEvents()
	Assert(t, err != nil && strings.Contains(err.Error(), "no such file or directory"), "expected {no such file or directory} error, got: %v", err)
	Assert(t, len(evts) == 0, "expected empty events list, got: %v", evts)
	Assert(t, len(filepaths) == 0, "expected empty file paths, got : %v", evts)

	// write invalid data to file and try reading it
	tmpDir, eventsFile = setup(t)
	defer os.RemoveAll(tmpDir)
	eventsFile.Write([]byte("invalid"))
	err = eventsFile.Rotate()
	AssertOk(t, err, "failed to rotate events file, err: %v", err)
	evts, filepaths, err = eventsFile.GetEvents()
	AssertOk(t, err, "failed to read events from .bak/* files, err: %v", err)
	Assert(t, len(evts) == 0, "expected: 0 events, got: %v", len(evts)) // invalid data(event) won't be returned
	Assert(t, len(filepaths) == 1, "expected: 1 file path, got: %v", len(filepaths))
}

// tests delete back files functionality
func TestFileStoreDeleteBackupFiles(t *testing.T) {
	tmpDir, eventsFile := setup(t)
	eventsFile.Write([]byte("test"))
	err := eventsFile.Rotate()
	AssertOk(t, err, "failed to rotate events file, err: %v", err)
	_, filepaths, err := eventsFile.GetEvents()
	AssertOk(t, err, "failed to read events from .bak/* files, err: %v", err)
	Assert(t, len(filepaths) == 1, "expected: 1 file path, got: %v", len(filepaths))
	os.RemoveAll(tmpDir)
	eventsFile.DeleteBackupFiles(filepaths)
}
