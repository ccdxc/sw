package vcprobe

import (
	"errors"

	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"
)

// PenDVSPortSettings represents a group of DVS port settings
// The key of this map represents the key of port on a DVS
// The value of this map represents the new setting of this port
type PenDVSPortSettings map[string]types.BaseVmwareDistributedVirtualSwitchVlanSpec

// AddPenDVS adds a new PenDVS to the given vcprobe instance
func (v *VCProbe) AddPenDVS(dcName string, dvsCreateSpec *types.DVSCreateSpec) error {
	dvsName := dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

	// Check if it exists first
	if _, err := v.getPenDVS(dcName, dvsName, finder); err == nil {
		// PenDVS exists
		// TODO: check error isn't intermittent
		return nil
	}

	dc, err := finder.Datacenter(v.ClientCtx, dcName)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		return err
	}

	finder.SetDatacenter(dc)

	folders, err := dc.Folders(v.ClientCtx)
	if err != nil {
		return err
	}

	task, err := folders.NetworkFolder.CreateDVS(v.ClientCtx, *dvsCreateSpec)
	if err != nil {
		v.Log.Errorf("Failed at creating dvs: %s, err: %s", dvsName, err)
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		v.Log.Errorf("Failed at waiting results of creating dvs: %s, err: %s", dvsName, err)
		return err
	}

	return nil
}

// GetPenDVS returns the DVS if it exists or an error
func (v *VCProbe) GetPenDVS(dcName, dvsName string) (*object.DistributedVirtualSwitch, error) {
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()
	return v.getPenDVS(dcName, dvsName, finder)
}

func (v *VCProbe) getPenDVS(dcName, dvsName string, finder *find.Finder) (*object.DistributedVirtualSwitch, error) {
	dc, err := finder.Datacenter(v.ClientCtx, dcName)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		return nil, err
	}

	finder.SetDatacenter(dc)

	net, err := finder.Network(v.ClientCtx, dvsName)
	if err != nil {
		v.Log.Errorf("Failed at finding network: %s, err: %s", dvsName, err)
		return nil, err
	}

	objDvs, ok := net.(*object.DistributedVirtualSwitch)
	if !ok {
		v.Log.Errorf("Failed at getting DVS object")
		return nil, errors.New("Failed at getting DVS object")
	}
	return objDvs, nil
}

// RemovePenDVS removes the DVS
func (v *VCProbe) RemovePenDVS(dcName, dvsName string) error {
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

	objDvs, err := v.getPenDVS(dcName, dvsName, finder)
	if err != nil {
		return err
	}

	task, err := objDvs.Destroy(v.ClientCtx)
	if err != nil {
		v.Log.Errorf("Failed at destroying DVS: %s from datacenter: %s, err: %s", dvsName, dcName, err)
		return err
	}

	err = task.Wait(v.ClientCtx)
	if err != nil {
		v.Log.Errorf("Failed at destroying DVS: %s from datacenter: %s, err: %s", dvsName, dcName, err)
		return err
	}

	return nil
}

// GetPenDVSPorts returns the port configuration of the given dvs
func (v *VCProbe) GetPenDVSPorts(dcName, dvsName string, criteria *types.DistributedVirtualSwitchPortCriteria) ([]types.DistributedVirtualPort, error) {
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()
	return v.getPenDVSPorts(dcName, dvsName, criteria, finder)
}

func (v *VCProbe) getPenDVSPorts(dcName, dvsName string, criteria *types.DistributedVirtualSwitchPortCriteria, finder *find.Finder) ([]types.DistributedVirtualPort, error) {

	dvsObj, err := v.getPenDVS(dcName, dvsName, finder)
	if err != nil {
		return nil, err
	}
	ret, err := dvsObj.FetchDVPorts(v.ClientCtx, criteria)
	if err != nil {
		v.Log.Errorf("Can't find ports, err: %s", err)
		return nil, err
	}
	return ret, nil
}

// UpdateDVSPortsVlan updates the port settings
func (v *VCProbe) UpdateDVSPortsVlan(dcName, dvsName string, portsSetting PenDVSPortSettings) error {
	numPorts := len(portsSetting)
	if numPorts == 0 {
		// Nothing to do
		return nil
	}

	portKeys := make([]string, numPorts)
	portSpecs := make([]types.DVPortConfigSpec, numPorts)

	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

	dvsObj, err := v.getPenDVS(dcName, dvsName, finder)
	if err != nil {
		return err
	}

	for k := range portsSetting {
		portKeys = append(portKeys, k)
	}

	criteria := &types.DistributedVirtualSwitchPortCriteria{
		PortKey: portKeys,
	}

	ports, err := v.getPenDVSPorts(dcName, dvsName, criteria, finder)
	if err != nil {
		return err
	}

	for i := 0; i < numPorts; i++ {
		portSpecs[i].ConfigVersion = ports[i].Config.ConfigVersion
		portSpecs[i].Key = ports[i].Key
		portSpecs[i].Operation = string("edit")
		portSpecs[i].Scope = ports[i].Config.Scope
		setting, ok := portSpecs[i].Setting.(*types.VMwareDVSPortSetting)
		if !ok {
			setting = &types.VMwareDVSPortSetting{}
		}
		setting.Vlan = portsSetting[ports[i].Key]
		portSpecs[i].Setting = setting
	}

	task, err := dvsObj.ReconfigureDVPort(v.ClientCtx, portSpecs)
	if err != nil {
		v.Log.Errorf("Failed at reconfig DVS ports, err: %s", err)
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		v.Log.Errorf("Failed at modifying DVS ports, err: %s", err)
		return err
	}

	return nil
}
