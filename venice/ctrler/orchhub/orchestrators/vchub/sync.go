package vchub

import (
	"context"
	"time"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
)

func (v *VCHub) sync() {
	v.Log.Infof("VCHub %v synching.", v)
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

	opts := api.ListWatchOptions{}
	// By the time this is called, the statemanager would have setup watchers to the API server
	// and synched the local cache with the API server
	nw, err := v.StateMgr.Controller().Network().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get network list. Err : %v", err)
	}

	hosts, err := v.StateMgr.Controller().Host().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get host list. Err : %v", err)
	}

	workloads, err := v.StateMgr.Controller().Workload().List(context.Background(), &opts)
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
	dcs := v.probe.ListDC()
	for _, dc := range dcs {
		// TODO: Remove
		if v.ForceDCname != "" && dc.Name != v.ForceDCname {
			v.Log.Infof("Skipping DC %s", dc.Name)
			continue
		}

		_, err := v.NewPenDC(dc.Name)
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
		v.syncHosts(dc, vcHosts, hosts)
		v.syncVMs(workloads, dc, dvsObjs, vms, pgs)
		v.syncVmkNics(&dc, dvsObjs, vcHosts, pgs)
	}

	v.Log.Infof("Sync done for VCHub. %v", v)
}

func (v *VCHub) syncHosts(dc mo.Datacenter, vcHosts []mo.HostSystem, hosts []*ctkit.Host) {
	v.Log.Debug("Syncing hosts")
	vcHostMap := map[string]bool{}
	for _, vcHost := range vcHosts {
		hostName := createHostName(v.VcID, dc.Self.Value, vcHost.Self.Value)
		vcHostMap[hostName] = true
	}

	// Deleting stale hosts
	for _, host := range hosts {
		if !isObjForDC(host.Name, v.VcID, dc.Self.Value) {
			// Filter out hosts not for this Orch/DC
			v.Log.Debugf("Skipping host %s", host.Name)
			continue
		}
		if _, ok := vcHostMap[host.Name]; !ok {
			// host no longer exists
			// Delete vmkWorkload if created for this host
			wlName := createVmkWorkLoadNameFromHostName(host.Name)
			v.deleteWorkloadByName(dc.Self.Value, wlName)

			v.Log.Debugf("Deleting host %s", host.Name)
			v.deleteHost(&host.Host)
		}
	}

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
			},
		}
		v.Log.Debugf("Creating host %s", host.Name)
		v.handleHost(m)
	}
}

func (v *VCHub) syncNetwork(networks []*ctkit.Network, dc mo.Datacenter, dvsObjs []mo.VmwareDistributedVirtualSwitch, pgs []mo.DistributedVirtualPortgroup) {
	v.Log.Debug("Syncing networks")
	// SYNCING DVS AND PG
	dcName := dc.Name
	penDC := v.GetDC(dcName)
	for _, dvs := range dvsObjs {
		if !isPensandoDVS(dvs.Name) {
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

		// For every PG, we mark the vlan it owns and create internal sate
		// For every stale PG, we then attempt to delete it and clear internal
		// state if it is successful
		// For New networks, we create PGs for them
		pgNameMap := map[string]api.ObjectMeta{}
		for _, nw := range networks {
			for _, orch := range nw.Network.Spec.Orchestrators {
				if orch.Name == v.VcID && orch.Namespace == dcName {
					pgName := createPGName(nw.Network.Name)
					pgNameMap[pgName] = nw.Network.ObjectMeta
				}
			}
		}

		for _, pg := range pgMap {
			pgConfig := pg.Config
			portConfig, ok := pgConfig.DefaultPortConfig.(*types.VMwareDVSPortSetting)
			if !ok {
				v.Log.Errorf("ignoring PG %s as casting to VMwareDVSPortSetting failed %+v", pg.Name, pgConfig.DefaultPortConfig)
				continue
			}
			vlanSpec, ok := portConfig.Vlan.(*types.VmwareDistributedVirtualSwitchPvlanSpec)
			if !ok {
				v.Log.Errorf("ignoring PG %s as casting to VmwareDistributedVirtualSwitchPvlanSpec failed %+v", pg.Name, portConfig.Vlan)
				continue
			}

			secondaryPvlan := vlanSpec.PvlanId
			primaryPvlan := secondaryPvlan - 1

			pgName := pg.Name

			// TODO: validate user didnt change pvlan
			v.Log.Infof("setting PG vlans %s ", pg.Name)
			// TODO: If a PG has the vlan already, we currently crash
			err := penDVS.UsegMgr.SetVlansForPG(pgName, int(primaryPvlan), int(secondaryPvlan))
			if err != nil {
				// TODO: Reassign pg if it fails to assign
				v.Log.Infof("Setting vlans %d %d for PG %s failed: err %s", primaryPvlan, secondaryPvlan, pg.Name, err)
				continue
			}
			meta, ok := pgNameMap[pgName]
			if !ok {
				v.Log.Infof("No venice network info is available for this pg %s", pgName)
			}
			err = penDVS.AddPenPG(pgName, meta)
			v.Log.Infof("Create PG %s returned %s", pgName, err)
		}

		for _, nw := range networks {
			for _, orch := range nw.Network.Spec.Orchestrators {
				if orch.Name == v.VcID && orch.Namespace == dcName {
					pgName := createPGName(nw.Network.Name)

					_, ok := pgMap[pgName]
					if !ok {
						// exists on venice not on VC
						// TODO: this order potentially limits scale if there are a lot
						// of stale objects
						err := penDVS.AddPenPG(pgName, nw.Network.ObjectMeta)
						v.Log.Infof("Create Pen PG %s returned %s", pgName, err)
					}
					delete(pgMap, pgName)
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
				err := penDVS.RemovePenPG(pg.Name)
				v.Log.Infof("Delete PG %s returned %s", pg.Name, err)
			} else {
				v.Log.Infof("Skipped deletion of PG as it is the uplink pg", pg.Name)
			}
		}
	}
}

func (v *VCHub) syncVMs(workloads []*ctkit.Workload, dc mo.Datacenter, dvsObjs []mo.VmwareDistributedVirtualSwitch, vms []mo.VirtualMachine, pgs []mo.DistributedVirtualPortgroup) {
	v.Log.Debug("Syncing vms")
	dcName := dc.Name
	penDC := v.GetDC(dcName)
	penDvs := penDC.GetPenDVS(createDVSName(dcName))

	pgKeyToName := map[string]string{}
	for _, pg := range pgs {
		pgKeyToName[pg.Self.Value] = pg.Name
	}

	// Set assignments.
	vmMap := map[string]mo.VirtualMachine{}

	for _, vm := range vms {
		// TODO: check the vm is for us
		vmName := utils.CreateGlobalKey(v.VcID, dc.Self.Value, vm.Self.Value)
		vmMap[vmName] = vm
	}

	// Deleting stale workloads and build useg state
	for _, workload := range workloads {
		if !isObjForDC(workload.Name, v.VcID, dc.Self.Value) {
			// Filter out workloads not for this Orch/DC
			v.Log.Debugf("Skipping workload %s", workload.Name)
			continue
		}
		if _, ok := vmMap[workload.Name]; !ok {
			// workload no longer exists
			v.Log.Debugf("Deleting workload %s", workload.Name)
			v.deleteWorkload(&workload.Workload, dcName)
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
			v.Log.Infof("Skipping port %s - port setting %v", portKey, port.Config.Setting)
			continue
		}
		vlanSpec, ok := portSetting.Vlan.(*types.VmwareDistributedVirtualSwitchVlanIdSpec)
		if !ok {
			// Skipping
			v.Log.Infof("Skipping port %s - port setting vlan type %v", portKey, portSetting.Vlan)
			continue
		}
		if vlanSpec.VlanId == 0 {
			v.Log.Info("Vlan override for port %s is 0", portKey)
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
			host := utils.CreateGlobalKey(v.VcID, dc.Self.Value, vm.Runtime.Host.Value)
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
	v.Log.Debugf("Resetting vlan overrides for unused ports %v", resetMap)
	err = penDvs.SetPvlanForPorts(resetMap)
	if err != nil {
		v.Log.Errorf("Resetting vlan overrides for unused ports failed, %s", err)
		// This error isn't worth failing the sync operation for
	}

	// Handle new VMs
	// We make create calls for all of the VMs
	// Redundant creates will no-op

	for _, vm := range vms {
		m := defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			Key:        vm.Self.Value,
			DcID:       dc.Self.Value,
			DcName:     dc.Name,
			Originator: v.VcID,
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Name: string(defs.VMPropConfig),
					Op:   "add",
					Val:  *(vm.Config),
				},
				types.PropertyChange{
					Name: string(defs.VMPropName),
					Op:   "add",
					Val:  vm.Name,
				},
				types.PropertyChange{
					Name: string(defs.VMPropRT),
					Op:   "add",
					Val:  vm.Runtime,
				},
			},
		}
		v.handleWorkload(m)
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

func (v *VCHub) syncVmkNics(dc *mo.Datacenter, dvsObjs []mo.VmwareDistributedVirtualSwitch, vcHosts []mo.HostSystem, pgs []mo.DistributedVirtualPortgroup) {
	vcHostRefMap := make(map[types.ManagedObjectReference]mo.HostSystem)
	for _, host := range vcHosts {
		vcHostRefMap[host.Reference()] = host
	}
	v.Log.Infof("SyncVmkNics found %d hosts", len(vcHostRefMap))

	penDC := v.GetDC(dc.Name)
	for _, dvs := range dvsObjs {
		if !isPensandoDVS(dvs.Name) {
			v.Log.Debugf("Skipping dvs %s", dvs.Name)
			continue
		}
		for _, hostMember := range dvs.Config.GetDVSConfigInfo().Host {
			host, ok := vcHostRefMap[*(hostMember.Config.Host)]
			if !ok {
				// non-pensando host
				continue
			}
			vmkNics := []vnicStruct{}
			wlName := createVmkWorkLoadName(v.VcID, dc.Self.Value, host.Self.Value)
			for _, vmkNic := range host.Config.Network.Vnic {
				v.Log.Infof("Processing VmkNic %s on host %s", vmkNic.Key, host.Name)
				if vmkNic.Portgroup != "" {
					// standard (non-DV) port gorup is not supported
					v.Log.Infof("Skip vnic - not in DV Port group")
					continue
				}
				pgKey := vmkNic.Spec.DistributedVirtualPort.PortgroupKey
				portKey := vmkNic.Spec.DistributedVirtualPort.PortKey
				if !ok {
					// Error
					v.Log.Errorf("PG not found for %s", pgKey)
					continue
				}
				penPG := penDC.GetPGByID(pgKey)
				if penPG == nil {
					// not a venice network
					v.Log.Errorf("PenPG not found for PG Id %s", pgKey)
					continue
				}
				macStr := vmkNic.Spec.Mac
				vmkNics = append(vmkNics, vnicStruct{
					ObjectMeta: *createVNICMeta(macStr),
					PG:         pgKey,
					Port:       portKey,
					Workload:   wlName,
				})
				v.Log.Infof("vmkInterface %s Port %s", vmkNic.Device, portKey)
			}
			// reconcile vmknics - Add all new vmknics, remove stale
			// Create workload if does not exists and add EPs to it
			v.syncHostVmkNics(dc, &dvs, &host, vmkNics)
		}
	}
}
