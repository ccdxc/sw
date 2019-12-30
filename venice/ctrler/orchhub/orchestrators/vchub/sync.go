package vchub

import (
	"context"
	"time"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
)

func (v *VCHub) sync() {
	defer v.Wg.Done()
	v.Log.Infof("VCHub %v synching.", v)
	// Check that probe session is connected
	count := 3
	for !v.probe.SessionReady && count > 0 {
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
		2. Per DC, list DVS, list VMS
		3. Per DVS,
				- list PGs
				- Build Useg Mgr
				- assign PG VLans and workload vlans
	  4.
		4, Delete stale venice objects push oper to store
		5. Push existing objects and creates.
	*/
	dcs := v.probe.ListDC()
	for _, dc := range dcs {
		// TODO: Remove
		if v.forceDCname != "" && dc.Name != v.forceDCname {
			v.Log.Infof("Skipping DC %s", dc.Name)
			continue
		}

		penDC, err := v.NewPenDC(dc.Name)
		if err != nil {
			v.Log.Errorf("Creating DC %s failed: %s", dc.Name, err)
			continue
		}
		v.Log.Debugf("Created DC %s", dc.Name)
		dcRef := dc.Reference()
		dvsObjs := v.probe.ListDVS(&dcRef)

		// SYNCING DVS AND PG
		for _, dvs := range dvsObjs {
			if dvs.Name != defs.DefaultDVSName {
				v.Log.Debugf("Skipping dvs %s", dvs.Name)
				continue
			}
			v.Log.Debugf("Processing dvs %s", dvs.Name)

			penDVS := penDC.GetPenDVS(dvs.Name)

			pgs := v.probe.ListPG(&dcRef)
			pgMap := map[string]mo.DistributedVirtualPortgroup{}
			for _, pg := range pgs {
				pgMap[pg.Name] = pg
			}

			networkObjs, _ := v.StateMgr.Controller().Network().List(v.Ctx, &api.ListWatchOptions{})

			// For every PG, we mark the vlan it owns and create internal sate
			// For every stale PG, we then attempt to delete it and clear internal
			// state if it is successful
			// For New networks, we create PGs for them
			pgNameMap := map[string]api.ObjectMeta{}
			for _, nw := range networkObjs {
				for _, orch := range nw.Network.Spec.Orchestrators {
					if orch.Name == v.VcID && orch.Namespace == dc.Name {
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

			for _, nw := range networkObjs {
				for _, orch := range nw.Network.Spec.Orchestrators {
					if orch.Name == v.VcID && orch.Namespace == dc.Name {
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

			for _, pg := range pgMap {
				err := penDVS.RemovePenPG(pg.Name)
				v.Log.Infof("Delete Pen PG %s returned %s", pg.Name, err)
			}
		}

		// TODO: don't assume all vms in the DC are for us
		// vms := v.ListVM(dc.Reference())
		// // TODO: check useg assignments

		// // Delete stale venice objects
		// // Workloads
		// // v.StateMgr.Controller().Workload().List()

		// workloads, err := v.stateMgr.Controller().Workload().List(v.ctx, nil)
		// if err != nil {
		// 	return err
		// }
		// workloadNames := []string{}
		// for _, workload := range workloads {
		// 	workloadNames = append(workloadNames, workload.Name)
		// }
		// vmNames := []string{}
		// for _, vm := range vms {
		// 	key := vm.Self.Value
		// 	vmName := utils.CreateGlobalKey(v.VcID, key)
		// 	vmNames = append(vmNames, vmName)
		// }

		// deletes, exists, creates := v.FindChangeSet(workloadNames, vmNames)
	}
	v.Log.Infof("Sync done for VCHub. %v", v)
}
