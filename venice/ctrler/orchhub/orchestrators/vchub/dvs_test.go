package vchub

import (
	"testing"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestDvsConfigCheck(t *testing.T) {
	type testCase struct {
		spec *types.DVSCreateSpec
		dvs  *mo.DistributedVirtualSwitch
		ret  bool
	}
	pvlanConfigSpecArray := []types.VMwareDVSPvlanConfigSpec{}

	// Setup all the pvlan allocations now
	// vlans 0 and 1 are reserved
	dvsConfig := &types.VMwareDVSConfigInfo{}
	badDvsConfig := &types.VMwareDVSConfigInfo{}
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

		dvsConfig.PvlanConfig = append(dvsConfig.PvlanConfig, pvlanMapEntry)
		if i%4 == 0 {
			badDvsConfig.PvlanConfig = append(dvsConfig.PvlanConfig, pvlanMapEntry)
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
	}

	var spec types.DVSCreateSpec
	spec.ConfigSpec = &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: pvlanConfigSpecArray,
	}

	tcs := []testCase{
		{
			spec: &spec,
			dvs: &mo.DistributedVirtualSwitch{
				Config: dvsConfig,
			},
			ret: true,
		},
		{
			spec: &spec,
			dvs: &mo.DistributedVirtualSwitch{
				Config: badDvsConfig,
			},
			ret: false,
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
		ret := dc.dvsConfigCheck(tc.spec, tc.dvs)
		AssertEquals(t, tc.ret, ret, "tc %d: returned value did not match", i)
	}
}
