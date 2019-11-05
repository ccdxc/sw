package vcprobe

import (
	"context"
	"errors"
	"fmt"
	"net/url"
	"sync"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	retryDelay = time.Second
)

var (
	connEstablished  = make(chan bool)
	connCheckRunning bool
	connCheckLock    sync.Mutex
)

// Shared fields with the tags client
type vcInst struct {
	vcURL  *url.URL
	VcID   string
	cancel context.CancelFunc
	ctx    context.Context
	Log    log.Logger
	wg     *sync.WaitGroup
}

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	vcInst
	client   *govmomi.Client
	outbox   chan<- defs.Probe2StoreMsg
	inbox    <-chan defs.Store2ProbeMsg
	viewMgr  *view.Manager
	tp       *tagsProbe
	stateMgr *statemgr.Statemgr
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(vcID string, vcURL *url.URL, hOutbox chan<- defs.Probe2StoreMsg, inbox <-chan defs.Store2ProbeMsg, stateMgr *statemgr.Statemgr, l log.Logger) *VCProbe {
	return &VCProbe{
		vcInst: vcInst{
			VcID:  vcID,
			vcURL: vcURL,
			Log:   l.WithContext("submodule", fmt.Sprintf("VCProbe-%s", vcID)),
			wg:    &sync.WaitGroup{},
		},
		outbox:   hOutbox,
		inbox:    inbox,
		stateMgr: stateMgr,
	}
	// Check we have correct permissions when we connect.
}

// Start creates a client and view manager
func (v *VCProbe) Start() error {
	if v.cancel != nil {
		return errors.New("Already started")
	}

	v.ctx, v.cancel = context.WithCancel(context.Background())
	go v.run()
	return nil
}

// Stop stops the sessions
func (v *VCProbe) Stop() {
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
		v.wg.Wait()
	}
}

// Run runs the probe
func (v *VCProbe) run() {
	// Connect and log in to vCenter
	var c *govmomi.Client
	var err error

	// Forever try to login until it succeeds
	for {
		c, err = govmomi.NewClient(v.ctx, v.vcURL, true)
		if err != nil {
			v.Log.Errorf("Login failed: %v", err)
			select {
			case <-v.ctx.Done():
				return
			case <-time.After(5 * retryDelay):
			}
		} else {
			break
		}
	}

	v.client = c
	v.viewMgr = view.NewManager(v.client.Client)
	v.newTagsProbe(v.ctx)

	tryForever := func(fn func()) {
		for v.ctx.Err() == nil {
			fn()
			time.Sleep(retryDelay)
		}
	}
	go tryForever(func() {
		v.startWatch(defs.HostSystem, []string{"config"}, v.sendToStore)
	})
	go tryForever(func() {
		vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
		v.startWatch(defs.VirtualMachine, vmProps, v.sendToStore)
	})
	go tryForever(func() {
		err = v.tp.tc.Login(v.tp.ctx, v.vcURL.User)
		if err != nil {
			return
		}
		v.tp.Start()
	})
}

// caller will be blocked until the connection is re-established
func (v *VCProbe) checkConnectionBlock() {
	// if a connection check routine is not running, we will start one
	// all callers will be blocked till the connection check routine
	// is able to re-establish a connection
	periodicCheck := func() {
		v.Log.Infof("connection check starting...")
		defer v.wg.Done()
		count := 0
		active := false
		for !active {
			select {
			case <-v.ctx.Done():
				return
			case <-time.After(retryDelay):
				var err error
				active, err = v.client.SessionManager.SessionIsActive(v.ctx)
				if err != nil {
					v.Log.Errorf("Received err %v while testing connection", err)
				} else if active {
					continue
				}

				if count == 5 {
					// Failed to reconnect for 5 seconds, set status
					// TODO: set connection status in apiserver
				}
				count++
			}
		}
		// connection has been re-established
		// TODO: Update connection status in apiserver
		v.Log.Infof("connection has been re-established")

		connCheckLock.Lock()
		// Notify watchers
		close(connEstablished)
		connCheckRunning = false
		// Reset channel for future watchers
		connEstablished = make(chan bool)
		connCheckLock.Unlock()
	}

	connCheckLock.Lock()
	if !connCheckRunning {
		connCheckRunning = true
		v.wg.Add(1)
		go periodicCheck()
	}
	// Save a reference to it, as periodicCheck will reset it
	// once it reconnects
	ch := connEstablished
	connCheckLock.Unlock()
	// wait until the connection has been established again
	select {
	case <-v.ctx.Done():
		return
	case <-ch:
		return
	}
}

func (v *VCProbe) startWatch(vcKind defs.VCObject, props []string, updateFn func(key string, obj defs.VCObject, pc []types.PropertyChange)) {
	kind := string(vcKind)

	var err error
	v.wg.Add(1)
	defer v.wg.Done()
	root := v.client.ServiceContent.RootFolder
	kinds := []string{}

	cView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		v.Log.Fatalf("CreateContainerView returned %v", err)
		return
	}

	// Fields to watch for change
	objRef := types.ManagedObjectReference{Type: kind}
	filter := new(property.WaitFilter).Add(cView.Reference(), objRef.Type, props, cView.TraversalSpec())
	updFunc := func(updates []types.ObjectUpdate) bool {
		for _, update := range updates {
			if update.Obj.Type != kind {
				v.Log.Errorf("Expected %s, got %+v", kind, update.Obj)
				continue
			}
			key := update.Obj.Value
			updateFn(key, vcKind, update.ChangeSet)
		}
		// Must return false, returning true will cause waitForUpdates to exit.
		return false
	}

	for {
		err = property.WaitForUpdates(v.ctx, property.DefaultCollector(v.client.Client), filter, updFunc)

		if err != nil {
			v.Log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ctx.Err() != nil {
			return
		}
		v.Log.Infof("%s property.WaitForView exited, checking connection", kind)
		v.checkConnectionBlock()
		v.Log.Infof("%s property.WaitForView, retrying...", kind)
	}
}

func (v *VCProbe) sendToStore(key string, obj defs.VCObject, pc []types.PropertyChange) {
	m := defs.Probe2StoreMsg{
		VcObject:   obj,
		Key:        key,
		Changes:    pc,
		Originator: v.VcID,
	}
	v.Log.Debugf("Sending message to store, key: %s, obj: %s, props: ", key, obj, pc)
	v.outbox <- m
}

// Sync triggers the probe to sync its inventory
func (v *VCProbe) Sync() {
}
