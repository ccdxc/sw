package vchub

import (
	"fmt"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/kvstore"
)

func (v *VCHub) handleNetworkEvent(evtType kvstore.WatchEventType, nw *network.Network) {
	v.Log.Infof("Handling network event nw %v", nw)

	switch evtType {
	case kvstore.Created:
		if len(nw.Spec.Orchestrators) == 0 {
			return
		}
		dcs := []string{}
		for _, orch := range nw.Spec.Orchestrators {
			if orch.Name == v.OrchConfig.GetName() {
				dcs = append(dcs, orch.Namespace)
			}
		}
		v.Log.Infof("Create network %s event for dcs %v", nw.Name, dcs)
		for _, dc := range dcs {
			v.DcMapLock.Lock()
			penDC, ok := v.DcMap[dc]
			v.DcMapLock.Unlock()
			if !ok {
				continue
			}
			pgName := createPGName(nw.Name)
			penDC.AddPG(pgName, nw.ObjectMeta, "")
		}
	case kvstore.Updated:
		// If wire vlan changes, workloads should be modified
		// TODO: update workloads
		// TODO: Update vcenter vlan tags
		v.Log.Info("Update network event")
	case kvstore.Deleted:
		if len(nw.Spec.Orchestrators) == 0 {
			return
		}
		dcs := []string{}
		for _, orch := range nw.Spec.Orchestrators {
			if orch.Name == v.OrchConfig.GetName() {
				dcs = append(dcs, orch.Namespace)
			}
		}
		v.Log.Infof("Delete network %s event for dcs %v", nw.Name, dcs)
		for _, dc := range dcs {
			v.DcMapLock.Lock()
			penDC, ok := v.DcMap[dc]
			v.DcMapLock.Unlock()
			if !ok {
				continue
			}
			pgName := createPGName(nw.Name)
			penDC.RemovePG(pgName, "")
		}
	}
}

func (v *VCHub) handlePG(m defs.VCEventMsg) {
	v.Log.Infof("Got handle PG event for PG %s in DC %s", m.Key, m.DcName)
	// If non-pensando PG, check whether we need to reserve useg space for it
	// If it is pensando PG, verify pvlan config has not been modified

	if m.UpdateType == types.ObjectUpdateKindLeave {
		// Object was deleted
		// TODO: Check if we have any vlans stored for it if it is non-pensando.
		return
	}

	// extract config
	if len(m.Changes) == 0 {
		v.Log.Errorf("Received pg event with no changes")
		return
	}

	var pgConfig *types.DVPortgroupConfigInfo

	for _, prop := range m.Changes {
		config, ok := prop.Val.(types.DVPortgroupConfigInfo)
		if !ok {
			v.Log.Errorf("Expected prop to be of type DVPortgroupConfigInfo, got %T", config)
		}
		pgConfig = &config
	}

	if pgConfig == nil || pgConfig.DistributedVirtualSwitch == nil {
		v.Log.Errorf("Insufficient PG config %p", pgConfig)
		return
	}

	// Check if it is for our DVS
	penDC := v.GetDC(m.DcName)
	if penDC == nil {
		v.Log.Errorf("DC not found for %s", m.DcName)
		return
	}
	dvs := penDC.GetPenDVS(createDVSName(m.DcName))
	if pgConfig.DistributedVirtualSwitch.Reference().Value != dvs.DvsRef.Value {
		// Not for pensando DVS
		return
	}

	// Check name change
	penPG := dvs.pgIDMap[m.Key]
	if penPG != nil && penPG.PgName != pgConfig.Name {
		// TODO: Raise event
		// Put object name back
		err := v.probe.RenamePG(m.DcName, pgConfig.Name, penPG.PgName)
		if err != nil {
			v.Log.Errorf("Failed to rename PG, %s", err)
		}
		// Don't check vlan config now, name change will trigger another event
		return
	}

	// Check vlan config
	if penPG == nil {
		// Not pensando PG
		// TODO: reserve vlan	if in useg space
		v.Log.Infof("Not a pensando PG - %s", m.Key)
	} else {
		// Pensando PG, reset config if changed
		_, secondary, err := dvs.UsegMgr.GetVlansForPG(pgConfig.Name)
		if err != nil {
			v.Log.Errorf("Failed to get assigned vlans for PG %s", err)
			return
		}
		vlanConfig, err := extractVlanConfig(*pgConfig)
		if err != nil {
			v.Log.Errorf("Skipping reconfiguring PG %s, %s", pgConfig.Name, err)
			return
		}

		switch vlanSpec := vlanConfig.(type) {
		case *types.VmwareDistributedVirtualSwitchPvlanSpec:
			vlan := int(vlanSpec.PvlanId)
			if secondary == vlan {
				return // nothing to do
			}
		}
		// Vlan spec is not what we expect, set it back
		err = dvs.AddPenPG(pgConfig.Name, penPG.NetworkMeta)
		if err != nil {
			v.Log.Errorf("Failed to set vlan config for PG %s, %s", pgConfig.Name, err)
		}
	}
}

func extractVlanConfig(pgConfig types.DVPortgroupConfigInfo) (types.BaseVmwareDistributedVirtualSwitchVlanSpec, error) {
	portConfig, ok := pgConfig.DefaultPortConfig.(*types.VMwareDVSPortSetting)
	if !ok {
		return nil, fmt.Errorf("ignoring PG %s as casting to VMwareDVSPortSetting failed %+v", pgConfig.Name, pgConfig.DefaultPortConfig)
	}
	return portConfig.Vlan, nil
}
