package vchub

import (
	"fmt"
	"sync"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/utils/events/recorder"
)

// PenDVS represents an instance of a Distributed Virtual Switch
type PenDVS struct {
	sync.Mutex
	*defs.State
	DcName  string
	DvsName string
	DvsRef  types.ManagedObjectReference
	// Map from PG display name to PenPG
	Pgs map[string]*PenPG
	// Map from PG ID to PenPG
	pgIDMap map[string]*PenPG
	UsegMgr useg.Inf
	probe   vcprobe.ProbeInf
}

// PenDVSPortSettings represents a group of DVS port settings
// The key of this map represents the key of port on a DVS
// The value of this map represents the new setting of this port
type PenDVSPortSettings map[string]*types.VMwareDVSPortSetting

// Only handles config spec being all creates, (not modifies or deletes)
func (d *PenDC) dvsConfigDiff(spec *types.DVSCreateSpec, dvs *mo.DistributedVirtualSwitch) *types.DVSCreateSpec {
	config, ok := dvs.Config.(*types.VMwareDVSConfigInfo)
	if !ok {
		d.Log.Infof("ConfigCheck: dvs.Config was of type %T", dvs.Config)
		return spec
	}

	configSpec, ok := spec.ConfigSpec.(*types.VMwareDVSConfigSpec)
	if !ok {
		d.Log.Infof("ConfigCheck: spec.ConfigSpec was of type %T", spec.ConfigSpec)
		return spec
	}
	pvlanItems := map[string]types.VMwareDVSPvlanMapEntry{}
	for _, item := range configSpec.PvlanConfigSpec {
		key := fmt.Sprintf("%d-%d", item.PvlanEntry.PrimaryVlanId, item.PvlanEntry.SecondaryVlanId)
		pvlanItems[key] = item.PvlanEntry
	}

	pvlanConfigSpecArray := []types.VMwareDVSPvlanConfigSpec{}

	for _, item := range config.PvlanConfig {
		key := fmt.Sprintf("%d-%d", item.PrimaryVlanId, item.SecondaryVlanId)
		entry, ok := pvlanItems[key]
		if !ok {
			// extra config - delete
			pvlanSpec := types.VMwareDVSPvlanConfigSpec{
				PvlanEntry: item,
				Operation:  "remove",
			}
			pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpec)
			continue
		}
		delete(pvlanItems, key)
		if item.PvlanType != entry.PvlanType {
			// config modified, edit back
			pvlanSpecProm := types.VMwareDVSPvlanConfigSpec{
				PvlanEntry: entry,
				Operation:  "edit",
			}
			pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpecProm)
		}
	}

	// Add remaining items
	for _, entry := range pvlanItems {
		pvlanSpec := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: entry,
			Operation:  "add",
		}
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpec)
	}
	if len(pvlanConfigSpecArray) == 0 {
		d.Log.Infof("existing DVS matches config")
		return nil
	}
	newSpec := &types.DVSCreateSpec{
		ConfigSpec: &types.VMwareDVSConfigSpec{
			PvlanConfigSpec: pvlanConfigSpecArray,
		},
	}
	return newSpec
}

// CreateDefaultDVSSpec returns the default create spec for a Pen DVS
func (d *PenDC) CreateDefaultDVSSpec() *types.DVSCreateSpec {
	dvsName := CreateDVSName(d.Name)
	// Create a pen dvs
	// Pvlan allocations on the dvs
	pvlanConfigSpecArray := []types.VMwareDVSPvlanConfigSpec{}

	// Setup all the pvlan allocations now
	// vlans 0 and 1 are reserved
	for i := useg.FirstPGVlan; i < useg.FirstUsegVlan; i += 2 {
		PvlanEntryProm := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "promiscuous",
			SecondaryVlanId: int32(i),
		}
		pvlanMapEntry := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "isolated",
			SecondaryVlanId: int32(i + 1),
		}
		pvlanSpecProm := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: PvlanEntryProm,
			Operation:  "add",
		}
		pvlanSpec := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: pvlanMapEntry,
			Operation:  "add",
		}
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpecProm)
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpec)
	}

	// TODO: Set number of uplinks
	var spec types.DVSCreateSpec
	spec.ConfigSpec = &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: pvlanConfigSpecArray,
	}
	spec.ConfigSpec.GetDVSConfigSpec().Name = dvsName
	spec.ProductInfo = new(types.DistributedVirtualSwitchProductSpec)
	spec.ProductInfo.Version = "6.5.0"
	return &spec
}

// AddPenDVS adds a new PenDVS to the given vcprobe instance
func (d *PenDC) AddPenDVS() error {
	d.Lock()
	defer d.Unlock()

	dvsName := CreateDVSName(d.Name)
	dcName := d.Name
	err := d.probe.AddPenDVS(dcName, d.CreateDefaultDVSSpec(), d.dvsConfigDiff, defaultRetryCount)
	if err != nil {
		d.Log.Errorf("Failed to create %s in DC %s: %s", dvsName, dcName, err)
		return err
	}

	dvs, err := d.probe.GetPenDVS(dcName, dvsName, defaultRetryCount)
	if err != nil {
		return err
	}

	penDVS := d.getPenDVS(dvsName)
	if penDVS == nil {
		useg, err := useg.NewUsegAllocator()
		if err != nil {
			d.Log.Errorf("Creating useg mgr for DC %s - penDVS %s failed, %v", dcName, dvsName, err)
			return err
		}
		penDVS = &PenDVS{
			State:   d.State,
			probe:   d.probe,
			DcName:  dcName,
			DvsName: dvsName,
			UsegMgr: useg,
			Pgs:     map[string]*PenPG{},
			pgIDMap: map[string]*PenPG{},
		}

		d.DvsMap[dvsName] = penDVS
	}

	penDVS.DvsRef = dvs.Reference()

	d.State.DvsMapLock.Lock()
	d.State.DvsIDMap[dvsName] = dvs.Reference()
	d.State.DvsMapLock.Unlock()

	err = d.probe.TagObjAsManaged(dvs.Reference())
	if err != nil {
		d.Log.Errorf("Failed to tag DVS as managed, %s", err)
		// Error isn't worth failing the operation for
	}

	return nil
}

// GetPenDVS returns the PenDVS with the given name
func (d *PenDC) GetPenDVS(dvsName string) *PenDVS {
	d.Lock()
	defer d.Unlock()
	return d.getPenDVS(dvsName)
}

func (d *PenDC) getPenDVS(dvsName string) *PenDVS {
	dvs, ok := d.DvsMap[dvsName]
	if !ok {
		return nil
	}
	return dvs
}

// GetPortSettings returns the port settings of the dvs
func (d *PenDVS) GetPortSettings() ([]types.DistributedVirtualPort, error) {
	return d.probe.GetPenDVSPorts(d.DcName, d.DvsName, &types.DistributedVirtualSwitchPortCriteria{}, 1)
}

// SetVlanOverride overrides the port settings with the given vlan
func (d *PenDVS) SetVlanOverride(port string, vlan int) error {
	// Get lock to prevent two different threads
	// configuring the dvs at the same time.
	d.Lock()
	defer d.Unlock()
	d.Log.Debugf("SetVlanOverride called with port: %v vlan:%v", port, vlan)
	ports := vcprobe.PenDVSPortSettings{
		port: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(vlan),
		},
	}
	err := d.probe.UpdateDVSPortsVlan(d.DcName, d.DvsName, ports, defaultRetryCount)
	if err != nil {
		d.Log.Errorf("Failed to set vlan override for DC %s - dvs %s, err %s", d.DcName, d.DvsName, err)

		evtMsg := fmt.Sprintf("Failed to set vlan override in DC %s. Traffic may be impacted.", d.DcName)
		recorder.Event(eventtypes.ORCH_CONFIG_PUSH_FAILURE, evtMsg, d.State.OrchConfig)
		return err
	}
	return nil
}

// Resetting vlan overrides is not needed. As soon as port is disconnected,
// the pvlan settings are restored.
//
// SetPvlanForPorts undoes the vlan override and restores the given ports with the pvlan of the given pg
// Input is a map from pg name to ports to set
// func (d *PenDVS) SetPvlanForPorts(pgMap map[string][]string) error {
// 	d.Log.Debugf("SetPvlanForPorts called with %v", pgMap)
// 	portSetting := vcprobe.PenDVSPortSettings{}
// 	for pg, ports := range pgMap {
// 		_, vlan2, err := d.UsegMgr.GetVlansForPG(pg)
// 		if err != nil {
// 			// Don't have assignments for this PG.
// 			d.Log.Infof("PG %s had no vlans, err %s", pg, err)
// 			continue
// 		}
// 		for _, p := range ports {
// 			portSetting[p] = &types.VmwareDistributedVirtualSwitchPvlanSpec{
// 				PvlanId: int32(vlan2),
// 			}
// 		}
// 	}
// 	err := d.probe.UpdateDVSPortsVlan(d.DcName, d.DvsName, portSetting, 1)
// 	if err != nil {
// 		d.Log.Errorf("Failed to set pvlans back for DC %s - dvs %s, err %s", d.DcName, d.DvsName, err)
// 		return err
// 	}
// 	return nil
// }

func (v *VCHub) handleDVS(m defs.VCEventMsg) {
	v.Log.Infof("Got handle DVS event for DVS %s in DC %s", m.Key, m.DcName)
	// If non-pensando DVS, do nothing
	// If it is pensando PG, pvlan config and dvs name are untouched
	// check name change in same loop

	penDC := v.GetDC(m.DcName)
	if penDC == nil {
		v.Log.Errorf("DC not found for %s", m.DcName)
		return
	}
	dvsName := CreateDVSName(m.DcName)
	dvs := penDC.GetPenDVS(dvsName)
	if dvs == nil {
		v.Log.Errorf("DVS state for DC %s was nil", m.DcName)
	}

	if m.Key != dvs.DvsRef.Value {
		// not pensando DVS
		v.Log.Debugf("Change to a non pensando DVS, ignore")
		return
	}

	if m.UpdateType == types.ObjectUpdateKindLeave {
		// Object was deleted, recreate
		// Remove ID first
		v.State.DvsMapLock.Lock()
		if _, ok := v.State.DvsIDMap[dvsName]; ok {
			delete(v.State.DvsIDMap, dvsName)
		}
		v.State.DvsMapLock.Unlock()

		err := penDC.AddPenDVS()
		if err != nil {
			v.Log.Errorf("Failed to recreate DVS for DC %s, %s", m.DcName, err)
			// Generate event
			evtMsg := fmt.Sprintf("Failed to recreate DVS in Datacenter %s. Network configuration cannot be pushed.", m.DcName)
			recorder.Event(eventtypes.ORCH_CONFIG_PUSH_FAILURE, evtMsg, v.State.OrchConfig)
		} else {
			// Recreate PGs
			v.checkNetworks(m.DcName)
		}
		return
	}

	// extract config
	if len(m.Changes) == 0 {
		v.Log.Errorf("Received dvs event with no changes")
		return
	}

	var dvsConfig *types.VMwareDVSConfigInfo
	var name string

	for _, prop := range m.Changes {
		switch val := prop.Val.(type) {
		case types.VMwareDVSConfigInfo:
			dvsConfig = &val
		case string:
			name = val
		default:
			v.Log.Errorf("Got unexpected type in dvs changes, got %T", val)
		}
	}

	// Check if we need to rename
	if len(name) != 0 && dvs.DvsName != name {
		// DVS renamed
		evtMsg := fmt.Sprintf("User renamed a Pensando created DVS. Name has been changed back.")
		recorder.Event(eventtypes.ORCH_INVALID_ACTION, evtMsg, v.State.OrchConfig)
		// Put object name back
		err := v.probe.RenameDVS(m.DcName, name, dvs.DvsName, defaultRetryCount)
		if err != nil {
			v.Log.Errorf("Failed to rename DVS, %s", err)
		}
	}

	if dvsConfig == nil {
		v.Log.Debugf("No DVS config change")
		return
	}

	spec := penDC.CreateDefaultDVSSpec()

	dvsMo := &mo.DistributedVirtualSwitch{
		Config: dvsConfig,
	}

	// Check config
	diff := penDC.dvsConfigDiff(spec, dvsMo)
	if diff == nil {
		v.Log.Debugf("Config is equal, nothing to write back")
		return
	}
	v.Log.Infof("DVS %s change detected, writing back", dvs.DvsName)
	err := penDC.AddPenDVS()
	if err != nil {
		v.Log.Errorf("Failed to write DVS %s config back, err %s ", dvs.DvsName, err)
	}
	evtMsg := fmt.Sprintf("User modified vlan settings for a Pensando created DVS. DVS settings have been changed back.")
	recorder.Event(eventtypes.ORCH_INVALID_ACTION, evtMsg, v.State.OrchConfig)
}
