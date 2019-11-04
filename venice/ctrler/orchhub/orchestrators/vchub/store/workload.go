package store

import (
	"reflect"
	"strconv"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ref"
)

func (v *VCHStore) handleWorkload(m defs.Probe2StoreMsg) {
	v.Log.Infof("Got handle workload event")
	meta := &api.ObjectMeta{
		Name: createGlobalKey(m.Originator, m.Key),
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	var workloadObj, existingWorkload *workload.Workload
	ctkitWorkload, err := v.stateMgr.Controller().Workload().Find(meta)
	if err != nil {
		existingWorkload = nil
	} else {
		existingWorkload = &ctkitWorkload.Workload
	}

	if existingWorkload == nil {
		v.Log.Infof("Existing workload is nil")
		workloadObj = &workload.Workload{
			TypeMeta: api.TypeMeta{
				Kind:       "Workload",
				APIVersion: "v1",
			},
			ObjectMeta: *meta,
		}
	} else {
		temp := ref.DeepCopy(*existingWorkload).(workload.Workload)
		workloadObj = &temp
	}

	toDelete := false
	for _, prop := range m.Changes {
		switch getStoreOp(prop.Op) {
		case defs.VCOpDelete:
			toDelete = true
			break
		case defs.VCOpSet:
			switch defs.VCProp(prop.Name) {
			case defs.VMPropConfig:
				v.processVMConfig(workloadObj, prop)
			case defs.VMPropName:
				v.processVMName(workloadObj, prop)
			case defs.VMPropRT:
				v.processVMRuntime(workloadObj, prop, m.Originator)
			case defs.VMPropOverallStatus:
			case defs.VMPropCustom:
			default:
				v.Log.Errorf("Unknown property %s", prop.Name)
				continue
			}
		}
	}
	if toDelete {
		if existingWorkload == nil {
			return
		}
		// Delete from apiserver
		v.stateMgr.Controller().Workload().Delete(workloadObj)
		return
	}
	// If different, write to apiserver
	if reflect.DeepEqual(workloadObj, existingWorkload) {
		// Nothing to do
		return
	}
	// TODO: Add retry to apiserver write
	// It's possible we received the VM event before
	// the event for creating the host in APIserver,
	// or apiserver is temporarily down
	if existingWorkload == nil {
		v.Log.Infof("existing workload is nil")
		v.stateMgr.Controller().Workload().Create(workloadObj)
	} else {
		v.Log.Infof("existing workload is not nil. calling update")
		v.stateMgr.Controller().Workload().Update(workloadObj)
	}
}

func (v *VCHStore) processVMRuntime(workload *workload.Workload, prop types.PropertyChange, VCId string) {
	if prop.Val == nil {
		return
	}
	rt := prop.Val.(types.VirtualMachineRuntimeInfo)
	// TODO: add naming convention
	host := rt.Host.Value
	workload.Spec.HostName = host
}

func (v *VCHStore) processVMName(workload *workload.Workload, prop types.PropertyChange) {
	if prop.Val == nil {
		return
	}
	name := prop.Val.(string)

	if len(name) == 0 {
		return
	}
	if workload.ObjectMeta.Labels == nil {
		workload.ObjectMeta.Labels = make(map[string]string)
	}
	workload.ObjectMeta.Labels["vm-name"] = name
}

func (v *VCHStore) processVMConfig(workload *workload.Workload, prop types.PropertyChange) {
	if prop.Val == nil {
		return
	}
	vmConfig, ok := prop.Val.(types.VirtualMachineConfigInfo)
	if !ok {
		v.Log.Errorf("Expected VirtualMachineConfigInfo, got %s", reflect.TypeOf(prop.Val).Name())
		return
	}
	interfaces := v.extractInterfaces(vmConfig)
	workload.Spec.Interfaces = interfaces
	// TODO: Determine label naming scheme
	if len(vmConfig.Name) == 0 {
		return
	}
	if workload.ObjectMeta.Labels == nil {
		workload.ObjectMeta.Labels = make(map[string]string)
	}
	workload.ObjectMeta.Labels["vm-name"] = vmConfig.Name
}

func (v *VCHStore) extractInterfaces(vmConfig types.VirtualMachineConfigInfo) []workload.WorkloadIntfSpec {
	var res []workload.WorkloadIntfSpec
	for _, d := range vmConfig.Hardware.Device {
		vec := v.GetVeth(d)
		if vec != nil {
			back, ok := vec.Backing.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo)
			if !ok {
				v.Log.Errorf("Expected types.VirtualEthernetCardDistributedVirtualPortBackingInfo, got %s", reflect.TypeOf(vec.Backing).Name())
				continue
			}
			vnic := workload.WorkloadIntfSpec{
				MACAddress:   vec.MacAddress,
				MicroSegVlan: portKeyToVLAN(back.Port.PortKey),
			}
			res = append(res, vnic)
		}
	}
	return res
}

// GetVeth checks if the base virtual device is a vnic and returns a pointer to
// VirtualEthernetCard
func (v *VCHStore) GetVeth(d types.BaseVirtualDevice) *types.VirtualEthernetCard {
	dKind := reflect.TypeOf(d).Elem()

	switch dKind {
	case reflect.TypeOf((*types.VirtualVmxnet3)(nil)).Elem():
		v3 := d.(*types.VirtualVmxnet3)
		return &v3.VirtualVmxnet.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualVmxnet2)(nil)).Elem():
		v2 := d.(*types.VirtualVmxnet2)
		return &v2.VirtualVmxnet.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualVmxnet)(nil)).Elem():
		v1 := d.(*types.VirtualVmxnet)
		return &v1.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualE1000)(nil)).Elem():
		e1 := d.(*types.VirtualE1000)
		return &e1.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualE1000e)(nil)).Elem():
		e1e := d.(*types.VirtualE1000e)
		return &e1e.VirtualEthernetCard
	default:
		v.Log.Infof("Ignoring virtual device %s", dKind.Name())
		return nil
	}
}

func getStoreOp(op types.PropertyChangeOp) defs.VCOp {
	switch op {
	case types.PropertyChangeOpRemove:
		return defs.VCOpDelete
	case types.PropertyChangeOpIndirectRemove:
		return defs.VCOpDelete
	default:
		return defs.VCOpSet
	}
}

func portKeyToVLAN(portKey string) uint32 {
	// TODO: the port might conflict with PVLAN settings or given port might
	// be outside the range of valid vlan numbers. Can't always assign port number.
	vlan, err := strconv.Atoi(portKey)
	if err != nil || vlan < 0 || vlan > 4095 {
		vlan = 0
	}

	return uint32(vlan)
}
