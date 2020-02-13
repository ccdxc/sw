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

					_, ok := v.ForceDCNames[dcName]
					if len(v.ForceDCNames) > 0 && !ok {
						v.Log.Infof("Skipping DC event for DC %s", dcName)
						return
					}

					// Starting watches on objects inside the given DC
					tryForever(func() {
						v.Log.Debugf("Host watch Started on DC %s", dcName)
						v.startWatch(defs.HostSystem, []string{"config", "name"}, v.vcEventHandlerForDC(update.Obj.Value, dcName), &update.Obj)
					})

					tryForever(func() {
						v.Log.Debugf("VM watch Started on DC %s", dcName)
						vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
						v.startWatch(defs.VirtualMachine, vmProps, v.vcEventHandlerForDC(update.Obj.Value, dcName), &update.Obj)
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

func (v *VCProbe) initEventTracker(refs []types.ManagedObjectReference) {
	v.EventTrackerLock.Lock()
	defer v.EventTrackerLock.Unlock()
	for _, r := range refs {
		v.Log.Debugf("Start Event Receiver for %s", r.Value)
		v.LastEvent[r.Value] = 0
	}
}

func (v *VCProbe) deleteEventTracker(refs []types.ManagedObjectReference) {
	v.EventTrackerLock.Lock()
	defer v.EventTrackerLock.Unlock()
	for _, ref := range refs {
		if _, ok := v.LastEvent[ref.Value]; ok {
			v.Log.Infof("Stop event receiver for %s", ref.Value)
			delete(v.LastEvent, ref.Value)
		}
	}
}

func (v *VCProbe) runEventReceiver(refs []types.ManagedObjectReference) {
	defer v.WatcherWg.Done()
	v.initEventTracker(refs)
	defer v.deleteEventTracker(refs)

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
		// TODO there should be a way to request events newer than a eventId, not clear if that is
		// supported
		eventMgr.Events(v.ClientCtx, refs, 10, false, false, v.receiveEvents)
		time.Sleep(500 * time.Millisecond)
	}
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
		if skipped := ev.GetEvent().Key - lastEvent; skipped > 1 {
			v.Log.Errorf("Skipped %d events on %s", skipped, ref.Value)
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

		// Ignore all events that do not point to Pen-DVS, e.Dvs.Dvs is not populated
		switch e := ev.(type) {
		case *types.VmEmigratingEvent:
			// TODO: this event does not contain useful information, but it is generated
			// after migration start event.. it may be a good time assume that traffic on
			// host1 has stopped at this time and change the vlan-overrides ??
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
		case *types.VmBeingHotMigratedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, e, e.Vm.Vm.Value)
			msg := defs.VMotionStartMsg{
				VMKey:        e.Vm.Vm.Value,
				DstHostKey:   e.DestHost.Host.Value,
				DcID:         ref.Value,
				HotMigration: true,
			}
			v.generateMigrationEvent(defs.VMotionStart, msg)
		case *types.VmBeingMigratedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionStartMsg{
				VMKey:      e.Vm.Vm.Value,
				DstHostKey: e.DestHost.Host.Value,
				DcID:       ref.Value,
			}
			v.generateMigrationEvent(defs.VMotionStart, msg)
		case *types.VmBeingRelocatedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionStartMsg{
				VMKey:      e.Vm.Vm.Value,
				DstHostKey: e.DestHost.Host.Value,
				DcID:       ref.Value,
			}
			v.generateMigrationEvent(defs.VMotionStart, msg)
		case *types.VmMigratedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionDoneMsg{
				VMKey:      e.Vm.Vm.Value,
				SrcHostKey: e.SourceHost.Host.Value,
				DcID:       ref.Value,
			}
			v.generateMigrationEvent(defs.VMotionDone, msg)
		case *types.VmRelocatedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionDoneMsg{
				VMKey:      e.Vm.Vm.Value,
				SrcHostKey: e.SourceHost.Host.Value,
				DcID:       ref.Value,
			}
			v.generateMigrationEvent(defs.VMotionDone, msg)
		case *types.VmRelocateFailedEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionFailedMsg{
				VMKey:      e.Vm.Vm.Value,
				DstHostKey: e.DestHost.Host.Value,
				DcID:       ref.Value,
				Reason:     e.Reason.LocalizedMessage,
			}
			v.generateMigrationEvent(defs.VMotionFailed, msg)
		case *types.VmFailedMigrateEvent:
			v.Log.Infof("Event %d - %s - %T for VM %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value)
			msg := defs.VMotionFailedMsg{
				VMKey:      e.Vm.Vm.Value,
				DstHostKey: e.DestHost.Host.Value,
				DcID:       ref.Value,
				Reason:     e.Reason.LocalizedMessage,
			}
			v.generateMigrationEvent(defs.VMotionFailed, msg)
		case *types.MigrationEvent:
			// TODO: This may be a generic vMotion failure.. but it does not have DestHost Info
			// Do we need to handle it?
			v.Log.Infof("Event %d - %s - %T for VM %s reason %s", e.GetEvent().Key, ref.Value, e, e.Vm.Vm.Value, e.Fault.LocalizedMessage)
			msg := defs.VMotionFailedMsg{
				VMKey:  e.Vm.Vm.Value,
				DcID:   ref.Value,
				Reason: e.Fault.LocalizedMessage,
			}
			v.generateMigrationEvent(defs.VMotionFailed, msg)
		case *types.EventEx:
			s := strings.Split(e.EventTypeId, ".")
			evType := s[len(s)-1]
			if evType == "VmHotMigratingWithEncryptionEvent" {
				v.Log.Debugf("EventEx %d - %s - TypeId %s", e.GetEvent().Key, ref.Value, e.EventTypeId)
				msg := defs.VMotionStartMsg{
					VMKey:        e.Vm.Vm.Value,
					DcID:         ref.Value,
					HotMigration: true,
				}
				argsMap := map[string]int{}
				for i, kvarg := range e.Arguments {
					argsMap[kvarg.Key] = i
				}
				if i, ok := argsMap["destHost"]; ok {
					msg.DstHostName, _ = e.Arguments[i].Value.(string)
				}
				if i, ok := argsMap["destDatacenter"]; ok {
					msg.DstDcName, _ = e.Arguments[i].Value.(string)
				}
				v.generateMigrationEvent(defs.VMotionStart, msg)
			} else {
				// v.Log.Debugf("Ignored EventEx %d - %s - TypeId %s...", e.GetEvent().Key, ref.Value, e.EventTypeId)
			}
		case *types.ExtendedEvent:
			// v.Log.Debugf("Ignored ExtendedEvent %d - TypeId %s ...", e.GetEvent().Key, ref.Value, e.EventTypeId)
		default:
			// v.Log.Debugf("Ignored Event %d - %s - %T received ... (+%v)", ev.GetEvent().Key, ref.Value, ev, ev)
			// v.Log.Debugf("Ignored Event %d - %s - %T received ...", ev.GetEvent().Key, ref.Value, ev)
		}
	}
	v.LastEvent[ref.Value] = lastEvent
	return nil
}

func (v *VCProbe) generateMigrationEvent(msgType defs.VCNotificationType, msg interface{}) {
	// TODO: create a new high priority channel for events
	switch msgType {
	case defs.VMotionStart:
		v.outbox <- defs.Probe2StoreMsg{
			MsgType: defs.VCNotification,
			Val: defs.VCNotificationMsg{
				Type: msgType,
				Msg:  msg.(defs.VMotionStartMsg),
			},
		}
	case defs.VMotionFailed:
		v.outbox <- defs.Probe2StoreMsg{
			MsgType: defs.VCNotification,
			Val: defs.VCNotificationMsg{
				Type: msgType,
				Msg:  msg.(defs.VMotionFailedMsg),
			},
		}
	case defs.VMotionDone:
		v.outbox <- defs.Probe2StoreMsg{
			MsgType: defs.VCNotification,
			Val: defs.VCNotificationMsg{
				Type: msgType,
				Msg:  msg.(defs.VMotionDoneMsg),
			},
		}
	default:
	}
}
