package vchub

import (
	"fmt"
	"testing"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestDvsConfigDiff(t *testing.T) {
	type testCase struct {
		spec *types.DVSCreateSpec
		dvs  *mo.DistributedVirtualSwitch
		ret  *types.DVSCreateSpec
	}
	pvlanConfigSpecArray := []types.VMwareDVSPvlanConfigSpec{}

	// Setup all the pvlan allocations now
	// vlans 0 and 1 are reserved
	dvsConfig := &types.VMwareDVSConfigInfo{}
	badDvsConfig := &types.VMwareDVSConfigInfo{}
	expDvsConfigResult := []types.VMwareDVSPvlanConfigSpec{}
	for i := 0; i < 10; i += 2 {
		PvlanEntryProm := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "promiscuous",
			SecondaryVlanId: int32(i),
		}
		pvlanMapEntry := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "isolated",
			SecondaryVlanId: int32(i + 1),
		}

		pvlanSpecProm := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: PvlanEntryProm,
			Operation:  "add",
		}
		pvlanSpec := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: pvlanMapEntry,
			Operation:  "add",
		}
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpecProm)
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpec)

		dvsConfig.PvlanConfig = append(dvsConfig.PvlanConfig, pvlanMapEntry)
		dvsConfig.PvlanConfig = append(dvsConfig.PvlanConfig, PvlanEntryProm)
		if i%4 == 0 {
			badEntry := types.VMwareDVSPvlanMapEntry{
				PrimaryVlanId:   int32(i),
				PvlanType:       "community",
				SecondaryVlanId: int32(i + 1),
			}
			correctiveEntry := types.VMwareDVSPvlanConfigSpec{
				PvlanEntry: pvlanMapEntry,
				Operation:  "edit",
			}
			badDvsConfig.PvlanConfig = append(badDvsConfig.PvlanConfig, badEntry)
			expDvsConfigResult = append(expDvsConfigResult, correctiveEntry)
			expDvsConfigResult = append(expDvsConfigResult, pvlanSpecProm)
		} else {
			expDvsConfigResult = append(expDvsConfigResult, pvlanSpecProm, pvlanSpec)
		}

	}

	// Add extra config to badDVS
	badEntry := types.VMwareDVSPvlanMapEntry{
		PrimaryVlanId:   int32(4000),
		PvlanType:       "promiscuous",
		SecondaryVlanId: int32(4000),
	}
	badDvsConfig.PvlanConfig = append(badDvsConfig.PvlanConfig, badEntry)
	correctiveEntry := types.VMwareDVSPvlanConfigSpec{
		PvlanEntry: badEntry,
		Operation:  "remove",
	}
	expDvsConfigResult = append(expDvsConfigResult, correctiveEntry)

	var spec types.DVSCreateSpec
	spec.ConfigSpec = &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: pvlanConfigSpecArray,
	}

	var correctiveSpec types.DVSCreateSpec
	correctiveSpec.ConfigSpec = &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: expDvsConfigResult,
	}

	tcs := []testCase{
		{
			spec: &spec,
			dvs: &mo.DistributedVirtualSwitch{
				Config: dvsConfig,
			},
			ret: nil,
		},
		{
			spec: &spec,
			dvs: &mo.DistributedVirtualSwitch{
				Config: badDvsConfig,
			},
			ret: &correctiveSpec,
		},
	}

	config := log.GetDefaultConfig("dvs_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	dc := &PenDC{
		State: &defs.State{
			Log: logger,
		},
	}

	for i, tc := range tcs {
		ret := dc.dvsConfigDiff(tc.spec, tc.dvs)
		if tc.ret == nil {
			Assert(t, ret == nil, "tc %d: returned value did not match", i)
			continue
		}
		expSpec := tc.ret.ConfigSpec.(*types.VMwareDVSConfigSpec).PvlanConfigSpec
		expMap := map[string]types.VMwareDVSPvlanMapEntry{}
		for _, item := range expSpec {
			key := fmt.Sprintf("%d-%d", item.PvlanEntry.PrimaryVlanId, item.PvlanEntry.SecondaryVlanId)
			expMap[key] = item.PvlanEntry
		}

		retSpec := ret.ConfigSpec.(*types.VMwareDVSConfigSpec).PvlanConfigSpec
		retMap := map[string]types.VMwareDVSPvlanMapEntry{}
		for _, item := range retSpec {
			key := fmt.Sprintf("%d-%d", item.PvlanEntry.PrimaryVlanId, item.PvlanEntry.SecondaryVlanId)
			retMap[key] = item.PvlanEntry
		}
		AssertEquals(t, expMap, retMap, "tc %d: returned value did not match", i)
	}
}
