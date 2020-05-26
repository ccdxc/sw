package vchub

import (
	"fmt"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestHosts(t *testing.T) {

	dcName := "DC1"
	dvsName := CreateDVSName(dcName)
	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	pNicMac2 := append(createPenPnicBase(), 0xbb, 0x00, 0x01)
	macStr := conv.MacString(pNicMac)
	macStr2 := conv.MacString(pNicMac2)

	testCases := []storeTC{
		{
			name: "host create",
			events: []defs.Probe2StoreMsg{
				// use multiple events to create host
				// send name property before config property
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-44",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "Host_Named_Foo",
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
						Key:        "hostsystem-44",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "config",
								Val: types.HostConfigInfo{
									Network: &types.HostNetworkInfo{
										Pnic: []types.PhysicalNic{
											types.PhysicalNic{
												Key: "pnic-1",
												Mac: macStr,
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1"},
											},
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-44"),
				}
				expHost := &cluster.Host{
					TypeMeta: api.TypeMeta{
						Kind:       "Host",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: cluster.HostSpec{
						DSCs: []cluster.DistributedServiceCardID{
							cluster.DistributedServiceCardID{
								MACAddress: macStr,
							},
						},
					},
				}

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err == nil, err
				}, "Host not in statemgr")

				hostAPI := v.StateMgr.Controller().Host()
				h, err := hostAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get host: err %v", err)
				Assert(t, h.ObjectMeta.Name == expHost.ObjectMeta.Name, "hosts are not same")
				Assert(t, h.Labels != nil, "No Labels found on the host")
				orchName, ok := h.Labels[utils.OrchNameKey]
				Assert(t, ok, "Failed to get Orch Name Label")
				AssertEquals(t, orchName, v.VcID, "Orch Name does not match")
				dispName, ok := h.Labels[NameKey]
				Assert(t, ok, "Failed to get Orch Name Label")
				AssertEquals(t, dispName, "Host_Named_Foo", "Orch Name does not match")
			},
		},
		{
			name: "host update",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
											},
											types.PhysicalNic{
												Mac: macStr2,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1"},
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
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
											},
											types.PhysicalNic{
												Mac: macStr2,
												Key: "pnic-2",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-2"},
											},
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}
				existingHost := &cluster.Host{
					TypeMeta: api.TypeMeta{
						Kind:       "Host",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: cluster.HostSpec{
						DSCs: []cluster.DistributedServiceCardID{
							cluster.DistributedServiceCardID{
								MACAddress: macStr,
							},
						},
					},
				}
				expHost := &cluster.Host{
					TypeMeta: api.TypeMeta{
						Kind:       "Host",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: cluster.HostSpec{
						DSCs: []cluster.DistributedServiceCardID{
							cluster.DistributedServiceCardID{
								MACAddress: macStr,
							},
						},
					},
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err == nil, err
				}, "Host not in statemgr")

				hostAPI := v.StateMgr.Controller().Host()
				h, err := hostAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get host")
				Assert(t, h.ObjectMeta.Name == expHost.ObjectMeta.Name, "hosts are not same")
				Assert(t, existingHost.ObjectMeta.Name == expHost.ObjectMeta.Name, "hosts are not same")
			},
		},
		{
			name: "host update - remove naples pnic",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{"pnic-1"},
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
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
											},
										},
									},
								},
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err != nil, nil
				}, "Host should not be in statemgr")
			},
		},
		{
			name: "host redundant update",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
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
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}
				existingHost := &cluster.Host{
					TypeMeta: api.TypeMeta{
						Kind:       "Host",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
					Spec: cluster.HostSpec{
						DSCs: []cluster.DistributedServiceCardID{
							cluster.DistributedServiceCardID{
								MACAddress: macStr,
							},
						},
					},
				}

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err == nil, err
				}, "Host not in statemgr")

				hostAPI := v.StateMgr.Controller().Host()
				h, err := hostAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get host")
				Assert(t, h.ObjectMeta.Name == existingHost.ObjectMeta.Name, "hosts are not same")
			},
		},
		{
			name: "Host delete",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
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
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "hostsystem_41",
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
						Key:        "hostsystem-41",
						Originator: "127.0.0.1:8990",
						Changes:    []types.PropertyChange{},
						UpdateType: types.ObjectUpdateKindLeave,
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err != nil, nil
				}, "Host should not be in statemgr")
			},
		},
		{
			name: "Host unknown property should no-op",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "invalid-property",
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					return err != nil, nil
				}, "Host should not be in statemgr")
			},
		},
		{
			name: "Host add remove add uplink",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
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
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "hostsystem_41",
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
										Value: "hostsystem-41",
									},
								},
							},
						},
					},
				},
				{ // Remove pnics from dvs
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
											},
										},
										ProxySwitch: []types.HostProxySwitch{
											types.HostProxySwitch{
												DvsName: dvsName,
												Pnic:    []string{},
											},
										},
									},
								},
							},
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "hostsystem_41",
							},
						},
					},
				},
				{ // add back pnics from dvs
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.HostSystem,
						DcID:       dcName,
						DcName:     dcName,
						Key:        "hostsystem-41",
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
												Key: "pnic-1",
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
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "hostsystem_41",
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller, eventRecorder *mockevtsrecorder.Recorder) {
				// Setup state for DC1
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().AddPenPG(dcName, dvsName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
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
				mockProbe.EXPECT().UpdateDVSPortsVlan(dcName, dvsName, gomock.Any(), gomock.Any(), gomock.Any()).Return(nil).AnyTimes()

				addDCState(t, vchub, dcName)

				orchInfo := []*network.OrchestratorInfo{
					{
						Name:      vchub.VcID,
						Namespace: dcName,
					},
				}
				statemgr.CreateNetwork(vchub.StateMgr, "default", "PG1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
				addPGState(t, vchub, dcName, CreatePGName("PG1"), "PG1", "PG1")

			},
			verify: func(v *VCHub, eventRecorder *mockevtsrecorder.Recorder) {
				expMeta := &api.ObjectMeta{
					Name: v.createHostName(dcName, "hostsystem-41"),
				}

				expWorkloadMeta := &api.ObjectMeta{
					Name:      v.createVMWorkloadName(dcName, "virtualmachine-41"),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}

				time.Sleep(50 * time.Millisecond)

				AssertEventually(t, func() (bool, interface{}) {
					hostAPI := v.StateMgr.Controller().Host()
					_, err := hostAPI.Find(expMeta)
					if err != nil {
						return false, err
					}

					_, err = v.StateMgr.Controller().Workload().Find(expWorkloadMeta)
					return err == nil, nil
				}, "Host and workload should be in statemgr")
			},
		},
	}

	runStoreTC(t, testCases)
}
