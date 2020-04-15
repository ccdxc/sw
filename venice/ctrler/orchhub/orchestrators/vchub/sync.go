package vchub

import (
	"time"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// returns whether sync executed
func (v *VCHub) sync() bool {
	v.Log.Infof("VCHub sync called")

	v.syncLock.Lock()
	v.syncDone = false
	defer func() {
		v.syncDone = true
		v.syncLock.Unlock()
	}()

	v.Log.Infof("VCHub %v syncing........", v)

	// Check that probe session is connected
	count := 3
	for !v.probe.IsSessionReady() && count > 0 {
		select {
		case <-v.Ctx.Done():
			return false
		case <-time.After(1 * time.Second):
			count--
		}
	}
	if count == 0 {
		v.Log.Infof("Probe session isn't connected, exiting sync...")
		return false
	}
	// Once syncLock is given up, network events should be acted upon
	v.processVeniceEventsLock.Lock()
	v.processVeniceEvents = true
	v.processVeniceEventsLock.Unlock()

	opts := api.ListWatchOptions{}
	// By the time this is called, the statemanager would have setup watchers to the API server
	// and synched the local cache with the API server
	nw, err := v.StateMgr.Controller().Network().List(v.Ctx, &opts)
	if err != nil {
		v.Log.Errorf("Failed to get network list. Err : %v", err)
	}

	hosts, err := v.StateMgr.Controller().Host().List(v.Ctx, &opts)
	if err != nil {
		v.Log.Errorf("Failed to get host list. Err : %v", err)
	}

	workloads, err := v.StateMgr.Controller().Workload().List(v.Ctx, &opts)
	if err != nil {
		v.Log.Errorf("Failed to get workload list. Err : %v", err)
	}

	// TODO remove the prints below. Added to calm down go fmt
	v.Log.Infof("Got Networks : %v", nw)
	v.Log.Infof("Got Workloads : %v", workloads)
	v.Log.Infof("Got Hosts : %v", hosts)

	/**
		1. List DCs
		2. Per DC, list DVS, list VMS, list Hosts
		3. Per DVS,
				- list PGs
				- Build Useg Mgr
				- assign PG Vlans and workload vlans
		4, Delete stale venice objects push oper to store
		5. Push existing objects and creates.
	**/
	vmkMap := map[string]bool{}

	dcs := v.probe.ListDC()
	for _, dc := range dcs {
		// TODO: Remove
		if !v.isManagedNamespace(dc.Name) {
			v.Log.Infof("Skipping DC %s", dc.Name)
			continue
		}

		_, err := v.NewPenDC(dc.Name, dc.Self.Value)
		if err != nil {
			v.Log.Errorf("Creating DC %s failed: %s", dc.Name, err)
			continue
		}
		v.Log.Debugf("Created DC %s", dc.Name)
		dcRef := dc.Reference()
		dvsObjs := v.probe.ListDVS(&dcRef)
		pgs := v.probe.ListPG(&dcRef)
		vms := v.probe.ListVM(&dcRef)
		vcHosts := v.ListPensandoHosts(&dcRef)

		v.syncNetwork(nw, dc, dvsObjs, pgs)
		v.syncNewHosts(dc, vcHosts, vmkMap)
		v.syncVMs(workloads, dc, vms, pgs, vmkMap)
		// Removing hosts after removing workloads to fix WL -> Host dependency
		v.syncStaleHosts(dc, vcHosts, hosts)
	}

	// Process any API events we have missed (migration.status.done/timeout)
	for _, workload := range workloads {
		if workload.Status.MigrationStatus != nil {
			v.handleWorkloadEvent(kvstore.Updated, &workload.Workload)
		}
	}

	v.Log.Infof("Sync done for VCHub. %v", v)
	return true
}

func (v *VCHub) syncNewHosts(dc mo.Datacenter, vcHosts []mo.HostSystem, vmkMap map[string]bool) {
	v.Log.Infof("Syncing new hosts on DC %s============", dc.Name)

	// Process all hosts
	for _, host := range vcHosts {
		m := defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			Key:        host.Self.Value,
			DcID:       dc.Self.Value,
			DcName:     dc.Name,
			Originator: v.VcID,
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Name: string(defs.HostPropConfig),
					Op:   "add",
					Val:  *(host.Config),
				},
				types.PropertyChange{
					Name: string(defs.HostPropName),
					Op:   "add",
					Val:  host.Name,
				},
			},
		}
		v.Log.Debugf("Process config change for host %s", host.Reference().Value)
		// check if host is added to PenDVS is not needed here, handleHost() will check it and
		// delete the host and associated workloads (including vmk workload)
		vmkMap[v.createVmkWorkloadName(dc.Reference().Value, host.Reference().Value)] = true
		v.handleHost(m)
	}
}

func (v *VCHub) syncStaleHosts(dc mo.Datacenter, vcHosts []mo.HostSystem, hosts []*ctkit.Host) {
	v.Log.Infof("Syncing stale hosts on DC %s============", dc.Name)
	vcHostMap := map[string]bool{}
	for _, vcHost := range vcHosts {
		hostName := v.createHostName(dc.Self.Value, vcHost.Self.Value)
		vcHostMap[hostName] = true
	}

	// Deleting stale hosts
	for _, host := range hosts {
		if !utils.IsObjForOrch(host.Labels, v.VcID, dc.Name) {
			// Filter out hosts not for this Orch
			v.Log.Debugf("Skipping host %s", host.Name)
			continue
		}
		if _, ok := vcHostMap[host.Name]; !ok {
			// host no longer exists
			// Delete vmkWorkload if created for this host
			wlName := createVmkWorkloadNameFromHostName(host.Name)
			v.deleteWorkloadByName(wlName)

			v.Log.Debugf("Deleting stale host %s", host.Name)
			v.deleteHost(&host.Host)
		}
	}
}

func (v *VCHub) syncNetwork(networks []*ctkit.Network, dc mo.Datacenter, dvsObjs []mo.VmwareDistributedVirtualSwitch, pgs []mo.DistributedVirtualPortgroup) {
	v.Log.Infof("Syncing networks on DC %s============", dc.Name)
	// SYNCING DVS AND PG
	dcName := dc.Name
	penDC := v.GetDC(dcName)
	for _, dvs := range dvsObjs {
		if !isPensandoDVS(dvs.Name, dcName) {
			v.Log.Debugf("Skipping dvs %s", dvs.Name)
			continue
		}
		v.Log.Debugf("Processing dvs %s", dvs.Name)

		penDVS := penDC.GetPenDVS(dvs.Name)

		pgMap := map[string]mo.DistributedVirtualPortgroup{}
		for _, pg := range pgs {
			// Only process PGs that belong to this DVS
			if pg.Config.DistributedVirtualSwitch.Value == dvs.Self.Value {
				pgMap[pg.Name] = pg
			}
		}

		// For every PG, we mark the vlan it owns if it is a pensando PG
		// If any PG of ours is not in PVLAN mode, we reset it
		// For New networks, we create PGs for them

		pgNameMap := map[string]api.ObjectMeta{}
		for _, nw := range networks {
			for _, orch := range nw.Network.Spec.Orchestrators {
				if orch.Name == v.VcID && orch.Namespace == dcName {
					pgName := CreatePGName(nw.Network.Name)
					pgNameMap[pgName] = nw.Network.ObjectMeta
				}
			}
		}

		for _, pg := range pgMap {
			// pgConfig := pg.Config
			vlanConfig, err := extractVlanConfig(pg.Config)
			if err != nil {
				v.Log.Errorf("%s", err)
				continue
			}

			pgName := pg.Name

			vlan := -1
			switch vlanSpec := vlanConfig.(type) {
			case *types.VmwareDistributedVirtualSwitchPvlanSpec:
				// If the pg is in promiscuous, the pvlanID is the the primary vlan
				// If it's isolated mode, it will be secondary VLAN
				// This will be handled in setVlansForPG
				vlan = int(vlanSpec.PvlanId)
				if !useg.IsPGVlanSecondary(vlan) {
					// Don't respect the given PG since it's not in secondary pvlan
					vlan = -1
				}
			case *types.VmwareDistributedVirtualSwitchVlanIdSpec:
				// TODO: Reserve vlanSpec.VlanId from useg space for all hosts.
			case *types.VmwareDistributedVirtualSwitchTrunkVlanSpec:
				// Do nothing for reserving vlans
				// TODO: Generate event
			}

			_, ok := pgNameMap[pgName]
			if !ok {
				v.Log.Infof("No venice network info is available for this pg %s in DC %s", pgName, dcName)
				continue
			}

			v.Log.Infof("setting PG vlans %s ", pg.Name)
			// It may still be -1 if the user modified the PGs vlan spec
			// Will set it back after we allocate the rest of the known pgs
			// We only care about storing the vlan if it is our PG
			if vlan != -1 && isPensandoPG(pgName) {
				err := penDVS.UsegMgr.SetVlansForPG(pgName, vlan)
				if err != nil {
					// PG will be reassigned to a new value later
					v.Log.Infof("Setting vlans %d for PG %s failed: err %s", vlan, pg.Name, err)
					continue
				}
			}
		}

		for _, nw := range networks {
			v.Log.Debugf("Checking nw %s", nw.Network.Name)
			for _, orch := range nw.Network.Spec.Orchestrators {
				if orch.Name == v.VcID && orch.Namespace == dcName {
					pgName := CreatePGName(nw.Network.Name)

					err := penDVS.AddPenPG(pgName, nw.Network.ObjectMeta)
					v.Log.Infof("Create Pen PG %s returned %s", pgName, err)

					delete(pgMap, pgName)
				} else {
					v.Log.Debugf("vcID %s  dcName %s does not match orch-spec %s - %s",
						v.VcID, dcName, orch.Name, orch.Namespace)
				}
			}
		}

		uplinkPGs := dvs.Config.GetDVSConfigInfo().UplinkPortgroup
		for _, pg := range pgMap {
			// Check if it is an uplink PG
			isUplink := false
			for _, uplinkPG := range uplinkPGs {
				if pg.Reference().Value == uplinkPG.Value {
					isUplink = true
					break
				}
			}
			if !isUplink {
				err := v.probe.RemovePenPG(dcName, pg.Name, 1)
				if err != nil {
					v.Log.Errorf("Failed to delete PG %s, removing management tag. Err %s", pg.Name, err)
					tagErrs := v.probe.RemovePensandoTags(pg.Reference())
					if len(tagErrs) != 0 {
						v.Log.Errorf("Failed to remove tags, errs %v", tagErrs)
					}
				}
				v.Log.Infof("Delete PG %s returned %s", pg.Name, err)
			} else {
				v.Log.Infof("Skipped deletion of PG as it is the uplink pg", pg.Name)
			}
		}
	}
}

func (v *VCHub) fetchVMs(penDC *PenDC) {
	v.Log.Infof("VCHub sync VMs only called")
	v.syncLock.Lock()
	defer func() {
		v.Log.Infof("VCHub sync VMs only done")
		v.syncLock.Unlock()
	}()

	v.Log.Infof("VCHub %v syncing VMs only........", v)
	// Check that probe session is connected
	count := 3
	for !v.probe.IsSessionReady() && count > 0 {
		select {
		case <-v.Ctx.Done():
			return
		case <-time.After(1 * time.Second):
			count--
		}
	}
	if count == 0 {
		v.Log.Infof("Probe session isn't connected, exiting sync...")
		return
	}

	vms := v.probe.ListVM(&penDC.dcRef)

	for _, vm := range vms {
		m := v.convertWorkloadToEvent(penDC.dcRef.Value, penDC.Name, vm)
		v.handleVM(m)
	}
}

func (v *VCHub) syncVMs(workloads []*ctkit.Workload, dc mo.Datacenter, vms []mo.VirtualMachine, pgs []mo.DistributedVirtualPortgroup, vmkMap map[string]bool) {
	v.Log.Infof("Syncing vms on DC %s============", dc.Name)
	dcName := dc.Name
	penDC := v.GetDC(dcName)
	penDvs := penDC.GetPenDVS(CreateDVSName(dcName))

	pgKeyToName := map[string]string{}
	for _, pg := range pgs {
		pgKeyToName[pg.Self.Value] = pg.Name
	}
	vmMap := map[string]mo.VirtualMachine{}
	for _, vm := range vms {
		// TODO: check the vm is for us
		vmName := v.createVMWorkloadName(dc.Self.Value, vm.Self.Value)
		vmMap[vmName] = vm
	}

	// Deleting stale workloads and build useg state
	for _, workload := range workloads {
		if !utils.IsObjForOrch(workload.Labels, v.VcID, dcName) {
			// Filter out workloads not for this Orch/DC
			v.Log.Debugf("Skipping workload %s", workload.Name)
			continue
		}
		if _, ok := vmkMap[workload.Name]; ok {
			// do not delete vmkWorkloads
			continue
		}
		if _, ok := vmMap[workload.Name]; !ok {
			// workload no longer exists
			v.Log.Infof("Found stale workload %s", workload.Name)
			if v.isWorkloadMigrating(&workload.Workload) && v.isVMotionAcrossDC(&workload.Workload) {
				// Fetch the workload to see where it is
				vmKey := v.parseVMKeyFromWorkloadName(workload.Workload.Name)
				// check finish migration
				_, err := v.probe.GetVM(vmKey, defaultRetryCount)
				if err != nil {
					v.Log.Infof("Failed to find VM %s, deleting %s", vmKey, err)
					v.deleteWorkload(&workload.Workload)
					continue
				}
				// Check if across DC vmotion is in final sync. If so, datapath may have already written state
				if v.isWorkloadMigrating(&workload.Workload) && workload.Workload.Status.MigrationStatus.Stage == stageMigrationFinalSync {
					v.Log.Infof("Across DC vmotion is in final sync, check datapath value...")
					v.handleWorkloadEvent(kvstore.Updated, &workload.Workload)
				}
				continue
			}
			v.Log.Infof("Deleting stale workload %s", workload.Name)
			v.deleteWorkload(&workload.Workload)
			continue
		}

		// build useg state
		hostName := workload.Spec.HostName
		for _, inf := range workload.Spec.Interfaces {
			if inf.MicroSegVlan != 0 {
				err := penDvs.UsegMgr.SetVlanForVnic(inf.MACAddress, hostName, int(inf.MicroSegVlan))
				if err != nil {
					v.Log.Errorf("Setting vlan %d for vnic %s returned %s", inf.MicroSegVlan, inf.MACAddress, err)
				}
			}
		}

		if v.isWorkloadMigrating(&workload.Workload) && workload.Workload.Status.MigrationStatus.Stage == stageMigrationFinalSync {
			v.handleWorkloadEvent(kvstore.Updated, &workload.Workload)
		}
	}

	// For each VM, we get its vlan overrides and reset any of them if needed
	ports, err := penDvs.GetPortSettings()
	if err != nil {
		v.Log.Errorf("Sync failed to get ports for dc %s dvs %s, %s", penDvs.DcName, penDvs.DvsName, err)
	}
	// Extract vnic overrides
	overrides := map[string]struct {
		vlan int
		pg   string
	}{}
	for _, port := range ports {
		portKey := port.Key
		portSetting, ok := port.Config.Setting.(*types.VMwareDVSPortSetting)
		if !ok {
			// skipping
			// v.Log.Infof("Skipping port %s - port setting %v", portKey, port.Config.Setting)
			continue
		}
		vlanSpec, ok := portSetting.Vlan.(*types.VmwareDistributedVirtualSwitchVlanIdSpec)
		if !ok {
			// Skipping
			// v.Log.Infof("Skipping port %s - port setting vlan type %v", portKey, portSetting.Vlan)
			continue
		}
		if vlanSpec.VlanId == 0 {
			v.Log.Infof("Vlan override for port %s is 0", portKey)
			continue
		}
		entry := struct {
			vlan int
			pg   string
		}{
			vlan: int(vlanSpec.VlanId),
			pg:   port.PortgroupKey,
		}
		overrides[portKey] = entry
	}
	v.Log.Debugf("Overrides currently set %+v", overrides)

	// Set assignments.
	for _, vm := range vms {
		for _, d := range vm.Config.Hardware.Device {
			macStr, port, _, ok := v.parseVnic(d)
			if !ok {
				continue
			}
			host := v.createHostName(dc.Self.Value, vm.Runtime.Host.Value)
			vlan, err := penDvs.UsegMgr.GetVlanForVnic(macStr, host)
			if err != nil {
				continue // Skipping delete from overrides map
				// We didn't assign this vm a useg yet.
			}
			if vlan != overrides[port].vlan {
				// assign the right override value
				v.Log.Errorf("Reassigning vlan to have the override of %v", vlan)
				// We have an assignment for this vnic that is different, change the override to the right value
				err := penDvs.SetVlanOverride(port, vlan)
				if err != nil {
					v.Log.Errorf("Failed to set vlan override. DC %s DVS %s port %s vlan %s. Err %s", penDvs.DcName, penDvs.DvsName, port, vlan)
					continue // Don't delete from overrides as it has the wrong vlan still. We'll try to reset the vlan completely
				}
			}
			delete(overrides, port)
		}
	}

	// Any ports left should be reset
	resetMap := map[string][]string{}
	for port, entry := range overrides {
		if entry.pg == "" {
			// Don't know what to set this PG back to, so we reset to 0
			// This should never happen
			continue
		}
		pgName := pgKeyToName[entry.pg]
		ports, ok := resetMap[pgName]
		if !ok {
			ports = []string{}
		}
		resetMap[pgName] = append(ports, port)
	}
	// Resetting vlan overrides is not needed. As soon as port is disconnected,
	// the pvlan settings are restored.
	// v.Log.Debugf("Resetting vlan overrides for unused ports %v", resetMap)
	// err = penDvs.SetPvlanForPorts(resetMap)
	// if err != nil {
	// v.Log.Errorf("Resetting vlan overrides for unused ports failed, %s", err)
	// This error isn't worth failing the sync operation for
	// }

	// Handle new VMs
	// We make create calls for all of the VMs
	// Redundant creates will no-op

	for _, vm := range vms {
		m := v.convertWorkloadToEvent(dc.Self.Value, dc.Name, vm)
		v.handleVM(m)
	}
}

func (v *VCHub) extractOverrides(ports []types.DistributedVirtualPort) map[string]int {
	overrides := map[string]int{}
	for _, port := range ports {
		portKey := port.Key
		portSetting, ok := port.Config.Setting.(*types.VMwareDVSPortSetting)
		if !ok {
			v.Log.Debugf("skipping port %s since type is not VmwareDVSPortSetting, %+v", port.Key, port.Config.Setting)
			// skipping
			continue
		}
		vlanSpec, ok := portSetting.Vlan.(*types.VmwareDistributedVirtualSwitchVlanIdSpec)
		if !ok {
			// Skipping
			v.Log.Debugf("skipping port %s since portSetting vlan is not VmwareDistributedVirtualSwitchVlanIdSpec, %+v", port.Key, portSetting.Vlan)
			continue
		}
		overrides[portKey] = int(vlanSpec.VlanId)
	}
	return overrides
}
