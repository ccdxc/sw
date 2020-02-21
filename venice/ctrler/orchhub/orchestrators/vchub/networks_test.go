package vchub

import (
	"testing"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
)

func TestNetworks(t *testing.T) {

	dcName := "DC1"
	dvsName := createDVSName(dcName)

	testCases := []storeTC{
		{
			name: "network delete",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.DistributedVirtualPortgroup,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "pg-1",
						Originator: "127.0.0.1:8990",
						Changes:    []types.PropertyChange{},
						UpdateType: types.ObjectUpdateKindLeave,
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).Times(2)
				mockProbe.EXPECT().GetPenPG(dcName, gomock.Any(), gomock.Any()).Return(&object.DistributedVirtualPortgroup{
					Common: object.NewCommon(nil,
						types.ManagedObjectReference{
							Type:  string(defs.DistributedVirtualPortgroup),
							Value: "PG-10",
						}),
				}, nil).AnyTimes()
				mockProbe.EXPECT().TagObjAsManaged(gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().TagObjWithVlan(gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().UpdateDVSPortsVlan(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()

				// Setup state for DC1
				addDCState(t, vchub, dcName)

				orchInfo := []*network.OrchestratorInfo{
					{
						Name:      vchub.VcID,
						Namespace: dcName,
					},
				}
				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
				addPGState(t, vchub, dcName, createPGName("PG1"), "pg-1", "PG1")
			},
			verify: func(v *VCHub) {
				// Verification is mockprobe AddPenPG getting called 2 times
			},
		},
		{
			name: "pg rename",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.DistributedVirtualPortgroup,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "pg-1",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.DVPortgroupConfigInfo{
									DistributedVirtualSwitch: &types.ManagedObjectReference{
										Type:  string(defs.VmwareDistributedVirtualSwitch),
										Value: "dvs-1",
									},
									Name: "RandomName",
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).Times(1)
				mockProbe.EXPECT().GetPenPG(dcName, gomock.Any(), gomock.Any()).Return(&object.DistributedVirtualPortgroup{
					Common: object.NewCommon(nil,
						types.ManagedObjectReference{
							Type:  string(defs.DistributedVirtualPortgroup),
							Value: "PG-10",
						}),
				}, nil).AnyTimes()
				mockProbe.EXPECT().TagObjAsManaged(gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().TagObjWithVlan(gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().UpdateDVSPortsVlan(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()

				mockProbe.EXPECT().RenamePG(dcName, "RandomName", createPGName("PG1"), gomock.Any()).Return(nil).Times(1)

				// Setup state for DC1
				addDCState(t, vchub, dcName)

				orchInfo := []*network.OrchestratorInfo{
					{
						Name:      vchub.VcID,
						Namespace: dcName,
					},
				}
				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
				addPGState(t, vchub, dcName, createPGName("PG1"), "pg-1", "PG1")
			},
			verify: func(v *VCHub) {
				// Verification is mockprobe RenamePG getting called
			},
		},
		{
			name: "pg pvlan config change",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.DistributedVirtualPortgroup,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "pg-1",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.DVPortgroupConfigInfo{
									DistributedVirtualSwitch: &types.ManagedObjectReference{
										Type:  string(defs.VmwareDistributedVirtualSwitch),
										Value: "dvs-1",
									},
									Name: createPGName("PG1"),
									DefaultPortConfig: &types.VMwareDVSPortSetting{
										Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
											PvlanId: 100,
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).Times(2)
				mockProbe.EXPECT().GetPenPG(dcName, gomock.Any(), gomock.Any()).Return(&object.DistributedVirtualPortgroup{
					Common: object.NewCommon(nil,
						types.ManagedObjectReference{
							Type:  string(defs.DistributedVirtualPortgroup),
							Value: "PG-10",
						}),
				}, nil).AnyTimes()
				mockProbe.EXPECT().TagObjAsManaged(gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().TagObjWithVlan(gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().UpdateDVSPortsVlan(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()

				// Setup state for DC1
				addDCState(t, vchub, dcName)

				orchInfo := []*network.OrchestratorInfo{
					{
						Name:      vchub.VcID,
						Namespace: dcName,
					},
				}
				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
				addPGState(t, vchub, dcName, createPGName("PG1"), "pg-1", "PG1")
			},
			verify: func(v *VCHub) {
				// Verification is mockprobe AddPenPG getting called 2 times
			},
		},
		{
			name: "pg pvlan config change to vlan",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.DistributedVirtualPortgroup,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "pg-1",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.DVPortgroupConfigInfo{
									DistributedVirtualSwitch: &types.ManagedObjectReference{
										Type:  string(defs.VmwareDistributedVirtualSwitch),
										Value: "dvs-1",
									},
									Name: createPGName("PG1"),
									DefaultPortConfig: &types.VMwareDVSPortSetting{
										Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
											VlanId: 4,
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).Times(2)
				mockProbe.EXPECT().GetPenPG(dcName, gomock.Any(), gomock.Any()).Return(&object.DistributedVirtualPortgroup{
					Common: object.NewCommon(nil,
						types.ManagedObjectReference{
							Type:  string(defs.DistributedVirtualPortgroup),
							Value: "PG-10",
						}),
				}, nil).AnyTimes()
				mockProbe.EXPECT().TagObjAsManaged(gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().TagObjWithVlan(gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().UpdateDVSPortsVlan(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()

				// Setup state for DC1
				addDCState(t, vchub, dcName)

				orchInfo := []*network.OrchestratorInfo{
					{
						Name:      vchub.VcID,
						Namespace: dcName,
					},
				}
				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
				addPGState(t, vchub, dcName, createPGName("PG1"), "pg-1", "PG1")
			},
			verify: func(v *VCHub) {
				// Verification is mockprobe RenamePG getting called
			},
		},
	}

	runStoreTC(t, testCases)
}
