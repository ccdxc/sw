package gosdk

import (
	"fmt"
	"sync"
	"time"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/gosdk/messenger"
	delphi_messenger "github.com/pensando/sw/nic/delphi/messenger/proto"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/venice/utils/log"
)

// maxQueueDepth maximum queue depth of update queue
const maxQueueDepth = 1000

// The main map holding the objects in the local copy of the databse.
type subtree map[string]clientApi.BaseObject

// Represents a change in on object, used to send changes through a channel
type change struct {
	obj clientApi.BaseObject
	op  delphi.ObjectOperation
}

// The internal state for the client.
type client struct {
	id             uint16
	nextObjID      uint32
	mclient        messenger.Client
	service        clientApi.Service
	mounts         []*delphi_messenger.MountData
	mountListeners []clientApi.MountListener
	watchers       map[string][]clientApi.BaseReactor
	subtrees       map[string]subtree
	changeQueue    chan *change
	globalLock     *sync.Mutex
	stopChan       chan bool
	isConnected    bool
	isMountAllowed bool
	revIndex       map[string]string
}

// Mount a kind to get notifications and/or make changes to the objects. Must
// be called before the calling `Dial`
func (c *client) MountKind(kind string, mode delphi.MountMode) error {
	log.Infof("Delphi: MountKind(): Kind: %v", kind)
	if !c.isMountAllowed {
		log.Fatalf("Delphi: Mount called after connection")
	}
	// FIXME: error out if already connected
	c.mounts = append(c.mounts, &delphi_messenger.MountData{
		Kind: kind,
		Mode: mode,
	})

	return nil
}

// Mount a kind to get notifications and/or make changes to the objects. Must
// be called before the calling `Dial`
func (c *client) MountKindKey(kind string, key string, mode delphi.MountMode) error {
	log.Infof("Delphi: MountKindKey() Kind: %v", kind)
	if !c.isMountAllowed {
		log.Fatalf("Delphi: Mount called after connection")
	}
	// FIXME: error out if already connected
	c.mounts = append(c.mounts, &delphi_messenger.MountData{
		Kind: kind,
		Key:  key,
		Mode: mode,
	})

	return nil
}

// Run runs the main event loop in the background
func (c *client) Run() {
	c.isMountAllowed = false
	err := c.connect()
	if err != nil {
		log.Fatalf("Error connecting to hub. Err: %v", err)
	}

	// send mount request
	err = c.mclient.SendMountReq(c.service.Name(), c.mounts)
	if err != nil {
		log.Fatalf("Error sending mount request to hub. Err: %v", err)
	}

	// run the event loop in the background
	c.loop()
}

// connect establishes connection to the HUB. Users must not `MountKind` after
// they called `Run``
func (c *client) connect() error {
	var connected bool

	// keep retrying to connect to delphi hib
	for !connected {
		err := c.mclient.Dial()
		if err != nil {
			//log.Errorf("Error connecting to delphi hub. Retrying.. Err: %v", err)
			time.Sleep(time.Second)
		} else {
			connected = true
		}
	}

	return nil
}

// IsConnected returns true if client is connected to dlephi hub
func (c *client) IsConnected() bool {
	return c.isConnected
}

// SetObject notifies about changes to an object. The user doesn't need to
// call this explicitly. It is getting called automatically when there is a
// change in any object.
func (c *client) SetObject(obj clientApi.BaseObject) error {
	log.Infof("Delphi: Client set object: %v", obj.GetDelphiKey())
	meta := obj.GetDelphiMeta()
	if meta == nil {
		obj.SetDelphiMeta(&delphi.ObjectMeta{
			Kind:   obj.GetDelphiKind(),
			Key:    obj.GetDelphiKey(),
			Path:   obj.GetDelphiPath(),
			Handle: c.newHandle(),
		})
	}

	c.queueChange(&change{
		obj: obj,
		op:  delphi.ObjectOperation_SetOp,
	})

	return nil
}

// GetObject returns the object of kind `kind` with key `key` if it
// exists in the local database, else it return nil
func (c *client) GetObject(kind string, key string) clientApi.BaseObject {
	c.globalLock.Lock()
	defer c.globalLock.Unlock()
	subtree := c.subtrees[kind]
	if subtree == nil {
		return nil
	}

	return subtree[key]
}

// Delete object, as it names sugests, deletes an object from the database.
// Users can use this, or just call <OBJECT>.Delete()
func (c *client) DeleteObject(obj clientApi.BaseObject) error {
	meta := obj.GetDelphiMeta()
	if meta == nil {
		obj.SetDelphiMeta(&delphi.ObjectMeta{
			Kind:   obj.GetDelphiKind(),
			Key:    obj.GetDelphiKey(),
			Path:   obj.GetDelphiPath(),
			Handle: c.newHandle(),
		})
	}
	c.queueChange(&change{
		obj: obj,
		op:  delphi.ObjectOperation_DeleteOp,
	})
	return nil
}

func (c *client) queueChange(change *change) {
	log.Infof("Delphi: Change queued: %v %v", change.op, change.obj.GetDelphiKey())
	c.changeQueue <- change
	// update subtree now so a back to back Set/Get will work
	c.updateSubtree(change.op, change.obj.GetDelphiMeta().Kind,
		change.obj.GetDelphiKey(), change.obj)
}

// WathcKind is used internally by the object to register reactors. Users
// should not call this directly
func (c *client) WatchKind(kind string, reactor clientApi.BaseReactor) error {
	rl := c.watchers[kind]
	if rl == nil {
		rl = make([]clientApi.BaseReactor, 0)
	}
	rl = append(rl, reactor)
	c.watchers[kind] = rl
	return nil
}

// WatchMount
func (c *client) WatchMount(listener clientApi.MountListener) error {
	c.mountListeners = append(c.mountListeners, listener)
	return nil
}

// GetFromIndex implementation
func (c *client) GetFromIndex(toKind string, fromKind string,
	fromKeyField string, toKey string) clientApi.BaseObject {

	key := c.revIndex[toKind+":"+fromKind+":"+fromKeyField+":"+toKey]

	return c.GetObject(fromKind, key)
}

// Close the connection the the hub
func (c *client) Close() {
	c.mclient.Close()
	close(c.stopChan)
}

// sendBatch is responsible for marshaling and sending over to the hub a
// number of changes
func (c *client) sendBatch(batch map[string]*change) error {
	objlist := make([]*delphi_messenger.ObjectData, 0)

	for _, chg := range batch {
		data, err := proto.Marshal(chg.obj.GetDelphiMessage())
		if err != nil {
			panic(err)
		}
		objlist = append(objlist,
			&delphi_messenger.ObjectData{
				Meta:    chg.obj.GetDelphiMeta(),
				Op:      chg.op,
				Persist: chg.obj.IsPersistent(),
				Data:    data,
			},
		)
	}

	err := c.mclient.SendChangeReq(objlist)
	if err != nil {
		return err
	}

	return nil
}

// A loop that listens for local changes to the database and send the changes
// over to the hub in given intervals
func (c *client) loop() {
	pending := make(map[string]*change)
	t := time.NewTimer(time.Millisecond * 5)
	tRunning := true
	for {
		select {
		case _ = <-t.C:
			tRunning = false
			if len(pending) > 0 {
				c.sendBatch(pending)
				pending = make(map[string]*change)
			}
		case change := <-c.changeQueue:
			o := change.obj
			path := o.GetDelphiMeta().GetKind() + ":" + o.GetDelphiKey()
			pending[path] = change
			if tRunning == false {
				t.Reset(time.Millisecond * 5)
			}
		case _, ok := <-c.stopChan:
			if !ok {
				log.Warnf("Stopping event loop")
				return
			}
		}
	}
}

// Update the subtree for a single object
func (c *client) updateSubtree(op delphi.ObjectOperation, kind string,
	key string, obj clientApi.BaseObject) {
	c.globalLock.Lock()
	defer c.globalLock.Unlock()
	switch op {
	case delphi.ObjectOperation_SetOp:
		subtr := c.subtrees[kind]
		if subtr == nil {
			subtr = make(subtree)
			c.subtrees[kind] = subtr
		}
		subtr[key] = obj
	case delphi.ObjectOperation_DeleteOp:
		subtr := c.subtrees[kind]
		if subtr == nil {
			break
		}
		delete(subtr, key)
	default:
		panic(fmt.Sprintf("Unknown operation %+v", op))
	}
}

// This function gets called when there are local or remote changes to the
// database. It invokes the reactors
func (c *client) updateSubtrees(objlist []*delphi_messenger.ObjectData, triggerEvents bool) {
	for _, obj := range objlist {
		factory := clientApi.Factories[obj.Meta.Kind]
		log.Infof("Delphi: Factory: %v", factory)
		log.Infof("Delphi: Obj: %v", obj)
		baseObj, err := factory(c, obj.Data)
		oldObj := c.GetObject(obj.Meta.Kind, obj.Meta.Key)
		if err != nil {
			log.Fatalf("Error unmarshalling object %+v. Err: %v", obj, err)
		} else {
			kind := obj.GetMeta().GetKind()
			key := obj.GetMeta().GetKey()
			c.updateSubtree(obj.GetOp(), kind, key, baseObj)
			// FIXME: update indexes, move somewhere else?
			index := clientApi.Indexes[kind]
			if index != nil {
				for field, toKind := range index {
					fke := clientApi.ForeignKeyExtractors[kind][field]
					toKey := fke(baseObj)
					c.revIndex[toKind+":"+kind+":"+field+":"+toKey] = key
				}
			}
			// FIXME: trigger events, also move somewhere else?
			if triggerEvents {
				rl := c.watchers[kind]
				if rl != nil {
					start := time.Now()
					baseObj.TriggerEvent(c, oldObj, obj.GetOp(), rl)
					duration := time.Since(start)
					log.Infof("Delphi: Reactor took: %v", duration)
					if duration.Seconds() > .100 {
						log.Errorf("Delphi: Reactor took more than 100ms")
					}
				}
			}
		}
	}
}

// Implementing the messegner.Handler interface
func (c *client) HandleMountResp(svcID uint16, status string, objlist []*delphi_messenger.ObjectData) error {
	// save the objects
	c.id = svcID
	c.updateSubtrees(objlist, false)

	// remember we are already connected
	c.isConnected = true

	// trigger mount complete callbacks
	c.service.OnMountComplete()
	for _, l := range c.mountListeners {
		l.OnMountComplete()
	}
	return nil
}

// Implementing the messegner.Handler interface
func (c *client) HandleNotify(objlist []*delphi_messenger.ObjectData) error {
	c.updateSubtrees(objlist, true)
	return nil
}

// Implementing the messegner.Handler interface
func (c *client) HandleStatusResp() error {
	return nil
}

// Dump the database state to the stderr for debugging purposes
func (c *client) DumpSubtrees() {
	for kind, subtr := range c.subtrees {
		for key, obj := range subtr {
			log.Infof("'%v'-'%v' -> '%+v'\n", kind, key, obj)
		}
	}
}

// DumpIndex dumps the indexes
func (c *client) DumpIndex() {
	for ck, k := range c.revIndex {
		log.Infof("'%v' -> '%v'\n", ck, k)
	}
}

// List all the objects in the database of a specific kind
func (c *client) List(kind string) []clientApi.BaseObject {
	c.globalLock.Lock()
	defer c.globalLock.Unlock()

	var objects []clientApi.BaseObject

	subtr, ok := c.subtrees[kind]
	if !ok {
		return objects
	}

	for _, obj := range subtr {
		objects = append(objects, obj)
	}

	return objects
}

func (c *client) newHandle() uint64 {
	var h uint64
	c.nextObjID++
	h = uint64(c.id)<<48 | uint64(c.nextObjID)
	return h
}

// NewClient should be called to create a new Delphi Client. A process is allowed
// to have more than one clients at the same time.
func NewClient(service clientApi.Service) (clientApi.Client, error) {
	client := &client{
		mounts:         make([]*delphi_messenger.MountData, 0),
		service:        service,
		subtrees:       make(map[string]subtree),
		watchers:       make(map[string][]clientApi.BaseReactor),
		changeQueue:    make(chan *change, maxQueueDepth),
		mountListeners: make([]clientApi.MountListener, 0),
		globalLock:     &sync.Mutex{},
		stopChan:       make(chan bool),
		revIndex:       make(map[string]string),
		isMountAllowed: true,
		isConnected:    false,
	}

	mc, err := messenger.NewClient(client)
	if err != nil {
		return nil, err
	}
	client.mclient = mc

	return client, nil
}
