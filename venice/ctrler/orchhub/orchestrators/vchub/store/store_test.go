package store

import (
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/types"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	logConfig = log.GetDefaultConfig("vcstore_integ_test")
	logger    = log.SetConfig(logConfig)
)

func newStateManager() (*statemgr.Statemgr, error) {
	stateMgr, err := statemgr.NewStatemgr(globals.APIServer, nil, logger, nil)
	if err != nil {
		return nil, err
	}

	return stateMgr, nil
}

func TestStoreRun(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	// If set, logger will output to console
	debugMode := false
	if debugMode {
		logConfig.LogToStdout = true
		logConfig.Filter = log.AllowAllFilter
		logger = log.SetConfig(logConfig)
	}
	testCases := []struct {
		name   string
		events []defs.Probe2StoreMsg
		verify func(*VCHStore)
	}{
		{
			name: "basic workload create without host",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
					Key:        "virtualmachine-41",
					Originator: "127.0.0.1:8990",
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
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

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache")
				workloadAPI := v.stateMgr.Controller().Workload()
				_, err := workloadAPI.Find(expMeta)
				Assert(t, err != nil, "Workload unexpectedly in stateMgr. Err: %v", err)
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "workload create with interfaces",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
					Key:        "virtualmachine-41",
					Originator: "127.0.0.1:8990",
					Changes: []types.PropertyChange{
						types.PropertyChange{
							Op:   types.PropertyChangeOpAdd,
							Name: "config",
							Val: types.VirtualMachineConfigInfo{
								Hardware: types.VirtualHardware{
									Device: []types.BaseVirtualDevice{
										generateVNIC("aa:bb:cc:dd:ee:ff", "10", "E1000e"),
										generateVNIC("aa:bb:cc:dd:dd:ff", "11", "E1000"),
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
										generateVNIC("aa:bb:cc:dd:ee:ff", "10", "E1000e"),
										generateVNIC("aa:bb:cc:dd:dd:ff", "11", "E1000"),
										generateVNIC("aa:bb:cc:dd:dd:ee", "5000", "Vmxnet"), // Outside vlan range
										generateVNIC("aa:bb:cc:dd:dd:ee", "-10", "Vmxnet2"), // Outside vlan range
										&types.VirtualLsiLogicSASController{},
									},
								},
							},
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
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
								MACAddress:   "aa:bb:cc:dd:ee:ff",
								MicroSegVlan: 10,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aa:bb:cc:dd:dd:ff",
								MicroSegVlan: 11,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aa:bb:cc:dd:dd:ee",
								MicroSegVlan: 0,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aa:bb:cc:dd:dd:ee",
								MicroSegVlan: 0,
							},
						},
					},
				}
				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache")
				Assert(t, item.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				AssertEquals(t, expWorkload.Spec.Interfaces, item.Spec.Interfaces, "Interfaces were not equal")
			},
		},
		{
			name: "workload create with vm name",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
				{
					VcObject:   defs.VirtualMachine,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-40",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: api.ObjectMeta{
						Name: "127.0.0.1:8990-virtualmachine-40",
						Labels: map[string]string{
							"vcenter.vm-name":   "test-vm",
							"vcenter.orch-name": "127.0.0.1:8990",
						},
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: workload.WorkloadSpec{},
				}

				workloadAPI := v.stateMgr.Controller().Workload()
				w, err := workloadAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get workload")
				wT := w.Workload
				Assert(t, w.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				AssertEquals(t, expWorkload.ObjectMeta.Labels, wT.ObjectMeta.Labels, "workload labels are not same")
			},
		},
		{
			name: "workload create with runtime info",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
				{
					VcObject:   defs.VirtualMachine,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
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

				workloadAPI := v.stateMgr.Controller().Workload()
				w, err := workloadAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get workload")
				Assert(t, w.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "workload update",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
				{
					VcObject:   defs.VirtualMachine,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
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
						HostName: "hostsystem-21",
					},
				}

				workloadAPI := v.stateMgr.Controller().Workload()
				w, err := workloadAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get workload")
				Assert(t, w.ObjectMeta.Name == expWorkload.ObjectMeta.Name, "workloads are not same")
				Assert(t, w.ObjectMeta.Name == existingWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "Workload redundant update",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
				{
					VcObject:   defs.VirtualMachine,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
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

				workloadAPI := v.stateMgr.Controller().Workload()
				w, err := workloadAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get workload")
				Assert(t, w.ObjectMeta.Name == existingWorkload.ObjectMeta.Name, "workloads are not same")
			},
		},
		{
			name: "Workload delete",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
					Key:        "virtualmachine-41",
					Originator: "127.0.0.1:8990",
					Changes: []types.PropertyChange{
						types.PropertyChange{
							Op:   types.PropertyChangeOpRemove,
							Name: "config",
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item == nil, "Workload should not be in pcache or stateMgr")
			},
		},
		{
			name: "workload with tags",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
					Labels: map[string]string{
						"vcenter.cat1":      "tag1:tag2",
						"vcenter.cat2":      "tag1",
						"vcenter.vm-name":   "test-vm",
						"vcenter.orch-name": "127.0.0.1:8990",
					},
				}
				expWorkload := &workload.Workload{
					TypeMeta: api.TypeMeta{
						Kind:       "Workload",
						APIVersion: "v1",
					},
					ObjectMeta: *expMeta,
				}

				item := v.pCache.GetWorkload(expMeta)
				Assert(t, item != nil, "Workload not in pcache")
				AssertEquals(t, expWorkload.ObjectMeta, item.ObjectMeta, "workloads are not same")
			},
		},
		{
			name: "host create",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name: "127.0.0.1:8990-hostsystem-41",
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
								MACAddress: "aa:bb:cc:dd:ee:ff",
							},
						},
					},
				}

				hostAPI := v.stateMgr.Controller().Host()
				h, err := hostAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get host: err %v", err)
				Assert(t, h.ObjectMeta.Name == expHost.ObjectMeta.Name, "hosts are not same")
			},
		},
		{
			name: "host update",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:dd:ff",
										},
									},
								},
							},
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name: "127.0.0.1:8990-hostsystem-41",
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
								MACAddress: "aa:bb:cc:dd:ee:ff",
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
								MACAddress: "aa:bb:cc:dd:dd:ff",
							},
						},
					},
				}

				hostAPI := v.stateMgr.Controller().Host()
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
					VcObject:   defs.HostSystem,
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
											Mac: "aa:bb:cc:dd:ee:ff",
										},
									},
								},
							},
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name: "127.0.0.1:8990-hostsystem-41",
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
								MACAddress: "aa:bb:cc:dd:ee:ff",
							},
						},
					},
				}

				hostAPI := v.stateMgr.Controller().Host()
				h, err := hostAPI.Find(expMeta)
				Assert(t, err == nil, "Failed to get host")
				Assert(t, h.ObjectMeta.Name == existingHost.ObjectMeta.Name, "hosts are not same")
			},
		},
		{
			name: "Host delete",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
					Key:        "hostsystem-41",
					Originator: "127.0.0.1:8990",
					Changes: []types.PropertyChange{
						types.PropertyChange{
							Op:   types.PropertyChangeOpRemove,
							Name: "config",
						},
					},
				},
			},
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name: "127.0.0.1:8990-hostsystem-41",
				}

				hostAPI := v.stateMgr.Controller().Host()
				_, err := hostAPI.Find(expMeta)
				Assert(t, err != nil, "Failed to get host")
			},
		},
		{
			name: "Host unknown property should no-op",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.HostSystem,
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
			verify: func(v *VCHStore) {
				expMeta := &api.ObjectMeta{
					Name: "127.0.0.1:8990-hostsystem-41",
				}

				hostAPI := v.stateMgr.Controller().Host()
				_, err := hostAPI.Find(expMeta)
				Assert(t, err != nil, "Failed to get host")
			},
		},
	}

	tsdb.Init(context.Background(), &tsdb.Opts{})

	ctx, cancelFn := context.WithCancel(context.Background())
	sm, err := newStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	pCache := pcache.NewPCache(sm, logger)
	store := &VCHStore{
		ctx:      ctx,
		Log:      logger,
		stateMgr: sm,
		pCache:   pCache,
	}
	pCache.SetValidator("Workload", store.validateWorkload)

	for _, tc := range testCases {
		if len(forceTestName) != 0 && tc.name != forceTestName {
			continue
		}
		t.Logf("running %s", tc.name)
		store.stateMgr.SetAPIClient(nil)
		inbox := make(chan defs.Probe2StoreMsg)
		store.inbox = inbox
		store.Start()
		// Push events
		for _, e := range tc.events {
			inbox <- e
		}
		// Time for events to process
		time.Sleep(20 * time.Millisecond)
		tc.verify(store)

		// Terminating store instance
		cancelFn()
		doneCh := make(chan bool)
		go func() {
			store.Stop()
			doneCh <- true
		}()
		select {
		case <-doneCh:
		case <-time.After(1 * time.Second):
			t.Fatalf("Store failed to shutdown within timeout")
		}
	}

	Assert(t, len(forceTestName) == 0, "focus test flag should not be checked in")
}

func generateVNIC(macAddress, portKey string, vnicType string) types.BaseVirtualDevice {
	ethCard := types.VirtualEthernetCard{
		VirtualDevice: types.VirtualDevice{
			Backing: &types.VirtualEthernetCardDistributedVirtualPortBackingInfo{
				Port: types.DistributedVirtualSwitchPortConnection{
					PortKey: portKey,
				},
			},
		},
		MacAddress: macAddress,
	}
	switch vnicType {
	case "Vmxnet3":
		return &types.VirtualVmxnet3{
			VirtualVmxnet: types.VirtualVmxnet{
				VirtualEthernetCard: ethCard,
			},
		}
	case "Vmxnet2":
		return &types.VirtualVmxnet2{
			VirtualVmxnet: types.VirtualVmxnet{
				VirtualEthernetCard: ethCard,
			},
		}
	case "Vmxnet":
		return &types.VirtualVmxnet{
			VirtualEthernetCard: ethCard,
		}
	case "E1000e":
		return &types.VirtualE1000e{
			VirtualEthernetCard: ethCard,
		}
	case "E1000":
		return &types.VirtualE1000{
			VirtualEthernetCard: ethCard,
		}
	default:
		return &types.VirtualE1000{
			VirtualEthernetCard: ethCard,
		}
	}
}
