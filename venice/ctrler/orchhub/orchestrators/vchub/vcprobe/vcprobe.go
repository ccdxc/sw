package vcprobe

import (
	"errors"
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
	ListVM(dcRef *types.ManagedObjectReference) []mo.VirtualMachine
	ListDC() []mo.Datacenter
	ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch
	ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup
	ListHosts(dcRef *types.ManagedObjectReference) []mo.HostSystem

	// port_group.go functions
	AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec) error
	GetPenPG(dcName string, pgName string) (*object.DistributedVirtualPortgroup, error)
	RemovePenPG(dcName, pgName string) error

	// distributed_vswitch.go functions
	AddPenDVS(dcName string, dvsCreateSpec *types.DVSCreateSpec) error
	GetPenDVS(dcName, dvsName string) (*object.DistributedVirtualSwitch, error)
	UpdateDVSPortsVlan(dcName, dvsName string, portsSetting PenDVSPortSettings) error
	GetPenDVSPorts(dcName, dvsName string, criteria *types.DistributedVirtualSwitchPortCriteria) ([]types.DistributedVirtualPort, error)

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
	v.ClearClientCtx()
	v.ClearSession()
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
		v.Log.Debugf("Sending message to store, key: %s, obj: %s, update: %+v", key, kind, update)
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
