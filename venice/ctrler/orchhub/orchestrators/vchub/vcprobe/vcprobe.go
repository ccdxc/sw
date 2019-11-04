package vcprobe

import (
	"context"
	"errors"
	"net/url"
	"sync"
	"time"

	"github.com/davecgh/go-spew/spew"
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

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	vcURL    *url.URL
	VcID     string
	client   *govmomi.Client
	viewMgr  *view.Manager
	cancel   context.CancelFunc
	ctx      context.Context
	outbox   chan<- defs.Probe2StoreMsg
	inbox    <-chan defs.Store2ProbeMsg
	wg       sync.WaitGroup
	tp       *tagsProbe
	Log      log.Logger
	stateMgr *statemgr.Statemgr
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(vcID string, vcURL *url.URL, hOutbox chan<- defs.Probe2StoreMsg, inbox <-chan defs.Store2ProbeMsg, stateMgr *statemgr.Statemgr, l log.Logger) *VCProbe {
	return &VCProbe{
		VcID:     vcID,
		vcURL:    vcURL,
		outbox:   hOutbox,
		inbox:    inbox,
		tp:       newTagsProbe(vcURL, hOutbox),
		Log:      l.WithContext("submodule", "VCProbe"),
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
	// Connect and log in to vCenter
	c, err := govmomi.NewClient(v.ctx, v.vcURL, true)
	if err != nil {
		v.Log.Errorf("Log in failed: %v", err)
		return err
	}

	v.client = c
	v.viewMgr = view.NewManager(v.client.Client)

	err = v.tp.Start(v.ctx)
	return err
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
func (v *VCProbe) Run() {
	tryForever := func(fn func()) {
		for v.ctx.Err() == nil {
			fn()
			time.Sleep(retryDelay)
		}
	}
	go tryForever(v.probeHosts)
	go tryForever(v.probeWorkloads)
	go tryForever(func() {
		v.tp.PollTags(&v.wg)
	})
}

// probeHosts probes the vCenter for hosts and smartnics
func (v *VCProbe) probeHosts() {
	var err error
	v.wg.Add(1)
	defer v.wg.Done()
	root := v.client.ServiceContent.RootFolder
	kinds := []string{"HostSystem"}
	// Which host objects to watch (all)
	hostView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		v.Log.Fatalf("CreateContainerView returned %v", err)
		return
	}

	// Fields to watch for change
	hostProps := []string{"config"}
	hostRef := types.ManagedObjectReference{Type: "HostSystem"}
	filter := new(property.WaitFilter).Add(hostView.Reference(), hostRef.Type, hostProps, hostView.TraversalSpec())

	updFunc := func(updates []types.ObjectUpdate) bool {
		for _, update := range updates {
			if update.Obj.Type != "HostSystem" {
				v.Log.Errorf("Expected HostSystem, got %+v", update.Obj)
				continue
			}
			hostKey := update.Obj.Value
			v.updateHost(hostKey, update.ChangeSet)
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

		v.Log.Infof("probeHosts property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateHost is the callback that sends a host event to the VCStore
func (v *VCProbe) updateHost(hostKey string, pc []types.PropertyChange) {
	v.Log.Infof("<== updateHost vcID: %s hostKey: %s ==>", v.VcID, hostKey)
	if len(pc) != 1 {
		v.Log.Errorf("Only a single property expected at this time.")
		spew.Dump(pc)
		return
	}
	m := defs.Probe2StoreMsg{
		VcObject:   defs.HostSystem,
		Key:        hostKey,
		Changes:    pc,
		Originator: v.VcID,
	}
	v.outbox <- m
}

// probeWorkloads probes the vCenter for VNICs
func (v *VCProbe) probeWorkloads() {
	var err error
	v.wg.Add(1)
	defer v.wg.Done()
	root := v.client.ServiceContent.RootFolder
	kinds := []string{"VirtualMachine"}
	vmView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		v.Log.Fatalf("CreateContainerView returned %v", err)
		return
	}

	vmRef := types.ManagedObjectReference{Type: "VirtualMachine"}
	// TODO: See which props we can get from summary alone
	// From vCenter docs, they optimize for watchers on the summary property.
	vmProps := []string{"config", "name", "runtime", "overallStatus", "customValue"}
	filter := new(property.WaitFilter).Add(vmView.Reference(), vmRef.Type, vmProps, vmView.TraversalSpec())

	updFunc := func(updates []types.ObjectUpdate) bool {
		for _, update := range updates {
			if update.Obj.Type != "VirtualMachine" {
				v.Log.Errorf("Expected VirtualMachine, got %+v", update.Obj)
				continue
			}
			vmKey := update.Obj.Value
			v.updateWorkload(vmKey, update.ChangeSet)
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

		v.Log.Infof("probeWorkloads property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateWorkload is the callback that injects a message to the store
func (v *VCProbe) updateWorkload(vmKey string, pc []types.PropertyChange) {
	var m defs.Probe2StoreMsg
	m = defs.Probe2StoreMsg{
		VcObject: defs.VirtualMachine,
		// Op:         getStoreOp(prop.Op),
		// Property:   defs.VMPropRT,
		Key:        vmKey,
		Changes:    pc,
		Originator: v.VcID,
	}
	v.Log.Infof("<== updateWorkload vcID: %s vmKey: %s ==>", v.VcID, vmKey)
	v.outbox <- m
}
