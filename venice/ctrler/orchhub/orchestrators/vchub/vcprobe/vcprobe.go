package vcprobe

import (
	"context"
	"errors"
	"fmt"
	"net/url"
	"sync"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
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
	finder *find.Finder
	ctx    context.Context
	Log    log.Logger
	wg     *sync.WaitGroup
}

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	vcInst
	client     *govmomi.Client
	outbox     chan<- defs.Probe2StoreMsg
	inbox      <-chan defs.Store2ProbeMsg
	viewMgr    *view.Manager
	tp         *tagsProbe
	stateMgr   *statemgr.Statemgr
	orchConfig *orchestration.Orchestrator
	dvsMap     map[string]*PenDVS
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(orchConfig *orchestration.Orchestrator, hOutbox chan<- defs.Probe2StoreMsg, inbox <-chan defs.Store2ProbeMsg, stateMgr *statemgr.Statemgr, l log.Logger, scheme string) *VCProbe {
	vcURL := &url.URL{
		Scheme: scheme,
		Host:   orchConfig.Spec.URI,
		Path:   "/sdk",
	}

	vcURL.User = url.UserPassword(orchConfig.Spec.Credentials.UserName, orchConfig.Spec.Credentials.Password)

	return &VCProbe{
		vcInst: vcInst{
			VcID:  orchConfig.GetName(),
			vcURL: vcURL,
			Log:   l.WithContext("submodule", fmt.Sprintf("VCProbe-%s", orchConfig.GetName())),
			wg:    &sync.WaitGroup{},
		},
		orchConfig: orchConfig,
		outbox:     hOutbox,
		inbox:      inbox,
		stateMgr:   stateMgr,
		dvsMap:     make(map[string]*PenDVS),
	}
	// Check we have correct permissions when we connect.
}

// Start creates a client and view manager
func (v *VCProbe) Start() error {
	if v.cancel != nil {
		return errors.New("Already started")
	}
	v.ctx, v.cancel = context.WithCancel(context.Background())
	// Connect and log in to vCenter
	v.wg.Add(1)
	go v.run()
	return nil
}

// Stop stops the sessions
func (v *VCProbe) Stop() {
	v.Log.Info("Stopping vcprobe")
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
		v.wg.Wait()
	}
}

// Run runs the probe
func (v *VCProbe) run() {
	defer v.wg.Done()
	// Connect and log in to vCenter
	var c *govmomi.Client
	var err error

	v.Log.Infof("Starting VCProbe : %v", v.vcURL)
	// Forever try to login until it succeeds
	for {
		v.Log.Infof("Starting VCProbe : %v", v.vcURL)
		c, err = govmomi.NewClient(v.ctx, v.vcURL, true)
		if err != nil {
			o, err := v.stateMgr.Controller().Orchestrator().Find(&v.orchConfig.ObjectMeta)
			if err == nil {
				log.Infof("Updating orchestrator connection status to %v", orchestration.OrchestratorStatus_Failure.String())
				o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Failure.String()
				o.Write()
			}
			v.Log.Errorf("Login failed: %v", err)
			select {
			case <-v.ctx.Done():
				return
			case <-time.After(5 * retryDelay):
			}
		} else {
			o, err := v.stateMgr.Controller().Orchestrator().Find(&v.orchConfig.ObjectMeta)
			if err == nil {
				log.Infof("Updating orchestrator connection status to %v", orchestration.OrchestratorStatus_Success.String())
				o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Success.String()
				o.Write()
			}
			break
		}
	}

	v.client = c
	v.finder = find.NewFinder(v.client.Client, true)
	v.viewMgr = view.NewManager(v.client.Client)
	v.newTagsProbe(v.ctx)

	v.wg.Add(1)
	go v.storeListen()
	// Perform a List and reconciliation of all the VCProbe objects synchronously before starting all the watches
	v.Sync()

	tryForever := func(fn func()) {
		v.wg.Add(1)
		go func() {
			defer v.wg.Done()
			for v.ctx.Err() == nil {
				fn()
				time.Sleep(retryDelay)
			}
		}()
	}

	tryForever(func() {
		v.wg.Add(1)
		v.startStatemgrWatch()
	})

	tryForever(func() {
		v.startWatch(defs.HostSystem, []string{"config"}, v.sendVCEvent)
	})

	tryForever(func() {
		vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
		v.startWatch(defs.VirtualMachine, vmProps, v.sendVCEvent)
	})

	tryForever(func() {
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

func (v *VCProbe) storeListen() {
	defer v.wg.Done()

	for {
		select {
		case <-v.ctx.Done():
			return

		case m, active := <-v.inbox:
			if !active {
				return
			}

			v.Log.Debugf("Store listen received %s, %v", m.MsgType, m.Val)
			switch m.MsgType {
			case defs.Useg:
				v.handleUseg(m.Val.(defs.UsegMsg))
			default:
				v.Log.Errorf("Unknown msg type %s", m.MsgType)
				continue
			}
		}
	}
}

func (v *VCProbe) handleUseg(msg defs.UsegMsg) {
	// TODO: implement
}

func (v *VCProbe) startWatch(vcKind defs.VCObject, props []string, updateFn func(key string, obj defs.VCObject, pc []types.PropertyChange)) {
	kind := string(vcKind)

	var err error
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

func (v *VCProbe) handleNetworkEvent(evtType kvstore.WatchEventType, nw *network.Network) {
	v.Log.Infof("Handling network event. %v", nw)

	switch evtType {
	case kvstore.Created:
		v.Log.Info("Create network event")
	case kvstore.Updated:
		v.Log.Info("Update network event")
	case kvstore.Deleted:
		v.Log.Info("Delete network event")
	}
}

func (v *VCProbe) startStatemgrWatch() {
	defer v.wg.Done()
	v.Log.Infof("Starting network objects watch for %s", v.orchConfig.Name)
	probeChannel, err := v.stateMgr.GetProbeChannel(v.orchConfig.GetKey())
	if err != nil {
		v.Log.Errorf("Could not get probe channel for %s. Err : %v", v.orchConfig.GetKey(), err)
		return
	}

	for {
		select {
		case <-v.ctx.Done():
			v.Log.Info("Network watch done")
			return
		case evt, ok := <-probeChannel:
			if ok {
				nw := evt.Object.(*network.Network)
				v.handleNetworkEvent(evt.Type, nw)
			}
		}
	}
}

func (v *VCProbe) sendVCEvent(key string, obj defs.VCObject, pc []types.PropertyChange) {
	m := defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   obj,
			Key:        key,
			Changes:    pc,
			Originator: v.VcID,
		},
	}
	v.Log.Debugf("Sending message to store, key: %s, obj: %s, props: ", key, obj, pc)
	v.outbox <- m
}

// Sync triggers the probe to sync its inventory
func (v *VCProbe) Sync() {
	v.Log.Infof("VCProbe %v synching.", v)
	opts := api.ListWatchOptions{}
	opts.LabelSelector = fmt.Sprintf("%s=%s", utils.OrchNameKey, v.orchConfig.GetKey())
	// By the time this is called, the statemanager would have setup watches to the API server
	// and synched the local cache with the API server
	nw, err := v.stateMgr.Controller().Network().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Failed to get network list. Err : %v", err)
	}

	hosts, err := v.stateMgr.Controller().Host().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Failed to get host list. Err : %v", err)
	}

	workloads, err := v.stateMgr.Controller().Workload().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Failed to get workload list. Err : %v", err)
	}

	// TODO remove the prints below. Added to calm down go fmt
	log.Infof("Got Networks : %v", nw)
	log.Infof("Got Workloads : %v", workloads)
	log.Infof("Got Hosts : %v", hosts)
	// TODO get all the vcenter objects

	log.Infof("Sync done for VCProbe. %v", v)
}
