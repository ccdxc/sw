package vchub

import (
	"fmt"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
)

const networkKind = "Network"

func (v *VCHub) handleNetworkEvent(evtType kvstore.WatchEventType, nw *network.Network) {
	v.Log.Infof("Handling network event nw %v", nw)
	v.syncLock.RLock()
	defer v.syncLock.RUnlock()
	// TODO: check res version to prevent double ops

	switch evtType {
	case kvstore.Created, kvstore.Deleted:
		if evtType == kvstore.Created && len(nw.Spec.Orchestrators) == 0 {
			return
		}
		dcs := []string{}
		for _, orch := range nw.Spec.Orchestrators {
			if orch.Name == v.OrchConfig.GetName() {
				dcs = append(dcs, orch.Namespace)
			}
		}
		v.Log.Infof("evt %s network %s event for dcs %v", evtType, nw.Name, dcs)
		for _, dc := range dcs {
			v.DcMapLock.Lock()
			penDC, ok := v.DcMap[dc]
			v.DcMapLock.Unlock()
			if !ok {
				continue
			}
			pgName := CreatePGName(nw.Name)
			if evtType == kvstore.Created {
				penDC.AddPG(pgName, nw.ObjectMeta, "")
			} else {
				// err is already logged inside function
				remainingAllocs, _ := penDC.RemovePG(pgName, "")
				// if we just deleted a workload check if we just went below capacity
				for _, count := range remainingAllocs {
					if count == useg.MaxPGCount-1 {
						// Need to recheck networks now that we have space for new networks
						v.checkNetworks(dc)
					}
				}
			}
		}
	case kvstore.Updated:
		v.Log.Info("Update network event")
		pgName := CreatePGName(nw.Name)
		dcs := map[string]bool{}
		for _, orch := range nw.Spec.Orchestrators {
			if orch.Name == v.OrchConfig.GetName() {
				dcs[orch.Namespace] = true
			}
		}

		v.DcMapLock.Lock()
		for _, dc := range v.DcMap {
			if dcs[dc.Name] {
				// Should exist/create
				dc.AddPG(pgName, nw.ObjectMeta, "")
			} else {
				// Check if we need to delete
				if dc.GetPG(pgName, "") != nil {
					dc.RemovePG(pgName, "")
				}
			}
		}
		v.DcMapLock.Unlock()
		// TODO: Update vcenter vlan tags
	}
}

func (v *VCHub) handlePG(m defs.VCEventMsg) {
	v.Log.Infof("Got handle PG event for PG %s in DC %s", m.Key, m.DcName)
	// If non-pensando PG, check whether we need to reserve useg space for it
	// If it is pensando PG, verify pvlan config has not been modified

	penDC := v.GetDC(m.DcName)
	if penDC == nil {
		v.Log.Errorf("DC not found for %s", m.DcName)
		return
	}
	dvs := penDC.GetPenDVS(CreateDVSName(m.DcName))
	if dvs == nil {
		v.Log.Errorf("DVS state for DC %s was nil", m.DcName)
	}

	if m.UpdateType == types.ObjectUpdateKindLeave {
		// Object was deleted
		penPG := penDC.GetPGByID(m.Key)
		if penPG == nil {
			// TODO: Check if we have any vlans stored for it if it is non-pensando.
			return
		}

		err := dvs.AddPenPG(penPG.PgName, penPG.NetworkMeta)
		if err != nil {
			v.Log.Errorf("Failed to set vlan config for PG %s, %s", penPG.PgName, err)
		}
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
	if pgConfig.DistributedVirtualSwitch.Reference().Value != dvs.DvsRef.Value {
		// Not for pensando DVS
		v.Log.Debugf("Skipping PG event as its not attached to a PenDVS")
		return
	}

	// Check name change
	penPG := dvs.pgIDMap[m.Key]
	if penPG != nil && penPG.PgName != pgConfig.Name {
		// TODO: Raise event
		evtMsg := fmt.Sprintf("User renamed a Pensando created Port Group. Port group name has been changed back.")
		recorder.Event(eventtypes.ORCH_INVALID_ACTION, evtMsg, v.State.OrchConfig)
		// Put object name back
		err := v.probe.RenamePG(m.DcName, pgConfig.Name, penPG.PgName, defaultRetryCount)
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

// checks if we need to create PGs for the given network
func (v *VCHub) checkNetworks(dcName string) {
	// Check if we have any networks for this new DC
	opts := api.ListWatchOptions{}
	networks, err := v.StateMgr.Controller().Network().List(v.Ctx, &opts)
	if err != nil {
		v.Log.Errorf("Failed to get network list. Err : %v", err)
	}
	for _, nw := range networks {
		v.Log.Debugf("Checking nw %s", nw.Network.Name)
		for _, orch := range nw.Network.Spec.Orchestrators {
			if orch.Name == v.VcID && orch.Namespace == dcName {
				penDC := v.GetDC(dcName)
				pgName := CreatePGName(nw.Network.Name)
				err := penDC.AddPG(pgName, nw.Network.ObjectMeta, "")
				v.Log.Infof("Create Pen PG %s returned %s", pgName, err)
			} else {
				v.Log.Debugf("vcID %s  dcName %s does not match orch-spec %s - %s",
					v.VcID, dcName, orch.Name, orch.Namespace)
			}
		}
	}
}
