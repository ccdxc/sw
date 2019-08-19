// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb2

import (
	"encoding/json"
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/venice/utils/log"

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
const WatchLen = 100000

// ErrObjectNotFound is returned when an object is not found
var ErrObjectNotFound = errors.New("object not found")

// Event is watch event notifications
type Event struct {
	EventType EventType
	Obj       Object
}

// objState maintains per object state
type objState struct {
	sync.Mutex
	obj       Object            // main object
	nodeState map[string]Object // per node state
}

//FilterFn filter function
type FilterFn func(Object) bool

//Watcher watcher attributes
type Watcher struct {
	Name            string
	Filter          FilterFn
	Channel         chan Event
	lastEventStatus map[EventType]Object
}

//GetName gets name of the watcher
func (watcher *Watcher) GetName() interface{} {
	return watcher.Name
}

//GetLastObject gets last object sent for this event.
func (watcher *Watcher) GetLastObject(event EventType) Object {
	object, ok := watcher.lastEventStatus[event]
	if ok {
		return object
	}

	return nil
}

// Objdb contains objects of a specific kind
type Objdb struct {
	sync.Mutex
	WatchLock sync.RWMutex
	objects   map[string]*objState
	watchers  []*Watcher
	watchMap  map[string]*Watcher
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
	for _, watcher := range od.watchers {
		if !watcher.Filter(ev.Obj) {
			continue
		}
		select {
		case watcher.Channel <- ev:
		default:
			log.Errorf("too slow agent and watcher events are greater than channel capacity")
			// TODO: too slow agent and watcher events are greater than channel capacity..
			// come up with a policy.. either close the connection or drop the events or something else
		}
		log.Infof("Sending  Event %v kind %v, object %+v to watcher %v",
			ev.EventType, ev.Obj.GetObjectKind(), ev.Obj.GetObjectMeta(), watcher.Name)
		watcher.lastEventStatus[et] = ev.Obj
	}
	od.WatchLock.RUnlock()

	return nil
}

//GetLastObjectForWatcher Gets last object sent for this watcher.
func (od *Objdb) GetLastObjectForWatcher(watchName string, event EventType) (Object, error) {
	od.WatchLock.RLock()
	defer od.WatchLock.RUnlock()

	watch, ok := od.watchMap[watchName]
	if !ok {
		return nil, errors.New("Watch not found")
	}

	if obj, ok := watch.lastEventStatus[event]; ok {
		return obj, nil
	}

	return nil, errors.New("Object for event not found")
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
	return ometa.GetKey()
}

// getObjdb returns object db for specific object kind
func (md *Memdb) getObjdb(kind string) *Objdb {
	if kind == "" {
		panic("object kind is empty")
	}
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
		objects:  make(map[string]*objState),
		watchMap: make(map[string]*Watcher),
	}

	// save it and return
	md.objdb[kind] = od
	return od
}

// WatchObjects watches for changes on an object kind
// TODO: Add support for watch support with resource version
func (md *Memdb) WatchObjects(kind string, watcher *Watcher) error {
	// get objdb
	od := md.getObjdb(kind)

	od.WatchLock.Lock()
	// add the channel to watch list and return
	od.watchers = append(od.watchers, watcher)
	od.watchMap[watcher.Name] = watcher
	watcher.lastEventStatus = make(map[EventType]Object)
	od.WatchLock.Unlock()
	return nil
}

// StopWatchObjects removes watches given the kind and watchChan
func (md *Memdb) StopWatchObjects(kind string, watcher *Watcher) error {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.WatchLock.Lock()
	for i, other := range od.watchers {
		if other == watcher {
			od.watchers = append(od.watchers[:i], od.watchers[i+1:]...)
			delete(od.watchMap, watcher.Name)
			break
		}
	}
	od.WatchLock.Unlock()
	return nil
}

// AddObject adds an object to memdb and sends out watch notifications
func (md *Memdb) AddObject(obj Object) error {
	if obj.GetObjectKind() == "" {
		log.Errorf("Object kind is empty: %+v", obj)
	}
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
	od.objects[key] = &objState{
		obj:       obj,
		nodeState: make(map[string]Object),
	}
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
	ostate, ok := od.objects[key]
	if !ok {
		od.Unlock()
		log.Errorf("Object {%+v} not found", obj.GetObjectMeta())
		return errors.New("Object not found")
	}

	// add it to db and send out watch notification
	ostate.obj = obj
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
		log.Errorf("Object {%+v} not found", obj.GetObjectMeta())
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
	ostate, ok := od.objects[memdbKey(ometa)]
	if !ok {
		return nil, ErrObjectNotFound
	}

	return ostate.obj, nil
}

// ListObjects returns a list of all objects of a kind
func (md *Memdb) ListObjects(kind string, filter FilterFn) []Object {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	// walk all objects and add it to return list
	var objs []Object
	for _, obj := range od.objects {
		if filter(obj.obj) {
			objs = append(objs, obj.obj)
		}
	}

	return objs
}

// AddNodeState adds node state to an object
func (md *Memdb) AddNodeState(nodeID string, obj Object) error {
	if obj.GetObjectKind() == "" {
		log.Errorf("Object kind is empty: %+v", obj)
	}
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())
	key := memdbKey(obj.GetObjectMeta())

	// if we have the object, make this an update
	od.Lock()
	ostate, ok := od.objects[key]
	od.Unlock()
	if !ok {
		return fmt.Errorf("Object %v/%v not found", obj.GetObjectKind(), key)
	}

	// lock the object state for parallel update
	ostate.Lock()
	defer ostate.Unlock()

	// save node state
	ostate.nodeState[nodeID] = obj
	return nil
}

// DelNodeState deletes node state from object
func (md *Memdb) DelNodeState(nodeID string, obj Object) error {
	if obj.GetObjectKind() == "" {
		log.Errorf("Object kind is empty: %+v", obj)
	}
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())
	key := memdbKey(obj.GetObjectMeta())

	// if we have the object, make this an update
	od.Lock()
	ostate, ok := od.objects[key]
	od.Unlock()
	if !ok {
		return fmt.Errorf("Object %v/%v not found", obj.GetObjectKind(), key)
	}

	// lock the object state for parallel update
	ostate.Lock()
	defer ostate.Unlock()

	// delete node state
	delete(ostate.nodeState, nodeID)
	return nil
}

// NodeStatesForObject returns all node states for an object
func (md *Memdb) NodeStatesForObject(kind string, ometa *api.ObjectMeta) (map[string]Object, error) {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	// see if we have the object
	ostate, ok := od.objects[memdbKey(ometa)]
	if !ok {
		return nil, ErrObjectNotFound
	}

	return ostate.nodeState, nil
}

//GetLastObjectForWatcher Gets last object sent for this watcher.
func (md *Memdb) GetLastObjectForWatcher(kind string, watchName string, event EventType) (Object, error) {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	return od.GetLastObjectForWatcher(watchName, event)
}

// MarshalJSON converts memdb contents to json
func (md *Memdb) MarshalJSON() ([]byte, error) {
	// lock the memdb for access
	md.Lock()
	defer md.Unlock()

	contents := map[string]struct {
		Object   map[string]Object
		Watchers []int
	}{}

	for kind, objs := range md.objdb {
		o := map[string]Object{}
		for name, obj := range objs.objects {
			o[name] = obj.obj
		}

		watchers := []int{}
		for _, watcher := range objs.watchers {
			watchers = append(watchers, len(watcher.Channel))
		}

		contents[kind] = struct {
			Object   map[string]Object
			Watchers []int
		}{Object: o, Watchers: watchers}

	}

	return json.Marshal(contents)
}
