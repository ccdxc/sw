// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"errors"
	"fmt"
	"sync"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
)

// Object is the interface all objects have to implement
type Object interface {
	GetObjectKind() string          // returns the object kind
	GetObjectMeta() *api.ObjectMeta // returns the object meta
}

// EventType for watch notifications
type EventType string

// event types
const (
	CreateEvent EventType = "Create"
	UpdateEvent EventType = "Update"
	DeleteEvent EventType = "Delete"
)

// WatchLen is size of the watch channel buffer
const WatchLen = 1000

// Event is watch event notifications
type Event struct {
	EventType EventType
	Obj       Object
}

// Objdb contains objects of a specific kind
type Objdb struct {
	sync.Mutex
	WatchLock sync.RWMutex
	objects   map[string]Object
	watchers  []chan Event
}

// Memdb is database of all objects
type Memdb struct {
	sync.Mutex
	objdb map[string]*Objdb
}

// watchEvent sends out watch event to all watchers
func (od *Objdb) watchEvent(obj Object, et EventType) error {
	// create the event
	ev := Event{
		EventType: et,
		Obj:       obj,
	}
	od.WatchLock.RLock()
	// send it to each watcher
	for _, watchChan := range od.watchers {
		select {
		case watchChan <- ev:
		default:
			// TODO: too slow agent and watcher events are greater than channel capacity..
			// come up with a policy.. either close the connection or drop the events or something else
		}
	}
	od.WatchLock.RUnlock()

	return nil
}

// NewMemdb creates a new memdb instance
func NewMemdb() *Memdb {
	// create memdb instance
	md := Memdb{
		objdb: make(map[string]*Objdb),
	}

	return &md
}

// memdbKey returns objdb key for the object
func memdbKey(ometa *api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", ometa.Tenant, ometa.Name)
}

// getObjdb returns object db for specific object kind
func (md *Memdb) getObjdb(kind string) *Objdb {
	// lock the memdb for access
	md.Lock()
	defer md.Unlock()

	// check if we already have the object db
	od, ok := md.objdb[kind]
	if ok {
		return od
	}

	// create new objectdb
	od = &Objdb{
		objects: make(map[string]Object),
	}

	// save it and return
	md.objdb[kind] = od
	return od
}

// WatchObjects watches for changes on an object kind
// TODO: Add support for watch support with resource version
func (md *Memdb) WatchObjects(kind string, watchChan chan Event) error {
	// get objdb
	od := md.getObjdb(kind)

	od.WatchLock.Lock()
	// add the channel to watch list and return
	od.watchers = append(od.watchers, watchChan)
	od.WatchLock.Unlock()
	return nil
}

// StopWatchObjects removes watches given the kind and watchChan
func (md *Memdb) StopWatchObjects(kind string, watchChan chan Event) error {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.WatchLock.Lock()
	for i, other := range od.watchers {
		if other == watchChan {
			od.watchers = append(od.watchers[:i], od.watchers[i+1:]...)
			break
		}
	}
	od.WatchLock.Unlock()
	return nil
}

// AddObject adds an object to memdb and sends out watch notifications
func (md *Memdb) AddObject(obj Object) error {
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())
	key := memdbKey(obj.GetObjectMeta())

	// if we have the object, make this an update
	od.Lock()
	_, ok := od.objects[key]
	if ok {
		od.Unlock()
		return md.UpdateObject(obj)
	}

	// add it to db and send out watch notification
	od.objects[key] = obj
	od.Unlock()

	od.watchEvent(obj, CreateEvent)
	return nil
}

// UpdateObject updates an object in memdb and sends out watch notifications
func (md *Memdb) UpdateObject(obj Object) error {
	key := memdbKey(obj.GetObjectMeta())
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())

	// lock object db
	od.Lock()

	// if we dont have the object, return error
	_, ok := od.objects[key]
	if !ok {
		od.Unlock()
		logrus.Errorf("Object {%+v} not found", obj.GetObjectMeta())
		return errors.New("Object not found")
	}

	// add it to db and send out watch notification
	od.objects[key] = obj
	od.Unlock()

	od.watchEvent(obj, UpdateEvent)

	return nil
}

// DeleteObject deletes an object from memdb and sends out watch notifications
func (md *Memdb) DeleteObject(obj Object) error {
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())

	// lock object db
	od.Lock()

	// if we dont have the object, return error
	_, ok := od.objects[memdbKey(obj.GetObjectMeta())]
	if !ok {
		od.Unlock()
		logrus.Errorf("Object {%+v} not found", obj.GetObjectMeta())
		return errors.New("Object not found")
	}

	// add it to db and send out watch notification
	delete(od.objects, memdbKey(obj.GetObjectMeta()))
	od.Unlock()
	od.watchEvent(obj, DeleteEvent)

	return nil
}

// FindObject returns the object by its meta
func (md *Memdb) FindObject(kind string, ometa *api.ObjectMeta) (Object, error) {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	// see if we have the object
	obj, ok := od.objects[memdbKey(ometa)]
	if !ok {
		return nil, errors.New("object not found")
	}

	return obj, nil
}

// ListObjects returns a list of all objects of a kind
func (md *Memdb) ListObjects(kind string) []Object {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	// walk all objects and add it to return list
	var objs []Object
	for _, obj := range od.objects {
		objs = append(objs, obj)
	}

	return objs
}
