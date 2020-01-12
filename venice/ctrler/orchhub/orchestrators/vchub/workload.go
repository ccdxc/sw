package vchub

import (
	"reflect"
	"sort"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ref"
	conv "github.com/pensando/sw/venice/utils/strconv"
)

const workloadKind = "Workload"

func (v *VCHub) validateWorkload(in interface{}) (bool, bool) {
	obj, ok := in.(*workload.Workload)
	if !ok {
		return false, false
	}
	if len(obj.Spec.HostName) == 0 {
		return false, false
	}
	hostMeta := &api.ObjectMeta{
		Name: obj.Spec.HostName,
	}
	// check that host has been created already
	if _, err := v.StateMgr.Controller().Host().Find(hostMeta); err != nil {
		v.Log.Errorf("Couldn't find host %s for workload %s", hostMeta.Name, obj.GetObjectMeta().Name)
		return false, false
	}
	// check that workload is no longer in pvlan mode
	for _, inf := range obj.Spec.Interfaces {
		if inf.MicroSegVlan == 0 {
			v.Log.Errorf("inf %s has no useg for workload %s", inf.MACAddress, obj.GetObjectMeta().Name)
			return false, false
		}
	}
	return true, true
}

func (v *VCHub) handleWorkload(m defs.VCEventMsg) {
	v.Log.Debugf("Got handle workload event")
	meta := &api.ObjectMeta{
		Name: utils.CreateGlobalKey(m.Originator, m.DcID, m.Key),
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
			Spec:       workload.WorkloadSpec{},
		}
	} else {
		// Copying to prevent modifying of ctkit state
		temp := ref.DeepCopy(*existingWorkload).(workload.Workload)
		workloadObj = &temp
	}

	if m.UpdateType == types.ObjectUpdateKindLeave {
		// Object was deleted
		if existingWorkload == nil {
			// Don't have object we received delete for
			return
		}
		v.deleteWorkload(workloadObj, m.DcID)
		return
	}

	if workloadObj.Labels == nil {
		workloadObj.Labels = map[string]string{}
	}

	if v.OrchConfig != nil {
		utils.AddOrchNameLabel(workloadObj.Labels, v.OrchConfig.GetName())
	}

	for _, prop := range m.Changes {
		switch defs.VCProp(prop.Name) {
		case defs.VMPropConfig:
			v.processVMConfig(prop, m.Originator, m.DcID, m.DcName, workloadObj)
		case defs.VMPropName:
			v.processVMName(prop, m.Originator, m.DcID, m.DcName, workloadObj)
		case defs.VMPropRT:
			v.processVMRuntime(prop, m.Originator, m.DcID, m.DcName, workloadObj)
		case defs.VMPropTag:
			v.processTags(prop, m.Originator, m.DcID, m.DcName, workloadObj)
		case defs.VMPropOverallStatus:
		case defs.VMPropCustom:
		default:
			v.Log.Errorf("Unknown property %s", prop.Name)
			continue
		}
	}

	v.assignUsegs(m.DcID, m.DcName, m.Key, workloadObj)

	v.pCache.Set(workloadKind, workloadObj)
}

func (v *VCHub) deleteWorkload(workloadObj *workload.Workload, dcName string) {
	v.Log.Debugf("Deleting workload %s", workloadObj.Name)
	// free vlans
	for _, inf := range workloadObj.Spec.Interfaces {
		// Check if workload has assignment
		if inf.MicroSegVlan != 0 {
			// TODO: send notification to probe to reset override
			// dc := v.DcMap[m.DcID]
			dc := v.GetDC(dcName)
			if dc == nil {
				v.Log.Errorf("Got Workload delete %s for a DC we don't have state for %s", workloadObj.Name, dcName)
				continue
			}

			// TODO: Remove hardcoded dvs name
			dvs := dc.GetPenDVS(createDVSName(dcName))
			err := dvs.UsegMgr.ReleaseVlanForVnic(inf.MACAddress, workloadObj.Spec.HostName)
			if err != nil {
				v.Log.Errorf("Failed to release vlan %v", err)
			}
		} else {
			// Clean up cache
			v.deleteVNIC(inf.MACAddress)
		}
	}
	v.pCache.Delete(workloadKind, workloadObj)
	return
}

// assignUsegs will set usegs for the workload, and send a message to
// the probe to override the ports
func (v *VCHub) assignUsegs(dcID, dcName, vmName string, workload *workload.Workload) {
	v.Log.Debugf("Assign usegs called for workload %s", workload.Name)
	if len(workload.Spec.HostName) == 0 {
		v.Log.Debugf("hostname not set yet for workload %s", workload.Name)
		return
	}
	host := workload.Spec.HostName
	for i, inf := range workload.Spec.Interfaces {
		v.Log.Debugf("processing inf %s", inf.MACAddress)
		// if we already have usegs in the workload object,
		// usegs update msgs have been sent to the probe already
		if inf.MicroSegVlan != 0 {
			v.Log.Debugf("inf %s is already assigned %d", inf.MACAddress, inf.MicroSegVlan)
			continue
		}

		entry := v.getVNIC(inf.MACAddress)
		if entry == nil {
			v.Log.Errorf("workload inf without useg was not in map, workload %s, mac %s", workload.Name, inf.MACAddress)
			return
		}

		// pg := entry.PG
		// port := entry.Port
		dc := v.GetDC(dcName)
		if dc == nil {
			v.Log.Errorf("Got workload %s create for DC we don't have state for: %s", vmName, dcName)
			continue
		}

		dvs := dc.GetPenDVS(createDVSName(dcName))

		vlan, err := dvs.UsegMgr.GetVlanForVnic(inf.MACAddress, host)
		if err == nil {
			// Already have an assignment
			workload.Spec.Interfaces[i].MicroSegVlan = uint32(vlan)
			v.deleteVNIC(inf.MACAddress)
			continue
		}

		vlan, err = dvs.UsegMgr.AssignVlanForVnic(inf.MACAddress, host)

		if err != nil {
			// TODO: if vlans are full, raise event
			v.Log.Errorf("Failed to assign vlan %v", err)
		} else {
			// Set useg
			workload.Spec.Interfaces[i].MicroSegVlan = uint32(vlan)
			v.Log.Debugf("inf %s assigned %d", inf.MACAddress, vlan)

			// TODO: Handle retries if it fails
			err = dvs.SetVlanOverride(entry.Port, vlan)
			if err != nil {
				v.Log.Errorf("Override vlan failed for workload %s, %s", vmName, err)
			}
		}

		v.deleteVNIC(inf.MACAddress)
	}

}

func (v *VCHub) processTags(prop types.PropertyChange, vcID string, dcID string, dcName string, workload *workload.Workload) {
	// Ovewrite old labels with new tags. API hook will ensure we don't overwrite user added tags.
	tagMsg := prop.Val.(defs.TagMsg)
	workload.Labels = generateLabelsFromTags(workload.Labels, tagMsg)
}

func (v *VCHub) processVMRuntime(prop types.PropertyChange, vcID string, dcID string, dcName string, workload *workload.Workload) {
	if prop.Val == nil {
		return
	}
	rt, ok := prop.Val.(types.VirtualMachineRuntimeInfo)
	if !ok {
		v.Log.Errorf("Property type wasn't VM runtime info, %v", prop.Val)
		return
	}
	if rt.Host == nil {
		v.Log.Errorf("Runtime's host property was empty")
		return
	}
	hostName := createHostName(vcID, dcID, rt.Host.Value)
	workload.Spec.HostName = hostName
}

func (v *VCHub) processVMName(prop types.PropertyChange, vcID string, dcID string, dcName string, workload *workload.Workload) {
	if prop.Val == nil {
		return
	}
	name := prop.Val.(string)

	if len(name) == 0 {
		return
	}
	addVMNameLabel(workload.Labels, name)
}

func (v *VCHub) processVMConfig(prop types.PropertyChange, vcID string, dcID string, dcName string, workload *workload.Workload) {
	if prop.Val == nil {
		return
	}
	vmConfig, ok := prop.Val.(types.VirtualMachineConfigInfo)
	if !ok {
		v.Log.Errorf("Expected VirtualMachineConfigInfo, got %v", reflect.TypeOf(prop.Val).Name())
		return
	}
	interfaces := v.extractInterfaces(workload.Name, dcID, dcName, vmConfig)
	workload.Spec.Interfaces = interfaces
	if len(vmConfig.Name) == 0 {
		return
	}
	addVMNameLabel(workload.Labels, vmConfig.Name)
}

// ParseVnic returns mac, port key, port group, ok
func (v *VCHub) parseVnic(vnic types.BaseVirtualDevice) (string, string, string, bool) {
	vec := v.GetVeth(vnic)
	if vec != nil {
		back, ok := vec.Backing.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo)
		if !ok {
			v.Log.Errorf("Expected types.VirtualEthernetCardDistributedVirtualPortBackingInfo, got %s", reflect.TypeOf(vec.Backing).Name())
			return "", "", "", false
		}
		mac, err := conv.ParseMacAddr(vec.MacAddress)
		if err != nil {
			v.Log.Errorf("Failed to parse mac addres. Err : %v", err)
			return "", "", "", false
		}
		pg := back.Port.PortgroupKey
		port := back.Port.PortKey
		return mac, port, pg, true
	}
	return "", "", "", false
}

func (v *VCHub) extractInterfaces(workloadName string, dcID string, dcName string, vmConfig types.VirtualMachineConfigInfo) []workload.WorkloadIntfSpec {
	var res []workload.WorkloadIntfSpec
	for _, d := range vmConfig.Hardware.Device {
		macStr, port, pgID, ok := v.parseVnic(d)
		if !ok {
			continue
		}

		entry := &vnicStruct{
			ObjectMeta: *createVNICMeta(macStr),
			PG:         pgID,
			Port:       port,
			Workload:   workloadName,
		}
		v.setVNIC(entry)

		var externalVlan uint32
		var pgObj *PenPG
		var nw *ctkit.Network
		var err error
		dc := v.GetDC(dcName)
		if dc != nil {
			pgObj = dc.GetPGByID(pgID)
			if pgObj != nil {
				nw, err = v.StateMgr.Controller().Network().Find(&pgObj.NetworkMeta)
				if err == nil {
					externalVlan = nw.Spec.VlanID
					v.Log.Debugf("Setting vlan %v for vnic %s", externalVlan, macStr)
				} else {
					v.Log.Errorf("Received EP with no corresponding venice network: PG: %s DC: %s Network meta %+v, err %s", pgID, dcName, pgObj.NetworkMeta, err)
				}
			} else {
				v.Log.Errorf("Received EP with PG we don't have state for: PG: %s DC: %s", pgID, dcName)
			}
		} else {
			v.Log.Errorf("Received EP for DC we don't have state for: %s", dcName)
		}

		vnic := workload.WorkloadIntfSpec{
			MACAddress:   macStr,
			ExternalVlan: externalVlan,
		}
		res = append(res, vnic)
	}
	return res
}

// GetVeth checks if the base virtual device is a vnic and returns a pointer to
// VirtualEthernetCard
func (v *VCHub) GetVeth(d types.BaseVirtualDevice) *types.VirtualEthernetCard {
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

func (v *VCHub) getWorkload(wlName string) *workload.Workload {
	meta := &api.ObjectMeta{
		Name: wlName,
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	// Check if already exists
	var workloadObj *workload.Workload
	existingWorkload := v.pCache.GetWorkload(meta)
	if existingWorkload != nil {
		// Each time Get is called, we endup creating a copy, if the retruned pointer is used
		// then we may have a copy in pCache and another being modied.
		temp := ref.DeepCopy(*existingWorkload).(workload.Workload)
		workloadObj = &temp
	}
	return workloadObj
}

func (v *VCHub) deleteWorkloadByName(dcID, wlName string) {
	meta := &api.ObjectMeta{
		Name: wlName,
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	// Check if already exists
	workload, err := v.StateMgr.Controller().Workload().Find(meta)
	if err == nil {
		v.deleteWorkload(&(*workload).Workload, dcID)
	}
}

func (v *VCHub) getVmkWorkload(dc *mo.Datacenter, wlName, hostName string) *workload.Workload {
	// Check if already exists
	workloadObj := v.getWorkload(wlName)
	if workloadObj == nil {
		meta := &api.ObjectMeta{
			Name: wlName,
			// TODO: Don't use default tenant
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Labels:    make(map[string]string),
		}
		workloadObj = &workload.Workload{
			TypeMeta: api.TypeMeta{
				Kind:       workloadKind,
				APIVersion: "v1",
			},
			ObjectMeta: *meta,
			Spec: workload.WorkloadSpec{
				HostName: hostName,
			},
		}
	}
	return workloadObj
}

func (v *VCHub) syncHostVmkNics(dc *mo.Datacenter, dvs *mo.VmwareDistributedVirtualSwitch, host *mo.HostSystem, vmknics []vnicStruct) {
	// workload name in the vnicStruct is set to WL name created for this host.
	wlName := createVmkWorkLoadName(v.VcID, dc.Self.Value, host.Self.Value)
	hostName := createHostName(v.VcID, dc.Self.Value, host.Self.Value)
	workloadObj := v.getVmkWorkload(dc, wlName, hostName)
	penDC := v.GetDC(dc.Name)
	newNicMap := map[string]bool{}
	oldNicMap := map[string]bool{}

	for _, vmknic := range vmknics {
		newNicMap[vmknic.Name] = true
	}
	for _, intf := range workloadObj.Spec.Interfaces {
		oldNicMap[intf.MACAddress] = true
		if _, ok := newNicMap[intf.MACAddress]; ok {
			// also in new.. so mark it as not new and not old
			newNicMap[intf.MACAddress] = false
			oldNicMap[intf.MACAddress] = false
		}
	}

	interfaces := []workload.WorkloadIntfSpec{}

	for _, existingNic := range workloadObj.Spec.Interfaces {
		if oldNicMap[existingNic.MACAddress] == true {
			// Delete this interface from workload
			v.Log.Infof("Deleting stale interface %v from workload %v", existingNic.MACAddress, wlName)
			// Remove useg vlan allocation for this
			penDC.GetPenDVS(createDVSName(dc.Name)).UsegMgr.ReleaseVlanForVnic(existingNic.MACAddress, workloadObj.Spec.HostName)
		}
	}
	for _, vmknic := range vmknics {
		// Find external vlan from venice network for the PG
		penPG := penDC.GetPGByID(vmknic.PG)
		var externalVlan uint32
		nw, err := v.StateMgr.Controller().Network().Find(&penPG.NetworkMeta)
		if err == nil {
			externalVlan = nw.Spec.VlanID
			v.Log.Infof("Setting vlan %v for vnic %s", externalVlan, vmknic.ObjectMeta.Name)
		} else {
			v.Log.Infof("No venice network for PG %s", vmknic.PG)
			continue
		}
		interfaces = append(interfaces, workload.WorkloadIntfSpec{
			MACAddress:   vmknic.ObjectMeta.Name,
			ExternalVlan: externalVlan,
		})
		// needed later by assignUsegs
		v.setVNIC(&vmknic)
	}
	if len(interfaces) == 0 {
		// vmkNics became zero, delete the workload
		v.Log.Infof("Delete vmkWorkLoad %s due to no EPs", wlName)
		v.deleteWorkloadByName(dc.Self.Value, wlName)
		return
	}
	sort.Slice(interfaces, func(i, j int) bool {
		return interfaces[i].MACAddress < interfaces[j].MACAddress
	})
	workloadObj.Spec.Interfaces = interfaces
	// Allocate useg vlans for all interface added to workload
	v.assignUsegs(dc.Self.Value, dc.Name, wlName, workloadObj)
	v.addWorkloadLabels(workloadObj, host.Name)
	v.Log.Infof("Create vmkWorkLoad %s", wlName)
	v.pCache.Set(workloadKind, workloadObj)
}

func (v *VCHub) addWorkloadLabels(workloadObj *workload.Workload, name string) {
	addVMNameLabel(workloadObj.Labels, name)
	utils.AddOrchNameLabel(workloadObj.Labels, v.OrchConfig.GetName())
}
