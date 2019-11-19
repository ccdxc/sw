package vcprobe

import (
	"errors"
	"sync"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

// PenDVS represents an instance of a Distributed Virtual Switch
type PenDVS struct {
	vcInst
	folders  *object.DatacenterFolders
	ObjDvs   *object.DistributedVirtualSwitch
	DcName   string
	DvsName  string
	dvsPgs   []*PenPG
	DvsMutex sync.Mutex
}

// PenDVSPortSettings represents a group of DVS port settings
// The key of this map represents the key of port on a DVS
// The value of this map represents the new setting of this port
type PenDVSPortSettings map[string]*types.VMwareDVSPortSetting

// AddPenDVS adds a new PenDVS to the given vcprobe instance
func (v *VCProbe) AddPenDVS(dcName string, dvsCreateSpec *types.DVSCreateSpec) (*PenDVS, error) {
	ctx := v.ctx

	dc, err := v.finder.Datacenter(ctx, dcName)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		return nil, err
	}

	v.finder.SetDatacenter(dc)

	folders, err := dc.Folders(ctx)
	if err != nil {
		return nil, err
	}

	dvsName := dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name

	task, err := folders.NetworkFolder.CreateDVS(ctx, *dvsCreateSpec)
	if err != nil {
		v.Log.Errorf("Failed at creating dvs: %s, err: %s", dvsName, err)
		return nil, err
	}

	_, err = task.WaitForResult(ctx, nil)
	if err != nil {
		v.Log.Errorf("Failed at waiting results of creating dvs: %s, err: %s", dvsName, err)
		return nil, err
	}

	net, err := v.finder.Network(v.ctx, dvsName)
	if err != nil {
		v.Log.Errorf("Failed at finding network: %s, err: %s", dvsName, err)
		return nil, err
	}

	objDvs, ok := net.(*object.DistributedVirtualSwitch)
	if !ok {
		v.Log.Errorf("Failed at getting DVS object")
		return nil, errors.New("Failed at getting DVS object")
	}

	penDVS := &PenDVS{
		vcInst:  v.vcInst,
		folders: folders,
		ObjDvs:  objDvs,
		DcName:  dcName,
		DvsName: dvsName,
	}

	v.dvsMap[dvsName] = penDVS
	return penDVS, nil
}

// RemovePenDVS removes PenDVS from the given vcprobe
func (v *VCProbe) RemovePenDVS(name string) error {
	d := v.dvsMap[name]

	task, err := d.ObjDvs.Destroy(d.ctx)
	if err != nil {
		d.Log.Errorf("Failed at destroying DVS: %s from datacenter: %s, err: %s", d.DvsName, d.DcName, err)
		return err
	}

	err = task.Wait(d.ctx)
	if err != nil {
		d.Log.Errorf("Failed at destroying DVS: %s from datacenter: %s, err: %s", d.DvsName, d.DcName, err)
		return err
	}

	delete(v.dvsMap, name)
	return nil
}

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
		d.Log.Errorf("Can't find ports, err: %s", err)
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
		d.Log.Errorf("Failed at reconfig DVS ports, err: %s", err)
		return ports, err
	}

	_, err = task.WaitForResult(d.ctx, nil)
	if err != nil {
		d.Log.Errorf("Failed at modifying DVS ports, err: %s", err)
		return ports, err
	}

	return ports, nil
}
*/

// getMoDVSRef converts object.DistributedVirtualSwitch to mo.DistributedVirtualSwitch
func (d *PenDVS) getMoDVSRef() (*mo.DistributedVirtualSwitch, error) {
	d.DvsMutex.Lock()
	defer d.DvsMutex.Unlock()

	var dvs mo.DistributedVirtualSwitch
	err := d.ObjDvs.Properties(d.ctx, d.ObjDvs.Reference(), nil, &dvs)
	if err != nil {
		d.Log.Errorf("Failed at getting DVS properties, err: %s", err)
		return nil, err
	}

	return &dvs, nil
}
