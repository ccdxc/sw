package vchub

import (
	"sync"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
)

// PenPG represents an instance of a port group on a DVS
type PenPG struct {
	*defs.State
	probe *vcprobe.VCProbe
	// ObjPg       *object.DistributedVirtualPortgroup
	PgName      string
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
				return err
			}
		}
	}
	return d.AddPenPGWithVlan(pgName, networkMeta, primaryVlan, secondaryVlan)
}

// AddPenPGWithVlan creates a PG with the given pvlan values
func (d *PenDVS) AddPenPGWithVlan(pgName string, networkMeta api.ObjectMeta, primaryVlan, secondaryVlan int) error {
	d.Lock()
	defer d.Unlock()

	if d.getPenPG(pgName) != nil {
		return nil
	}

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

	err := d.probe.AddPenPG(d.DcName, d.DvsName, &spec)
	if err != nil {
		return err
	}

	penPG := &PenPG{
		State:       d.State,
		probe:       d.probe,
		PgName:      pgName,
		NetworkMeta: networkMeta,
	}

	d.Pgs[pgName] = penPG

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

// RemovePenPG removes the pg from the dvs
func (d *PenDVS) RemovePenPG(pgName string) error {
	d.Lock()
	defer d.Unlock()

	err := d.probe.RemovePenPG(d.DcName, pgName)
	if err != nil {
		return err
	}

	if _, ok := d.Pgs[pgName]; ok {
		delete(d.Pgs, pgName)
	}
	return nil

}
