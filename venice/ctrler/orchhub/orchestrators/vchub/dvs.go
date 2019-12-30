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
	Pgs     map[string]*PenPG
	UsegMgr useg.Inf
	probe   *vcprobe.VCProbe
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

// Commenting out for now as this function isn't called by anyone
// RemovePenDVS removes PenDVS from the DC
// func (d *PenDC) RemovePenDVS(dvsName string) error {

// 	d.Lock()
// 	defer d.Unlock()

// 	if _, ok := d.DvsMap[dvsName]; ok {

// 		err := d.probe.RemovePenDVS(d.Name, dvsName)
// 		if err != nil {
// 			return err
// 		}

// 		delete(d.DvsMap, dvsName)
// 	} else {
// 		err := fmt.Errorf("Received remove dvs call for DVS we don't have state for: %s", dvsName)
// 		d.Log.Errorf("%s", err)
// 		return err
// 	}

// 	return nil
// }

/*
// TODO: Need to re-enable it.
// This test works fine with real VC. However, in vcsim environment,
// after adding portgroups to DVS, pg.PortKeys[i] (pg is mo.DistributedVirtualPortgroup)
// is not initialized properly. I already filed a ticket to VMware for future assistant.
// Since this is unit testing and can't expect my real VC stand up forever,
// we temporarily skip it. Once we hear from VMware or we figure out why this happens
// by ourselves, we can unlock it.

// UpdatePorts updates port(s) on a given DVS based on the input mapping(portsSetting)
func (d *PenDVS) UpdatePorts(portsSetting *PenDVSPortSettings) ([]types.DistributedVirtualPort, error) {
	numPorts := len(*portsSetting)

	portKeys := make([]string, numPorts)
	portSpecs := make([]types.DVPortConfigSpec, numPorts)

	for k := range *portsSetting {
		portKeys = append(portKeys, k)
	}

	criteria := types.DistributedVirtualSwitchPortCriteria{
		PortKey: portKeys,
	}

	d.DvsMutex.Lock()
	defer d.DvsMutex.Unlock()

	ports, err := d.ObjDvs.FetchDVPorts(d.ctx, &criteria)
	if err != nil {
		d.log.Errorf("Can't find ports, err: %s", err)
		return nil, err
	}

	for i := 0; i < numPorts; i++ {
		portSpecs[i].ConfigVersion = ports[i].Config.ConfigVersion
		portSpecs[i].Key = ports[i].Key
		portSpecs[i].Operation = string("edit")
		portSpecs[i].Scope = ports[i].Config.Scope
		portSpecs[i].Setting = (*portsSetting)[ports[i].Key]
	}

	task, err := d.ObjDvs.ReconfigureDVPort(d.ctx, portSpecs)
	if err != nil {
		d.log.Errorf("Failed at reconfig DVS ports, err: %s", err)
		return ports, err
	}

	_, err = task.WaitForResult(d.ctx, nil)
	if err != nil {
		d.log.Errorf("Failed at modifying DVS ports, err: %s", err)
		return ports, err
	}

	return ports, nil
}
*/

// getMoDVSRef converts object.DistributedVirtualSwitch to mo.DistributedVirtualSwitch
// func (d *PenDVS) getMoDVSRef() (*mo.DistributedVirtualSwitch, error) {
// 	d.DvsMutex.Lock()
// 	defer d.DvsMutex.Unlock()

// 	var dvs mo.DistributedVirtualSwitch
// 	err := d.ObjDvs.Properties(d.ctx, d.ObjDvs.Reference(), nil, &dvs)
// 	if err != nil {
// 		d.log.Errorf("Failed at getting DVS properties, err: %s", err)
// 		return nil, err
// 	}

// 	return &dvs, nil
// }
