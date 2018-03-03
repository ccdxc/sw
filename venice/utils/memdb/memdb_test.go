// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"fmt"
	"runtime"
	"testing"
	"time"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type testObj struct {
	api.TypeMeta
	api.ObjectMeta
	field string
}

func TestMemdbAddDelete(t *testing.T) {
	// create a new memdb
	md := NewMemdb()

	// tets object
	obj := testObj{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		field: "testField",
	}

	// add an object
	err := md.AddObject(&obj)
	AssertOk(t, err, "Error creating object")

	// verify find works
	fobj, err := md.FindObject("testObj", &obj.ObjectMeta)
	AssertOk(t, err, "Error finding the object")
	AssertEquals(t, obj.Name, fobj.GetObjectMeta().Name, "Found invalid object")
	switch tp := fobj.(type) {
	case *testObj:
		tobj := fobj.(*testObj)
		Assert(t, (tobj.field == obj.field), "Got invalid object", tobj)
	default:
		t.Fatalf("Found object of invalid type: %v", tp)
	}

	// verify list works
	objs := md.ListObjects("testObj")
	Assert(t, (len(objs) == 1), "List returned incorrect number of objs", objs)
	Assert(t, (objs[0].GetObjectMeta().Name == obj.Name), "Got invalid object", objs)

	// verify we can't update an object that doesn't exist
	newObj := obj
	newObj.Name = "testName2"
	err = md.UpdateObject(&newObj)
	Assert(t, (err != nil), "Updating non-existing object did not fail", newObj)

	// verify delete works
	err = md.DeleteObject(&obj)
	AssertOk(t, err, "Error deleting the object")

	// verify object is not found after delete
	_, err = md.FindObject("testObj", &obj.ObjectMeta)
	Assert(t, (err != nil), "Object found when expecting error", md)
	objs = md.ListObjects("testObj")
	Assert(t, (len(objs) == 0), "List returned incorrect number of objs", objs)
}

// waitForWatch wait for a watch event or timeout
func waitForWatch(t *testing.T, watchChan chan Event, et EventType) Object {
	// verify we get a watch event
	select {
	case evt, ok := <-watchChan:
		Assert(t, ok, "Error reading from channel", evt)
		logrus.Infof("Received watch event {%+v} obj {%+v}", evt, evt.Obj.GetObjectMeta())
		AssertEquals(t, evt.EventType, et, "Received incorrect event type")
		return evt.Obj
	case <-time.After(10 * time.Millisecond):
		t.Fatalf("Timed out while waiting for channel event")
	}

	return nil
}

// verifyObjField verifies field val
func verifyObjField(t *testing.T, fobj Object, fieldVal string) {
	switch tp := fobj.(type) {
	case *testObj:
		tobj := fobj.(*testObj)
		Assert(t, (tobj.field == fieldVal), "Got invalid object", tobj)
	default:
		t.Fatalf("Found object of invalid type: %v", tp)
	}
}

func TestMemdbWatch(t *testing.T) {
	// create a new memdb
	md := NewMemdb()

	// test object
	obj := testObj{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		field: "testField",
	}

	// start watch on objects
	watchChan := make(chan Event, WatchLen)
	md.WatchObjects("testObj", watchChan)

	// add an object
	err := md.AddObject(&obj)
	AssertOk(t, err, "Error creating object")

	// verify we get a watch event
	wobj := waitForWatch(t, watchChan, CreateEvent)
	Assert(t, (wobj.GetObjectMeta().Name == obj.Name), "Received invalid object", wobj)
	verifyObjField(t, wobj, "testField")

	// verify duplicate add results in update
	err = md.AddObject(&obj)
	AssertOk(t, err, "Error adding duplicate object")
	wobj = waitForWatch(t, watchChan, UpdateEvent)

	// update the object
	newObj := obj
	newObj.field = "updatedField"
	err = md.UpdateObject(&newObj)
	AssertOk(t, err, "Error updating object")

	// verify we received the update
	wobj = waitForWatch(t, watchChan, UpdateEvent)
	verifyObjField(t, wobj, "updatedField")

	// delete the object
	err = md.DeleteObject(&obj)
	AssertOk(t, err, "Error deleting object")
	wobj = waitForWatch(t, watchChan, DeleteEvent)
	Assert(t, (wobj.GetObjectMeta().Name == obj.Name), "Received invalid object", wobj)

	// verify we cant delete non-existing object
	err = md.DeleteObject(&obj)
	Assert(t, (err != nil), "Deleting non-existing object was successful")
}

func TestMemdbConcurrency(t *testing.T) {
	// concurrency parameters
	const watchConcur = 200
	const objConcur = 200
	runtime.GOMAXPROCS(32)
	objErrChan := make(chan error, (objConcur + 1))
	watchDoneChan := make(chan Event, ((watchConcur + 1) * (objConcur + 1)))

	// create a new memdb
	md := NewMemdb()

	// test object
	obj := testObj{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		field: "testField",
	}

	// setup concurrent watches
	watchers := make([]chan Event, watchConcur)
	for i := 0; i < watchConcur; i++ {
		watchChan := make(chan Event, (objConcur + 100))
		err := md.WatchObjects("testObj", watchChan)
		AssertOk(t, err, "Error creating watcher")
		watchers[i] = watchChan
	}

	// add objects concurrently
	for i := 0; i < objConcur; i++ {
		go func(objId int) {
			newObj := obj
			newObj.Name = fmt.Sprintf("%s-%d", obj.Name, objId)
			err := md.AddObject(&newObj)
			objErrChan <- err
		}(i)
	}

	// watch concurrently
	for i := 0; i < watchConcur; i++ {
		go func(wid int) {
			for objID := 0; objID < objConcur; objID++ {
				evt, ok := <-watchers[wid]
				if !ok {
					logrus.Errorf("Error receiving from channel %d", wid)
					close(watchDoneChan)
				}
				watchDoneChan <- evt

			}
		}(i)
	}

	// wait for adds to complete
	for i := 0; i < objConcur; i++ {
		err := <-objErrChan
		AssertOk(t, err, "Error creating object")
	}

	logrus.Infof("Created %d objects concurrently", objConcur)

	// wait for watches to complete
	for i := 0; i < watchConcur*objConcur; i++ {
		evt, ok := <-watchDoneChan
		Assert(t, ok, "Error receiving from watch channel")
		AssertEquals(t, evt.EventType, CreateEvent, "Invalid event received")
	}

	logrus.Infof("Received %d create events successfully", watchConcur*objConcur)

	// verify all objects were created
	objs := md.ListObjects("testObj")
	Assert(t, (len(objs) == objConcur), "Incorrect number of objects got created", objs)

	// add objects concurrently
	for i := 0; i < objConcur; i++ {
		go func(objId int) {
			newObj := obj
			newObj.Name = fmt.Sprintf("%s-%d", obj.Name, objId)
			err := md.DeleteObject(&newObj)
			objErrChan <- err
		}(i)
	}

	// wait for deletes to complete
	for i := 0; i < objConcur; i++ {
		err := <-objErrChan
		AssertOk(t, err, "Error creating object")
	}

	// verify all of them were deleted
	objs = md.ListObjects("testObj")
	Assert(t, (len(objs) == 0), "Some objects were not deleted", objs)
}

func TestStopWatchObjects(t *testing.T) {
	numWatchers := 10

	// create a new memdb
	md := NewMemdb()

	// test object
	obj := testObj{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		field: "testField",
	}

	// setup watchers
	watchers := map[int]chan Event{}
	for i := 0; i < numWatchers; i++ {
		watchers[i] = make(chan Event, 2)
		err := md.WatchObjects("testObj", watchers[i])
		AssertOk(t, err, "Error creating watcher")
	}
	err := md.AddObject(&obj)
	AssertOk(t, err, "error creating object")

	for _, w := range watchers {
		_ = waitForWatch(t, w, CreateEvent)
	}

	for i := 0; i < numWatchers; i++ {
		md.StopWatchObjects(obj.GetKind(), watchers[i])
		delete(watchers, i)
		storedObj := md.getObjdb(obj.GetKind())
		// check number of channels
		Assert(t, len(storedObj.watchers) == (numWatchers-i-1),
			fmt.Sprintf("mismatch in number of watchers, expected %d (got %d) %+v",
				numWatchers-i-1, len(storedObj.watchers), storedObj))

		err := md.UpdateObject(&obj)
		AssertOk(t, err, "error updating object")

		for _, w := range watchers {
			_ = waitForWatch(t, w, UpdateEvent)
		}
	}
}
