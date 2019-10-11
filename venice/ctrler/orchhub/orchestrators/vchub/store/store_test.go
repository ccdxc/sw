package store

import (
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/vim25/types"
	"golang.org/x/net/context"
	"google.golang.org/grpc/codes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	mockapi "github.com/pensando/sw/api/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	mocks "github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/store/mocks"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	logger = log.SetConfig(log.GetDefaultConfig("spyglass_integ_test"))
)

func TestStoreRun(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []struct {
		name      string
		events    []defs.Probe2StoreMsg
		setupMock func(*mocks.MockAPIClient)
	}{
		{
			name: "basic workload create",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
					Key:        "virtualmachine-41",
					Originator: "127.0.0.1:8990",
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
				mWorkload.EXPECT().Create(gomock.Any(), expWorkload).Return(nil, nil)
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
										generateVNIC("aaaa:bbbb:cccc", "10", "E1000e"),
										generateVNIC("aaaa:bbbb:dddd", "11", "E1000"),
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
										generateVNIC("aaaa:bbbb:cccc", "10", "E1000e"),
										generateVNIC("aaaa:bbbb:dddd", "11", "E1000"),
										generateVNIC("aaaa:bbbb:eeee", "5000", "Vmxnet"), // Outside vlan range
										generateVNIC("aaaa:bbbb:eeee", "-10", "Vmxnet2"), // Outside vlan range
										&types.VirtualLsiLogicSASController{},
									},
								},
							},
						},
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
								MACAddress:   "aaaa:bbbb:cccc",
								MicroSegVlan: 10,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aaaa:bbbb:dddd",
								MicroSegVlan: 11,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aaaa:bbbb:eeee",
								MicroSegVlan: 0,
							},
							workload.WorkloadIntfSpec{
								MACAddress:   "aaaa:bbbb:eeee",
								MicroSegVlan: 0,
							},
						},
					},
				}
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
				mWorkload.EXPECT().Create(gomock.Any(), expWorkload).Return(nil, nil)
			},
		},
		{
			name: "workload create with vm name",
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
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
					ObjectMeta: api.ObjectMeta{
						Name: "127.0.0.1:8990-virtualmachine-41",
						Labels: map[string]string{
							"vm-name": "test-vm",
						},
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: workload.WorkloadSpec{},
				}
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
				mWorkload.EXPECT().Create(gomock.Any(), expWorkload).Return(nil, nil)
			},
		},
		{
			name: "workload create with runtime info",
			events: []defs.Probe2StoreMsg{
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
				mWorkload.EXPECT().Create(gomock.Any(), expWorkload).Return(nil, nil)
			},
		},
		{
			name: "workload update",
			events: []defs.Probe2StoreMsg{
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(existingWorkload, nil)
				mWorkload.EXPECT().Update(gomock.Any(), expWorkload).Return(nil, nil)
			},
		},
		{
			name: "Workload redundant update",
			events: []defs.Probe2StoreMsg{
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(existingWorkload, nil)
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
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
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				mWorkload.EXPECT().Get(gomock.Any(), expMeta).Return(existingWorkload, nil)
				mWorkload.EXPECT().Delete(gomock.Any(), expMeta).Return(nil, nil)
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
											Mac: "aaaa:bbbb:cccc",
										},
									},
								},
							},
						},
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
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
								MACAddress: "aaaa:bbbb:cccc",
							},
						},
					},
				}
				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mHost.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
				mHost.EXPECT().Create(gomock.Any(), expHost).Return(nil, nil)
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
											Mac: "aaaa:bbbb:dddd",
										},
									},
								},
							},
						},
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
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
								MACAddress: "aaaa:bbbb:cccc",
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
								MACAddress: "aaaa:bbbb:dddd",
							},
						},
					},
				}
				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				mHost.EXPECT().Get(gomock.Any(), expMeta).Return(existingHost, nil)
				mHost.EXPECT().Update(gomock.Any(), expHost).Return(nil, nil)
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
											Mac: "aaaa:bbbb:cccc",
										},
									},
								},
							},
						},
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
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
								MACAddress: "aaaa:bbbb:cccc",
							},
						},
					},
				}
				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				mHost.EXPECT().Get(gomock.Any(), expMeta).Return(existingHost, nil)
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
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
								MACAddress: "aaaa:bbbb:cccc",
							},
						},
					},
				}
				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				mHost.EXPECT().Get(gomock.Any(), expMeta).Return(existingHost, nil)
				mHost.EXPECT().Delete(gomock.Any(), expMeta).Return(nil, nil)
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
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.NotFound), "", nil)
				mHost.EXPECT().Get(gomock.Any(), expMeta).Return(nil, err)
			},
		},
		// Negative cases
		{
			name: "5XX error",
			events: []defs.Probe2StoreMsg{
				{
					VcObject:   defs.VirtualMachine,
					Key:        "virtualmachine-41",
					Originator: "127.0.0.1:8990",
				},
				{
					VcObject:   defs.HostSystem,
					Key:        "hostsystem-41",
					Originator: "127.0.0.1:8990",
					Changes: []types.PropertyChange{
						types.PropertyChange{
							Op:   types.PropertyChangeOpAdd,
							Name: "config",
						},
					},
				},
			},
			setupMock: func(mockApicl *mocks.MockAPIClient) {
				expWorkloadMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-virtualmachine-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				expHostMeta := &api.ObjectMeta{
					Name:      "127.0.0.1:8990-hostsystem-41",
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				workloadGroup := mockApicl.WorkloadV1().(*mocks.MockWorkloadV1)
				mWorkload := workloadGroup.MWorkload.(*mockapi.MockWorkloadV1WorkloadInterface)
				err := apierrors.ToGrpcError("missing object", []string{"Object not found"}, int32(codes.Unavailable), "", nil)
				mWorkload.EXPECT().Get(gomock.Any(), expWorkloadMeta).Return(nil, err)

				clusterGroup := mockApicl.ClusterV1().(*mocks.MockClusterV1)
				mHost := clusterGroup.MHost.(*mockapi.MockClusterV1HostInterface)
				mHost.EXPECT().Get(gomock.Any(), expHostMeta).Return(nil, err)
			},
		},
	}

	ctx, cancelFn := context.WithCancel(context.Background())

	store := &VCHStore{
		ctx:      ctx,
		Log:      logger,
		stateMgr: &statemgr.Statemgr{},
	}

	for _, tc := range testCases {
		if len(forceTestName) != 0 && tc.name != forceTestName {
			continue
		}
		t.Logf("running %s", tc.name)
		ctrl := gomock.NewController(t)

		mWorkload := mockapi.NewMockWorkloadV1WorkloadInterface(ctrl)
		mWorkloadGroup := &mocks.MockWorkloadV1{
			MWorkload: mWorkload,
		}
		mHost := mockapi.NewMockClusterV1HostInterface(ctrl)
		mClusterGroup := &mocks.MockClusterV1{
			MHost: mHost,
		}
		mockApicl := &mocks.MockAPIClient{
			AWorkloadV1: mWorkloadGroup,
			AClusterV1:  mClusterGroup,
		}
		store.stateMgr.SetAPIClient(mockApicl)
		inbox := make(chan defs.Probe2StoreMsg)
		store.inbox = inbox
		tc.setupMock(mockApicl)
		store.Run()
		// Push events
		for _, e := range tc.events {
			inbox <- e
		}
		// Time for events to process
		time.Sleep(10 * time.Millisecond)
		ctrl.Finish()
	}

	// Terminating store instance
	cancelFn()
	doneCh := make(chan bool)
	go func() {
		store.WaitForExit()
		doneCh <- true
	}()
	select {
	case <-doneCh:
	case <-time.After(1 * time.Second):
		t.Fatalf("Store failed to shutdown within timeout")
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
