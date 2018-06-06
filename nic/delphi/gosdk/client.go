package gosdk

import (
	"fmt"
	"log"
	"time"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/gosdk/messenger"
	"github.com/pensando/sw/nic/delphi/messanger/proto"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// Service Implement this to use the sdk
type Service interface {
	OnMountComplete()
	Name() string
}

// Client This is the main SDK Client API
type Client interface {
	// Mount a kind to get notifications and/or make changes to the objects.
	// This *MUST* be called before the `Dial` function
	MountKind(kind string, mode delphi.MountMode) error
	// Dial establishes connection to the HUB
	Dial() error
	// SetObject notifies about changes to an object. The user doesn't need to
	// call this explicitly. It is getting called automatically when there is a
	// change in any object.
	SetObject(obj BaseObject) error
	// Delete object, as it names sugests, deletes an object from the database.
	// Users can use this, or just call <OBJECT>.Delete()
	DeleteObject(obj BaseObject) error
	// WathcKind is used internally by the object to register reactors. Users
	// should not call this directly
	WatchKind(kind string, reactor BaseReactor) error
	// Close, as the name suggests, closes the connection to the hub.
	Close()
	// DumpSubtrees prints the local database state to stderr. It's meant to be
	// used for debugging purposes.
	DumpSubtrees()
}

// The main map holding the objects in the local copy of the databse.
type subtree map[string]BaseObject

// Represents a change in on object, used to send changes through a channel
type change struct {
	obj BaseObject
	op  delphi.ObjectOperation
}

// The internal state for the client.
type client struct {
	id          uint16
	nextObjID   uint32
	mclient     messenger.Client
	service     Service
	mounts      []*delphi_messanger.MountData
	watchers    map[string][]BaseReactor
	subtrees    map[string]subtree
	changeQueue chan *change
}

// Mount a kind to get notifications and/or make changes to the objects. Must
// be called before the calling `Dial`
func (c *client) MountKind(kind string, mode delphi.MountMode) error {

	// FIXME: error out if already connected
	c.mounts = append(c.mounts, &delphi_messanger.MountData{
		Kind: kind,
		Mode: mode,
	})

	return nil
}

// Dial establishes connection to the HUB. Users must not `MountKind` after
// they called `Dial``
func (c *client) Dial() error {
	err := c.mclient.Dial()
	if err != nil {
		return err
	}

	err = c.mclient.SendMountReq(c.service.Name(), c.mounts)
	if err != nil {
		return err
	}

	go c.run()

	return nil
}

// SetObject notifies about changes to an object. The user doesn't need to
// call this explicitly. It is getting called automatically when there is a
// change in any object.
func (c *client) SetObject(obj BaseObject) error {
	obj.GetMeta().Key = obj.GetKeyString()
	obj.GetMeta().Handle = c.newHandle()

	c.changeQueue <- &change{
		obj: obj,
		op:  delphi.ObjectOperation_SetOp,
	}
	return nil
}

// Delete object, as it names sugests, deletes an object from the database.
// Users can use this, or just call <OBJECT>.Delete()
func (c *client) DeleteObject(obj BaseObject) error {
	c.changeQueue <- &change{
		obj: obj,
		op:  delphi.ObjectOperation_DeleteOp,
	}
	return nil
}

// WathcKind is used internally by the object to register reactors. Users
// should not call this directly
func (c *client) WatchKind(kind string, reactor BaseReactor) error {
	rl := c.watchers[kind]
	if rl == nil {
		rl = make([]BaseReactor, 0)
	}
	rl = append(rl, reactor)
	c.watchers[kind] = rl
	return nil
}

// Close the connection the the hub
func (c *client) Close() {
	c.mclient.Close()
}

// sendBatch is responsible for marshaling and sending over to the hub a
// number of changes
func (c *client) sendBatch(batch map[string]*change) error {
	objlist := make([]*delphi_messanger.ObjectData, 0)

	for _, chg := range batch {
		data, err := proto.Marshal(chg.obj.GetMessage())
		if err != nil {
			panic(err)
		}
		objlist = append(objlist,
			&delphi_messanger.ObjectData{
				Meta: chg.obj.GetMeta(),
				Op:   chg.op,
				Data: data,
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
func (c *client) run() {
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
			pending[change.obj.GetKeyString()] = change
			if tRunning == false {
				t.Reset(time.Millisecond * 5)
			}
		}
	}
}

// This function gets called when there are local or remove changes to the
// database. It invokes the reactors
func (c *client) updateSubtrees(objlist []*delphi_messanger.ObjectData) {
	for _, obj := range objlist {
		factory := factories[obj.Meta.Kind]
		baseObj, err := factory(c, obj.Data)
		if err != nil {
			panic(err)
		} else {
			switch obj.GetOp() {
			case delphi.ObjectOperation_SetOp:
				subtr := c.subtrees[obj.GetMeta().GetKind()]
				if subtr == nil {
					subtr = make(subtree)
					c.subtrees[obj.GetMeta().GetKind()] = subtr
				}
				subtr[obj.GetMeta().GetKey()] = baseObj
			case delphi.ObjectOperation_DeleteOp:
				subtr := c.subtrees[obj.GetMeta().GetKind()]
				if subtr == nil {
					break
				}
				delete(subtr, obj.GetMeta().GetKey())
			default:
				panic(fmt.Sprintf("Unknown operation %+v", obj.GetOp()))
			}
			// FIXME: move somewhere else?
			rl := c.watchers[obj.GetMeta().GetKind()]
			if rl != nil {
				baseObj.TriggerEvent(nil, obj.GetOp(), rl)
			}
		}
	}
}

// Implementing the messegner.Handler interface
func (c *client) HandleMountResp(svcID uint16, status string, objlist []*delphi_messanger.ObjectData) error {
	c.id = svcID
	c.updateSubtrees(objlist)
	c.service.OnMountComplete()
	return nil
}

// Implementing the messegner.Handler interface
func (c *client) HandleNotify(objlist []*delphi_messanger.ObjectData) error {
	c.updateSubtrees(objlist)
	return nil
}

// Implementing the messegner.Handler interface
func (c *client) HandleStatusResp() error {
	return nil
}

// Dump the database state to the stderr for debugging purposes
func (c *client) DumpSubtrees() {
	for _, subtr := range c.subtrees {
		for key, obj := range subtr {
			log.Printf("%v -> %+v\n", key, obj)
		}
	}
}

func (c *client) newHandle() uint64 {
	var h uint64
	c.nextObjID++
	h = uint64(c.id)<<48 | uint64(c.nextObjID)
	return h
}

// NewClient should be called to create a new Delphi Client. A process is allowed
// to have more than one clients at the same time.
func NewClient(service Service) (Client, error) {
	client := &client{
		mounts:      make([]*delphi_messanger.MountData, 0),
		service:     service,
		subtrees:    make(map[string]subtree),
		watchers:    make(map[string][]BaseReactor),
		changeQueue: make(chan *change),
	}

	mc, err := messenger.NewClient(client)
	if err != nil {
		return nil, err
	}
	client.mclient = mc

	return client, nil
}
