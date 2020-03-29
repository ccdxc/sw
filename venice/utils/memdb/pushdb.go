package memdb

import (
	"errors"
	"fmt"
	"sync"

	"github.com/willf/bitset"

	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb/objReceiver"
)

//PushObjectHandle has the handle for Target object
type PushObjectHandle interface {

	//Update
	UpdateObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error

	//Delete
	DeleteObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error

	AddObjReceivers(receviers []objReceiver.Receiver) error
	RemoveObjReceivers(receviers []objReceiver.Receiver) error
	IsReceiver(recv objReceiver.Receiver) (bool, error)
	RemoveAllObjReceivers() error

	Object() Object
}

// objState maintains per object state
type pObjState struct {
	objState
	bitMap     *bitset.BitSet
	sentBitMap *bitset.BitSet //Keep track of all sent ones
	delBitMap  *bitset.BitSet //Keep track of all del sent ones
	pushdb     *pushDB        //back pointer to memdb to access registered receivers
}

// pushObjdb contains objects of a specific kind
type pushObjDB struct {
	sync.Mutex
	WatchLock  sync.RWMutex
	objects    map[string]*pObjState
	pushFilter *objPushFilter
}

func (od *pushObjDB) dbType() objDBType {
	return pushObjDBType
}

func (od *pushObjDB) setObject(key string, obj objIntf) {
	od.objects[key] = obj.(*pObjState)
}

func (od *pushObjDB) deleteObject(key string) {
	delete(od.objects, key)
}

type pushDB struct {
	objPushFilter
	pObjDB map[string]*pushObjDB //DB for all push objects
	md     *Memdb
}

type objPushFilter struct {
	//Keep track of bitmaps
	sync.Mutex
	bitMap *bitset.BitSet
	//map of receiver ID to the receiver
	idMap        map[string]objReceiver.Receiver
	watchMap     map[uint]map[string]*Watcher
	enabledKinds map[string]bool
}

func newPushDB(md *Memdb) *pushDB {
	return &pushDB{
		objPushFilter: objPushFilter{
			bitMap:       bitset.New(maxReceivers),
			idMap:        make(map[string]objReceiver.Receiver),
			watchMap:     make(map[uint]map[string]*Watcher),
			enabledKinds: make(map[string]bool),
		},
		pObjDB: make(map[string]*pushObjDB),
		md:     md,
	}
}

func (p *pushDB) getPushObjdb(kind string) *pushObjDB {
	if kind == "" {
		panic("object kind is empty")
	}
	// lock the memdb for access
	p.Lock()
	defer p.Unlock()

	// check if we already have the object db
	od, ok := p.pObjDB[kind]
	if ok {
		return od
	}

	return nil

}

//EnableKind enables kind fitering
func (pf *objPushFilter) EnableKind(kind string) error {
	pf.Lock()
	defer pf.Unlock()
	pf.enabledKinds[kind] = true

	return nil
}

//DisableKind disables kind filter
func (pf *objPushFilter) DisableKind(kind string) error {
	pf.Lock()
	defer pf.Unlock()
	delete(pf.enabledKinds, kind)
	return nil
}

//DisableKind disables kind filter
func (pf *objPushFilter) KindEnabled(kind string) bool {
	pf.Lock()
	defer pf.Unlock()
	if _, ok := pf.enabledKinds[kind]; ok {
		return true
	}

	return false
}

//AddReceiver adds a receiver
func (pf *objPushFilter) FindReceiver(ID string) (objReceiver.Receiver, error) {
	pf.Lock()
	defer pf.Unlock()

	pushFilter := pf
	if recv, ok := pushFilter.idMap[ID]; ok {
		return recv, nil
	}

	return nil, fmt.Errorf("Receiver not found %v", ID)
}

func (pf *objPushFilter) ReceiversCount() int {
	pf.Lock()
	pf.Unlock()
	return len(pf.watchMap)
}

func (pf *objPushFilter) KindReceiversCount(kind string) int {
	pf.Lock()
	defer pf.Unlock()
	count := 0
	for _, kindMap := range pf.watchMap {
		if _, ok := kindMap[kind]; ok {
			count++
		}
	}
	return count
}

func (pf *objPushFilter) ReceiverEnabled(kind string, ID string) bool {
	pf.Lock()
	defer pf.Unlock()

	recv, ok := pf.idMap[ID]
	if !ok {
		return false
	}

	receiver, ok := recv.(*receiver)
	if !ok {
		return false
	}

	kindMap, ok := pf.watchMap[receiver.bitID]
	if !ok {
		return false
	}

	if _, ok := kindMap[kind]; ok {
		return true
	}

	return false
}

func (pf *objPushFilter) AddWatcher(kind string, watcher *Watcher) error {
	pf.Lock()
	defer pf.Unlock()
	if recv, ok := pf.idMap[watcher.Name]; ok {
		bitID := recv.(*receiver).bitID
		if _, ok := pf.watchMap[bitID]; !ok {
			pf.watchMap[bitID] = make(map[string]*Watcher)
		}
		log.Infof("PushDB watch for kind %v name %v", kind, watcher.Name)
		pf.watchMap[bitID][kind] = watcher
		return nil
	}

	return fmt.Errorf("Watcher %v not registered yet", watcher.Name)
}

func (pf *objPushFilter) GetWatchers(kind string) (*DBWatchers, error) {
	pf.Lock()
	defer pf.Unlock()
	dbwatchers := &DBWatchers{DBType: "PushDB", Kind: kind}
	for _, recv := range pf.idMap {
		bitID := recv.(*receiver).bitID
		if _, ok := pf.watchMap[bitID]; ok {
			if watcher, ok := pf.watchMap[bitID][kind]; ok {
				dbwatchers.Watchers = append(dbwatchers.Watchers,
					DBWatch{Name: watcher.Name, Status: "Active"})
			}
		}
	}
	return dbwatchers, nil
}

func (pf *objPushFilter) RemoveWatcher(kind string, watcher *Watcher) error {
	pf.Lock()
	defer pf.Unlock()
	if recv, ok := pf.idMap[watcher.Name]; ok {
		bitID := recv.(*receiver).bitID
		delete(pf.watchMap, bitID)
		log.Infof("Pushdb unwatch for kind %v name %v", kind, watcher.Name)
		return nil
	}

	return fmt.Errorf("Watcher %v not registered yet", watcher.Name)
}

//AddObjReceivears add receivers for the object
func (pObjState *pObjState) AddObjReceivers(receviers []objReceiver.Receiver) error {

	pObjState.Lock()
	defer pObjState.Unlock()
	for _, recv := range receviers {
		r, ok := recv.(*receiver)
		if !ok {
			return errors.New("Invalid receiver")
		}
		_, err := pObjState.pushdb.FindReceiver(r.ID)
		if err != nil {
			return fmt.Errorf("Receiver %v not found", r.ID)
		}

		if !pObjState.bitMap.Test(r.bitID) {
			//Send to the receiver.
			if pObjState.isResolved() {
				pObjState.pushdb.Lock()
				kindWatchMap, ok := pObjState.pushdb.watchMap[r.bitID]
				if ok {
					ev := Event{
						EventType: CreateEvent,
						Obj:       pObjState.Object(),
					}
					watcher, ok := kindWatchMap[pObjState.obj.GetObjectKind()]
					if ok {
						log.Infof("Sending to receiver %v %p %v %v", r.bitID, watcher, watcher.Name, ev.Obj.GetObjectKind())
						sendToWatcher(ev, watcher)
					}
				}
				pObjState.pushdb.Unlock()
				pObjState.bitMap.Set(r.bitID)
			}
		}
	}
	return nil
}

//AddObjReceivers add receivers for the object
func (pObjState *pObjState) IsReceiver(recv objReceiver.Receiver) (bool, error) {

	pObjState.Lock()
	defer pObjState.Unlock()
	r, ok := recv.(*receiver)
	if !ok {
		return false, errors.New("Invalid receiver")
	}
	_, err := pObjState.pushdb.FindReceiver(r.ID)
	if err != nil {
		return false, fmt.Errorf("Receiver %v not found", r.ID)
	}

	if pObjState.bitMap.Test(r.bitID) {
		return true, nil
	}
	return false, nil
}

// watchEvent sends out watch event to all watchers
func (od *pushObjDB) watchEvent(obj objIntf, et EventType) error {
	pobj, ok := obj.(*pObjState)
	if !ok {
		return fmt.Errorf("Invalid object ")
	}

	i := uint(0)
	for true {
		i, ok = pobj.bitMap.NextSet(i)
		if !ok {
			log.Infof("Pushdb publish of object %v done ", obj.Key())
			break
		}

		od.pushFilter.Lock()
		kindWatchMap, ok := od.pushFilter.watchMap[i]
		if ok {
			ev := Event{
				EventType: et,
				Obj:       pobj.Object(),
			}
			watcher, ok := kindWatchMap[pobj.obj.GetObjectKind()]
			if ok {
				log.Infof("Sending to receiver %v %v %v", watcher, watcher.Name, pobj.obj.GetObjectKind())
				sendToWatcher(ev, watcher)
				pobj.sentBitMap.Set(i)
				pobj.delBitMap.Clear(i)
			}
		}
		od.pushFilter.Unlock()
		i++

	}

	return nil
}

func (pObjState *pObjState) UpdateObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error {
	if key == "" {
		key = memdbKey(obj.GetObjectMeta())
	}

	updObj := &objState{
		key:       key,
		obj:       obj,
		nodeState: make(map[string]Object),
	}
	md := pObjState.pushdb.md

	return md.updateObject(md.getPObjectDBByType(obj.GetObjectKind()), key, updObj, refs)
}

func (pObjState *pObjState) DeleteObjectWithReferences(key string, obj Object, refs map[string]apiintf.ReferenceObj) error {
	if key == "" {
		key = memdbKey(obj.GetObjectMeta())
	}

	md := pObjState.pushdb.md

	delObj := &objState{
		key:       key,
		obj:       obj,
		nodeState: make(map[string]Object),
	}
	return md.deleteObject(md.getPObjectDBByType(obj.GetObjectKind()), key, delObj, refs)
}

//RemoveObjReceivers remove receiver for the object
func (pObjState *pObjState) RemoveObjReceivers(receviers []objReceiver.Receiver) error {

	pObjState.Lock()
	defer pObjState.Unlock()
	md := pObjState.pushdb.md
	for _, recv := range receviers {
		r, ok := recv.(*receiver)
		if !ok {
			return errors.New("Invalid receiver")
		}
		_, err := md.FindReceiver(r.ID)
		if err != nil {
			return fmt.Errorf("Receiver %v not found", r.ID)
		}

		md.removeObjReceiverByBitID(pObjState, r.bitID)
	}
	return nil
}

//RemoveAllObjReceivers remove all receivers
func (pObjState *pObjState) RemoveAllObjReceivers() error {

	pObjState.Lock()
	defer pObjState.Unlock()
	md := pObjState.pushdb.md
	i := uint(0)
	var ok bool
	for true {
		i, ok = pObjState.bitMap.NextSet(i)
		if !ok {
			break
		}

		md.removeObjReceiverByBitID(pObjState, i)
		i++
	}
	pObjState.bitMap = pObjState.bitMap.ClearAll()
	pObjState.sentBitMap = pObjState.sentBitMap.ClearAll()
	pObjState.delBitMap = pObjState.delBitMap.ClearAll()

	return nil
}

//AddReceiver adds a receiver
func (pf *objPushFilter) AddReceiver(ID string) (objReceiver.Receiver, error) {
	pf.Lock()
	defer pf.Unlock()

	pushFilter := pf
	if recvr, ok := pushFilter.idMap[ID]; ok {
		return recvr, nil
	}
	//Allocate a unique ID for this
	idBit, ok := pushFilter.bitMap.NextClear(0)
	if !ok || idBit >= maxReceivers {
		msg := fmt.Sprintf("Not able to allocate ID for receiver %v", ID)
		log.Error(msg)
		return nil, fmt.Errorf(msg)
	}
	recv := &receiver{bitID: idBit, ID: ID}

	pushFilter.bitMap.Set(idBit)

	pushFilter.idMap[ID] = recv
	pushFilter.watchMap[idBit] = make(map[string]*Watcher)
	return recv, nil
}

//DeleteReceiver delete receiver
func (pf *objPushFilter) DeleteReceiver(recvr objReceiver.Receiver) error {

	pf.Lock()
	defer pf.Unlock()
	var mReceiver objReceiver.Receiver
	var ok bool
	var r *receiver

	pushFilter := pf
	r, ok = recvr.(*receiver)
	if !ok {
		return fmt.Errorf("Delete receiver error : invalid receiver")
	}

	if mReceiver, ok = pushFilter.idMap[r.ID]; !ok {
		return fmt.Errorf("Delete receiver error : %v not added  ", r.ID)
	}

	if mReceiver != r {
		return fmt.Errorf("Delete receiver error : %v receiver don't match current entry ", r.ID)
	}

	delete(pushFilter.idMap, r.ID)
	delete(pushFilter.watchMap, r.bitID)
	pushFilter.bitMap.Clear(r.bitID)
	return nil
}

// ListObjects returns a list of all objects of a kind
func (p *pushDB) ListObjects(kind string, receiverID string) []Object {
	// get objdb, ignorig filter for now

	od := p.getPushObjdb(kind)

	// lock object db
	od.Lock()
	defer od.Unlock()

	recvr, err := p.FindReceiver(receiverID)
	if err != nil {
		return nil
	}

	r, ok := recvr.(*receiver)
	if !ok {
		log.Errorf("Invalid receiver..")
		return nil
	}

	var objs []Object
	for _, pobj := range od.objects {
		if pobj.bitMap.Test(r.bitID) {
			objs = append(objs, pobj.obj)
		}
	}

	return objs
}
