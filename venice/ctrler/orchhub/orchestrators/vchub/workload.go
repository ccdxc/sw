package vchub

import (
	"fmt"
	"reflect"
	"sort"

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

var (
	// shorthand names for migration stages and status
	stageMigrationNone  = workload.WorkloadMigrationStatus_MIGRATION_NONE.String()
	stageMigrationStart = workload.WorkloadMigrationStatus_MIGRATION_START.String()
	stageMigrationDone  = workload.WorkloadMigrationStatus_MIGRATION_DONE.String()
	stageMigrationAbort = workload.WorkloadMigrationStatus_MIGRATION_ABORT.String()
	// Dataplane Status
	statusNone     = workload.WorkloadMigrationStatus_NONE.String()
	statusStarted  = workload.WorkloadMigrationStatus_STARTED.String()
	statusDone     = workload.WorkloadMigrationStatus_DONE.String()
	statusFailed   = workload.WorkloadMigrationStatus_FAILED.String()
	statusTimedOut = workload.WorkloadMigrationStatus_TIMED_OUT.String()
)

func (v *VCHub) validateWorkload(in interface{}) (bool, bool) {
	obj, ok := in.(*workload.Workload)
	if !ok {
		return false, false
	}
	if len(obj.Spec.HostName) == 0 {
		return false, true
	}
	hostMeta := &api.ObjectMeta{
		Name: obj.Spec.HostName,
	}
	// check that host has been created already
	if _, err := v.StateMgr.Controller().Host().Find(hostMeta); err != nil {
		v.Log.Errorf("Couldn't find host %s for workload %s", hostMeta.Name, obj.GetObjectMeta().Name)
		return false, true
	}
	if len(obj.Spec.Interfaces) == 0 {
		v.Log.Errorf("workload %s has no interfaces", obj.GetObjectMeta().Name)
		return false, true
	}
	// check that workload is no longer in pvlan mode
	for _, inf := range obj.Spec.Interfaces {
		if inf.MicroSegVlan == 0 {
			v.Log.Errorf("inf %s has no useg for workload %s", inf.MACAddress, obj.GetObjectMeta().Name)
			// TODO check if this should be removed from apiserver
			return false, false
		}
	}
	return true, true
}

func (v *VCHub) handleWorkload(m defs.VCEventMsg) {
	v.Log.Debugf("Got handle workload event for %s", m.Key)
	meta := &api.ObjectMeta{
		Name: createVMWorkloadName(m.Originator, m.DcID, m.Key),
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	existingWorkload := v.pCache.GetWorkload(meta)
	var workloadObj *workload.Workload

	if existingWorkload == nil {
		v.Log.Debugf("This is a new workload - %s", meta.Name)
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

	v.syncUsegs(m.DcID, m.DcName, m.Key, existingWorkload, workloadObj)
	v.pCache.Set(workloadKind, workloadObj)
}

func (v *VCHub) handleVMotionStart(m defs.VMotionStartMsg) {
	if !m.HotMigration {
		// TODO: check vcenter events wrt when does it report hot vs cold vmotion
		// in cold vmotion, there is no traffic, so just act upon receiving the VM config
		// update
		v.Log.Infof("Ignore COLD VMotionStart Event for VM %s", m.VMKey)
		return
	}
	wlName := createVMWorkloadName(v.VcID, m.DcID, m.VMKey)
	hostName := createHostName(v.VcID, m.DcID, m.DstHostKey)
	workloadObj := v.pCache.GetWorkloadByName(wlName)
	if workloadObj == nil {
		v.Log.Debugf("Ignore VMotionStart Event for VM %s - No workload", m.VMKey)
		return
	}
	// We keep partial workload objects in pCache.. so workloads that are not on Pen-hosts can
	// be present in pCache. Check its host too
	curHostName := workloadObj.Spec.HostName
	if curHostName == "" || v.findHostByName(curHostName) == nil {
		// VMs coming from non-pensando hosts requires special flow state handling
		v.Log.Infof("Ignore VMotionStart Event for VM %s from non-pensando host %s", m.VMKey, curHostName)
		// This will be new WL creation, which will happen when we receive WL watch. the additional
		// info needed by DSC is to allow existing sessions to continue - TBD
		return
	}
	// Check new host
	if v.findHostByName(hostName) == nil {
		v.Log.Infof("Ignore VMotion Event for VM %s - to non-pensando host %s", m.VMKey, hostName)
		// Workload update will happen as part of WL watch when vMotion is complete
		return
	}
	if curHostName == hostName {
		// This could be an old vmotion event - Ignore it
		v.Log.Infof("Ignore VMotionStart Event for VM %s - same host", m.VMKey)
		return
	}
	if v.isWorkloadMigrating(workloadObj) {
		// TODO: need to work thru' cases on when this happens and how to handle it, corner case
		v.Log.Errorf("Back to Back vMotion for %s is not supported - must wait for first migration to complete",
			m.VMKey)
		return
	}
	dc := v.GetDCFromID(m.DcID)
	if dc == nil {
		v.Log.Errorf("Ignore vMotion for %s - DC %s not found", m.VMKey, m.DcID)
		return
	}
	// Both src and destination hosts are pensando, Trigger vMotion
	v.Log.Infof("Trigger vMotion for %s from %s to %s host", wlName, curHostName, hostName)

	wlCopy := *workloadObj
	// change the host to new host and allocate useg vlans from the new host's pool
	// TODO: Try to keep the same useg vlan values if free
	wlCopy.Spec.HostName = hostName
	_ = v.reassignUsegs(dc.Name, &wlCopy)

	// Start migration action will copy information from spec to status and install new
	// config into the spec
	if _, err := v.StateMgr.Controller().Workload().StartMigration(&wlCopy); err != nil {
		v.Log.Errorf("Could not start migration on workload %s - %s", wlName, err)
	}
}

func (v *VCHub) handleVMotionFailed(m defs.VMotionFailedMsg) {
	wlName := createVMWorkloadName(v.VcID, m.DcID, m.VMKey)
	hostName := createHostName(v.VcID, m.DcID, m.DstHostKey)
	wlObj := v.pCache.GetWorkloadByName(wlName)
	if wlObj == nil {
		v.Log.Errorf("Ignore Cancel vMotion for %s - no workload found", wlName)
		return
	}
	if wlObj.Spec.HostName != hostName {
		v.Log.Errorf("Ignore cancel vMotion for %s - incorrect host information %s", m.VMKey, hostName)
		return
	}
	if v.isWorkloadMigrating(wlObj) {
		v.Log.Infof("Cancel vMotion for %s to host %s", wlName, hostName)
		if _, err := v.StateMgr.Controller().Workload().AbortMigration(wlObj); err != nil {
			v.Log.Errorf("Could not cancel vmotion on workload %s - %s", wlName, err)
		}
		// since useg vlan override is not changed, no need to revert it
	} else {
		v.Log.Errorf("Cannot Cancel vMotion for %s to host %s - Not Migrating", wlName, hostName)
	}
}

func (v *VCHub) handleVMotionDone(m defs.VMotionDoneMsg) {
	wlName := createVMWorkloadName(v.VcID, m.DcID, m.VMKey)
	wlObj := v.pCache.GetWorkloadByName(wlName)
	if wlObj == nil {
		v.Log.Errorf("Cannot Complete vMotion for %s - no workload found", wlName)
		return
	}
	if !v.isWorkloadMigrating(wlObj) {
		v.Log.Errorf("Cannot Complete vMotion for %s - not migrating", wlName)
		return
	}
	// update useg vlan overrides in vCenter, indicate to dataplane to complete migration (last sync etc)
	if wlObj.Status.MigrationStatus != nil && wlObj.Status.MigrationStatus.Stage == stageMigrationStart {
		// If workload update watch is processed ahead of this event, it may have already signalled
		// migration completion.
		_ = v.finishMigration(m.DcID, wlObj)
	}
}

func (v *VCHub) finishMigration(dcID string, wlObj *workload.Workload) error {
	wlName := wlObj.Name
	if err := v.setVlanOverride(dcID, wlObj); err != nil {
		v.Log.Errorf("Cannot Complete vMotion for %s - not set vlan overrides", wlName)
		return err
	}
	// TODO: There should be no interfaces added/removed during migration,
	// if they are, handle them here?
	// release the old useg vlans (from the status)
	if err := v.releaseOldUsegVlans(dcID, wlObj); err != nil {
		v.Log.Errorf("Cannot Complete vMotion for %s - %s", wlName, err)
		return err
	}
	if _, err := v.StateMgr.Controller().Workload().FinishMigration(wlObj); err != nil {
		v.Log.Errorf("Could not complete migration on workload %s - %s", wlName, err)
		return err
	}
	return nil
}

func (v *VCHub) releaseOldUsegVlans(dcID string, wlObj *workload.Workload) error {
	// free the old useg vlans which are stored in the status of workload object
	// these vlans are usegs vlans that were used before migration
	hostName := wlObj.Status.HostName
	if hostName == "" {
		return fmt.Errorf("Cannot free microseg vlans for workload %s - No host", wlObj.Name)
	}

	dc := v.GetDCFromID(dcID)
	if dc == nil {
		return fmt.Errorf("Cannot free microseg vlans - No DC for workload %s", wlObj.Name)
	}
	// TODO: Remove hardcoded dvs name
	dvs := dc.GetPenDVS(createDVSName(dc.Name))
	for _, intf := range wlObj.Status.Interfaces {
		err := dvs.UsegMgr.ReleaseVlanForVnic(intf.MACAddress, hostName)
		if err != nil {
			return fmt.Errorf("Failed to release vlan %d on host %s", intf.MicroSegVlan, hostName)
		}
	}
	return nil
}

func (v *VCHub) setVlanOverride(dcID string, wlObj *workload.Workload) error {
	// for every microseg vlan of workload interfaces set it override vlan on
	// corresponding interface of the DVS
	dc := v.GetDCFromID(dcID)
	dvs := dc.GetPenDVS(createDVSName(dc.Name))
	for _, inf := range wlObj.Spec.Interfaces {
		entry := v.getVnicInfoForWorkload(wlObj.Name, inf.MACAddress)
		if entry == nil {
			errMsg := fmt.Errorf("Vnic port information not found for workload %s, mac %s", wlObj.Name, inf.MACAddress)
			v.Log.Errorf("%s", errMsg)
			return errMsg
		}
		// TODO: Handle retries if it fails
		if err := dvs.SetVlanOverride(entry.Port, int(inf.MicroSegVlan)); err != nil {
			v.Log.Errorf("Override vlan failed for workload %s, %s", wlObj.Name, err)
			return err
		}
	}
	return nil
}

func (v *VCHub) reassignUsegs(dcName string, wlObj *workload.Workload) error {
	// assign new useg vlans for workload interfaces. Try to reuse existing value if available on
	// the new host.
	// Do not set overrides in this function

	hostName := wlObj.Spec.HostName
	if len(hostName) == 0 {
		errMsg := fmt.Errorf("Host not provided for microseg vlan allocation")
		v.Log.Errorf("%s", errMsg)
		return errMsg
	}
	v.Log.Debugf("Reassign usegs called for workload %s on host %s", wlObj.Name, wlObj.Spec.HostName)
	dc := v.GetDC(dcName)
	if dc == nil {
		errMsg := fmt.Errorf("Reassign microseg vlans: DC %s not found for workload %s", dcName, wlObj.Name)
		v.Log.Errorf("%s", errMsg)
		return errMsg
	}
	dvs := dc.GetPenDVS(createDVSName(dcName))

	for i, inf := range wlObj.Spec.Interfaces {
		// if we already have usegs in the workload object, try to allocate the same if avaialble
		// in the new host's allocator
		if inf.MicroSegVlan != 0 {
			err := dvs.UsegMgr.SetVlanForVnic(inf.MACAddress, hostName, int(inf.MicroSegVlan))
			if err == nil {
				v.Log.Debugf("Reassigned vlan %d for vnic %s", inf.MicroSegVlan, inf.MACAddress)
				continue
			}
			// could not reassign the same vlan id, allocate new one
			v.Log.Debugf("Could not reassign vlan %d for vnic %s", inf.MicroSegVlan, inf.MACAddress)
		}
		vlan, err := dvs.UsegMgr.AssignVlanForVnic(inf.MACAddress, hostName)
		if err != nil {
			errMsg := fmt.Errorf("Failed to assign vlan %v", err)
			v.Log.Errorf("%s", errMsg)
			return errMsg
		}
		wlObj.Spec.Interfaces[i].MicroSegVlan = uint32(vlan)
		v.Log.Debugf("vnic %s assigned %d", inf.MACAddress, vlan)
	}
	return nil
}

func (v *VCHub) isWorkloadMigrating(wlObj *workload.Workload) bool {
	if wlObj.Status.MigrationStatus == nil {
		return false
	}
	if wlObj.Status.MigrationStatus.Stage == stageMigrationNone ||
		wlObj.Status.MigrationStatus.Status == statusDone ||
		wlObj.Status.MigrationStatus.Status == statusFailed {
		return false
	}
	return true
}

func (v *VCHub) deleteWorkload(workloadObj *workload.Workload, dcName string) {
	v.Log.Debugf("Deleting workload %s", workloadObj.Name)
	// free vlans
	for _, inf := range workloadObj.Spec.Interfaces {
		v.releaseInterface(dcName, &inf, workloadObj, true)
	}
	v.pCache.Delete(workloadKind, workloadObj)
	return
}

func (v *VCHub) releaseInterface(dcName string, inf *workload.WorkloadIntfSpec, workloadObj *workload.Workload, deleteVnicMetadata bool) {
	if inf.MicroSegVlan != 0 {
		if workloadObj.Spec.HostName == "" {
			v.Log.Infof("Release interface called for workload %s (DC %s) that has no host", workloadObj.Name, dcName)
			return
		}
		dc := v.GetDC(dcName)
		if dc == nil {
			v.Log.Errorf("Got Workload delete %s for a DC we don't have state for %s", workloadObj.Name, dcName)
			return
		}

		// TODO: Remove hardcoded dvs name
		dvs := dc.GetPenDVS(createDVSName(dcName))
		err := dvs.UsegMgr.ReleaseVlanForVnic(inf.MACAddress, workloadObj.Spec.HostName)
		if err != nil {
			v.Log.Errorf("Failed to release vlan %v", err)
		}
		inf.MicroSegVlan = 0
		v.Log.Debugf("Released interface %s for workload %s", inf.MACAddress, workloadObj.Name)
	}
	if deleteVnicMetadata {
		v.removeVnicInfoForWorkload(workloadObj.Name, inf.MACAddress)
	}
}

func (v *VCHub) syncUsegs(dcID, dcName, vmName string, existingWorkload, workloadObj *workload.Workload) {
	if workloadObj.Spec.HostName == "" && existingWorkload != nil &&
		existingWorkload.Spec.HostName != "" {
		// Cannot allocate/sync useg vlans until host is known
		// TODO In case of vmotion across vcenters, it seems that vcenter creates VM in new VC with
		// empty host and updates it later with correct host information after vmotion is complete
		// this needs to handled separately
		v.Log.Infof("VM %s - host got removed, could be a vmotion across vCenters - TDB", vmName)
		return
	}
	// Handle releasing usegs for vnics that are removed
	if existingWorkload != nil && existingWorkload.Spec.HostName != "" {
		// If we are going from 1+ vnics to 0 vnics, we delete the workload from apiserver,
		// and store the new state of the workload (0 infs) in pcache
		if len(existingWorkload.Spec.Interfaces) > 0 && len(workloadObj.Spec.Interfaces) == 0 {
			// Delete workload will release the usegs it has assigned
			v.deleteWorkload(existingWorkload, dcName)
			return
		}

		// If host has changed, we need to free all the old usegs
		if existingWorkload.Spec.HostName != workloadObj.Spec.HostName {
			if v.isWorkloadMigrating(existingWorkload) {
				v.Log.Infof("Ingore VM host change during migration for vm %s", vmName)
				return
			}
			v.Log.Infof("Vm changed hosts, releasing current usegs")
			for _, inf := range existingWorkload.Spec.Interfaces {
				// don't remove vnic info, as it is needed to assign new useg values
				v.releaseInterface(dcName, &inf, existingWorkload, false)
			}
			// Remove useg value in the workloadObj
			for i := range workloadObj.Spec.Interfaces {
				workloadObj.Spec.Interfaces[i].MicroSegVlan = 0
			}

		} else {
			if v.isWorkloadMigrating(existingWorkload) {
				// existing workload's host is changed to new host when migration starts
				// vcenter keeps the old host in its vm object until migration is complete
				// so when host from vcenter matches the host in existing workload object, it can be
				// treated as vmotion complete
				if existingWorkload.Status.MigrationStatus != nil &&
					existingWorkload.Status.MigrationStatus.Stage == stageMigrationStart {
					// If migration done event is not already processed, mark migration complete
					if err := v.finishMigration(dcID, existingWorkload); err != nil {
						v.Log.Errorf("Cannot Finish migration for %s", vmName)
						return
					}
				}
			}
			// Host is same, check if any interfaces have been removed
			infs := map[string]bool{}
			for _, inf := range workloadObj.Spec.Interfaces {
				infs[inf.MACAddress] = true
			}

			for _, inf := range existingWorkload.Spec.Interfaces {
				if _, ok := infs[inf.MACAddress]; !ok {
					// inf has been removed
					v.releaseInterface(dcName, &inf, existingWorkload, true)
				}
			}
		}
	}

	v.assignUsegs(dcID, dcName, vmName, workloadObj)
}

// assignUsegs will set usegs for the workload, and send a message to
// the probe to override the ports
func (v *VCHub) assignUsegs(dcID, dcName, vmName string, workload *workload.Workload) {
	v.Log.Debugf("Assign usegs called for workload %s on host %s", workload.Name, workload.Spec.HostName)
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

		entry := v.getVnicInfoForWorkload(workload.Name, inf.MACAddress)
		if entry == nil {
			v.Log.Errorf("workload inf without useg was not in map, workload %s, mac %s", workload.Name, inf.MACAddress)
			return
		}

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
			// v.Log.Errorf("Expected types.VirtualEthernetCardDistributedVirtualPortBackingInfo, got %s", reflect.TypeOf(vec.Backing).Name())
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
					continue
				}
			} else {
				v.Log.Errorf("Received EP with PG we don't have state for: PG: %s DC: %s", pgID, dcName)
				continue
			}
		} else {
			v.Log.Errorf("Received EP for DC we don't have state for: %s", dcName)
			continue
		}

		entry := &vnicEntry{
			PG:         pgID,
			Port:       port,
			MacAddress: macStr,
		}
		v.addVnicInfoForWorkload(workloadName, entry)

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
		// v.Log.Infof("Ignoring virtual device %s", dKind.Name())
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
	// Check if already exists
	var workloadObj *workload.Workload
	existingWorkload := v.pCache.GetWorkloadByName(wlName)
	if existingWorkload != nil {
		// Each time Get is called, we endup creating a copy, if the retruned pointer is used
		// then we may have a copy in pCache and another being modied.
		temp := ref.DeepCopy(*existingWorkload).(workload.Workload)
		workloadObj = &temp
	}
	return workloadObj
}

func (v *VCHub) deleteWorkloadByName(dcID, wlName string) {
	// Check if already exists
	workload := v.pCache.GetWorkloadByName(wlName)
	if workload != nil {
		v.deleteWorkload(workload, dcID)
	}
}

func (v *VCHub) getVmkWorkload(penDC *PenDC, wlName, hostName string) *workload.Workload {
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

func (v *VCHub) syncHostVmkNics(penDC *PenDC, penDvs *PenDVS, hKey, hName string, hConfig *types.HostConfigInfo) {
	if !isPensandoHost(hConfig) {
		return
	}
	wlName := createVmkWorkloadName(v.VcID, penDC.dcRef.Value, hKey)
	hostName := createHostName(v.VcID, penDC.dcRef.Value, hKey)
	workloadObj := v.getVmkWorkload(penDC, wlName, hostName)
	newNicMap := map[string]bool{}
	interfaces := []workload.WorkloadIntfSpec{}
	vmkNicInfo := workloadVnics{
		ObjectMeta: *createWorkloadVnicsMeta(wlName),
		Interfaces: map[string]*vnicEntry{},
	}
	for _, vmkNic := range hConfig.Network.Vnic {
		v.Log.Infof("Processing VmkNic %s on host %s", vmkNic.Key, hKey)
		if vmkNic.Portgroup != "" {
			// standard (non-DV) port gorup is not supported
			v.Log.Infof("Skip vnic - not in DV Port group")
			continue
		}
		// Create mac address in venice format
		macStr, err := conv.ParseMacAddr(vmkNic.Spec.Mac)
		if err != nil {
			v.Log.Errorf("Incorrectly formed MAC address %s for vmknic", vmkNic.Spec.Mac)
			continue
		}
		pgKey := vmkNic.Spec.DistributedVirtualPort.PortgroupKey
		portKey := vmkNic.Spec.DistributedVirtualPort.PortKey
		penPG := penDC.GetPGByID(pgKey)
		if penPG == nil {
			// not a venice network
			v.Log.Errorf("PenPG not found for PG Id %s", pgKey)
			continue
		}
		var externalVlan uint32
		nw, err := v.StateMgr.Controller().Network().Find(&penPG.NetworkMeta)
		if err == nil {
			externalVlan = nw.Spec.VlanID
			v.Log.Infof("Setting vlan %v for vnic %s", externalVlan, macStr)
		} else {
			v.Log.Infof("No venice network for PG %s", pgKey)
			continue
		}
		interfaces = append(interfaces, workload.WorkloadIntfSpec{
			MACAddress:   macStr,
			ExternalVlan: externalVlan,
		})
		// needed later by assignUsegs
		vmkNicInfo.Interfaces[macStr] = &vnicEntry{
			PG:         pgKey,
			Port:       portKey,
			MacAddress: macStr,
		}
		v.setWorkloadVnicsObject(&vmkNicInfo)
		v.Log.Infof("Add vmkInterface %s Port %s", vmkNic.Device, portKey)
		newNicMap[macStr] = true
	}

	for _, intf := range workloadObj.Spec.Interfaces {
		if _, ok := newNicMap[intf.MACAddress]; !ok {
			// Delete this old interface from workload
			v.Log.Infof("Deleting stale interface %v from workload %v", intf.MACAddress, wlName)
			// Remove useg vlan allocation for this
			penDC.GetPenDVS(createDVSName(penDC.Name)).UsegMgr.ReleaseVlanForVnic(intf.MACAddress, workloadObj.Spec.HostName)
		}
	}
	if len(interfaces) == 0 {
		// vmkNics became zero, delete the workload
		v.Log.Infof("Delete vmkWorkload %s due to no EPs", wlName)
		v.deleteWorkloadByName(penDC.dcRef.Value, wlName)
		return
	}
	sort.Slice(interfaces, func(i, j int) bool {
		return interfaces[i].MACAddress < interfaces[j].MACAddress
	})
	workloadObj.Spec.Interfaces = interfaces
	// Allocate useg vlans for all interface added to workload
	// TODO it may be better to pass dvs to assignUseg in future
	v.assignUsegs(penDC.dcRef.Value, penDC.Name, wlName, workloadObj)
	v.addWorkloadLabels(workloadObj, hName)
	v.Log.Infof("Create/Update vmkWorkload %s", wlName)
	v.pCache.Set(workloadKind, workloadObj)
}

func (v *VCHub) addWorkloadLabels(workloadObj *workload.Workload, name string) {
	addVMNameLabel(workloadObj.Labels, name)
	utils.AddOrchNameLabel(workloadObj.Labels, v.OrchConfig.GetName())
}
