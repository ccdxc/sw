package vchub

import (
	"sync"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
)

// PenDVS represents an instance of a Distributed Virtual Switch
type PenDVS struct {
	sync.Mutex
	*defs.State
	DcName  string
	DvsName string
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

// AddPenDVS adds a new PenDVS to the given vcprobe instance
func (d *PenDC) AddPenDVS(dvsCreateSpec *types.DVSCreateSpec) error {
	d.Lock()
	defer d.Unlock()
	dcName := d.Name
	dvsName := dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name

	if d.getPenDVS(dvsName) != nil {
		return nil
	}

	err := d.probe.AddPenDVS(dcName, dvsCreateSpec)
	if err != nil {
		d.Log.Errorf("Failed to create %s in DC %s: %s", dvsName, dcName, err)
		return err
	}

	useg, err := useg.NewUsegAllocator()
	if err != nil {
		d.Log.Errorf("Creating useg mgr for DC %s - penDVS %s failed, %v", dcName, dvsName, err)
		return err
	}
	penDVS := &PenDVS{
		State:   d.State,
		probe:   d.probe,
		DcName:  dcName,
		DvsName: dvsName,
		UsegMgr: useg,
		Pgs:     map[string]*PenPG{},
		pgIDMap: map[string]*PenPG{},
	}

	d.DvsMap[dvsName] = penDVS
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
	return d.probe.GetPenDVSPorts(d.DcName, d.DvsName, &types.DistributedVirtualSwitchPortCriteria{})
}

// SetVlanOverride overrides the port settings with the given vlan
func (d *PenDVS) SetVlanOverride(port string, vlan int) error {
	d.Log.Debugf("SetVlanOverride called with port: %v vlan:%v", port, vlan)
	ports := vcprobe.PenDVSPortSettings{
		port: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(vlan),
		},
	}
	err := d.probe.UpdateDVSPortsVlan(d.DcName, d.DvsName, ports)
	if err != nil {
		d.Log.Errorf("Failed to set vlan override for DC %s - dvs %s, err %s", d.DcName, d.DvsName, err)
		return err
	}
	return nil
}

// SetPvlanForPorts undoes the vlan override and restores the given ports with the pvlan of the given pg
// Input is a map from pg name to ports to set
func (d *PenDVS) SetPvlanForPorts(pgMap map[string][]string) error {
	d.Log.Debugf("SetPvlanForPorts called with %v", pgMap)
	portSetting := vcprobe.PenDVSPortSettings{}
	for pg, ports := range pgMap {
		_, vlan2, err := d.UsegMgr.GetVlansForPG(pg)
		if err != nil {
			// Don't have assignments for this PG.
			d.Log.Infof("PG %s had no vlans, err %s", pg, err)
			continue
		}
		for _, p := range ports {
			portSetting[p] = &types.VmwareDistributedVirtualSwitchPvlanSpec{
				PvlanId: int32(vlan2),
			}
		}
	}
	err := d.probe.UpdateDVSPortsVlan(d.DcName, d.DvsName, portSetting)
	if err != nil {
		d.Log.Errorf("Failed to set pvlans back for DC %s - dvs %s, err %s", d.DcName, d.DvsName, err)
		return err
	}
	return nil
}
