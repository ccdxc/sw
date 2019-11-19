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
	conv "github.com/pensando/sw/venice/utils/strconv"
)

const workloadKind = "Workload"

func (v *VCHStore) validateWorkload(in interface{}) bool {
	obj, ok := in.(*workload.Workload)
	if !ok {
		return false
	}
	if len(obj.Spec.HostName) == 0 {
		return false
	}
	hostMeta := &api.ObjectMeta{
		Name: obj.Spec.HostName,
	}
	// check that host has been created already
	if _, err := v.stateMgr.Controller().Host().Find(hostMeta); err != nil {
		v.Log.Errorf("Couldn't find host %s for workload %s", hostMeta.Name, obj.GetObjectMeta().Name)
		return false
	}
	// TODO: Add check that workload is no longer in pvlan mode
	return true
}

func (v *VCHStore) handleWorkload(m defs.Probe2StoreMsg) {
	v.Log.Infof("Got handle workload event")
	meta := &api.ObjectMeta{
		Name: createGlobalKey(m.Originator, m.Key),
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	var workloadObj *workload.Workload
	existingWorkload := v.pCache.GetWorkload(meta)

	if existingWorkload == nil {
		v.Log.Infof("Existing workload is nil")
		workloadObj = &workload.Workload{
			TypeMeta: api.TypeMeta{
				Kind:       workloadKind,
				APIVersion: "v1",
			},
			ObjectMeta: *meta,
		}
	} else {
		temp := ref.DeepCopy(*existingWorkload).(workload.Workload)
		workloadObj = &temp
	}

	if workloadObj.Labels == nil {
		workloadObj.Labels = map[string]string{}
		addOrchNameLabel(workloadObj.Labels, m.Originator)
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
			case defs.VMPropTag:
				v.processTags(workloadObj, prop, m.Originator)
			case defs.VMPropOverallStatus:
			case defs.VMPropCustom:
			default:
				v.Log.Errorf("Unknown property %s", prop.Name)
				continue
			}
		}
	}
	if toDelete {
		v.pCache.Delete(workloadKind, workloadObj)
		return
	}

	v.pCache.Set(workloadKind, workloadObj)
}

func (v *VCHStore) processTags(workload *workload.Workload, prop types.PropertyChange, vcID string) {
	// Ovewrite old labels with new tags. API hook will ensure we don't overwrite user added tags.
	tagMsg := prop.Val.(defs.TagMsg)
	workload.Labels = generateLabelsFromTags(workload.Labels, tagMsg)
}

func (v *VCHStore) processVMRuntime(workload *workload.Workload, prop types.PropertyChange, vcID string) {
	if prop.Val == nil {
		return
	}
	rt := prop.Val.(types.VirtualMachineRuntimeInfo)
	host := createGlobalKey(vcID, rt.Host.Value)
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
	addVMNameLabel(workload.Labels, name)
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
	if len(vmConfig.Name) == 0 {
		return
	}
	addVMNameLabel(workload.Labels, vmConfig.Name)
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
			macStr, err := conv.ParseMacAddr(vec.MacAddress)
			if err != nil {
				v.Log.Errorf("Failed to parse mac addres. Err : %v", err)
				continue
			}
			vnic := workload.WorkloadIntfSpec{
				MACAddress:   macStr,
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
