package mock

import (
	"context"
	"fmt"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	vcprobe "github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
)

// ProbeMock calls through to real probe except for functions that don't work with vcsim
type ProbeMock struct {
	*vcprobe.VCProbe
	// dc -> pgName -> config
	PgStateMap map[string](map[string]*types.DVPortgroupConfigSpec)
	// dc -> dvsName -> portKey -> port info
	DvsStateMap map[string](map[string](map[string]types.DistributedVirtualPort))
}

// NewProbeMock creates a mock wrapper around the given probe
func NewProbeMock(probe *vcprobe.VCProbe) *ProbeMock {
	return &ProbeMock{
		VCProbe:     probe,
		PgStateMap:  map[string](map[string]*types.DVPortgroupConfigSpec){},
		DvsStateMap: map[string](map[string](map[string]types.DistributedVirtualPort)){},
	}
}

// AddPenPG creates a PG
func (v *ProbeMock) AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec, isEqual vcprobe.IsPGConfigEqual, retry int) error {
	dc := v.PgStateMap[dcName]
	if dc == nil {
		dc = map[string]*types.DVPortgroupConfigSpec{}
		v.PgStateMap[dcName] = dc
	}
	dc[pgConfigSpec.Name] = pgConfigSpec
	return v.VCProbe.AddPenPG(dcName, dvsName, pgConfigSpec, isEqual, retry)
}

func extractVlanSpec(spec *types.DVPortgroupConfigSpec) (types.BaseVmwareDistributedVirtualSwitchVlanSpec, error) {
	setting, ok := spec.DefaultPortConfig.(*types.VMwareDVSPortSetting)
	if !ok {
		return nil, fmt.Errorf("Spec does not have pvlan info")
	}
	return setting.Vlan, nil
}

func extractPvlanID(spec *types.DVPortgroupConfigSpec) (int32, error) {
	setting, ok := spec.DefaultPortConfig.(*types.VMwareDVSPortSetting)
	if !ok {
		return -1, fmt.Errorf("Spec does not have pvlan info")
	}
	pvlanSpec, ok := setting.Vlan.(*types.VmwareDistributedVirtualSwitchVlanIdSpec)
	if !ok {
		return -1, fmt.Errorf("Spec does not have pvlan info")
	}
	return pvlanSpec.VlanId, nil
}

// ListPG lists PGs
func (v *ProbeMock) ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup {
	pgs := v.VCProbe.ListPG(dcRef)

	client := v.GetClientWithRLock()
	finder := v.CreateFinder(client)
	dcList, err := finder.DatacenterList(context.Background(), "*")
	if err != nil {
		v.Log.Errorf("Mock Probe failed to list datacenters, %s", err)
		v.VCProbe.ReleaseClientsRLock()
		return []mo.DistributedVirtualPortgroup{}
	}
	v.VCProbe.ReleaseClientsRLock()

	var dcName string
	for _, dc := range dcList {
		if dc.Reference().Value == (*dcRef).Value {
			dcName = dc.Name()
			break
		}
	}

	pgMap := v.PgStateMap[dcName]
	for _, pg := range pgs {
		config := pgMap[pg.Name]
		if config == nil {
			continue
		}
		vlanConfig, err := extractVlanSpec(config)
		if err == nil {
			pg.Config.DefaultPortConfig = &types.VMwareDVSPortSetting{
				Vlan: vlanConfig,
			}
		}
	}
	return pgs
}

// GetPGConfig gets PG configuration
func (v *ProbeMock) GetPGConfig(dcName string, pgName string, ps []string, retry int) (*mo.DistributedVirtualPortgroup, error) {
	pgObj, err := v.VCProbe.GetPGConfig(dcName, pgName, ps, retry)
	if err != nil {
		return nil, err
	}
	pgMap := v.PgStateMap[dcName]
	config := pgMap[pgName]
	if config == nil {
		return pgObj, nil
	}
	vlanConfig, err := extractVlanSpec(config)
	if err == nil {
		pgObj.Config.DefaultPortConfig = &types.VMwareDVSPortSetting{
			Vlan: vlanConfig,
		}
	}
	return pgObj, nil
}

// ListDVS lists DVS objects
func (v *ProbeMock) ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch {
	var dvsObjs []mo.DistributedVirtualSwitch
	var ret []mo.VmwareDistributedVirtualSwitch
	v.VCProbe.ListObj("DistributedVirtualSwitch", []string{"name", "config"}, &dvsObjs, dcRef)
	for _, obj := range dvsObjs {
		ret = append(ret, mo.VmwareDistributedVirtualSwitch{
			DistributedVirtualSwitch: obj,
		})
	}
	return ret
}

// UpdateDVSPortsVlan stores the changes locally since vcsim does not support reconfigureDVS
func (v *ProbeMock) UpdateDVSPortsVlan(dcName, dvsName string, portsSetting vcprobe.PenDVSPortSettings, retry int) error {
	dc := v.DvsStateMap[dcName]
	portConfigs := map[string]types.DistributedVirtualPort{}
	if dc == nil {
		dc = map[string](map[string]types.DistributedVirtualPort){}
	} else {
		portConfigs = dc[dvsName]
	}

	for key, settings := range portsSetting {
		switch vlanConfig := settings.(type) {
		case *types.VmwareDistributedVirtualSwitchVlanIdSpec:
			// Vlan override
			portConfig := types.DistributedVirtualPort{
				Key: key,
				Config: types.DVPortConfigInfo{
					Setting: &types.VMwareDVSPortSetting{
						Vlan: vlanConfig,
					},
				},
			}
			portConfigs[key] = portConfig
		case *types.VmwareDistributedVirtualSwitchPvlanSpec:
			// Pvlan settings
			portConfig := types.DistributedVirtualPort{
				Key: key,
				Config: types.DVPortConfigInfo{
					Setting: &types.VMwareDVSPortSetting{
						Vlan: vlanConfig,
					},
				},
			}
			portConfigs[key] = portConfig
		default:
			fmt.Printf("unknown type %T!\n", vlanConfig)
		}
	}

	dc[dvsName] = portConfigs
	v.DvsStateMap[dcName] = dc

	return nil
}

// GetPenDVSPorts returns port settings
func (v *ProbeMock) GetPenDVSPorts(dcName, dvsName string, criteria *types.DistributedVirtualSwitchPortCriteria, retry int) ([]types.DistributedVirtualPort, error) {
	dc := v.DvsStateMap[dcName]
	portsRet := []types.DistributedVirtualPort{}

	portSettings := map[string]types.DistributedVirtualPort{}
	if dc != nil {
		portSettings = dc[dvsName]
	}
	portSettingsUsed := map[string]bool{}

	// Get existing port info
	ports, err := v.VCProbe.GetPenDVSPorts(dcName, dvsName, criteria, retry)
	if err != nil {
		return ports, err
	}
	// Override setting for any ports we have local info about
	for _, p := range ports {
		setting, ok := portSettings[p.Key]
		if ok {
			portSettingsUsed[p.Key] = true
			setting.PortgroupKey = p.PortgroupKey
			portsRet = append(portsRet, setting)
		} else {
			portsRet = append(portsRet, p)
		}
	}

	// Add any other local info we have
	for key, p := range portSettings {
		if used, ok := portSettingsUsed[key]; !ok || !used {
			portsRet = append(portsRet, p)
		}
	}
	return portsRet, nil
}

// StartWatchers start the watchers
func (v *ProbeMock) StartWatchers() {
	v.VCProbe.StartWatchers()
}
