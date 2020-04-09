package vchub

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
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
				v.Log.Infof("no state for DC %s", dc)
				continue
			}
			pgName := CreatePGName(nw.Name)
			if evtType == kvstore.Created {
				v.Log.Infof("Adding PG %s in DC %s", pgName, dc)
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
