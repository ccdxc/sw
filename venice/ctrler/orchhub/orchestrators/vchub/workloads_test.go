package vchub

import (
	"fmt"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestWorkloads(t *testing.T) {

	dcName := "DC1"
	dvsName := CreateDVSName(dcName)
	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	macStr := conv.MacString(pNicMac)

	testCases := []storeTC{
		{
			name: "basic workload create without host",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						DcID:       dcName,
						DcName:     dcName,
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
				}

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				workloadAPI := v.StateMgr.Controller().Workload()
				_, err := workloadAPI.Find(expMeta)
				Assert(t, err != nil, "Workload unexpectedly in StateMgr. Err: %v", err)
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "workload create with runtime info",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						HostName: "hostsystem-21",
					},
				}

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Failed to get workload")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "workload create with interfaces",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAssign, // same as add
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										// Should ignore other devices
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
											generateVNIC("aa:bb:cc:dd:dd:ee", "100", "PG3", "Vmxnet2"),
											generateVNIC("aa:bb:cc:dd:cc:ee", "5000", "PG4", "Vmxnet"), // Outside vlan range
											&types.VirtualLsiLogicSASController{},
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
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
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().GetPGConfig(dcName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("doesn't exist")).AnyTimes()
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
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG2", "10.1.1.0/24", "10.1.1.1", 200, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG2"), "PG2", "PG2")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG3", "10.1.1.0/24", "10.1.1.1", 300, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG3"), "PG3", "PG3")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG4", "10.1.1.0/24", "10.1.1.1", 400, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG4"), "PG4", "PG4")

			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						Interfaces: []workload.WorkloadIntfSpec{
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.eeff",
								Network:     "PG1",
								IpAddresses: []string{},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.ddff",
								Network:     "PG2",
								IpAddresses: []string{},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.ddee",
								Network:     "PG3",
								IpAddresses: []string{},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.ccee",
								Network:     "PG4",
								IpAddresses: []string{},
							},
						},
					},
				}
				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				for i, inf := range item.Spec.Interfaces {
					Assert(t, inf.MicroSegVlan != 0, "Useg was not set for inf %s", inf.MACAddress)
					expWorkload.Spec.Interfaces[i].MicroSegVlan = inf.MicroSegVlan
				}
				AssertEquals(t, expWorkload.Spec.Interfaces, item.Spec.Interfaces, "Interfaces were not equal")
			},
		},
		{
			name: "workload update with interfaces",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:aa:cc:dd:dd:ff", "12", "PG2", "E1000"),
											generateVNIC("aa:aa:aa:dd:dd:ff", "13", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "guest",
								Val: types.GuestInfo{
									Net: []types.GuestNicInfo{
										{
											MacAddress: "aa:bb:cc:dd:ee:ff",
											Network:    CreatePGName("PG1"),
											IpAddress:  []string{"1.1.1.1", "fe80::eede:2031:aa18:ff3b", "1.1.1.2"},
										},
										{
											MacAddress: "aa:aa:cc:dd:dd:ff",
											Network:    CreatePGName("PG2"),
											IpAddress:  []string{"fe80::eede:2031:aa18:ff3b"},
										},
										{ // PG3 shouldn't be added since we don't have state for it
											MacAddress: "aa:aa:aa:dd:dd:ff",
											Network:    "PG3",
											IpAddress:  []string{"3.3.3.1", "3.3.3.2"},
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
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
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().GetPGConfig(dcName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("doesn't exist")).AnyTimes()
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
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG2", "10.1.1.0/24", "10.1.1.1", 200, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG2"), "PG2", "PG2")
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						Interfaces: []workload.WorkloadIntfSpec{
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.eeff",
								Network:     "PG1",
								IpAddresses: []string{"1.1.1.1", "1.1.1.2"},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aaaa.ccdd.ddff",
								Network:     "PG2",
								IpAddresses: []string{},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aaaa.aadd.ddff",
								Network:     "PG2",
								IpAddresses: []string{},
							},
						},
					},
				}
				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					if item == nil {
						return false, nil
					}
					if len(item.Spec.Interfaces) != 3 {
						return false, nil
					}
					return true, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				for i, inf := range item.Spec.Interfaces {
					Assert(t, inf.MicroSegVlan != 0, "Useg was not set for inf %s", inf.MACAddress)
					expWorkload.Spec.Interfaces[i].MicroSegVlan = inf.MicroSegVlan
				}
				AssertEquals(t, expWorkload.Spec.Interfaces, item.Spec.Interfaces, "Interfaces were not equal")
				usegMgr := v.GetDC(dcName).GetPenDVS(dvsName).UsegMgr
				_, err := usegMgr.GetVlanForVnic("aabb.ccdd.ddff", v.createHostName(dcName, "hostsystem-21"))
				Assert(t, err != nil, "Vlan should not have still be assigned for the deleted inf")
			},
		},
		{
			name: "workload with interfaces update host",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-25",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-25",
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
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().GetPGConfig(dcName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("doesn't exist")).AnyTimes()
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
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG2", "10.1.1.0/24", "10.1.1.1", 200, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG2"), "PG2", "PG2")

			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						Interfaces: []workload.WorkloadIntfSpec{
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.eeff",
								Network:     "PG1",
								IpAddresses: []string{},
							},
							workload.WorkloadIntfSpec{
								MACAddress:  "aabb.ccdd.ddff",
								Network:     "PG2",
								IpAddresses: []string{},
							},
						},
					},
				}
				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					if item == nil {
						return false, nil
					}
					if len(item.Spec.Interfaces) != 2 {
						return false, nil
					}
					if item.Spec.HostName != v.createHostName(dcName, "hostsystem-25") {
						return false, nil
					}
					return true, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				for i, inf := range item.Spec.Interfaces {
					Assert(t, inf.MicroSegVlan != 0, "Useg was not set for inf %s", inf.MACAddress)
					expWorkload.Spec.Interfaces[i].MicroSegVlan = inf.MicroSegVlan
				}
				AssertEquals(t, expWorkload.Spec.Interfaces, item.Spec.Interfaces, "Interfaces were not equal")
				usegMgr := v.GetDC(dcName).GetPenDVS(dvsName).UsegMgr
				_, err := usegMgr.GetVlanForVnic("aabb.ccdd.ddff", v.createHostName(dcName, "hostsystem-21"))
				Assert(t, err != nil, "Vlan should not have still be assigned for the inf on the old host")
				_, err = usegMgr.GetVlanForVnic("aabb.ccdd.ddff", v.createHostName(dcName, "hostsystem-25"))
				AssertOk(t, err, "Vlan should be assigned on the new host")
			},
		},
		{
			name: "workload with many interfaces updated to zero",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{},
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
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().GetPGConfig(dcName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("doesn't exist")).AnyTimes()
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
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG2", "10.1.1.0/24", "10.1.1.1", 200, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG2"), "PG2", "PG2")
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						Interfaces: []workload.WorkloadIntfSpec{},
					},
				}
				AssertEventually(t, func() (bool, interface{}) {
					workloadAPI := v.StateMgr.Controller().Workload()
					_, err := workloadAPI.Find(expMeta)
					if err == nil {
						return false, nil
					}
					item := v.pCache.GetWorkload(expMeta)
					if item == nil {
						return false, nil
					}

					return true, nil
				}, "Workload not in pcache/statemgr")

				workloadAPI := v.StateMgr.Controller().Workload()
				_, err := workloadAPI.Find(expMeta)
				Assert(t, err != nil, "workload should have been deleted from statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				AssertEquals(t, len(expWorkload.Spec.Interfaces), len(item.Spec.Interfaces), "Interfaces were not equal")

				usegMgr := v.GetDC(dcName).GetPenDVS(dvsName).UsegMgr
				_, err = usegMgr.GetVlanForVnic("aabb.ccdd.ddff", v.createHostName(dcName, "hostsystem-21"))
				Assert(t, err != nil, "Vlan should not have still be assigned for the inf on the old host")
			},
		},
		{
			name: "workload inf assign for deleted workload",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Hardware: types.VirtualHardware{
										Device: []types.BaseVirtualDevice{
											generateVNIC("aa:bb:cc:dd:ee:ff", "10", "PG1", "E1000e"),
											generateVNIC("aa:bb:cc:dd:dd:ff", "11", "PG2", "E1000"),
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes:    []types.PropertyChange{},
						UpdateType: types.ObjectUpdateKindLeave,
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
				mockProbe.EXPECT().GetPGConfig(dcName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("doesn't exist")).AnyTimes()
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
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG2", "10.1.1.0/24", "10.1.1.1", 200, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG2"), "PG2", "PG2")
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				// Sleep to let initial create get processed
				time.Sleep(50 * time.Millisecond)
				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item == nil, item
				}, "Workload should be deleted")
			},
		},
		{
			name: "workload create with vm name",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-40",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAssign, // same as add
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Name: "test-vm",
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-40"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: api.ObjectMeta{
						Name: v.createVMWorkloadName(dcName, "virtualmachine-40"),
						Labels: map[string]string{
							NameKey:            "test-vm",
							utils.NamespaceKey: dcName,
							utils.OrchNameKey:  v.VcID,
						},
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: workload.WorkloadSpec{},
				}

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				AssertEquals(t, expWorkload.ObjectMeta.Labels, item.ObjectMeta.Labels, "workload labels are not same")
			},
		},

		{
			name: "workload update",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				existingWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec:       workload.WorkloadSpec{},
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						HostName: v.createHostName(dcName, "hostsystem-21"),
					},
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Failed to get workload")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				Assert(t, item.ObjectMeta.Name == existingWorkload.ObjectMeta.Name, "workloads are not same")
				AssertEquals(t, expWorkload.Spec, item.Spec, "Spec for objects was not equal")
			},
		},
		{
			name: "Workload redundant update",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				existingWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: workload.WorkloadSpec{
						HostName: "hostsystem-21",
					},
				}

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Failed to get workload")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				Assert(t, item.ObjectMeta.Name == existingWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "Workload delete",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes:    []types.PropertyChange{},
						UpdateType: types.ObjectUpdateKindLeave,
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item == nil, nil
				}, "Workload should not be in pcache/statemgr")
			},
		},
		{
			name: "workload with tags",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-21",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Mac: macStr,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1", "pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.VirtualMachine,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "virtualmachine-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAssign, // same as add
								Name: "config",
								Val: types.VirtualMachineConfigInfo{
									Name: "test-vm",
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "runtime",
								Val: types.VirtualMachineRuntimeInfo{
									Host: &types.ManagedObjectReference{
										Type:  "HostSystem",
										Value: "hostsystem-21",
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: string(defs.VMPropTag),
								Val: defs.TagMsg{
									Tags: []defs.TagEntry{
										defs.TagEntry{
											Name:     "tag1",
											Category: "cat1",
										},
										defs.TagEntry{
											Name:     "tag2",
											Category: "cat1",
										},
										defs.TagEntry{
											Name:     "tag1",
											Category: "cat2",
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
				expMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
					Labels: map[string]string{
						fmt.Sprintf("%s%s", VcLabelPrefix, "cat1"): "tag1:tag2",
						fmt.Sprintf("%s%s", VcLabelPrefix, "cat2"): "tag1",
						NameKey:            "test-vm",
						utils.NamespaceKey: dcName,
						utils.OrchNameKey:  "127.0.0.1:8990",
					},
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
				}

				AssertEventually(t, func() (bool, interface{}) {
					item := v.pCache.GetWorkload(expMeta)
					return item != nil, nil
				}, "Workload not in pcache/statemgr")

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache/statemgr")
				AssertEquals(t, expWorkload.ObjectMeta, item.ObjectMeta, "workloads are not same")
			},
		},
	}

	runStoreTC(t, testCases)
}
