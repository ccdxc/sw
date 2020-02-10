package vcprobe

import (
	"errors"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/session"
)

const (
	retryDelay = time.Second
)

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	*defs.State
	*session.Session
	outbox      chan<- defs.Probe2StoreMsg
	tp          *tagsProbe
	Started     bool
	vcProbeLock sync.Mutex
}

// ProbeInf is the interface Probe implements
type ProbeInf interface {
	Start() error
	IsSessionReady() bool
	ClearState()
	StartWatchers()
	StartEventReceiver(refs []types.ManagedObjectReference)
	TestReceiveEvents(ref types.ManagedObjectReference, events []types.BaseEvent)
	ListVM(dcRef *types.ManagedObjectReference) []mo.VirtualMachine
	ListDC() []mo.Datacenter
	ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch
	ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup
	ListHosts(dcRef *types.ManagedObjectReference) []mo.HostSystem

	// port_group.go functions
	AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec) error
	GetPenPG(dcName string, pgName string) (*object.DistributedVirtualPortgroup, error)
	GetPGConfig(dcName string, pgName string, ps []string) (*mo.DistributedVirtualPortgroup, error)
	RenamePG(dcName, oldName string, newName string) error
	RemovePenPG(dcName, pgName string) error

	// distributed_vswitch.go functions
	AddPenDVS(dcName string, dvsCreateSpec *types.DVSCreateSpec) error
	GetPenDVS(dcName, dvsName string) (*object.DistributedVirtualSwitch, error)
	UpdateDVSPortsVlan(dcName, dvsName string, portsSetting PenDVSPortSettings) error
	GetPenDVSPorts(dcName, dvsName string, criteria *types.DistributedVirtualSwitchPortCriteria) ([]types.DistributedVirtualPort, error)
	RemovePenDVS(dcName, dvsName string) error

	// Tag methods
	TagObjAsManaged(ref types.ManagedObjectReference) error
	RemoveTagObjManaged(ref types.ManagedObjectReference) error
	TagObjWithVlan(ref types.ManagedObjectReference, vlan int) error
	RemoveTagObjVlan(ref types.ManagedObjectReference) error
	RemovePensandoTags(ref types.ManagedObjectReference) []error
}

// NewVCProbe returns a new probe
func NewVCProbe(hOutbox chan<- defs.Probe2StoreMsg, state *defs.State) *VCProbe {
	probe := &VCProbe{
		State:   state,
		Started: false,
		outbox:  hOutbox,
		Session: session.NewSession(state.Ctx, state.VcURL, state.Log),
	}
	probe.newTagsProbe()
	return probe
	// TODO: Check we have correct permissions when we connect.
}

// Start creates a client and view manager
func (v *VCProbe) Start() error {
	v.Log.Info("Starting probe")
	v.vcProbeLock.Lock()
	if v.Started {
		v.vcProbeLock.Unlock()
		return errors.New("Already Started")
	}
	v.Started = true
	v.Wg.Add(1)
	go v.PeriodicSessionCheck(v.Wg)

	v.tp.Start()

	v.Wg.Add(1)
	go v.run()
	v.vcProbeLock.Unlock()

	return nil
}

func (v *VCProbe) run() {
	defer v.Wg.Done()
	v.Log.Infof("Probe Run Started")
	// Listen for session updates
	v.connectionListen()

	// Context must have been cancelled
	v.waitForExit()
}

// WaitForExit stops the sessions
func (v *VCProbe) waitForExit() {
	v.Log.Info("Stopping vcprobe")
	v.vcProbeLock.Lock()
	v.WatcherWg.Wait()
	v.Started = false
	v.vcProbeLock.Unlock()
}

// ClearState tears down Session state
func (v *VCProbe) ClearState() {
	v.vcProbeLock.Lock()
	v.ClearSessionWithLock()
	v.vcProbeLock.Unlock()
}

// Run runs the probe
func (v *VCProbe) connectionListen() {
	v.Log.Infof("Connection listen Started")
	// Listen to the session connection and update orch object
	for {
		select {
		case <-v.Ctx.Done():
			return
		case connected := <-v.ConnUpdate:
			o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
			if err != nil {
				// orchestrator object does not exists anymore, not need to run the probe
				v.Log.Infof("Orchestrator Object %v does not exist, no need to start the probe",
					v.OrchConfig.GetKey())
				return
			}
			if connected {
				v.Log.Infof("Updating orchestrator connection status to %v",
					orchestration.OrchestratorStatus_Success.String())
				o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Success.String()
				o.Write()
			} else {
				v.Log.Infof("Updating orchestrator connection status to %v",
					orchestration.OrchestratorStatus_Failure.String())
				o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Failure.String()
				o.Write()
			}
		}
	}
}

// StartWatchers starts the watches for vCenter objects
func (v *VCProbe) StartWatchers() {
	defer v.Wg.Done()

	for !v.Started {
		if v.Ctx.Err() != nil {
			return
		}
		time.Sleep(1 * time.Second)
	}
	v.Log.Debugf("Start Watchers starting")

	for v.Ctx.Err() == nil {

		tryForever := func(fn func()) {
			for !v.SessionReady {
				select {
				case <-v.Ctx.Done():
					return
				case <-time.After(50 * time.Millisecond):
				}
			}
			v.WatcherWg.Add(1)
			go func() {
				defer v.WatcherWg.Done()
				for {
					if v.ClientCtx.Err() != nil {
						return
					}
					fn()
					time.Sleep(retryDelay)
				}
			}()
		}

		// DC watch
		tryForever(func() {
			v.Log.Debugf("DC watch Started")
			v.startWatch(defs.Datacenter, []string{"name"},
				func(update types.ObjectUpdate, kind defs.VCObject) {
					// Sending dc event
					v.sendVCEvent(update, kind)
					var dcName string
					for _, prop := range update.ChangeSet {
						dcName = prop.Val.(string)
					}
					if v.ForceDCname != "" && dcName != v.ForceDCname {
						v.Log.Infof("Skipping DC event for DC %s", dcName)
						return
					}

					// Starting watches on objects inside the given DC
					tryForever(func() {
						v.Log.Debugf("Host watch Started")
						v.startWatch(defs.HostSystem, []string{"config"}, v.vcEventHandlerForDC(update.Obj.Value, dcName), &update.Obj)
					})

					tryForever(func() {
						v.Log.Debugf("VM watch Started")
						vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
						v.startWatch(defs.VirtualMachine, vmProps, v.vcEventHandlerForDC(update.Obj.Value, dcName), &update.Obj)
					})
					tryForever(func() {
						v.Log.Debugf("Task watch Started")
						v.startTaskWatch(&update.Obj)
					})

					tryForever(func() {
						v.Log.Debugf("PG watch Started")
						v.startWatch(defs.DistributedVirtualPortgroup, []string{"config"}, v.vcEventHandlerForDC(update.Obj.Value, dcName), &update.Obj)
					})

				}, nil)
		})

		tryForever(func() {
			v.tp.StartWatch()
			v.Log.Infof("tag probe finished")
		})
		// tryForever(func() {
		// 	// Should create hosts from this event
		// 	v.startWatch(defs.VmwareDistributedVirtualSwitch, []string{"config"}, v.processDVSEvent)
		// })

		// If watcher context ends, we restart the watches
		select {
		case <-v.ClientCtx.Done():
		}

	}

	return
}

func (v *VCProbe) startTaskWatch(container *types.ManagedObjectReference) {
	_, _, viewMgr, _ := v.GetClientsWithRLock()
	v.ReleaseClientsRLock()
	taskView, err := viewMgr.CreateTaskView(v.ClientCtx, container)
	if err != nil {
		v.Log.Errorf("Cannot create a task view = %s", err)
	}
	for v.ClientCtx.Err() == nil {
		taskView.Collect(v.ClientCtx, func(tasks []types.TaskInfo) {
		})
		// XXX reconnect....
		time.Sleep(1 * time.Second)
	}
}

func (v *VCProbe) startWatch(vcKind defs.VCObject, props []string, updateFn func(objUpdate types.ObjectUpdate, kind defs.VCObject), container *types.ManagedObjectReference) {
	v.Log.Debugf("Start watch called for %s", vcKind)
	kind := string(vcKind)

	var err error
	client, _, viewMgr, _ := v.GetClientsWithRLock()
	v.ReleaseClientsRLock()

	root := client.ServiceContent.RootFolder
	if container == nil {
		container = &root
	}
	kinds := []string{}

	cView, err := viewMgr.CreateContainerView(v.ClientCtx, *container, kinds, true)
	if err != nil {
		v.Log.Errorf("CreateContainerView returned %v", err)
		v.CheckSession = true
		time.Sleep(1 * time.Second)
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
			updateFn(update, vcKind)
		}
		// Must return false, returning true will cause waitForUpdates to exit.
		return false
	}

	for {
		err = property.WaitForUpdates(v.ClientCtx, property.DefaultCollector(client.Client), filter, updFunc)

		if err != nil {
			v.Log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ClientCtx.Err() != nil {
			return
		}
		v.Log.Infof("%s property.WaitForView exited", kind)
		v.CheckSession = true
		time.Sleep(1 * time.Second)
	}
}

func (v *VCProbe) sendVCEvent(update types.ObjectUpdate, kind defs.VCObject) {

	key := update.Obj.Value
	m := defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   kind,
			Key:        key,
			Changes:    update.ChangeSet,
			UpdateType: update.Kind,
			Originator: v.VcID,
		},
	}
	v.Log.Debugf("Sending message to store, key: %s, obj: %s, props: %+v", key, kind, update.ChangeSet)
	v.outbox <- m
}

func (v *VCProbe) vcEventHandlerForDC(dcID string, dcName string) func(update types.ObjectUpdate, kind defs.VCObject) {
	return func(update types.ObjectUpdate, kind defs.VCObject) {
		key := update.Obj.Value
		m := defs.Probe2StoreMsg{
			MsgType: defs.VCEvent,
			Val: defs.VCEventMsg{
				VcObject:   kind,
				DcID:       dcID,
				DcName:     dcName,
				Key:        key,
				Changes:    update.ChangeSet,
				UpdateType: update.Kind,
				Originator: v.VcID,
			},
		}
		v.Log.Debugf("Sending message to store, key: %s, obj: %s", key, kind)
		v.outbox <- m
	}
}

// func (v *VCProbe) processDVSEvent(key string, obj defs.VCObject, pc []types.PropertyChange) {
// for _, prop := range pc {
// 	val := prop.Val.(types.VMwareDVSConfigInfo)
// 	for _, hostConfig := range val.Host {
// 		// TODO: handle host removal
// 		v.Log.Infof("DVS EVENT %s %s %v", key, obj, hostConfig)
// 		ref := hostConfig.Config.Host.Reference()
// 		var hostMO mo.HostSystem
// 		host := object.NewHostSystem(v.client.Client, ref)
// 		err := host.Properties(v.Ctx, ref, []string{"config"}, hostMO)
// 		if err == nil {
// 			v.Log.Errorf("Failed to get host ")
// 		}
// 		hostPc := []types.PropertyChange{
// 			{
// 				Op:  types.PropertyChangeOpAssign,
// 				Val: hostMO.Config,
// 			},
// 		}
// 		v.sendVCEvent(ref.Value, defs.HostSystem, hostPc)
// 	}
// }
// }

// ListObj performs a list operation in vCenter
func (v *VCProbe) ListObj(vcKind defs.VCObject, props []string, dst interface{}, container *types.ManagedObjectReference) error {
	client, _, viewMgr, _ := v.GetClientsWithRLock()
	defer v.ReleaseClientsRLock()

	root := client.ServiceContent.RootFolder

	if container == nil {
		container = &root
	}
	kinds := []string{}
	cView, err := viewMgr.CreateContainerView(v.Ctx, *container, kinds, true)
	if err != nil {
		return err
	}
	defer cView.Destroy(v.Ctx)
	err = cView.Retrieve(v.Ctx, []string{string(vcKind)}, props, dst)
	return err
}

// ListVM returns a list of vms
func (v *VCProbe) ListVM(dcRef *types.ManagedObjectReference) []mo.VirtualMachine {
	var vms []mo.VirtualMachine
	v.ListObj(defs.VirtualMachine, []string{"config", "name", "runtime", "overallStatus", "customValue"}, &vms, dcRef)
	return vms
}

// ListDC returns a list of DCs
func (v *VCProbe) ListDC() []mo.Datacenter {
	var dcs []mo.Datacenter
	v.ListObj(defs.Datacenter, []string{"name"}, &dcs, nil)
	return dcs
}

// ListDVS returns a list of DVS objects
func (v *VCProbe) ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch {
	var dcs []mo.VmwareDistributedVirtualSwitch
	// any properties changed here need to be changed in probe's mock.go
	v.ListObj(defs.VmwareDistributedVirtualSwitch, []string{"name", "config"}, &dcs, dcRef)
	return dcs
}

// ListPG returns a list of PGs
func (v *VCProbe) ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup {
	var dcs []mo.DistributedVirtualPortgroup
	v.ListObj(defs.DistributedVirtualPortgroup, []string{"config", "name", "tag"}, &dcs, dcRef)
	return dcs
}

// ListHosts returns a list of Hosts
func (v *VCProbe) ListHosts(dcRef *types.ManagedObjectReference) []mo.HostSystem {
	var hosts []mo.HostSystem
	v.ListObj(defs.HostSystem, []string{"config", "name"}, &hosts, dcRef)
	return hosts
}

// TagObjAsManaged tags the given ref with a Pensando managed tag
func (v *VCProbe) TagObjAsManaged(ref types.ManagedObjectReference) error {
	return v.tp.TagObjAsManaged(ref)
}

// RemoveTagObjManaged removes the pensando managed tag
func (v *VCProbe) RemoveTagObjManaged(ref types.ManagedObjectReference) error {
	return v.tp.RemoveTagObjManaged(ref)
}

// TagObjWithVlan tags the object with the given vlan value
func (v *VCProbe) TagObjWithVlan(ref types.ManagedObjectReference, vlanValue int) error {
	return v.tp.TagObjWithVlan(ref, vlanValue)
}

// RemoveTagObjVlan removes the vlan tag on the given object
func (v *VCProbe) RemoveTagObjVlan(ref types.ManagedObjectReference) error {
	return v.tp.RemoveTagObjVlan(ref)
}

// RemovePensandoTags removes all pensando tags
func (v *VCProbe) RemovePensandoTags(ref types.ManagedObjectReference) []error {
	return v.tp.RemovePensandoTags(ref)
}

// StartEventReceiver starts receiving events on specified objects
func (v *VCProbe) StartEventReceiver(refs []types.ManagedObjectReference) {
	// This is called from another watcher, so ClientCtx is protected by WatcherWg
	// Increment the WatcherWg counter
	// It is also called from store when newDC is detected. Use Wg for that
	if v.Ctx == nil {
		// This can happen during unit tests where client login may not be done at all
		return
	}
	v.WatcherWg.Add(1)
	go v.runEventReceiver(refs)
}

func (v *VCProbe) runEventReceiver(refs []types.ManagedObjectReference) {
	defer v.WatcherWg.Done()
	v.EventTrackerLock.Lock()
	for _, r := range refs {
		v.Log.Debugf("Start Event Receiver for %s", r.Value)
		v.LastEvent[r.Value] = 0
	}
	v.EventTrackerLock.Unlock()

releaseEventTracker:
	for v.ClientCtx.Err() == nil {
		for !v.SessionReady {
			select {
			case <-v.ClientCtx.Done():
				break releaseEventTracker
			case <-time.After(50 * time.Millisecond):
			}
		}
		eventMgr := v.GetEventManagerWithRLock()
		v.ReleaseClientsRLock()
		if eventMgr == nil {
			break releaseEventTracker
		}
		// Events() return after delivering last N events, restart it
		// TODO there should be a way to request events older than a eventId, not clear if that is
		// supported
		eventMgr.Events(v.ClientCtx, refs, 10, false, false, v.receiveEvents)
		time.Sleep(500 * time.Millisecond)
	}
	v.EventTrackerLock.Lock()
	for _, r := range refs {
		v.Log.Debugf("Stop Event Receiver for %s", r.Value)
		delete(v.LastEvent, r.Value)
	}
	v.EventTrackerLock.Unlock()
	v.Log.Infof("Event Receiver Exited")
}

func (v *VCProbe) receiveEvents(ref types.ManagedObjectReference, events []types.BaseEvent) error {
	// This executes in the context of go routine
	// I think events are ordered as latest event first.. we should process them from back to front
	// and process only new ones
	if len(events) == 0 {
		return nil
	}
	v.EventTrackerLock.Lock()
	defer v.EventTrackerLock.Unlock()
	lastEvent, ok := v.LastEvent[ref.Value]
	if !ok {
		v.Log.Errorf("Incorrect object key for received event")
		return nil
	}
	// sort events based on key - oldest to latest
	sort.Slice(events, func(i, j int) bool {
		return events[i].GetEvent().Key < events[j].GetEvent().Key
	})
	for _, ev := range events {
		// TODO handle roll-over or becoming -ve
		if ev.GetEvent().Key <= lastEvent {
			// old event - already processed
			// v.Log.Infof("Old Event %d, last %d", ev.GetEvent().Key, v.LastEvent)
			continue
		}
		if skipped := lastEvent - ev.GetEvent().Key; skipped > 1 {
			v.Log.Errorf("Skipped %d events", skipped)
		}
		lastEvent = ev.GetEvent().Key

		// Capture Vmotion events -
		// VmBeingMitrages/Relocated are received at the start of vMotion (about 2sec delay obsered
		// 	on lightly loaded VC/Venice).
		// VmMigrated/Relocated event is received on successful vmotion. Also VM watcher is triggered
		// as VM's runtime info now points to new host. So there is no additinal processing to be
		// done on Migrated event.
		// RelocateFailedEvents are received when vmotion fails or cancelled. In this case there is
		// no workload watch event, as there is no change done to workload.

		var msgType defs.VCNotificationType
		vmKey := ""
		hostKey := ""
		reason := ""
		dstHostName := ""
		dstDcName := ""
		hotMigration := false

		// Ignore all events that do not point to Pen-DVS, e.Dvs.Dvs is not populated
		switch ev.(type) {
		case *types.VmEmigratingEvent:
			// TODO: this event does not contain useful information, but it is generated
			// after migration start event.. it may be a good time assume that traffic on
			// host1 has stopped at this time and change the vlan-overrides ??
			e := ev.(*types.VmEmigratingEvent)
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
		case *types.VmBeingHotMigratedEvent:
			e := ev.(*types.VmBeingHotMigratedEvent)
			msgType = defs.VMotionStart
			vmKey = e.Vm.Vm.Value
			hostKey = e.DestHost.Host.Value
			hotMigration = true
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Dest Host %v", e.DestHost.Host.Value)
		case *types.VmBeingMigratedEvent:
			e := ev.(*types.VmBeingMigratedEvent)
			msgType = defs.VMotionStart
			vmKey = e.Vm.Vm.Value
			hostKey = e.DestHost.Host.Value
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Dest Host %v", e.DestHost.Host.Value)
		case *types.VmBeingRelocatedEvent:
			e := ev.(*types.VmBeingRelocatedEvent)
			msgType = defs.VMotionStart
			vmKey = e.Vm.Vm.Value
			hostKey = e.DestHost.Host.Value
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Dest Host %v", e.DestHost.Host.Value)
		case *types.VmMigratedEvent:
			e := ev.(*types.VmMigratedEvent)
			msgType = defs.VMotionDone
			vmKey = e.Vm.Vm.Value
			hostKey = e.SourceHost.Host.Value
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Src Host %v", e.SourceHost.Host.Value)
		case *types.VmRelocatedEvent:
			e := ev.(*types.VmRelocatedEvent)
			msgType = defs.VMotionDone
			vmKey = e.Vm.Vm.Value
			hostKey = e.SourceHost.Host.Value
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Src Host %v", e.SourceHost.Host.Value)
		case *types.VmRelocateFailedEvent:
			e := ev.(*types.VmRelocateFailedEvent)
			msgType = defs.VMotionFailed
			vmKey = e.Vm.Vm.Value
			hostKey = e.DestHost.Host.Value
			reason = e.Reason.LocalizedMessage
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Dest Host %v", e.DestHost.Host.Value)
		case *types.VmFailedMigrateEvent:
			e := ev.(*types.VmFailedMigrateEvent)
			msgType = defs.VMotionFailed
			vmKey = e.Vm.Vm.Value
			hostKey = e.DestHost.Host.Value
			reason = e.Reason.LocalizedMessage
			v.Log.Infof("Event %d - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			v.Log.Debugf("Dest Host %v", e.DestHost.Host.Value)
		case *types.MigrationEvent:
			// TODO: This may be a generic vMotion failure.. but it does not have DestHost Info
			// Do we need to handle it?
			e := ev.(*types.MigrationEvent)
			reason = e.Fault.LocalizedMessage
			v.Log.Infof("Event %d - %T for VM %s reason %s", e.GetEvent().Key, e, e.Vm.Vm.Value, reason)
		case *types.EventEx:
			e := ev.(*types.EventEx)
			s := strings.Split(e.EventTypeId, ".")
			evType := s[len(s)-1]
			if evType == "VmHotMigratingWithEncryptionEvent" {
				v.Log.Debugf("EventEx %d - TypeId %s", e.GetEvent().Key, e.EventTypeId)
				argsMap := map[string]int{}
				for i, kvarg := range e.Arguments {
					argsMap[kvarg.Key] = i
				}
				msgType = defs.VMotionStart
				hotMigration = true
				vmKey = e.Vm.Vm.Value
				if i, ok := argsMap["destHost"]; ok {
					dstHostName, _ = e.Arguments[i].Value.(string)
				}
				if i, ok := argsMap["destDatacenter"]; ok {
					dstDcName, _ = e.Arguments[i].Value.(string)
				}
			} else {
				v.Log.Debugf("Ignored EventEx %d - TypeId %s... (+%v)", e.GetEvent().Key,
					e.EventTypeId, e)
			}
		case *types.ExtendedEvent:
			e := ev.(*types.ExtendedEvent)
			v.Log.Debugf("Ignored ExtendedEvent %d - TypeId %s ... (+%v)", e.GetEvent().Key,
				e.EventTypeId, e)
		default:
			v.Log.Debugf("Ignored Event %d - %T received ... (+%v)", ev.GetEvent().Key, ev, ev)
		}
		switch msgType {
		case defs.VMotionStart:
			// TODO: create a new hight priority channel for events
			v.outbox <- defs.Probe2StoreMsg{
				MsgType: defs.VCNotification,
				Val: defs.VCNotificationMsg{
					Type: msgType,
					Msg: defs.VMotionStartMsg{
						VMKey:        vmKey,
						DstHostKey:   hostKey,
						DstHostName:  dstHostName,
						DcID:         ref.Value,
						DstDcName:    dstDcName,
						HotMigration: hotMigration,
					},
				},
			}
		case defs.VMotionFailed:
			v.outbox <- defs.Probe2StoreMsg{
				MsgType: defs.VCNotification,
				Val: defs.VCNotificationMsg{
					Type: msgType,
					Msg: defs.VMotionFailedMsg{
						VMKey:      vmKey,
						Reason:     reason,
						DstHostKey: hostKey,
						DcID:       ref.Value,
					},
				},
			}
		case defs.VMotionDone:
			// TODO: create a new hight priority channel for events
			v.outbox <- defs.Probe2StoreMsg{
				MsgType: defs.VCNotification,
				Val: defs.VCNotificationMsg{
					Type: msgType,
					Msg: defs.VMotionDoneMsg{
						VMKey:      vmKey,
						SrcHostKey: hostKey,
						DcID:       ref.Value,
					},
				},
			}
		default:
		}
	}
	v.LastEvent[ref.Value] = lastEvent
	return nil
}

// TestReceiveEvents provides entry point for unit testing events
func (v *VCProbe) TestReceiveEvents(ref types.ManagedObjectReference, events []types.BaseEvent) {
	v.Log.Debugf("Test ReceiveEvents")
	v.EventTrackerLock.Lock()
	v.LastEvent[ref.Value] = 0
	v.EventTrackerLock.Unlock()

	v.receiveEvents(ref, events)

	v.EventTrackerLock.Lock()
	delete(v.LastEvent, ref.Value)
	v.EventTrackerLock.Unlock()
}
