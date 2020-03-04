// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"runtime"
	"strconv"
	"strings"
	"sync"

	"testing"
	"time"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb/objReceiver"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type testObj struct {
	api.TypeMeta
	api.ObjectMeta
	field     string
	depMap    map[string]*apiintf.ReferenceObj
	revDepMap map[string]*apiintf.ReferenceObj
}

var (
	testObjStore map[string][]testObj
)

func (obj *testObj) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {
	for key, value := range obj.depMap {
		value := value
		resp[key] = *value
	}
}

func (obj *testObj) Referrers(tenant string, path string, resp map[string]apiintf.ReferenceObj) {
	for key, value := range obj.revDepMap {
		value := value
		resp[key] = *value
	}
}

func getObjectName(kind string, i int) string {
	return "test-" + kind + "-" + strconv.Itoa(i)
}

func generateObjects(kind string, numOfObjects int) []testObj {
	objs := []testObj{}
	for i := 0; i < numOfObjects; i++ {
		obj := testObj{
			TypeMeta: api.TypeMeta{Kind: kind},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant",
				Name:   getObjectName(kind, i),
			},
			depMap:    make(map[string]*apiintf.ReferenceObj),
			revDepMap: make(map[string]*apiintf.ReferenceObj),
		}

		objs = append(objs, obj)
	}
	return objs
}

type reference struct {
	kind            string
	numberOfObjects int
}
type objReference struct {
	obj  testObj
	refs []reference
}

type relation struct {
	sKind string
	dKind string
	start int
	end   int
}

func initObjectStore() {
	testObjStore = make(map[string][]testObj)

	aObjects := generateObjects("a", 100)
	testObjStore["a"] = aObjects
	bObjects := generateObjects("b", 100)
	testObjStore["b"] = bObjects
	cObjects := generateObjects("c", 100)
	testObjStore["c"] = cObjects
	dObjects := generateObjects("d", 100)
	testObjStore["d"] = dObjects
}

type doOperation func(key string, obj Object, refs map[string]apiintf.ReferenceObj) error

func sendObjects(op doOperation, kind string, start, end int) error {
	// add an object
	if start == 0 && end == 0 {
		end = len(testObjStore[kind])
	}
	objs := testObjStore[kind][start:end]
	errs := make(chan error, len(objs))
	for _, obj := range objs {
		obj := obj
		go func() {
			refs := make(map[string]apiintf.ReferenceObj)
			obj.References(obj.GetObjectMeta().GetTenant(), "", refs)
			errs <- op(obj.GetObjectMeta().GetKey(), &obj, refs)
		}()
	}

	for ii := 0; ii < len(objs); ii++ {
		err := <-errs
		if err != nil {
			return err
		}
	}
	return nil
}

type doPushOperation func(key string, obj Object, refs map[string]apiintf.ReferenceObj, recvrs []objReceiver.Receiver) (PushObjectHandle, error)
type doPushObjOperation func(key string, obj Object, refs map[string]apiintf.ReferenceObj) error

func sendPushObjects(op doPushOperation, kind string, start, end int, recvrs []objReceiver.Receiver) ([]PushObjectHandle, error) {
	// add an object
	if start == 0 && end == 0 {
		end = len(testObjStore[kind])
	}
	objs := testObjStore[kind][start:end]
	errs := make(chan error, len(objs))
	handles := make([]PushObjectHandle, len(objs))
	for index, obj := range objs {
		obj := obj
		index := index
		go func() {
			refs := make(map[string]apiintf.ReferenceObj)
			obj.References(obj.GetObjectMeta().GetTenant(), "", refs)
			handle, err := op(obj.GetObjectMeta().GetKey(), &obj, refs, recvrs)
			errs <- err
			handles[index] = handle
		}()
	}

	for ii := 0; ii < len(objs); ii++ {
		err := <-errs
		if err != nil {
			return handles, err
		}
	}
	return handles, nil
}

func doUpdateObjectOp(pobjs []PushObjectHandle) error {
	// add an object
	errs := make(chan error, len(pobjs))
	for _, pobj := range pobjs {
		pobj := pobj
		go func() {
			refs := make(map[string]apiintf.ReferenceObj)
			obj := pobj.Object().(*testObj)
			obj.References(pobj.Object().GetObjectMeta().GetTenant(), "", refs)
			err := pobj.UpdateObjectWithReferences(obj.GetObjectMeta().GetKey(), obj, refs)
			errs <- err
		}()
	}

	for ii := 0; ii < len(pobjs); ii++ {
		err := <-errs
		if err != nil {
			return err
		}
	}
	return nil
}

func doDeleteObjectOp(pobjs []PushObjectHandle) error {
	// add an object
	errs := make(chan error, len(pobjs))
	for _, pobj := range pobjs {
		pobj := pobj
		go func() {
			refs := make(map[string]apiintf.ReferenceObj)
			obj := pobj.Object().(*testObj)
			obj.References(pobj.Object().GetObjectMeta().GetTenant(), "", refs)
			err := pobj.DeleteObjectWithReferences(obj.GetObjectMeta().GetKey(), obj, refs)
			errs <- err
		}()
	}

	for ii := 0; ii < len(pobjs); ii++ {
		err := <-errs
		if err != nil {
			return err
		}
	}
	return nil
}

func addReferences(obj testObj, refs []testObj) {
	for _, ref := range refs {
		if _, ok := obj.depMap[ref.TypeMeta.Kind]; !ok {
			obj.depMap[ref.TypeMeta.Kind] = &apiintf.ReferenceObj{}
		}
		entry, _ := obj.depMap[ref.TypeMeta.Kind]
		entry.Refs = append(entry.Refs, ref.GetObjectMeta().GetKey())
		entry.RefKind = ref.TypeMeta.Kind

		if _, ok := ref.revDepMap[obj.TypeMeta.Kind]; !ok {
			ref.revDepMap[obj.TypeMeta.Kind] = &apiintf.ReferenceObj{}
		}
		entry, _ = ref.revDepMap[obj.TypeMeta.Kind]
		entry.Refs = append(entry.Refs, obj.GetObjectMeta().GetKey())
		entry.RefKind = ref.TypeMeta.Kind
	}
}

// waitForWatch wait for a watch event or timeout
func waitForWatch(t *testing.T, watchChan chan Event) (Event, error) {
	// verify we get a watch event
	select {
	case evt, ok := <-watchChan:
		Assert(t, ok, "Error reading from channel", evt)
		logrus.Infof("Received watch event {%+v} obj {%+v} %v", evt, evt.Obj.GetObjectMeta(), len(watchChan))
		return evt, nil
	case <-time.After(100 * time.Millisecond):
		return Event{}, errors.New("Timed out while waiting for channel event")
	}

}

func generateObjectReferences(relations []relation) {

	for _, rel := range relations {
		sObjects := testObjStore[rel.sKind]
		dObjects := testObjStore[rel.dKind]
		for _, sobject := range sObjects {
			addReferences(sobject, dObjects[rel.start:rel.end])
		}
	}
}

func verifyObjects(t *testing.T, memdb *Memdb, watcher *Watcher, evKindMap map[EventType]map[string]int,
	duration time.Duration) error {

	timedOutEvent := time.After(duration)

	for true {
		eventObj, err := waitForWatch(t, watcher.Channel)
		fmt.Printf("Received objects...%v\n", eventObj.Obj)
		wobj := eventObj.Obj
		if err == nil {
			kindMap, ok := evKindMap[eventObj.EventType]
			if !ok {
				return errors.New("Unexpected event type object recived")
			}
			_, ok = kindMap[wobj.GetObjectKind()]
			if !ok {
				return errors.New("Unexpected object received")
			}
			kindMap[wobj.GetObjectKind()]--
			refs := make(map[string]apiintf.ReferenceObj)
			node := memdb.objGraph.References(wobj.GetObjectMeta().GetKey())
			fmt.Printf("Checking obj %+v\n", eventObj)
			if eventObj.EventType == CreateEvent {
				wobj.(*testObj).References(wobj.GetObjectMeta().GetTenant(), "", refs)
				kindMap, ok = evKindMap[DeleteEvent]
				deleteExpect := 0
				if ok {
					deleteExpect = kindMap[wobj.GetObjectKind()]
				}
				Assert(t, deleteExpect != 0 || len(refs) == 0 || node != nil, "Add node not found")
			} else if eventObj.EventType == UpdateEvent {
			} else {
				fmt.Printf("Node refs %+v\n", node)
				kindMap, ok = evKindMap[CreateEvent]
				createExpect := 0
				if ok {
					createExpect = kindMap[wobj.GetObjectKind()]
				}

				Assert(t, createExpect != 0 || node == nil || len(node.Refs) == 0, "Delete node still has references")
				wobj.(*testObj).Referrers(wobj.GetObjectMeta().GetTenant(), "", refs)
			}
			for _, ref := range refs {
				for _, objKey := range ref.Refs {
					parts := strings.Split(objKey, "/")
					objMeta := api.ObjectMeta{Tenant: parts[0],
						Namespace: parts[1], Name: parts[2]}
					_, err := memdb.FindObject(ref.RefKind, &objMeta)
					if eventObj.EventType == CreateEvent {
						//fmt.Printf("Create error %v\n", eventObj.Obj)
						kindMap, ok = evKindMap[DeleteEvent]
						deleteExpect := 0
						if ok {
							deleteExpect = kindMap[ref.RefKind]
						}
						Assert(t, deleteExpect != 0 || err == nil, "Error Finding object object")
					} else if eventObj.EventType == UpdateEvent {
					} else {
						kindMap, ok = evKindMap[CreateEvent]
						createExpect := 0
						if ok {
							createExpect = kindMap[ref.RefKind]
						}
						Assert(t, createExpect != 0 || err != nil, "found referring object still")

					}
				}
			}
		}

		if len(evKindMap) != 0 {
			allObjectsReceived := true
		L:
			for _, kindMap := range evKindMap {
				for _, count := range kindMap {
					if count != 0 {
						allObjectsReceived = false
						break L
					}
				}

			}
			//all objects received.
			if allObjectsReceived {
				return nil
			}
			select {
			case <-timedOutEvent:
				return errors.New("Not received")
			default:

			}
		} else {
			//No objects expected, return true if duration is done.
			select {
			case <-timedOutEvent:
				return nil
			default:

			}
		}

	}

	return nil
}

func registerKinds(md *Memdb) {

	md.RegisterKind("a")
	md.RegisterKind("b")
	md.RegisterKind("c")
	md.RegisterKind("d")
	md.RegisterKind("e")
	md.RegisterKind("f")
}

func TestMemdbDepAddTest_1(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(1*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 1
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(1*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbDepAddTest_2(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 1},
		{"a", "c", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 1)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 1
	kindMap[CreateEvent]["c"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepAddTest_3(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 1},
		{"b", "c", 0, 1},
		{"c", "d", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "d", 0, 1)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 1
	kindMap[CreateEvent]["c"] = 1
	kindMap[CreateEvent]["b"] = 1
	kindMap[CreateEvent]["d"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepAddTest_4(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 50)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 50, 99)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 49
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 99, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["c"] = 1
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepAddTest_5(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
		{"c", "d", 0, 100},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "d", 0, 0)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepAddTest_6(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbDepDelTest_1(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 1},
	}

	///
	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 1
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 1
	kindMap[DeleteEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelTest_2(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 10},
		{"a", "c", 0, 10},
	}

	///
	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 10)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 10)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 10)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 10
	kindMap[CreateEvent]["b"] = 10
	kindMap[CreateEvent]["c"] = 10
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 10)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 10)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 10)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 10
	kindMap[DeleteEvent]["b"] = 10
	kindMap[DeleteEvent]["c"] = 10
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelTest_3(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
		{"c", "d", 0, 100},
	}

	///
	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "d", 0, 100)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[CreateEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "d", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	kindMap[DeleteEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelTest_4(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"a", "c", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 50)
	AssertOk(t, err, "Error deleting object")
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 50
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 50, 99)
	AssertOk(t, err, "Error deleting object")
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 49
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 99, 100)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 1
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelTest_5(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
		{"c", "d", 0, 100},
	}

	///
	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "d", 0, 100)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[CreateEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "d", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error deleting object")
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error deleting object")
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["c"] = 100
	kindMap[DeleteEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelTest_6(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error deleting object")
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["b"] = 100

	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbAddDelete(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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
	objs := md.ListObjects("testObj", nil)
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
	objs = md.ListObjects("testObj", nil)
	Assert(t, (len(objs) == 0), "List returned incorrect number of objs", objs)
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

func TestMemdbDepDelAddTest_1(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 1
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Now lets send add of b kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 1)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 1
	kindMap[DeleteEvent]["b"] = 1
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(50*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelAddTest_2(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error deleting object")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Now lets send add of b kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Now lets send add of c kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDepDelAddTest_3(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
		{"c", "d", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "d", 0, 0)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[CreateEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error deleting object")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error deleting object")

	err = sendObjects(md.DeleteObjectWithReferences, "d", 0, 0)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Now lets send add of b kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Now lets send add of c kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Now lets send add of d kind as they are still in marked for delete state
	err = sendObjects(md.AddObjectWithReferences, "d", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error deleting object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	kindMap[DeleteEvent]["d"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[CreateEvent]["d"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(200*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbAddUpdateTest_1(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 50},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Send 50-100
	err = sendObjects(md.AddObjectWithReferences, "c", 50, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()
	relations = []relation{
		{"a", "b", 0, 1},
		{"b", "c", 50, 100},
	}
	generateObjectReferences(relations)
	//Send 50-100
	err = sendObjects(md.UpdateObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbAddUpdateTest_2(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error deleting object")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Send a fake update
	err = sendObjects(md.UpdateObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error deleting object")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error deleting object")

	//No updates should be received.
	kindMap = make(map[EventType]map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbAddUpdateTest_3(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 50},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 50)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()
	relations = []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}
	generateObjectReferences(relations)
	//Send 50-100
	err = sendObjects(md.UpdateObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Send 50-100
	err = sendObjects(md.AddObjectWithReferences, "c", 50, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[UpdateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	kindMap[UpdateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbAddUpdateTest_4(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 50},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 50)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()
	relations = []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}
	generateObjectReferences(relations)
	//Send 50-100
	err = sendObjects(md.UpdateObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Send 50-100
	err = sendObjects(md.AddObjectWithReferences, "c", 50, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[UpdateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbAddUpdateTest_5(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 50},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)
	md.WatchObjects("d", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 50)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()
	relations = []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}
	generateObjectReferences(relations)
	//Send 50-100
	err = sendObjects(md.UpdateObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Send 50-100
	err = sendObjects(md.AddObjectWithReferences, "c", 50, 100)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[UpdateEvent] = make(map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[CreateEvent]["c"] = 50
	kindMap[UpdateEvent]["b"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["a"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")
}

func TestMemdbDelAddTest_2(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(1*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(1*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbDelAddTest_3(t *testing.T) {
	initObjectStore()
	kindMap := make(map[EventType]map[string]int)

	relations := []relation{
		{"a", "b", 0, 100},
		{"b", "c", 0, 100},
	}

	generateObjectReferences(relations)
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("a", &watcher)
	md.WatchObjects("b", &watcher)
	md.WatchObjects("c", &watcher)

	err := sendObjects(md.AddObjectWithReferences, "a", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")

	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["a"] = 100
	kindMap[CreateEvent]["b"] = 100
	kindMap[CreateEvent]["c"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.DeleteObjectWithReferences, "b", 0, 0)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.DeleteObjectWithReferences, "c", 0, 0)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	kindMap = make(map[EventType]map[string]int)
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(md.AddObjectWithReferences, "b", 0, 100)
	AssertOk(t, err, "Error creating object")
	err = sendObjects(md.AddObjectWithReferences, "c", 0, 100)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(md.DeleteObjectWithReferences, "a", 0, 100)
	AssertOk(t, err, "Error creating object")
	//Make sure we receive objects of b as it has no deps
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[DeleteEvent] = make(map[string]int)
	kindMap[DeleteEvent]["a"] = 100
	kindMap[DeleteEvent]["b"] = 100
	kindMap[DeleteEvent]["c"] = 100
	kindMap[CreateEvent]["c"] = 100
	kindMap[CreateEvent]["b"] = 100
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}
func TestMemdbWatch(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("testObj", &watcher)

	// add an object
	err := md.AddObject(&obj)
	AssertOk(t, err, "Error creating object")

	// verify we get a watch event
	eventObj, _ := waitForWatch(t, watcher.Channel)
	wobj := eventObj.Obj
	Assert(t, eventObj.EventType == CreateEvent, "Received invalid object", wobj)
	Assert(t, (wobj.GetObjectMeta().Name == obj.Name), "Received invalid object", wobj)
	verifyObjField(t, wobj, "testField")

	// verify duplicate add results in update
	err = md.AddObject(&obj)
	AssertOk(t, err, "Error adding duplicate object")
	eventObj, _ = waitForWatch(t, watcher.Channel)
	wobj = eventObj.Obj
	Assert(t, eventObj.EventType == UpdateEvent, "Received invalid object", eventObj)

	// update the object
	newObj := obj
	newObj.field = "updatedField"
	err = md.UpdateObject(&newObj)
	AssertOk(t, err, "Error updating object")

	// verify we received the update
	eventObj, _ = waitForWatch(t, watcher.Channel)
	wobj = eventObj.Obj
	Assert(t, eventObj.EventType == UpdateEvent, "Received invalid object", wobj)
	verifyObjField(t, wobj, "updatedField")

	// delete the object
	err = md.DeleteObject(&obj)
	AssertOk(t, err, "Error deleting object")
	eventObj, _ = waitForWatch(t, watcher.Channel)
	wobj = eventObj.Obj
	Assert(t, eventObj.EventType == DeleteEvent, "Received invalid object", wobj)
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
	registerKinds(md)

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
	watchers := make([]*Watcher, watchConcur)
	for i := 0; i < watchConcur; i++ {
		watcher := Watcher{}
		watcher.Channel = make(chan Event, (objConcur + 100))
		err := md.WatchObjects("testObj", &watcher)
		AssertOk(t, err, "Error creating watcher")
		watchers[i] = &watcher
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
				evt, ok := <-watchers[wid].Channel
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
	objs := md.ListObjects("testObj", nil)
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
	objs = md.ListObjects("testObj", nil)
	Assert(t, (len(objs) == 0), "Some objects were not deleted", objs)
}

func (w *Watcher) Start(context context.Context) {

	for true {

		// verify we get a watch event
		select {
		case evt, ok := <-w.Channel:
			if ok {
				watchMap[w.Name].evtsRcvd.Update(evt, evt.Obj.GetObjectKind())
			}
		case <-context.Done():
			log.Infof("Stopped watcher")
			return
		}
	}

}

type eventsMap struct {
	sync.Mutex
	evKindMap map[EventType]map[string]int
}

var watchMap map[string]*watchWrapper

func ResetWatchMap() {
	watchMap = make(map[string]*watchWrapper)
}

func addWatcher(watcher *Watcher) *watchWrapper {
	evtRcvd := &eventsMap{}
	evtExp := &eventsMap{}
	evtRcvd.Reset()
	ctx, cancel := context.WithCancel(context.Background())
	watchMap[watcher.Name] = &watchWrapper{ctx: ctx, cancel: cancel}
	watchMap[watcher.Name].evtsRcvd = evtRcvd
	watchMap[watcher.Name].evtsExp = evtExp
	watchMap[watcher.Name].watcher = watcher
	go watcher.Start(ctx)
	return watchMap[watcher.Name]
}

func (e *eventsMap) Reset() {
	e.Lock()
	defer e.Unlock()
	e.evKindMap = make(map[EventType]map[string]int)
	e.evKindMap[CreateEvent] = make(map[string]int)
	e.evKindMap[UpdateEvent] = make(map[string]int)
	e.evKindMap[DeleteEvent] = make(map[string]int)
}

func (e *eventsMap) Update(evt Event, kind string) {
	e.Lock()
	defer e.Unlock()
	kindMap, _ := e.evKindMap[evt.EventType]
	kindMap[kind]++
}

func (e *eventsMap) Equal(other *eventsMap) bool {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		otherKindMap, _ := other.evKindMap[evt]
		if len(otherKindMap) != len(kindMap) {
			//Still check from
			if len(otherKindMap) < len(kindMap) {
				tmp := kindMap
				kindMap = otherKindMap
				otherKindMap = tmp

			}
			for kind, cnt := range otherKindMap {
				if val, ok := kindMap[kind]; !ok {
					if cnt != 0 {
						return false
					}
				} else if cnt != val {
					return false
				}

			}
		}
		for kind, cnt := range kindMap {
			if otherKindMap[kind] != cnt {
				return false
			}
		}
	}
	return true
}

func (e *eventsMap) Diff(exp *eventsMap) {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		expKindMap, _ := exp.evKindMap[evt]
		for kind, cnt := range kindMap {
			expKindCnt, ok := expKindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)

			} else if expKindCnt != cnt {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)
			}
		}
		//Check for kinds not present but where expected
		for kind, cnt := range expKindMap {
			otherKindCnt, ok := kindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, cnt, otherKindCnt)
				log.Infof("MAP : %v ", e.evKindMap)
			}
		}
	}
}

func verifyEvObjects(t *testing.T, watch *watchWrapper,
	duration time.Duration) error {

	timedOutEvent := time.After(duration)
	for true {
		select {
		case <-timedOutEvent:
			watch.evtsRcvd.Diff(watch.evtsExp)
			log.Infof("fail Matched...")
			return errors.New("Event expectation failed")
		default:
			if watch.evtsRcvd.Equal(watch.evtsExp) {
				return nil
			}
			time.Sleep(50 * time.Millisecond)

		}
	}
	return nil
}

func TestStopWatchObjects(t *testing.T) {
	numWatchers := 10

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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
	watchers := map[int]*Watcher{}
	for i := 0; i < numWatchers; i++ {
		watcher := Watcher{}
		watcher.Channel = make(chan Event, 2)
		watchers[i] = &watcher
		err := md.WatchObjects("testObj", watchers[i])
		AssertOk(t, err, "Error creating watcher")
	}
	err := md.AddObject(&obj)
	AssertOk(t, err, "error creating object")

	for _, w := range watchers {
		eventObj, _ := waitForWatch(t, w.Channel)
		Assert(t, eventObj.EventType == CreateEvent, "Received invalid object", eventObj.Obj)
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
			eventObj, _ := waitForWatch(t, w.Channel)
			Assert(t, eventObj.EventType == UpdateEvent, "Received invalid object", eventObj.Obj)
		}
	}
}

func TestMarshal(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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

	// start watch on objects
	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("testObj", &watcher)

	mo, err := md.MarshalJSON()
	AssertOk(t, err, "Error marshaling object")

	o := map[string]struct {
		Watchers []int
	}{}

	err = json.Unmarshal(mo, &o)
	AssertOk(t, err, "Error unmarshalling ")

	Assert(t, len(o) == 1, "invalid number of objects", len(o))

	for _, v := range o {
		Assert(t, len(v.Watchers) == 1, "invalid number of watchers", v)
	}
}

func TestMemdbNodeState(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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

	// add some node state
	err = md.AddNodeState("node-id", &obj)
	AssertOk(t, err, "Error adding node state")

	// list all node states and verify we fine the node
	objs, err := md.NodeStatesForObject("testObj", &obj.ObjectMeta)
	AssertOk(t, err, "Error getting list of node states")
	Assert(t, (len(objs) == 1), "Invalid number of node states")

	// delete node state and verify its gone
	err = md.DelNodeState("node-id", &obj)
	AssertOk(t, err, "Error deleting node state")
	objs, err = md.NodeStatesForObject("testObj", &obj.ObjectMeta)
	AssertOk(t, err, "Error getting list of node states")
	Assert(t, (len(objs) == 0), "Invalid number of node states after delete")
}

func TestMemdbAddDeleteWithDep(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	registerKinds(md)

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
	objs := md.ListObjects("testObj", nil)
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
	objs = md.ListObjects("testObj", nil)
	Assert(t, (len(objs) == 0), "List returned incorrect number of objs", objs)
}

func TestMemdbAddDeleteReceivers(t *testing.T) {
	// create a new memdb
	md := NewMemdb()

	recvrs := []objReceiver.Receiver{}

	for i := 0; i < maxReceivers; i++ {
		id := strconv.Itoa(i)
		recv, err := md.AddReceiver(id)
		Assert(t, err == nil && recv != nil, "Add failed")
		Assert(t, recv.(*receiver).ID == id, "ID mismatch")
		recvrs = append(recvrs, recv)
	}

	for i := 0; i < maxReceivers; i++ {
		id := strconv.Itoa(i)
		recv, err := md.AddReceiver(id)
		Assert(t, err == nil && recv != nil, "Add failed")
	}

	Assert(t, md.pushdb.bitMap.Count() == maxReceivers, " used matched")

	for i := maxReceivers; i < 2*maxReceivers; i++ {
		id := strconv.Itoa(i)
		recv, err := md.AddReceiver(id)
		Assert(t, err != nil && recv == nil, "Add failed")
	}

	for i := 0; i < maxReceivers; i++ {
		id := strconv.Itoa(i)
		recv, err := md.FindReceiver(id)
		Assert(t, err == nil && recv != nil, "Find failed")
	}

	for i := 0; i < maxReceivers; i++ {
		err := md.DeleteReceiver(recvrs[i])
		Assert(t, err == nil, "Delete failed")
	}

	for i := 0; i < maxReceivers; i++ {
		id := strconv.Itoa(i)
		recv, err := md.FindReceiver(id)
		Assert(t, err != nil && recv == nil, "Find succeded")
	}

	for i := 0; i < maxReceivers; i++ {
		err := md.DeleteReceiver(recvrs[i])
		Assert(t, err != nil, "Delete succeded")
	}

	fmt.Printf("Storage size %v", md.pushdb.bitMap.BinaryStorageSize())
	Assert(t, md.pushdb.bitMap.Len() == maxReceivers, "Storage matched")

}

func TestMemdbEnableDisableKindPushFilter(t *testing.T) {
	// create a new memdb
	md := NewMemdb()

	md.EnableSelctivePush("a")

	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")

	md.DisableKindPushFilter("a")
	Assert(t, !md.pushdb.KindEnabled("a"), "kind disabled")
}

func TestMemdbObjPushWatcher(t *testing.T) {
	// create a new memdb
	md := NewMemdb()
	const watchConcur = maxReceivers
	const objConcur = 200

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	md.EnableSelctivePush("c")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	watchers := make([]*Watcher, watchConcur)
	receivers := make([]objReceiver.Receiver, watchConcur)
	for i := 0; i < watchConcur; i++ {
		watcher := Watcher{Name: "watcher-" + strconv.Itoa(i)}
		watcher.Channel = make(chan Event, (objConcur + 100))
		err := md.WatchObjects("a", &watcher)
		Assert(t, err != nil, "Watcher registered")
		watchers[i] = &watcher
	}

	for i := 0; i < watchConcur; i++ {
		r, err := md.AddReceiver(watchers[i].Name)
		AssertOk(t, err, "Error adding receiver")
		Assert(t, r != nil, "Error adding receiver")
		receivers[i] = r
	}

	// setup concurrent watches, registration should succeed now
	for i := 0; i < watchConcur; i++ {
		watcher := Watcher{Name: "watcher-" + strconv.Itoa(i)}
		watcher.Channel = make(chan Event, (objConcur + 100))
		err := md.WatchObjects("a", &watcher)
		AssertOk(t, err, "Watcher registered")
		err = md.WatchObjects("b", &watcher)
		AssertOk(t, err, "Watcher registered")
		err = md.WatchObjects("c", &watcher)
		AssertOk(t, err, "Watcher registered")
		watchers[i] = &watcher
	}

	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")

	Assert(t, md.pushdb.KindReceiversCount("a") == watchConcur, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("b") == watchConcur, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("c") == watchConcur, "Kind count did not match")

	// setup concurrent watches, registration should succeed now
	for i := 0; i < watchConcur/2; i++ {
		err := md.DeleteReceiver(receivers[i])
		AssertOk(t, err, "Watcher unregistered")
	}
	Assert(t, md.pushdb.ReceiversCount() == watchConcur/2, "Count did not match")
	Assert(t, md.pushdb.KindReceiversCount("a") == watchConcur/2, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("b") == watchConcur/2, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("c") == watchConcur/2, "Kind count did not match")

	// setup concurrent watches, registration should succeed now
	for i := watchConcur / 2; i < watchConcur; i++ {
		err := md.DeleteReceiver(receivers[i])
		AssertOk(t, err, "Watcher unregistered")
	}
	Assert(t, md.pushdb.ReceiversCount() == 0, "Count did not match")
	Assert(t, md.pushdb.KindReceiversCount("a") == 0, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("b") == 0, "Kind count did not match")
	Assert(t, md.pushdb.KindReceiversCount("c") == 0, "Kind count did not match")
	Assert(t, md.pushdb.bitMap.Len() == maxReceivers, "Storage matched")
}

type watchWrapper struct {
	watcher  *Watcher
	evtsRcvd *eventsMap
	evtsExp  *eventsMap
	ctx      context.Context
	cancel   context.CancelFunc
}

func watcherName(i int) string {
	return "watcher-" + strconv.Itoa(i)
}

func addReceivers(t *testing.T, md *Memdb, watchConcur int) []objReceiver.Receiver {

	receivers := make([]objReceiver.Receiver, watchConcur)

	for i := 0; i < watchConcur; i++ {
		r, err := md.AddReceiver(watcherName(i))
		AssertOk(t, err, "Error adding receiver")
		Assert(t, r != nil, "Error adding receiver")
		receivers[i] = r
	}

	return receivers
}

func startWatchers(t *testing.T, watchConcur int) []*watchWrapper {

	watchers := []*watchWrapper{}
	// setup concurrent watches, registration should succeed now
	for i := 0; i < watchConcur; i++ {
		watcher := Watcher{Name: watcherName(i)}
		watcher.Channel = make(chan Event, (1000))
		watchWrap := addWatcher(&watcher)
		watchers = append(watchers, watchWrap)
	}

	return watchers
}

func startWatchForKinds(t *testing.T, md *Memdb, watchers []*watchWrapper, kinds []string) {

	for _, watch := range watchers {
		for _, kind := range kinds {
			err := md.WatchObjects(kind, watch.watcher)
			AssertOk(t, err, "Error watching object")
			ok := md.pushdb.ReceiverEnabled(kind, watch.watcher.Name)
			Assert(t, ok, "Receiver registerd with kind")
		}
	}
}

func stopWatchForKinds(t *testing.T, md *Memdb, watchers []*watchWrapper, kinds []string) {

	for _, watch := range watchers {
		for _, kind := range kinds {
			err := md.StopWatchObjects(kind, watch.watcher)
			AssertOk(t, err, "Error watching object")
			ok := md.pushdb.ReceiverEnabled(kind, watch.watcher.Name)
			Assert(t, !ok, "Receiver registerd with kind")
		}
	}
}

func stopWatchers(t *testing.T, watchers []*watchWrapper) {

	// setup concurrent watches, registration should succeed now
	for _, watch := range watchers {
		watch.cancel()
	}
}

func TestMemdbObjPushWatcher_1(t *testing.T) {

	initObjectStore()
	ResetWatchMap()

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	//const watchConcur = maxReceivers
	const watchConcur = 10
	const objConcur = 1
	const totalObjects = 10

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	md.EnableSelctivePush("c")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("c"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b", "c"})

	pObjhandles, err := sendPushObjects(md.AddPushObject, "b", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjhandles), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < totalObjects; ii++ {
		err := pObjhandles[ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

}

func TestMemdbObjPushWatcher_2(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

}

func TestMemdbObjPushWatcher_3(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

func TestMemdbObjPushWatcher_4(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].RemoveObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.evKindMap[DeleteEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < watchConcur; ii++ {
		ok := md.pushdb.ReceiverEnabled("", watchers[ii].watcher.Name)
		Assert(t, !ok, "Receiver registerd with kind")
	}
	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

func TestMemdbObjPushWatcher_5(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for i := 0; i < 10; i++ {

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.Reset()
			watch.evtsRcvd.Reset()
			watch.evtsExp.evKindMap[DeleteEvent]["a"] = totalObjects
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].RemoveObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(5*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].AddObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(5*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}
	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

func TestMemdbObjPushWatcher_6(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for i := 0; i < 10; i++ {

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.Reset()
			watch.evtsRcvd.Reset()
			watch.evtsExp.evKindMap[DeleteEvent]["a"] = totalObjects
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].RemoveAllObjReceivers()
			AssertOk(t, err, "Error Adding receivers")
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(5*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].AddObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(5*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}
	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

func TestMemdbObjPushWatcher_7(t *testing.T) {

	initObjectStore()
	ResetWatchMap()

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 100
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("b")
	md.EnableSelctivePush("a")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjhandles, err := sendPushObjects(md.AddPushObject, "b", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjhandles), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err := pObjhandles[ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsRcvd.Reset()
		watch.evtsExp.evKindMap[UpdateEvent]["b"] = totalObjects
	}

	//SendUpdate
	err = doUpdateObjectOp(pObjhandles)
	AssertOk(t, err, "Error updating object")
	AssertEquals(t, len(pObjhandles), totalObjects, "Error updating object")

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

func TestMemdbObjPushWatcher_8(t *testing.T) {

	initObjectStore()
	ResetWatchMap()

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 1
	const objConcur = 1
	const totalObjects = 1

	md.EnableSelctivePush("b")
	md.EnableSelctivePush("a")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjhandles, err := sendPushObjects(md.AddPushObject, "b", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjhandles), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < totalObjects; ii++ {
		err := pObjhandles[ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsRcvd.Reset()
		watch.evtsExp.evKindMap[DeleteEvent]["b"] = totalObjects
	}

	//Senddelete
	err = doDeleteObjectOp(pObjhandles)
	AssertOk(t, err, "Error updating object")
	AssertEquals(t, len(pObjhandles), totalObjects, "Error updating object")

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(5*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
}

/*
func TestMemdbObjPushWatcher_9(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
		{"b", "c", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 50
	const objConcur = 1
	const totalObjects = 100

	md.EnableSelctivePush("a")
	md.EnableSelctivePush("b")
	md.EnableSelctivePush("c")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("b"), "kind disabled")
	Assert(t, md.pushdb.KindEnabled("c"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a", "b"})

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["c"] = 0
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["b"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["c"] = 0
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	pObjMaps["c"], err = sendPushObjects(md.AddPushObject, "c", 0, totalObjects, receivers)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["c"]), totalObjects, "Error creating object")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["c"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
		watch.evtsExp.evKindMap[CreateEvent]["c"] = totalObjects
	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[0].watcher.Name], time.Duration(3*time.Second))
		}()

	}

	stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

}
*/

func TestMemdbObjPushAndMemdbWatcher_1(t *testing.T) {

	initObjectStore()
	ResetWatchMap()
	pObjMaps := make(map[string][]PushObjectHandle)
	var err error

	relations := []relation{
		{"a", "b", 0, 1},
	}

	generateObjectReferences(relations)

	// create a new memdb
	md := NewMemdb()
	registerKinds(md)
	//const watchConcur = maxReceivers
	const watchConcur = 1
	const objConcur = 1
	const totalObjects = 1

	md.EnableSelctivePush("a")
	//md.EnableSelctivePush("b")
	Assert(t, md.pushdb.KindEnabled("a"), "kind disabled")
	Assert(t, !md.pushdb.KindEnabled("b"), "kind disabled")

	// setup concurrent watches, registration will fail if not enabled
	receivers := addReceivers(t, md, watchConcur)

	// setup concurrent watches, registration should succeed now
	watchers := startWatchers(t, watchConcur)
	Assert(t, md.pushdb.ReceiversCount() == watchConcur, "Count did not match")
	defer stopWatchers(t, watchers)

	startWatchForKinds(t, md, watchers, []string{"a"})
	//md.WatchObjects("b", &watcher)

	watcher := Watcher{}
	watcher.Channel = make(chan Event, WatchLen)
	md.WatchObjects("b", &watcher)
	err = sendObjects(md.AddObjectWithReferences, "b", 0, 1)
	AssertOk(t, err, "Error creating object")

	pObjMaps["a"], err = sendPushObjects(md.AddPushObject, "a", 0, totalObjects, nil)
	AssertOk(t, err, "Error creating object")
	AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

	kindMap := make(map[EventType]map[string]int)
	kindMap = make(map[EventType]map[string]int)
	kindMap[CreateEvent] = make(map[string]int)
	kindMap[CreateEvent]["b"] = 1
	err = verifyObjects(t, md, &watcher, kindMap, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	for ii := 0; ii < totalObjects; ii++ {
		err = pObjMaps["a"][ii].AddObjReceivers(receivers)
		AssertOk(t, err, "Error Adding receivers")
	}

	for ii := 0; ii < watchConcur; ii++ {
		watch, _ := watchMap[watchers[ii].watcher.Name]
		watch.evtsExp.Reset()
		watch.evtsExp.evKindMap[CreateEvent]["b"] = 0
		watch.evtsExp.evKindMap[CreateEvent]["a"] = 1
	}

	errs := make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	errs = make(chan error, watchConcur)
	for ii := 0; ii < watchConcur; ii++ {
		ii := ii
		go func() {
			errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for ii := 0; ii < watchConcur; ii++ {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	/*
		pObjMaps["b"], err = sendPushObjects(md.AddPushObject, "b", 0, totalObjects, receivers)
		AssertOk(t, err, "Error creating object")
		AssertEquals(t, len(pObjMaps["a"]), totalObjects, "Error creating object")

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["b"][ii].AddObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.Reset()
			watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
			watch.evtsExp.evKindMap[CreateEvent]["a"] = 0
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].AddObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.Reset()
			watch.evtsExp.evKindMap[CreateEvent]["b"] = totalObjects
			watch.evtsExp.evKindMap[CreateEvent]["a"] = totalObjects
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for ii := 0; ii < totalObjects; ii++ {
			err = pObjMaps["a"][ii].RemoveObjReceivers(receivers)
			AssertOk(t, err, "Error Adding receivers")
		}

		for ii := 0; ii < watchConcur; ii++ {
			watch, _ := watchMap[watchers[ii].watcher.Name]
			watch.evtsExp.evKindMap[DeleteEvent]["a"] = totalObjects
		}

		errs = make(chan error, watchConcur)
		for ii := 0; ii < watchConcur; ii++ {
			ii := ii
			go func() {
				errs <- verifyEvObjects(t, watchMap[watchers[ii].watcher.Name], time.Duration(1*time.Second))
			}()

		}

		for ii := 0; ii < watchConcur; ii++ {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for ii := 0; ii < watchConcur; ii++ {
			ok := md.pushdb.ReceiverEnabled("", watchers[ii].watcher.Name)
			Assert(t, !ok, "Receiver registerd with kind")
		}
		stopWatchForKinds(t, md, watchers, []string{"a", "b", "c"})
	*/
}
