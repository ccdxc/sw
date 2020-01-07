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
	pgStateMap map[string](map[string]*types.DVPortgroupConfigSpec)
}

// NewProbeMock creates a mock wrapper around the given probe
func NewProbeMock(probe *vcprobe.VCProbe) *ProbeMock {
	return &ProbeMock{
		VCProbe:    probe,
		pgStateMap: map[string](map[string]*types.DVPortgroupConfigSpec){},
	}
}

// AddPenPG creates a PG
func (v *ProbeMock) AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec) error {
	dc := v.pgStateMap[dcName]
	if dc == nil {
		dc = map[string]*types.DVPortgroupConfigSpec{}
	}
	dc[pgConfigSpec.Name] = pgConfigSpec
	return v.VCProbe.AddPenPG(dcName, dvsName, pgConfigSpec)
}

func extractPvlanID(spec *types.DVPortgroupConfigSpec) (int32, error) {
	setting, ok := spec.DefaultPortConfig.(*types.VMwareDVSPortSetting)
	if !ok {
		return -1, fmt.Errorf("Spec does not have pvlan info")
	}
	pvlanSpec, ok := setting.Vlan.(*types.VmwareDistributedVirtualSwitchPvlanSpec)
	if !ok {
		return -1, fmt.Errorf("Spec does not have pvlan info")
	}
	return pvlanSpec.PvlanId, nil
}

// ListPG lists PGs
func (v *ProbeMock) ListPG(dcRef *types.ManagedObjectReference) []mo.DistributedVirtualPortgroup {
	pgs := v.VCProbe.ListPG(dcRef)

	_, finder, _ := v.VCProbe.GetClientWithRLock()
	dcList, err := finder.DatacenterList(context.Background(), "*")
	if err != nil {
		v.Log.Errorf("Mock Probe failed to list datacenters, %s", err)
		v.VCProbe.ReleaseClientRLock()
		return []mo.DistributedVirtualPortgroup{}
	}
	v.VCProbe.ReleaseClientRLock()

	var dcName string
	for _, dc := range dcList {
		if dc.Reference().Value == (*dcRef).Value {
			dcName = dc.Name()
			break
		}
	}

	pgMap := v.pgStateMap[dcName]
	for _, pg := range pgs {
		config := pgMap[pg.Name]
		if config == nil {
			continue
		}
		pvlanID, err := extractPvlanID(config)
		if err == nil {
			pg.Config.DefaultPortConfig = &types.VMwareDVSPortSetting{
				Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
					PvlanId: pvlanID,
				},
			}
		}
	}
	return pgs
}

// ListDVS lists DVS objects
func (v *ProbeMock) ListDVS(dcRef *types.ManagedObjectReference) []mo.VmwareDistributedVirtualSwitch {
	var dvsObjs []mo.DistributedVirtualSwitch
	var ret []mo.VmwareDistributedVirtualSwitch
	v.VCProbe.ListObj("DistributedVirtualSwitch", []string{"name"}, &dvsObjs, dcRef)
	for _, obj := range dvsObjs {
		ret = append(ret, mo.VmwareDistributedVirtualSwitch{
			DistributedVirtualSwitch: obj,
		})
	}
	return ret
}
