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

	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	retryDelay = time.Second
)

// VCProbe represents an instance of a vCenter Interface
// This is comprised of a SOAP interface and a REST interface
type VCProbe struct {
	vcURL   *url.URL
	vcID    string
	client  *govmomi.Client
	viewMgr *view.Manager
	cancel  context.CancelFunc
	ctx     context.Context
	outBox  chan<- defs.StoreMsg
	wg      sync.WaitGroup
	tp      *tagsProbe
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(vcID string, vcURL *url.URL, hOutBox chan<- defs.StoreMsg) *VCProbe {
	return &VCProbe{
		vcID:   vcID,
		vcURL:  vcURL,
		outBox: hOutBox,
		tp:     newTagsProbe(vcURL, hOutBox),
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
		log.Errorf("Log in failed: %v", err)
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
		log.Fatalf("CreateContainerView returned %v", err)
		return
	}

	// Fields to watch for change
	hostProps := []string{"config"}
	hostRef := types.ManagedObjectReference{Type: "HostSystem"}
	filter := new(property.WaitFilter).Add(hostView.Reference(), hostRef.Type, hostProps, hostView.TraversalSpec())

	updFunc := func(updates []types.ObjectUpdate) bool {
		for _, update := range updates {
			if update.Obj.Type != "HostSystem" {
				log.Errorf("Expected HostSystem, got %+v", update.Obj)
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
			log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ctx.Err() != nil {
			return
		}

		log.Infof("probeHosts property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateHost is the callback that injects a message to the smartnic store
func (v *VCProbe) updateHost(hostKey string, pc []types.PropertyChange) {
	log.Infof("<== updateHost vcID: %s hostKey: %s ==>", v.vcID, hostKey)
	if len(pc) != 1 {
		log.Errorf("Only a single property expected at this time.")
		spew.Dump(pc)
		return
	}
	m := defs.StoreMsg{
		VcObject:   defs.HostSystem,
		Key:        hostKey,
		Changes:    pc,
		Originator: v.vcID,
	}
	v.outBox <- m

	// build a vSphereHost based on the update
	// vhNew := &defs.ESXHost{DvsMap: make(map[string]*defs.DvsInstance), PenNICs: make(map[string]*defs.NICInfo)}

	// globalHostKey := v.vcID + ":" + hostKey
	// msg := defs.StoreMsg{
	// 	Op:         defs.VCOpSet,
	// 	Property:   defs.HostPropConfig,
	// 	Key:        globalHostKey,
	// 	Value:      vhNew,
	// 	Originator: v.vcID,
	// }

	// for _, prop := range pc {
	// 	if prop.Op == types.PropertyChangeOpRemove || prop.Op == types.PropertyChangeOpIndirectRemove {
	// 		msg.Op = defs.VCOpDelete
	// 		break
	// 	}
	// 	hConfig, ok := prop.Val.(types.HostConfigInfo)
	// 	if !ok {
	// 		log.Errorf(">>>Bad prop<<<")
	// 		spew.Dump(prop)
	// 		return
	// 	}

	// 	nwInfo := hConfig.Network
	// 	if nwInfo == nil {
	// 		log.Errorf("Missing hConfig.Network")
	// 		return
	// 	}

	// 	for _, pnic := range nwInfo.Pnic {
	// 		// TODO check for vendor field to identify Pensando NICs
	// 		vhNew.PenNICs[pnic.Device] = &defs.NICInfo{Mac: pnic.Mac, Name: pnic.Device}
	// 	}

	// 	for _, sx := range nwInfo.ProxySwitch {
	// 		// TODO ignore if it's not our dvs
	// 		if sx.Spec.Backing == nil {
	// 			log.Errorf("DVS backing is nil")
	// 			continue
	// 		}

	// 		backing, ok := sx.Spec.Backing.(*types.DistributedVirtualSwitchHostMemberPnicBacking)
	// 		if !ok {
	// 			log.Errorf("Expected DistributedVirtualSwitchHostMemberPnicBacking")
	// 			continue
	// 		}

	// 		dvs := new(defs.DvsInstance)
	// 		dvs.Name = sx.DvsName
	// 		for _, ps := range backing.PnicSpec {
	// 			if i, found := vhNew.PenNICs[ps.PnicDevice]; found {
	// 				dvs.Uplinks = append(dvs.Uplinks, ps.PnicDevice)
	// 				i.DvsUUID = sx.DvsUuid // updates map

	// 			} else {
	// 				// TODO: this should be an alarm
	// 				log.Errorf("%s not recognized", ps.PnicDevice)
	// 			}
	// 		}
	// 		vhNew.DvsMap[sx.DvsUuid] = dvs
	// 	}
	// }

	// v.outBox <- msg
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
		log.Fatalf("CreateContainerView returned %v", err)
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
				log.Errorf("Expected VirtualMachine, got %+v", update.Obj)
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
			log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ctx.Err() != nil {
			return
		}

		log.Infof("probeWorkloads property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateWorkload is the callback that injects a message to the nwif store
func (v *VCProbe) updateWorkload(vmKey string, pc []types.PropertyChange) {
	var m defs.StoreMsg
	m = defs.StoreMsg{
		VcObject: defs.VirtualMachine,
		// Op:         getStoreOp(prop.Op),
		// Property:   defs.VMPropRT,
		Key:        vmKey,
		Changes:    pc,
		Originator: v.vcID,
	}
	log.Infof("<== updateWorkload vcID: %s vmKey: %s ==>", v.vcID, vmKey)
	v.outBox <- m

	//spew.Dump(pc)
	// for _, prop := range pc {
	// 	switch defs.VCProp(prop.Name) {
	// 	case defs.VMPropConfig:
	// 		op := getStoreOp(prop.Op)
	// 		value := &defs.VMConfig{}
	// 		if op != defs.VCOpDelete {
	// 			vnics, err := getWorkloadInterfaces(&prop)
	// 			if err != nil {
	// 				continue
	// 			}
	// 			value = &defs.VMConfig{Vnics: vnics}
	// 		}
	// 		m = defs.StoreMsg{
	// 			Op:         getStoreOp(prop.Op),
	// 			Property:   defs.VMPropConfig,
	// 			Key:        globalVMKey,
	// 			Value:      value,
	// 			Originator: v.vcID,
	// 		}
	// 	case defs.VMPropName:
	// 		m = defs.StoreMsg{
	// 			Op:         getStoreOp(prop.Op),
	// 			Property:   defs.VMPropName,
	// 			Key:        globalVMKey,
	// 			Value:      prop.Val.(string),
	// 			Originator: v.vcID,
	// 		}
	// 	case defs.VMPropRT:
	// 		rt := prop.Val.(types.VirtualMachineRuntimeInfo)
	// 		m = defs.StoreMsg{
	// 			Op:         getStoreOp(prop.Op),
	// 			Property:   defs.VMPropRT,
	// 			Key:        globalVMKey,
	// 			Value:      &defs.VMRuntime{HostKey: v.vcID + ":" + rt.Host.Value, PowerState: string(rt.PowerState)},
	// 			Originator: v.vcID,
	// 		}
	// 	//case defs.VMPropTag:
	// 	//case defs.VMPropCustom:
	// 	default:
	// 		log.Errorf("Unknown property %s", prop.Name)
	// 		continue
	// 	}

	// v.outBox <- m
	// }
}

// getWorkloadInterfaces extracts vnic info from the property
// func getWorkloadInterfaces(p *types.PropertyChange) (map[string]*defs.VirtualNIC, error) {
// 	var name string
// 	res := make(map[string]*workload.WorkloadIntfSpec)

// 	vmConfig, ok := p.Val.(types.VirtualMachineConfigInfo)
// 	if !ok {
// 		log.Errorf("Expected VirtualMachineConfigInfo, got %s", reflect.TypeOf(p.Val).Name())
// 		return nil, errors.New("reflect error")
// 	}

// 	for _, d := range vmConfig.Hardware.Device {
// 		vec := GetVeth(d)
// 		if vec != nil {
// 			back, ok := vec.Backing.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo)
// 			if !ok {
// 				log.Errorf("Expected types.VirtualEthernetCardDistributedVirtualPortBackingInfo, got %s", reflect.TypeOf(vec.Backing).Name())
// 				continue
// 			}
// 			desc, ok := vec.VirtualDevice.DeviceInfo.(*types.Description)
// 			if ok {
// 				name = desc.Label
// 			} else {
// 				name = ""
// 			}
// 			vnic := &workload.WorkloadIntfSpec{
// 				Name:         name,
// 				MacAddress:   vec.MacAddress,
// 				PortgroupKey: back.Port.PortgroupKey,
// 				PortKey:      back.Port.PortKey,
// 				SwitchUUID:   back.Port.SwitchUuid,
// 			}

// 			res[vec.MacAddress] = vnic
// 		}
// 	}

// 	return res, nil
// }
