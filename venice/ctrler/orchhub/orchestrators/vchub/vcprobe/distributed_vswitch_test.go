package vcprobe

import (
	"testing"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIsOverrideEqual(t *testing.T) {
	type testCase struct {
		spec  PenDVSPortSettings
		ports []types.DistributedVirtualPort
		ret   bool
	}

	tcs := []testCase{
		{
			spec: PenDVSPortSettings{
				"10": &types.VmwareDistributedVirtualSwitchVlanIdSpec{
					VlanId: int32(10),
				},
				"11": &types.VmwareDistributedVirtualSwitchVlanIdSpec{
					VlanId: int32(11),
				},
			},
			ports: []types.DistributedVirtualPort{
				types.DistributedVirtualPort{
					Key: "10",
					Config: types.DVPortConfigInfo{
						Setting: &types.VMwareDVSPortSetting{
							Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
								VlanId: 10,
							},
						},
					},
				},
				types.DistributedVirtualPort{
					Key: "11",
					Config: types.DVPortConfigInfo{
						Setting: &types.VMwareDVSPortSetting{
							Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
								VlanId: 11,
							},
						},
					},
				},
			},
			ret: true,
		},
		{
			spec: PenDVSPortSettings{
				"10": &types.VmwareDistributedVirtualSwitchVlanIdSpec{
					VlanId: int32(10),
				},
				"11": &types.VmwareDistributedVirtualSwitchVlanIdSpec{
					VlanId: int32(11),
				},
			},
			ports: []types.DistributedVirtualPort{
				types.DistributedVirtualPort{
					Key: "10",
					Config: types.DVPortConfigInfo{
						Setting: &types.VMwareDVSPortSetting{
							Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
								VlanId: 10,
							},
						},
					},
				},
				types.DistributedVirtualPort{
					Key: "11",
					Config: types.DVPortConfigInfo{
						Setting: &types.VMwareDVSPortSetting{
							Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
								VlanId: 12,
							},
						},
					},
				},
			},
			ret: false,
		},
	}

	config := log.GetDefaultConfig("dvs_override_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	probe := &VCProbe{
		State: &defs.State{
			Log: logger,
		},
	}

	for i, tc := range tcs {
		ret := probe.isOverrideEqual(tc.spec, tc.ports)
		AssertEquals(t, tc.ret, ret, "tc %d: returned value did not match", i)
	}
}
