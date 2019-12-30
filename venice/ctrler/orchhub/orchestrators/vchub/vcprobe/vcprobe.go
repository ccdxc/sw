package vcprobe

import (
	"errors"
	"sync"
	"time"

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

					// Starting watches on objects inside the given DC
					go tryForever(func() {
						v.Log.Debugf("Host watch Started")
						v.startWatch(defs.HostSystem, []string{"config"}, v.vcEventHandlerForDC(update.Obj.Value), &update.Obj)
					})

					go tryForever(func() {
						v.Log.Debugf("VM watch Started")
						vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
						v.startWatch(defs.VirtualMachine, vmProps, v.vcEventHandlerForDC(update.Obj.Value), &update.Obj)
					})
				}, nil)
		})

		tryForever(func() {
			v.tp.Start()
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
	client, _, viewMgr := v.GetClientWithRLock()
	v.ReleaseClientRLock()

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
			Originator: v.VcID,
		},
	}
	v.Log.Debugf("Sending message to store, key: %s, obj: %s, props: ", key, kind)
	v.outbox <- m
}

func (v *VCProbe) vcEventHandlerForDC(dc string) func(update types.ObjectUpdate, kind defs.VCObject) {
	return func(update types.ObjectUpdate, kind defs.VCObject) {
		key := update.Obj.Value
		m := defs.Probe2StoreMsg{
			MsgType: defs.VCEvent,
			Val: defs.VCEventMsg{
				VcObject:   kind,
				DC:         dc,
				Key:        key,
				Changes:    update.ChangeSet,
				Originator: v.VcID,
			},
		}
		v.Log.Debugf("Sending message to store, key: %s, obj: %s, props: ", key, kind)
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

func (v *VCProbe) listObj(vcKind defs.VCObject, props []string, dst interface{}, container *types.ManagedObjectReference) error {
	client, _, viewMgr := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

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
	v.listObj(defs.VirtualMachine, []string{"config", "name", "runtime", "overallStatus", "customValue"}, &vms, dcRef)
	return vms
}

// ListDC returns a list of DCs
func (v *VCProbe) ListDC() []mo.Datacenter {
	var dcs []mo.Datacenter
	v.listObj(defs.Datacenter, []string{"name"}, &dcs, nil)
	return dcs
}

// ListDVS returns a list of DVS objects
func (v *VCProbe) ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch {
	var dcs []mo.VmwareDistributedVirtualSwitch
	v.listObj(defs.VmwareDistributedVirtualSwitch, []string{"name"}, &dcs, dcRef)
	return dcs
}

// ListPG returns a list of PGs
func (v *VCProbe) ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup {
	var dcs []mo.DistributedVirtualPortgroup
	v.listObj(defs.DistributedVirtualPortgroup, []string{"config", "name"}, &dcs, dcRef)
	return dcs
}
