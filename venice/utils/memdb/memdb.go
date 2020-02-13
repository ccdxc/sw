// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"sync"

	"github.com/pensando/sw/api/graph"
	apiintf "github.com/pensando/sw/api/interfaces"
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

var (
	errObjNotFound = errors.New("Object not found")
)

// WatchLen is size of the watch channel buffer
const WatchLen = 100000

// ErrObjectNotFound is returned when an object is not found
var ErrObjectNotFound = errors.New("object not found")

// Event is watch event notifications
type Event struct {
	EventType EventType
	Obj       Object
	refs      map[string]apiintf.ReferenceObj
	key       string
}

type resolveState int

const (
	unresolvedAdd    resolveState = iota //Object added but dep unsolved
	unresolvedUpdate                     //Object update but dep unsolved
	resolved                             //Object fully resolved
	unresolvedDelete                     //Object delete initiated but referrers are stil presebt
)

func isAddUnResolved(state resolveState) bool {
	return state == unresolvedAdd
}

func isUpdateUnResolved(state resolveState) bool {
	return state == unresolvedUpdate
}

func isResolved(state resolveState) bool {
	return state == resolved
}

func isDeleteUnResolved(state resolveState) bool {
	return state == unresolvedDelete
}

// objState maintains per object state
type objState struct {
	sync.Mutex
	key          string
	obj          Object            // main object
	nodeState    map[string]Object // per node state
	resolveState resolveState      //resolve state
	//If the current object is marked for delete, we have to queue them.
	// Required if del/add tests where delete may not have been completed.
	pendingObjs []Event // pending events when objet is in unresolved delete
}

func (obj *objState) addToPending(event EventType, key string, newObj Object,
	refs map[string]apiintf.ReferenceObj) {
	obj.pendingObjs = append(obj.pendingObjs,
		Event{EventType: event, Obj: newObj, refs: refs, key: key})
}

func (obj *objState) getAndClearPending() []Event {
	defer func() { obj.pendingObjs = []Event{} }()
	return obj.pendingObjs
}

func (obj *objState) isResolved() bool {
	return isResolved(obj.resolveState)
}

func (obj *objState) isDelUnResolved() bool {
	return isDeleteUnResolved(obj.resolveState)
}

func (obj *objState) isAddUnResolved() bool {
	return isAddUnResolved(obj.resolveState)
}

func (obj *objState) isUpdateUnResolved() bool {
	return isUpdateUnResolved(obj.resolveState)
}

func (obj *objState) resolved() {
	obj.resolveState = resolved
}

func (obj *objState) addUnResolved() {
	obj.resolveState = unresolvedAdd
}

func (obj *objState) deleteUnResolved() {
	obj.resolveState = unresolvedDelete
}

func (obj *objState) updateUnResolved() {
	obj.resolveState = unresolvedUpdate
}

//FilterFn filter function
type FilterFn func(obj, prev Object) bool

//Watcher watcher attributes
type Watcher struct {
	Name    string
	Filters map[string][]FilterFn
	Channel chan Event
}

//GetName gets name of the watcher
func (watcher *Watcher) GetName() interface{} {
	return watcher.Name
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
	objdb         map[string]*Objdb
	objGraph      graph.Interface
	dbAddResolver resolver
	dbDelResolver resolver
}

// watchEvent sends out watch event to all watchers
func (od *Objdb) watchEvent(obj Object, et EventType) error {
	done := false
	// create the event
	ev := Event{
		EventType: et,
		Obj:       obj,
	}
	od.WatchLock.RLock()
	// send it to each watcher
	for _, watcher := range od.watchers {
		if len(watcher.Filters) != 0 {
			if filters, ok := watcher.Filters[obj.GetObjectKind()]; ok {
				for _, flt := range filters {
					if !flt(ev.Obj, nil) {
						done = true
						break
					}
				}
				if done == true {
					done = false
					continue
				}
			}
		}
		//fmt.Printf("Sending obj evemt %v %v\n", ev, obj.GetObjectMeta().GetKey())
		select {
		case watcher.Channel <- ev:
		default:
			log.Errorf("too slow agent and watcher events are greater than channel capacity")
			// TODO: too slow agent and watcher events are greater than channel capacity..
			// come up with a policy.. either close the connection or drop the events or something else
		}
		//log.Infof("Sending  Event %v kind %v, object %+v to watcher %v",
		//	ev.EventType, ev.Obj.GetObjectKind(), ev.Obj.GetObjectMeta(), watcher.Name)
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

	md.dbAddResolver = newAddResolver(&md)
	md.dbDelResolver = newDeleteResolver(&md)

	md.objGraph, _ = graph.NewCayleyStore()

	return &md
}

// memdbKey returns objdb key for the object
func memdbKey(ometa *api.ObjectMeta) string {
	return ometa.GetObjectMeta().GetKey()
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

//get the key for relation in graphdb
//constructed for source kind , desitnation kind and actual field
func getKeyForGraphDB(skind, dkind, key string) string {
	return skind + "-" + dkind + "-" + key
}

func memDbKind(in string) string {
	if in == "VirtualRouter" {
		return "Vrf"
	}
	return in
}

func getSKindDKindFieldKey(key string) (string, string, string) {
	parts := strings.Split(key, "-")
	return memDbKind(parts[0]), memDbKind(parts[1]), parts[2]
}

//AddObjectWithReferences add object with refs
func (md *Memdb) AddObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error {
	if obj.GetObjectKind() == "" {
		log.Errorf("Object kind is empty: %+v", obj)
	}
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())
	if key == "" {
		key = memdbKey(obj.GetObjectMeta())
	}

	// if we have the object, make this an update
	od.Lock()
	existingObj, ok := od.objects[key]
	if ok {
		//If delete is not resolved, add to pending
		od.Unlock()
		if existingObj.isDelUnResolved() {
			existingObj.addToPending(CreateEvent, key, obj, refs)
			return nil
		}
		return md.UpdateObjectWithReferences(key, obj, refs)
	}

	md.updateReferences(key, obj, refs)
	// add it to db and send out watch notification
	newObj := &objState{
		key:       key,
		obj:       obj,
		nodeState: make(map[string]Object),
	}
	od.objects[key] = newObj
	od.Unlock()

	if md.dbAddResolver.resolvedCheck(key, obj) {
		log.Infof("Add Object key %v resolved", key)
		newObj.Lock()
		newObj.resolved()
		newObj.Unlock()
		od.watchEvent(obj, CreateEvent)
		md.dbAddResolver.trigger(key, obj)
	} else {
		newObj.Lock()
		newObj.addUnResolved()
		newObj.Unlock()
		log.Infof("Add Object key %v unresolved, refs %v", key, refs)
	}
	return nil
}

// AddObject adds an object to memdb and sends out watch notifications
func (md *Memdb) AddObject(obj Object) error {
	return md.AddObjectWithReferences("", obj, nil)
}

// UpdateObjectWithReferences updates an object in memdb and sends out watch notifications
func (md *Memdb) UpdateObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error {
	if key == "" {
		key = memdbKey(obj.GetObjectMeta())
	}
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())

	// lock object db
	od.Lock()

	// if we dont have the object, return error
	ostate, ok := od.objects[key]
	if !ok {
		od.Unlock()
		log.Errorf("Object {%+v} not found", obj.GetObjectMeta())
		return errObjNotFound
	}

	// add it to db and send out watch notification

	ostate.Lock()
	od.Unlock()
	if ostate.isDelUnResolved() {
		//If it is marked for delete, wait it out
		//reason to wait out is because create could be queued too.
		ostate.addToPending(UpdateEvent, key, obj, refs)
		ostate.Unlock()
		return nil
	}

	md.updateReferences(key, obj, refs)
	ostate.obj = obj

	if md.dbAddResolver.resolvedCheck(key, obj) {
		//log.Infof("Update Object key %v resolved", key)
		event := UpdateEvent
		if ostate.isAddUnResolved() {
			//It was not resolved before, hence set it to create now
			//change even to create event as we never sent the object
			event = CreateEvent
		}
		ostate.resolved()
		ostate.Unlock()
		od.watchEvent(obj, event)
		md.dbAddResolver.trigger(key, obj)
	} else {
		//log.Infof("Update Object key %v unresolved", key)
		ostate.updateUnResolved()
		ostate.Unlock()
	}
	return nil
}

//UpdateObject update object with references
func (md *Memdb) UpdateObject(obj Object) error {
	return md.UpdateObjectWithReferences("", obj, nil)
}

func (md *Memdb) clearReferences(key string) {
	node := graph.Node{
		This: key,
		Refs: make(map[string][]string),
	}
	md.objGraph.UpdateNode(&node)
}

func (md *Memdb) updateReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) {
	if refs == nil {
		return
	}
	node := graph.Node{
		This: key,
		Refs: make(map[string][]string),
	}

	for field, refs := range refs {
		objKey := getKeyForGraphDB(obj.GetObjectKind(), refs.RefKind, field)
		node.Refs[objKey] = []string{}
		for _, ref := range refs.Refs {
			node.Refs[objKey] = append(node.Refs[objKey], ref)
		}
	}

	log.Infof("updating Node with references for [%v][%v]", key, node.Refs)
	md.objGraph.UpdateNode(&node)
}

// DeleteObjectWithReferences deletes an object from memdb and sends out watch notifications
func (md *Memdb) DeleteObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error {
	// get objdb
	od := md.getObjdb(obj.GetObjectKind())

	// lock object db
	od.Lock()

	// if we dont have the object, return error
	if key == "" {
		key = memdbKey(obj.GetObjectMeta())
	}
	existingObj, ok := od.objects[key]
	if !ok {
		od.Unlock()
		log.Errorf("Object {%+v} not found", obj.GetObjectMeta())
		return errors.New("Object not found")
	}
	od.Unlock()
	//If create is not resolved, wait for delete
	existingObj.Lock()
	if !(existingObj.isResolved()) {
		log.Infof("Exisiting object unresolved, pending add for %v", key)
		existingObj.addToPending(DeleteEvent, key, obj, refs)
		existingObj.Unlock()
		return nil
	}

	if md.dbDelResolver.resolvedCheck(key, obj) {
		// add it to db and send out watch notification
		log.Infof("Delete Object key %v resolved, refs %v", key, refs)
		existingObj.Unlock()
		od.Lock()
		delete(od.objects, key)
		od.Unlock()
		od.watchEvent(existingObj.obj, DeleteEvent)
		md.dbDelResolver.trigger(key, obj)
	} else {
		log.Infof("Delete Object key %v unresolved, refs %v", key, refs)
		existingObj.deleteUnResolved()
		existingObj.Unlock()
	}

	return nil
}

// DeleteObject deletes an object from memdb and sends out watch notifications
func (md *Memdb) DeleteObject(obj Object) error {
	return md.DeleteObjectWithReferences("", obj, nil)
}

// FindObject returns the object by its meta
func (md *Memdb) FindObject(kind string, ometa *api.ObjectMeta) (Object, error) {
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()
	key := memdbKey(ometa)
	// see if we have the object
	ostate, ok := od.objects[key]
	if !ok {
		return nil, ErrObjectNotFound
	}

	return ostate.obj, nil
}

// ListObjects returns a list of all objects of a kind
func (md *Memdb) ListObjects(kind string, filters []FilterFn) []Object {
	done := false
	// get objdb
	od := md.getObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	// walk all objects and add it to return list
	var objs []Object
	for _, obj := range od.objects {
		if filters == nil {
			objs = append(objs, obj.obj)
		} else {
			for _, filter := range filters {
				if !filter(obj.obj, nil) {
					done = true
					break
				}
			}
			if done == true {
				done = false
			} else {
				objs = append(objs, obj.obj)
			}
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

	key := memdbKey(ometa)
	// see if we have the object
	ostate, ok := od.objects[key]
	if !ok {
		return nil, ErrObjectNotFound
	}

	return ostate.nodeState, nil
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
