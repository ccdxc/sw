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
	retryCount = 3
)

// Shared fields with the tags client
type vcInst struct {
	vcURL            *url.URL
	VcID             string
	ctx              context.Context
	cancel           context.CancelFunc
	finder           *find.Finder
	Log              log.Logger
	wg               *sync.WaitGroup
	watcherWg        *sync.WaitGroup
	watcherCtx       context.Context
	watcherCancel    context.CancelFunc
	checkSession     bool
	checkSessionLock *sync.Mutex
}

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	vcInst
	client      *govmomi.Client
	outbox      chan<- defs.Probe2StoreMsg
	inbox       <-chan defs.Store2ProbeMsg
	viewMgr     *view.Manager
	tp          *tagsProbe
	stateMgr    *statemgr.Statemgr
	orchConfig  *orchestration.Orchestrator
	dvsMap      map[string]*PenDVS
	vcProbeLock sync.Mutex
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
			VcID:             orchConfig.GetName(),
			vcURL:            vcURL,
			Log:              l.WithContext("submodule", fmt.Sprintf("VCProbe-%s", orchConfig.GetName())),
			wg:               &sync.WaitGroup{},
			watcherWg:        &sync.WaitGroup{},
			checkSessionLock: &sync.Mutex{},
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
	v.vcProbeLock.Lock()
	v.ctx, v.cancel = context.WithCancel(context.Background())
	v.wg.Add(1)
	go v.storeListen()
	v.wg.Add(1)
	go v.startStatemgrWatch()
	// Connect and log in to vCenter
	v.wg.Add(1)
	go v.run()
	v.vcProbeLock.Unlock()
	return nil
}

// Stop stops the sessions
func (v *VCProbe) Stop() {
	v.Log.Info("Stopping vcprobe")
	v.vcProbeLock.Lock()
	if v.cancel != nil {
		v.cancel()
		v.wg.Wait()
		v.cancel = nil
	}
	v.watcherWg.Wait()
	v.watcherCancel = nil
	if v.client != nil {
		v.client.Logout(v.ctx)
		v.client = nil
	}
	v.vcProbeLock.Unlock()
}

// Run runs the probe
func (v *VCProbe) run() {
	defer v.wg.Done()
	// Connect and log in to vCenter
	var c *govmomi.Client

	for {
		if v.ctx.Err() != nil {
			return
		}
		// derive from ctx
		v.watcherCtx, v.watcherCancel = context.WithCancel(v.ctx)
		// Forever try to login until it succeeds
		for {
			o, err := v.stateMgr.Controller().Orchestrator().Find(&v.orchConfig.ObjectMeta)
			if err != nil {
				// orchestrator object does not exists anymore, not need to run the probe
				v.Log.Infof("Orchestrator Object %v does not exist, no need to start the probe",
					v.orchConfig.GetKey())
				return
			}
			v.Log.Infof("Starting VCProbe : %v", v.vcURL)
			c, err = govmomi.NewClient(v.watcherCtx, v.vcURL, true)
			if err == nil {
				v.Log.Infof("Updating orchestrator connection status to %v",
					orchestration.OrchestratorStatus_Success.String())
				o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Success.String()
				o.Write()
				break
			}
			v.Log.Infof("Updating orchestrator connection status to %v",
				orchestration.OrchestratorStatus_Failure.String())
			o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Failure.String()
			o.Write()

			v.Log.Errorf("Login failed: %v", err)
			select {
			case <-v.ctx.Done():
				return
			case <-time.After(5 * retryDelay):
			}
		}

		v.client = c
		v.finder = find.NewFinder(v.client.Client, true)
		v.viewMgr = view.NewManager(v.client.Client)
		v.newTagsProbe()

		// Perform a List and reconciliation of all the VCProbe objects synchronously
		// before starting all the watchers
		// TODO: return if sync failed (lost connection to vCenter etc)
		v.Sync()

		// start watchers on vCenter objects/events
		v.startWatchers()

		// Run periodic check on the session, if session is dead, Stop and Restart
		v.Log.Infof("Session check starting...")
		count := retryCount
		for count > 0 {
			select {
			case <-v.ctx.Done():
				return
			case <-time.After(retryDelay):
				// if any of the watchers is experiencing the problem, check the session state
				if v.checkSession {
					active, err := v.client.SessionManager.SessionIsActive(v.watcherCtx)
					if err != nil {
						v.Log.Errorf("Received err %v while testing session", err)
						count--
					} else if active {
						count = retryCount
						v.checkSessionLock.Lock()
						v.checkSession = false
						v.checkSessionLock.Unlock()
					} else {
						v.Log.Infof("Session is not active .. retry")
						count--
					}
				}
			}
		}
		// Logout, stop watchers and retry
		if v.watcherCancel != nil {
			v.watcherCancel()
		}
		v.watcherWg.Wait()
		if v.client != nil {
			v.client.Logout(v.watcherCtx)
		}
		v.client = nil
		v.watcherCancel = nil
	}
}

func (v *VCProbe) startWatchers() {
	v.checkSession = false
	tryForever := func(fn func()) {
		v.watcherWg.Add(1)
		go func() {
			defer v.watcherWg.Done()
			for v.watcherCtx.Err() == nil {
				fn()
				time.Sleep(retryDelay)
			}
		}()
	}

	tryForever(func() {
		v.startWatch(defs.HostSystem, []string{"config"}, v.sendVCEvent)
	})

	tryForever(func() {
		vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
		v.startWatch(defs.VirtualMachine, vmProps, v.sendVCEvent)
	})

	tryForever(func() {
		err := v.tp.tc.Login(v.tp.ctx, v.vcURL.User)
		if err != nil {
			return
		}
		v.tp.Start()
	})
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

	cView, err := v.viewMgr.CreateContainerView(v.watcherCtx, root, kinds, true)
	if err != nil {
		v.Log.Errorf("CreateContainerView returned %v", err)
		v.checkSessionLock.Lock()
		v.checkSession = true
		v.checkSessionLock.Unlock()
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
		err = property.WaitForUpdates(v.watcherCtx, property.DefaultCollector(v.client.Client), filter, updFunc)

		if err != nil {
			v.Log.Errorf("property.WaitForView returned %v", err)
		}

		if v.watcherCtx.Err() != nil {
			return
		}
		v.Log.Infof("%s property.WaitForView exited", kind)
		v.checkSessionLock.Lock()
		v.checkSession = true
		v.checkSessionLock.Unlock()
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
	// By the time this is called, the statemanager would have setup watchers to the API server
	// and synched the local cache with the API server
	nw, err := v.stateMgr.Controller().Network().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get network list. Err : %v", err)
	}

	hosts, err := v.stateMgr.Controller().Host().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get host list. Err : %v", err)
	}

	workloads, err := v.stateMgr.Controller().Workload().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get workload list. Err : %v", err)
	}

	// TODO remove the prints below. Added to calm down go fmt
	v.Log.Infof("Got Networks : %v", nw)
	v.Log.Infof("Got Workloads : %v", workloads)
	v.Log.Infof("Got Hosts : %v", hosts)
	// TODO get all the vcenter objects

	v.Log.Infof("Sync done for VCProbe. %v", v)
}
