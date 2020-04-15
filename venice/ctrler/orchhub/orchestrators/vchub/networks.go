package vchub

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/utils/kvstore"
)

const networkKind = "Network"

func (v *VCHub) handleNetworkEvent(evtType kvstore.WatchEventType, nw *network.Network) {
	// Update calls might actually be create calls for us, since the active flag would not be set on object creation
	v.Log.Infof("Handling network event nw %v", nw)
	v.syncLock.RLock()
	defer v.syncLock.RUnlock()
	// TODO: check res version to prevent double ops

	pgName := CreatePGName(nw.Name)

	if evtType == kvstore.Created && len(nw.Spec.Orchestrators) == 0 {
		return
	}
	dcs := map[string]bool{}
	for _, orch := range nw.Spec.Orchestrators {
		if orch.Name == v.OrchConfig.GetName() {
			dcs[orch.Namespace] = true
		}
	}
	v.Log.Infof("evt %s network %s event for dcs %v", evtType, nw.Name, dcs)
	for _, penDC := range v.DcMap {
		if _, ok := dcs[penDC.Name]; ok {
			if evtType == kvstore.Created || evtType == kvstore.Updated {
				v.Log.Infof("Adding PG %s in DC %s", pgName, penDC.Name)
				// IF PG NOT in our local state, but does already exist in vCenter,
				// then we need to resync workloads after creating internal state
				resync := false
				if penDC.GetPG(pgName, "") == nil {
					_, err := v.probe.GetPenPG(penDC.Name, pgName, defaultRetryCount)
					if err == nil {
						resync = true
					}
				}
				errs := penDC.AddPG(pgName, nw.ObjectMeta, "")
				if len(errs) == 0 && resync {
					v.syncLock.RUnlock()
					v.fetchVMs(penDC)
					v.syncLock.RLock()
				}
			} else {
				// err is already logged inside function
				remainingAllocs, _ := penDC.RemovePG(pgName, "")
				// if we just deleted a workload check if we just went below capacity
				for _, count := range remainingAllocs {
					if count == useg.MaxPGCount-1 {
						// Need to recheck networks now that we have space for new networks
						v.checkNetworks(penDC.Name)
					}
				}
			}
		} else if evtType == kvstore.Updated {
			// Check if we need to delete
			if penDC.GetPG(pgName, "") != nil {
				penDC.RemovePG(pgName, "")
			}
		}
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
