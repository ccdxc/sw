package vchub

import (
	"fmt"
	"sync"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/utils/events/recorder"
)

// PenPG represents an instance of a port group on a DVS
type PenPG struct {
	*defs.State
	DcName      string
	probe       vcprobe.ProbeInf
	PgName      string
	PgRef       types.ManagedObjectReference
	NetworkMeta api.ObjectMeta
	PgMutex     sync.Mutex
}

// AddPenPG creates a PG
func (d *PenDVS) AddPenPG(pgName string, networkMeta api.ObjectMeta) error {
	primaryVlan, secondaryVlan, err := d.UsegMgr.GetVlansForPG(pgName)
	if err != nil {
		{
			primaryVlan, secondaryVlan, err = d.UsegMgr.AssignVlansForPG(pgName)
			if err != nil {
				d.Log.Errorf("Failed to assign vlans for PG")
				if err.Error() == "PG limit reached" {
					// Generate error
					evtMsg := fmt.Sprintf("Failed to create Network %s in Datacenter %s. Max network limit reached.", networkMeta.Name, d.DcName)
					d.Log.Errorf(evtMsg)

					recorder.Event(eventtypes.ORCH_CONFIG_PUSH_FAILURE, evtMsg, d.State.OrchConfig)
				}
				return err
			}
		}
	}
	d.Log.Debugf("Adding PG %s with pvlan of %d and %d", pgName, primaryVlan, secondaryVlan)
	err = d.AddPenPGWithVlan(pgName, networkMeta, primaryVlan, secondaryVlan)
	if err != nil {
		evtMsg := fmt.Sprintf("Failed to set configuration for network %s in Datacenter %s", networkMeta.Name, d.DcName)
		d.Log.Errorf("%s, err %s", evtMsg, err)
		recorder.Event(eventtypes.ORCH_CONFIG_PUSH_FAILURE, evtMsg, d.State.OrchConfig)
	}
	return err
}

func (d *PenDVS) createPGConfigCheck(pvlan int) vcprobe.IsPGConfigEqual {
	return func(spec *types.DVPortgroupConfigSpec, config *mo.DistributedVirtualPortgroup) bool {

		policy, ok := config.Config.Policy.(*types.VMwareDVSPortgroupPolicy)
		if !ok {
			d.Log.Infof("ConfigCheck: dvs.Config was of type %T", config.Config.Policy)
			return false
		}
		if !policy.VlanOverrideAllowed ||
			!policy.PortConfigResetAtDisconnect {
			d.Log.Infof("ConfigCheck: dvs policy settings were incorrect")
			return false
		}
		portConfig, ok := config.Config.DefaultPortConfig.(*types.VMwareDVSPortSetting)
		if !ok {
			d.Log.Infof("ConfigCheck: portConfig was of type %T", config.Config.DefaultPortConfig)
			return false
		}
		pvlanConfig, ok := portConfig.Vlan.(*types.VmwareDistributedVirtualSwitchPvlanSpec)
		if !ok {
			d.Log.Infof("ConfigCheck: pvlanConfig was of type %T", portConfig.Vlan)
			return false
		}
		if pvlanConfig.PvlanId != int32(pvlan) {
			d.Log.Infof("ConfigCheck: pvlan did not match")
			return false
		}
		return true
	}
}

// AddPenPGWithVlan creates a PG with the given pvlan values
func (d *PenDVS) AddPenPGWithVlan(pgName string, networkMeta api.ObjectMeta, primaryVlan, secondaryVlan int) error {
	d.Lock()
	defer d.Unlock()

	spec := types.DVPortgroupConfigSpec{
		Name: pgName,
		Type: string(types.DistributedVirtualPortgroupPortgroupTypeEarlyBinding),
		Policy: &types.VMwareDVSPortgroupPolicy{
			DVPortgroupPolicy: types.DVPortgroupPolicy{
				BlockOverrideAllowed:               true,
				ShapingOverrideAllowed:             false,
				VendorConfigOverrideAllowed:        false,
				LivePortMovingAllowed:              false,
				PortConfigResetAtDisconnect:        true,
				NetworkResourcePoolOverrideAllowed: types.NewBool(false),
				TrafficFilterOverrideAllowed:       types.NewBool(false),
			},
			VlanOverrideAllowed:           true,
			UplinkTeamingOverrideAllowed:  false,
			SecurityPolicyOverrideAllowed: false,
			IpfixOverrideAllowed:          types.NewBool(false),
		},
		DefaultPortConfig: &types.VMwareDVSPortSetting{
			Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
				PvlanId: int32(secondaryVlan),
			},
		},
	}
	moPG, err := d.probe.GetPGConfig(d.DcName, pgName, []string{"config"}, 1)

	if err == nil {
		policySpec, ok := moPG.Config.Policy.(*types.VMwareDVSPortgroupPolicy)
		if ok {
			policySpec.VlanOverrideAllowed = true
			policySpec.PortConfigResetAtDisconnect = true
			spec.Policy = policySpec
		}
	}

	err = d.probe.AddPenPG(d.DcName, d.DvsName, &spec, d.createPGConfigCheck(secondaryVlan), defaultRetryCount)
	if err != nil {
		return err
	}

	pg, err := d.probe.GetPenPG(d.DcName, pgName, defaultRetryCount)
	if err != nil {
		return err
	}

	penPG := d.getPenPG(pgName)
	if penPG == nil {
		penPG = &PenPG{
			State:  d.State,
			probe:  d.probe,
			DcName: d.DcName,
			PgName: pgName,
		}
		d.Pgs[pgName] = penPG
	}
	penPG.PgRef = pg.Reference()
	d.pgIDMap[pg.Reference().Value] = penPG

	penPG.NetworkMeta = networkMeta

	err = d.probe.TagObjAsManaged(penPG.PgRef)
	if err != nil {
		d.Log.Errorf("Failed to tag PG %s as managed, %s", pgName, err)
		// Error isn't worth failing the operation for
	}

	nw, err := d.StateMgr.Controller().Network().Find(&networkMeta)
	if err == nil {
		externalVlan := int(nw.Spec.VlanID)
		err = d.probe.TagObjWithVlan(penPG.PgRef, externalVlan)
		if err != nil {
			d.Log.Errorf("Failed to tag PG %s with vlan tag, %s", pgName, err)
			// Error isn't worth failing the operation for
		}
	} else {
		d.Log.Errorf("Couldn't tag PG %s with vlan tag since we couldn't find the network info: networkMeta %v, err %s", pgName, networkMeta, err)
		// Error isn't worth failing the operation for
	}

	return nil
}

// GetPenPG returns the PenPG with the given name
func (d *PenDVS) GetPenPG(pgName string) *PenPG {
	d.Lock()
	defer d.Unlock()
	return d.getPenPG(pgName)
}

func (d *PenDVS) getPenPG(pgName string) *PenPG {
	pg, ok := d.Pgs[pgName]
	if !ok {
		return nil
	}
	return pg
}

// GetPenPGByID fetches the pen PG object by ID
func (d *PenDVS) GetPenPGByID(pgID string) *PenPG {
	d.Lock()
	defer d.Unlock()
	pg, ok := d.pgIDMap[pgID]
	if !ok {
		return nil
	}
	return pg
}

// RemovePenPG removes the pg from the dvs
func (d *PenDVS) RemovePenPG(pgName string) error {
	d.Lock()
	defer d.Unlock()

	var ref types.ManagedObjectReference
	if penPG, ok := d.Pgs[pgName]; ok {
		ref = penPG.PgRef
		id := penPG.PgRef.Value
		delete(d.Pgs, pgName)

		if _, ok := d.pgIDMap[id]; ok {
			delete(d.pgIDMap, id)
		} else {
			d.Log.Errorf("Removed entry in PG map that wasn't in pgIDMap, pgName %s", pgName)
		}
	}

	err := d.probe.RemovePenPG(d.DcName, pgName, defaultRetryCount)
	if err != nil {
		d.Log.Errorf("Failed to delete PG %s, removing management tag. Err %s", pgName, err)
		tagErrs := d.probe.RemovePensandoTags(ref)
		if len(tagErrs) != 0 {
			d.Log.Errorf("Failed to remove tags, errs %v", tagErrs)
		}
		return err
	}

	return nil

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
			continue
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
	if penPG == nil {
		// Not pensando PG
		v.Log.Debugf("Not a pensando PG - %s", m.Key)
		return
	}

	if penPG.PgName != pgConfig.Name {
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
	// Pensando PG, reset config if changed
	_, secondary, err := dvs.UsegMgr.GetVlansForPG(pgConfig.Name)
	if err != nil {
		v.Log.Errorf("Failed to get assigned vlans for PG %s", err)
		return
	}

	pgMo := &mo.DistributedVirtualPortgroup{
		Config: *pgConfig,
	}
	equal := dvs.createPGConfigCheck(secondary)(nil, pgMo)
	if equal {
		v.Log.Debugf("Config is equal, nothing to write back")
		return
	}
	v.Log.Infof("PG %s change detected, writing back", pgConfig.Name)
	// Vlan spec is not what we expect, set it back
	err = dvs.AddPenPG(pgConfig.Name, penPG.NetworkMeta)
	if err != nil {
		v.Log.Errorf("Failed to set vlan config for PG %s, %s", pgConfig.Name, err)
	}
	evtMsg := fmt.Sprintf("User modified vlan settings for a Pensando created Port Group. Port group settings have been changed back.")
	recorder.Event(eventtypes.ORCH_INVALID_ACTION, evtMsg, v.State.OrchConfig)
}

func extractVlanConfig(pgConfig types.DVPortgroupConfigInfo) (types.BaseVmwareDistributedVirtualSwitchVlanSpec, error) {
	portConfig, ok := pgConfig.DefaultPortConfig.(*types.VMwareDVSPortSetting)
	if !ok {
		return nil, fmt.Errorf("ignoring PG %s as casting to VMwareDVSPortSetting failed %+v", pgConfig.Name, pgConfig.DefaultPortConfig)
	}
	return portConfig.Vlan, nil
}
