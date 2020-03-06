package vchub

import (
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestHosts(t *testing.T) {

	dcName := "DC1"
	dvsName := CreateDVSName(dcName)
	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	macStr := conv.MacString(pNicMac)

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
												Mac: macStr,
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
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
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
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
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
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
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
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
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
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				// Setup state for DC1
				addDCState(t, vchub, dcName)
			},
			verify: func(v *VCHub) {
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
	}

	runStoreTC(t, testCases)
}
