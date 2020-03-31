package vchub

import (
	"testing"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestPGConfigCheck(t *testing.T) {
	type testCase struct {
		vlan int
		pg   *mo.DistributedVirtualPortgroup
		ret  bool
	}

	tcs := []testCase{
		{
			vlan: 10,
			pg: &mo.DistributedVirtualPortgroup{
				Config: types.DVPortgroupConfigInfo{
					Policy: &types.VMwareDVSPortgroupPolicy{
						DVPortgroupPolicy: types.DVPortgroupPolicy{
							PortConfigResetAtDisconnect: true,
						},
						VlanOverrideAllowed: true,
					},
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: int32(10),
						},
					},
				},
			},
			ret: true,
		},
		{ // Wrong vlan
			vlan: 10,
			pg: &mo.DistributedVirtualPortgroup{
				Config: types.DVPortgroupConfigInfo{
					Policy: &types.VMwareDVSPortgroupPolicy{
						DVPortgroupPolicy: types.DVPortgroupPolicy{
							PortConfigResetAtDisconnect: true,
						},
						VlanOverrideAllowed: true,
					},
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: int32(11),
						},
					},
				},
			},
			ret: false,
		},
		{ // Wrong PortConfigResetAtDisconnect
			vlan: 10,
			pg: &mo.DistributedVirtualPortgroup{
				Config: types.DVPortgroupConfigInfo{
					Policy: &types.VMwareDVSPortgroupPolicy{
						DVPortgroupPolicy: types.DVPortgroupPolicy{
							PortConfigResetAtDisconnect: false,
						},
						VlanOverrideAllowed: true,
					},
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: int32(10),
						},
					},
				},
			},
			ret: false,
		},
		{ // Wrong vlan
			vlan: 10,
			pg: &mo.DistributedVirtualPortgroup{
				Config: types.DVPortgroupConfigInfo{
					Policy: &types.VMwareDVSPortgroupPolicy{
						DVPortgroupPolicy: types.DVPortgroupPolicy{
							PortConfigResetAtDisconnect: true,
						},
						VlanOverrideAllowed: false,
					},
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: int32(10),
						},
					},
				},
			},
			ret: false,
		},
	}

	config := log.GetDefaultConfig("pg_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	dvs := &PenDVS{
		State: &defs.State{
			Log: logger,
		},
	}

	for i, tc := range tcs {
		ret := dvs.createPGConfigCheck(tc.vlan)(nil, tc.pg)
		AssertEquals(t, tc.ret, ret, "tc %d: returned value did not match", i)
	}
}
