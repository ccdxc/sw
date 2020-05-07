package vchub

import (
	"context"
	"fmt"
	"net"
	"net/url"
	"os"
	"sync"
	"testing"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/hooks/apiserver/utils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	orchutils "github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Used later for mocking migration actions
type actionInput func(l log.Logger, oldObj, in *workload.Workload) (*workload.Workload, error)

type actionOutput func(in *workload.Workload) (*workload.Workload, error)

func TestVmotion(t *testing.T) {
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("vmotion_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	// SETTING UP STATE MANAGER
	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	// CREATING ORCH CONFIG
	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	err = sm.Controller().Orchestrator().Create(orchConfig)

	// SETTING UP VCSIM
	vcURL := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	vcURL.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: vcURL.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	// SETTING UP MOCK
	// Real probe that will be used by mock probe when possible
	vchub := setupTestVCHub(vcURL, sm, orchConfig, logger)
	vcp := vcprobe.NewVCProbe(vchub.vcReadCh, vchub.vcEventCh, vchub.State)
	mockProbe := mock.NewProbeMock(vcp)
	vchub.probe = mockProbe
	mockProbe.Start(false)
	AssertEventually(t, func() (bool, interface{}) {
		if !mockProbe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")
	defer vchub.Destroy(false)

	// Add DC
	dc, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	// Add it using vcHub so mockProbe gets the needed info ???
	// This will also create PenDVS
	logger.Infof("Creating PenDC for %s\n", dc.Obj.Reference().Value)
	_, err = vchub.NewPenDC(defaultTestParams.TestDCName, dc.Obj.Self.Value)
	// Add DVS
	dvsName := CreateDVSName(defaultTestParams.TestDCName)
	dvs, ok := dc.GetDVS(dvsName)
	if !ok {
		logger.Info("GetDVS Failed")
		os.Exit(1)
	}
	Assert(t, ok, "failed dvs create")

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	// Create one PG for vmkNic
	pgConfigSpec := []types.DVPortgroupConfigSpec{
		types.DVPortgroupConfigSpec{
			Name:     CreatePGName("vMotion_PG"),
			NumPorts: 8,
			DefaultPortConfig: &types.VMwareDVSPortSetting{
				Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
					PvlanId: int32(100),
				},
			},
		},
	}

	smmock.CreateNetwork(sm, "default", "vMotion_PG", "11.1.1.0/24", "11.1.1.1", 500, nil, orchInfo1)
	// Add PG to mockProbe (this is weird, this should be part of sim)
	// vcHub should provide this function ??
	mockProbe.AddPenPG(defaultTestParams.TestDCName, dvsName, &pgConfigSpec[0], nil, 1)
	pg, err := mockProbe.GetPenPG(defaultTestParams.TestDCName, CreatePGName("vMotion_PG"), 1)
	AssertOk(t, err, "failed to add portgroup")

	logger.Infof("===== Pre-Sync Initial config =====")
	vchub.sync()
	// Create Host1 (pensando)
	penHost1, err := dc.AddHost("host1")
	AssertOk(t, err, "failed to add Host to DC")
	err = dvs.AddHost(penHost1)
	AssertOk(t, err, "failed to add Host to DVS")

	// Create Host2 (pensndo)
	penHost2, err := dc.AddHost("host2")
	AssertOk(t, err, "failed to add Host to DC")
	err = dvs.AddHost(penHost2)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	// Make it Pensando host
	penHost1.ClearNics()
	err = penHost1.AddNic("vmnic0", conv.MacString(pNicMac))
	AssertOk(t, err, "failed to add pNic")

	pNicMac2 := append(createPenPnicBase(), 0xcc, 0x00, 0x00)
	// Make it Pensando host
	penHost2.ClearNics()
	err = penHost2.AddNic("vmnic0", conv.MacString(pNicMac2))
	AssertOk(t, err, "failed to add pNic")

	penHost1.ClearVmkNics()
	penHost2.ClearVmkNics()

	// Create a VM on host1
	vm1, err := dc.AddVM("vm1", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aa:aa:bb:bb:dd:dd",
			PortgroupKey: pg.Reference().Value,
			PortKey:      "11",
		},
	})
	AssertOk(t, err, "Failed to create vm1")

	logger.Infof("===== Sync Initial config =====")
	vchub.sync()

	// Verify that VM is on host1 by getting workload for the vm

	testWorkloads := map[string]string{} // [workload] = host

	vmName := vchub.createVMWorkloadName(dc.Obj.Self.Value, vm1.Reference().Value)
	host1Name := vchub.createHostName(dc.Obj.Self.Value, penHost1.Obj.Self.Value)
	testWorkloads[vmName] = host1Name

	// Add Validations
	// Check that workload for the host with its vmknics as EPs is created
	verifyVMWorkloads := func(testWls map[string]string, errMsg string) {
		AssertEventually(t, func() (bool, interface{}) {
			opts := api.ListWatchOptions{}
			wls, err := sm.Controller().Workload().List(context.Background(), &opts)
			if err != nil {
				logger.Infof("Cannot get workloads - err %s", err)
				return false, nil
			}
			if len(wls) != len(testWls) {
				logger.Infof("Got %d workloads, Expected %d", len(wls), len(testWls))
				return false, nil
			}
			for testVMName := range testWls {
				meta := &api.ObjectMeta{
					Name:      testVMName,
					Tenant:    "default",
					Namespace: "default",
				}
				_, err := sm.Controller().Workload().Find(meta)
				if err != nil {
					return false, nil
				}
			}
			return true, nil
		}, errMsg, "1s", "10s")
	}
	verifyVMWorkloads(testWorkloads, "VM not found on host1")

	// move VM to host2 - build the event and call event receiver directly

	vchub.Wg.Add(1)
	go vchub.startEventsListener()

	logger.Infof("===== Move VM to host2 =====")

	startMsg1 := defs.VMotionStartMsg{
		VMKey:        vm1.Self.Value,
		DstHostKey:   penHost2.Obj.Self.Value,
		DcID:         dc.Obj.Self.Value,
		HotMigration: false,
	}
	m := defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}

	vchub.handleVCNotification(m)

	startMsg1.HotMigration = true
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	err = dc.UpdateVMHost(vm1, "host2")
	AssertOk(t, err, "VM host update failed")
	// vcsim does not send events ???
	// XXX call handleVM() directly
	vchub.sync()

	doneMsg1 := defs.VMotionDoneMsg{
		VMKey:      vm1.Self.Value,
		SrcHostKey: penHost1.Obj.Self.Value,
		DcID:       dc.Obj.Self.Value,
	}
	m = defs.VCNotificationMsg{
		Type: defs.VMotionDone,
		Msg:  doneMsg1,
	}
	vchub.handleVCNotification(m)

	// Send erroneous vmotion start events and test that vm does not move
	// send start to host2 again
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)
	// use DstHost name similar to EventEx
	startMsg1.DstHostKey = ""
	startMsg1.DstHostName = "junkHost"
	startMsg1.DstDcName = defaultTestParams.TestDCName
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	startMsg1.DstHostName = ""
	startMsg1.DstDcName = defaultTestParams.TestDCName
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	startMsg1.DstHostName = "junkHost"
	startMsg1.DstDcName = "junkDC"
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	startMsg1.DstHostName = ""
	startMsg1.DstDcName = ""
	startMsg1.DcID = "junkDC"
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)
	startMsg1.DcID = ""
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	// Verify that VM is on host2 by getting workload for the vm
	host2Name := vchub.createHostName(dc.Obj.Self.Value, penHost2.Obj.Self.Value)
	testWorkloads[vmName] = host2Name
	verifyVMWorkloads(testWorkloads, "VM not found on host2")

	logger.Infof("===== Move VM to host1 and Abort migration =====")
	// Generate migration start event
	startMsg1.VMKey = vm1.Self.Value
	startMsg1.DstHostKey = penHost1.Obj.Self.Value
	startMsg1.DcID = dc.Obj.Self.Value
	startMsg1.HotMigration = true
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)

	// XXX Workload object should show on host1 (spec not status) - actions not implemented in local env
	failedMsg1 := defs.VMotionFailedMsg{
		VMKey:      vm1.Self.Value,
		DstHostKey: penHost2.Obj.Self.Value,
		DcID:       dc.Obj.Self.Value,
		Reason:     "Testing",
	}
	m = defs.VCNotificationMsg{
		Type: defs.VMotionFailed,
		Msg:  failedMsg1,
	}
	vchub.handleVCNotification(m)

	// work should be back on host2

	// VcSim BUG:
	// In vcsim there is some problem in setting up the pnics for host.. all hosts are getting
	// affected as pnics are added to any one host. That makes host3 also pensdo host.
	// for now remove pensando nic from host to run this test further

	// Create Host3 (non-pensndo)
	host3, err := dc.AddHost("host3")
	AssertOk(t, err, "failed to add Host to DC")
	err = dvs.AddHost(host3)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac3 := net.HardwareAddr{}
	pNicMac3 = append(pNicMac3, 0x00)
	pNicMac3 = append(pNicMac3, 0x01)
	pNicMac3 = append(pNicMac3, 0x02)
	pNicMac3 = append(pNicMac3, 0xcc)
	pNicMac3 = append(pNicMac3, 0x00)
	pNicMac3 = append(pNicMac3, 0x00)

	host3.ClearNics()
	err = host3.AddNic("vmnic0", conv.MacString(pNicMac3))
	AssertOk(t, err, "failed to add pNic")

	// move VM to host3, non-pensando host
	logger.Infof("===== Move VM to non-pensando host3 =====")
	startMsg1.VMKey = vm1.Self.Value
	startMsg1.DstHostKey = host3.Obj.Self.Value
	startMsg1.DcID = dc.Obj.Self.Value
	startMsg1.HotMigration = true
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)
	err = dc.UpdateVMHost(vm1, "host3")
	AssertOk(t, err, "VM host update failed")
	vchub.sync()
	delete(testWorkloads, vmName)
	verifyVMWorkloads(testWorkloads, "VM not removed")

	logger.Infof("===== Move VM from non-pensando host3 to pensando host =====")
	startMsg1.VMKey = vm1.Self.Value
	startMsg1.DstHostKey = penHost1.Obj.Self.Value
	startMsg1.DcID = dc.Obj.Self.Value
	startMsg1.HotMigration = true
	m = defs.VCNotificationMsg{
		Type: defs.VMotionStart,
		Msg:  startMsg1,
	}
	vchub.handleVCNotification(m)
	err = dc.UpdateVMHost(vm1, "host1")
	AssertOk(t, err, "VM host update failed")
	vchub.sync()
	testWorkloads[vmName] = host1Name
	verifyVMWorkloads(testWorkloads, "VM not found")

}

// Tests vmotion by triggering watch events
func TestVmotionWithWatchers(t *testing.T) {
	var vchub *VCHub
	var s *sim.VcSim
	testCtx, cancel := context.WithCancel(context.Background())
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("vmotion_test_with_watchers")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	defer func() {
		logger.Infof("Tearing Down")
		cancel()
		if vchub != nil {
			vchub.Destroy(false)
		}
		if s != nil {
			s.Destroy()
		}
	}()

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	dc2, err := s.AddDC("DC2")
	AssertOk(t, err, "failed dc create")

	// SETTING UP STATE MANAGER
	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	RegisterMigrationActions(sm, logger)

	// CREATING CONFIG
	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{orchutils.ManageAllDcs}
	orchConfig.Status.OrchID = 1

	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "failed to create orch config")

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
		{
			Name:      orchConfig.Name,
			Namespace: "DC2",
		},
	}
	_, err = smmock.CreateNetwork(sm, "default", "n1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	AssertOk(t, err, "Failed to create network")

	// Create VC objects (network, host, 2 vm)
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)
	dvs1, err := dc1.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")

	pvlanConfigSpecArray = testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec = testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)
	dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name = CreateDVSName("DC2")
	dvs2, err := dc2.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")

	orchID := fmt.Sprintf("orch%d", orchConfig.Status.OrchID)
	state := defs.State{
		VcURL:        u,
		OrchID:       orchID,
		VcID:         "VCProbe",
		Ctx:          testCtx,
		Log:          logger.WithContext("submodule", "vcprobe"),
		Wg:           &sync.WaitGroup{},
		ForceDCNames: map[string]bool{orchutils.ManageAllDcs: true},
		DcIDMap:      map[string]types.ManagedObjectReference{},
		DvsIDMap:     map[string]types.ManagedObjectReference{},
		OrchConfig:   orchConfig,
	}

	vcp := vcprobe.NewVCProbe(nil, nil, &state)
	mockProbe := mock.NewProbeMock(vcp)
	mockProbe.Start(true)
	AssertEventually(t, func() (bool, interface{}) {
		if !mockProbe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	spec := testutils.GenPGConfigSpec(CreatePGName("n1"), 2, 3)
	err = mockProbe.AddPenPG(dc1.Obj.Name, dvs1.Obj.Name, &spec, nil, defaultRetryCount)
	AssertOk(t, err, "failed to create pg")
	pg, err := mockProbe.GetPenPG(defaultTestParams.TestDCName, CreatePGName("n1"), 1)
	AssertOk(t, err, "Failed to get pg")

	spec = testutils.GenPGConfigSpec(CreatePGName("n1"), 2, 3)
	err = mockProbe.AddPenPG(dc2.Obj.Name, dvs2.Obj.Name, &spec, nil, defaultRetryCount)
	AssertOk(t, err, "failed to create pg")
	pg2, err := mockProbe.GetPenPG("DC2", CreatePGName("n1"), 1)
	AssertOk(t, err, "Failed to get pg")

	// Create Host1 (pensando)
	penHost1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed to add Host to DC")
	penHost1.Obj.Name = "host1"
	err = dvs1.AddHost(penHost1)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	// Make it Pensando host
	err = penHost1.AddNic("vmnic0", conv.MacString(pNicMac))
	AssertOk(t, err, "failed to add pNic")

	// Create Host2 (pensando)
	penHost2, err := dc1.AddHost("host2")
	AssertOk(t, err, "failed to add Host to DC")
	penHost2.Obj.Name = "host2"
	err = dvs1.AddHost(penHost2)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac2 := append(createPenPnicBase(), 0xcc, 0x00, 0x00)
	// Make it Pensando host
	err = penHost2.AddNic("vmnic0", conv.MacString(pNicMac2))
	AssertOk(t, err, "failed to add pNic")

	// Create Host3 (pensando) on DC2
	penHost3, err := dc2.AddHost("host3")
	AssertOk(t, err, "failed to add Host to DC")
	penHost3.Obj.Name = "host3"
	err = dvs2.AddHost(penHost3)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac3 := append(createPenPnicBase(), 0xdd, 0x00, 0x00)
	// Make it Pensando host
	err = penHost3.AddNic("vmnic0", conv.MacString(pNicMac3))
	AssertOk(t, err, "failed to add pNic")

	penHost1.ClearVmkNics()
	penHost2.ClearVmkNics()
	penHost3.ClearVmkNics()

	// Create a VM on host1
	vm1, err := dc1.AddVM("vm1", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aa:aa:bb:bb:dd:dd",
			PortgroupKey: pg.Reference().Value,
			PortKey:      "11",
		},
	})
	AssertOk(t, err, "Failed to create vm1")

	logger.Infof("===== Launch VCHub =====")
	vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
	wlName := vchub.createVMWorkloadName("", vm1.Self.Value)
	deleteVM := func(sendEvent bool) {
		// VM must be back on host in DC1 for deletions to work
		// Move host back to host1 (cold migrate)
		err = dc1.UpdateVMHost(vm1, "host1")
		AssertOk(t, err, "VM host update failed")
		err := dc1.DeleteVM(vm1)
		AssertOk(t, err, "Failed to delete vm")
		if sendEvent {
			m := defs.Probe2StoreMsg{
				MsgType: defs.VCEvent,
				Val: defs.VCEventMsg{
					VcObject:   defs.VirtualMachine,
					Key:        vm1.Self.Value,
					DcID:       dc1.Obj.Self.Value,
					DcName:     dc1.Obj.Name,
					Originator: vchub.VcID,
					Changes:    []types.PropertyChange{},
					UpdateType: types.ObjectUpdateKindLeave,
				},
			}
			vchub.vcReadCh <- m
		}
	}

	// Build helper functions for the test cases
	waitForWatch := func() {
		AssertEventually(t, func() (bool, interface{}) {
			if vchub.AreWatchersStarted() {
				return true, nil
			}
			return false, fmt.Errorf("Watchers have not been started yet")
		}, "Watchers have not been started yet", "1s", "20s")
	}

	waitVMReady := func() {
		logger.Infof("---- waiting for VM to be ready ----")
		// Wait for vm to be present in vchub
		AssertEventually(t, func() (bool, interface{}) {
			meta := &api.ObjectMeta{
				Name: vchub.createVMWorkloadName("", vm1.Self.Value),
				// TODO: Don't use default tenant
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
			}

			_, err := sm.Controller().Workload().Find(meta)
			if err != nil {
				return false, err
			}
			return true, nil
		}, "workload not found")
	}

	vMotionStart := func(dstHost *sim.Host, dstDC *sim.Datacenter) {
		logger.Infof("---- Starting Vmotion ----")
		vchub.vcEventCh <- createVmotionStartEvent(vm1.Self.Value, dstHost.Obj.Self.Value, dstDC.Obj.Self.Value)
	}

	addVnic := func(dstDC *sim.Datacenter, isAcrossDC bool) {
		logger.Infof("---- Adding vnic ----")
		var pgObj *object.DistributedVirtualPortgroup
		port := "11"
		if isAcrossDC {
			pgObj = pg2
			port = "21"
			err := dc1.RemoveVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "11",
			})
			AssertOk(t, err, "Failed to add vnic")

			// Update vnics to new DC pg
			err = dc1.AddVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg2.Reference().Value,
				PortKey:      "21", // change port to check stale port is removed
			})
			AssertOk(t, err, "Failed to add vnic")
		} else {
			pgObj = pg
		}

		dc1.AddVnic(vm1, sim.VNIC{
			MacAddress:   "aa:aa:bb:bb:ee:ee",
			PortgroupKey: pgObj.Reference().Value,
			PortKey:      "12",
		})

		guestNic, err := dc1.UpdateVMIP(vm1, "aa:aa:bb:bb:ee:ee", CreatePGName("n1"), []string{"1.1.1.1"})
		AssertOk(t, err, "Failed to update VM IP")

		// Send event since watch does not work
		vchub.vcReadCh <- createVnicUpdateEvent(vchub.OrchID, dstDC.Obj.Name, dstDC.Obj.Self.Value, vm1.Self.Value, guestNic,
			generateVNIC("aa:aa:bb:bb:dd:dd", port, pgObj.Reference().Value, "E1000"),
			generateVNIC("aa:aa:bb:bb:ee:ee", "12", pgObj.Reference().Value, "E1000"))
	}

	vmConfigUpdate := func(dstHost *sim.Host, dstDC *sim.Datacenter, isAcrossDC bool) {
		logger.Infof("---- updating VM Config ----")
		err = dstDC.UpdateVMHost(vm1, dstHost.Obj.Name)
		AssertOk(t, err, "VM host update failed")
		ev := createVMHostUpdateEvent(vchub.OrchID, dstDC.Obj.Name, dstDC.Obj.Self.Value, vm1.Self.Value, dstHost.Obj.Self.Value)
		vchub.vcReadCh <- ev
	}

	vmAbort := func(dstHost *sim.Host, dstDC *sim.Datacenter, isAcrossDC bool) {
		logger.Infof("---- Aborting VM ----")
		if isAcrossDC {
			// Set vnics back - blanket remove all and add all
			dstDC.RemoveVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "11",
			})
			// AssertOk(t, err, "failed to add vnic")
			dstDC.RemoveVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:ee:ee",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "12",
			})
			// AssertOk(t, err, "failed to add vnic")
			dstDC.RemoveVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg2.Reference().Value,
				PortKey:      "11",
			})
			// AssertOk(t, err, "failed to add vnic")
			dstDC.RemoveVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:ee:ee",
				PortgroupKey: pg2.Reference().Value,
				PortKey:      "12",
			})
			// AssertOk(t, err, "failed to add vnic")

			// Update vnics to new DC pg
			err = dc1.AddVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "11",
			})
			AssertOk(t, err, "failed to add vnic")
			err = dc1.AddVnic(vm1, sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:ee:ee",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "12",
			})
			AssertOk(t, err, "failed to add vnic")
		}

		vchub.vcEventCh <- createVmotionAbortEvent(vm1.Self.Value, dstHost.Obj.Self.Value, dstDC.Obj.Self.Value)
	}

	updateMigrationStatus := func(status string) {
		logger.Infof("---- updating migration status to %s ----", status)
		meta := &api.ObjectMeta{
			Name: vchub.createVMWorkloadName("", vm1.Self.Value),
			// TODO: Don't use default tenant
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		}
		wl, err := sm.Controller().Workload().Find(meta)
		AssertOk(t, err, "failed to get workload")
		wlObj := wl.Workload
		newWl := ref.DeepCopy(wlObj).(workload.Workload)
		newWl.Status.MigrationStatus.Status = status
		sm.Controller().Workload().SyncUpdate(&newWl)
	}

	verifyWorkload := func(specHost *sim.Host, statusHost *sim.Host, numVnics int, isAcrossDC bool) {
		logger.Infof("---- Verifying workload ----")
		AssertEventually(t, func() (bool, interface{}) {
			meta := &api.ObjectMeta{
				Name: vchub.createVMWorkloadName("", vm1.Self.Value),
				// TODO: Don't use default tenant
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
			}

			specHostName := ""
			statusHostName := ""
			if specHost != nil {
				specHostName = vchub.createHostName("", specHost.Obj.Self.Value)
			}
			if statusHost != nil {
				statusHostName = vchub.createHostName("", statusHost.Obj.Self.Value)
			}

			wl, err := sm.Controller().Workload().Find(meta)
			if err != nil {
				return false, err
			}
			if len(wl.Spec.Interfaces) != numVnics {
				return false, fmt.Errorf("expected %d interfaces, found %d", numVnics, len(wl.Spec.Interfaces))
			}
			if numVnics == 2 {
				// 2nd vnic should have IP info
				found := false
				for _, inf := range wl.Spec.Interfaces {
					if len(inf.IpAddresses) != 0 {
						found = true
					}
				}
				if !found {
					return false, fmt.Errorf("expected one of the interfaces to have an IP")
				}
				if isAcrossDC && statusHost != nil {
					port := "11"
					if statusHost.Obj.Name != "host1" {
						port = "21"
					}
					// port override should be on port 21
					probe := vchub.probe.(*mock.ProbeMock)
					config := probe.DvsStateMap["DC2"][CreateDVSName("DC2")]
					if _, ok := config[port]; !ok {
						return false, fmt.Errorf("No vlan override was set on portkey %s", port)
					}
				}
			}
			if wl.Spec.HostName != specHostName {
				return false, fmt.Errorf("wl spec not on correct host, expected %s but found %s", specHostName, wl.Spec.HostName)
			}
			if wl.Status.HostName != statusHostName {
				return false, fmt.Errorf("wl status not on correct host, expected %s but found %s", statusHostName, wl.Status.HostName)
			}
			return true, nil
		}, "workload verify failed")
	}

	recreateVM := func() {
		logger.Infof("---- recreating VM ----")
		vnics := []sim.VNIC{
			sim.VNIC{
				MacAddress:   "aa:aa:bb:bb:dd:dd",
				PortgroupKey: pg.Reference().Value,
				PortKey:      "11",
			},
		}
		vm1, err = dc1.AddVM("vm1", "host1", vnics)
		AssertOk(t, err, "Failed to create vm1")

		// Reset port overrides
		probe := vchub.probe.(*mock.ProbeMock)
		config := probe.DvsStateMap["DC2"][CreateDVSName("DC2")]
		ports := []string{}
		for key := range config {
			ports = append(ports, key)
		}
		for _, key := range ports {
			delete(config, key)
		}

		vchub.vcReadCh <- createVMEvent(dc1.Obj.Name, dc1.Obj.Self.Value, "vm1", vm1.Self.Value, penHost1.Obj.Self.Value, vnics)
		wlName = vchub.createVMWorkloadName("", vm1.Self.Value)
	}

	waitVMReady()
	testCases := func(dstHost *sim.Host, dstDC *sim.Datacenter) {
		isAcrossDC := dstDC.Obj.Name != defaultTestParams.TestDCName
		{
			/** ---------- VMotion Happy case ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. Datapath says completed
			4. VM migration completed
			*/

			logger.Infof("===== Test: VMotion Happy case - AcrossDC: %v =====", isAcrossDC)

			// Start migration
			vMotionStart(dstHost, dstDC)
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

			// Add vnic, should be ignored since we are migrating
			addVnic(dstDC, isAcrossDC)

			// Trigger config update
			vmConfigUpdate(dstHost, dstDC, isAcrossDC)

			// Check Migration final sync
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			// Update data path
			updateMigrationStatus(statusDone)

			// Check finish migration
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationDone)

			// Now that we are done migrating, verify new vnic was added
			verifyWorkload(dstHost, nil, 2, isAcrossDC)

			// CLEANUP
			vchub.Destroy(false)
			deleteVM(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
		}

		{
			/** ---------- VMotion Happy abort ----------
			1. VM moves from h1 -> h2
			2. VM abort
			3. Datapath says failed
			4. VM migration completed
			*/
			logger.Infof("===== Test: VMotion Happy abort - AcrossDC: %v =====", isAcrossDC)

			// Start migration
			vMotionStart(dstHost, dstDC)

			// Add vnic, should be ignored since we are migrating
			addVnic(dstDC, isAcrossDC)

			// Check Migration Start
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			// Trigger abort
			vmAbort(dstHost, dstDC, isAcrossDC)

			// Check Migration abort
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationAbort)

			// Update data path to failed
			updateMigrationStatus(statusFailed)

			// Now that we are done migrating, verify new vnic was added
			verifyWorkload(penHost1, nil, 2, isAcrossDC)

			// CLEANUP
			// remove vnic, move back vm
			vchub.Destroy(false)
			deleteVM(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
			waitVMReady()
		}

		{
			/** ---------- VMotion datapath fail ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. Datapath says failed
			4. VM migration completed
			*/

			logger.Infof("===== Test: VMotion datapath fail - AcrossDC: %v =====", isAcrossDC)

			// Start migration
			vMotionStart(dstHost, dstDC)

			// Check Migration Start
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

			// Add vnic, should be ignored since we are migrating
			addVnic(dstDC, isAcrossDC)

			// Trigger config update
			vmConfigUpdate(dstHost, dstDC, isAcrossDC)

			// Check Migration final sync
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			// Update data path
			updateMigrationStatus(statusFailed)

			// Check finish migration
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationDone)

			// Now that we are done migrating, verify new vnic was added
			verifyWorkload(dstHost, nil, 2, isAcrossDC)

			// CLEANUP
			vchub.Destroy(false)
			deleteVM(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
			waitVMReady()
		}

		{
			/** ---------- VMotion datapath timeout (missed vm Complete) -------
			1. VM moves from h1 -> h2
			2. Datapath says timeout
			4. VM gets resynced
			*/

			logger.Infof("===== Test: VMotion datapath timeout - AcrossDC: %v =====", isAcrossDC)

			// Start migration
			vMotionStart(dstHost, dstDC)

			// Check Migration Start
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

			// Add vnic, should be ignored since we are migrating
			// isAcrossDC is always false here as
			// we won't finish migration to dest host
			addVnic(dc1, false)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			// Update data path
			updateMigrationStatus(statusTimedOut)

			// Check finish migration
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationDone)

			// Now that we are done migrating, verify new vnic was added
			// verify we are on the old host
			verifyWorkload(penHost1, nil, 2, isAcrossDC)

			// CLEANUP
			vchub.Destroy(false)
			deleteVM(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
			waitVMReady()
		}

		{
			/** ---------- VMotion delete before datapath finishes ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. VM deleted
			*/

			logger.Infof("===== Test: VMotion delete before datapath finishes - AcrossDC: %v =====", isAcrossDC)
			vMotionStart(dstHost, dstDC)
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

			addVnic(dstDC, isAcrossDC)

			vmConfigUpdate(dstHost, dstDC, isAcrossDC)

			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			deleteVM(true)

			AssertEventually(t, func() (bool, interface{}) {
				meta := &api.ObjectMeta{
					Name: vchub.createVMWorkloadName("", vm1.Self.Value),
					// TODO: Don't use default tenant
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}

				workload, err := sm.Controller().Workload().Find(meta)
				if !isAcrossDC && err == nil {
					return false, fmt.Errorf("Expected workload to be deleted, found %v", workload)
				} else if isAcrossDC && err != nil {
					return false, fmt.Errorf("Expected workload to be present since we should have ignored DC delete event")
				}
				return true, nil
			}, "workload should have been deleted")

			// CLEANUP - recreate workload on host1
			vchub.Destroy(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
			waitVMReady()
		}

		{
			/** ---------- Across DC, host update comes before valid VNIC info ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. VM update vnics to correct info
			4. VM now moves to final sync
			3. Datapath says completed
			4. VM migration completed
			*/

			if isAcrossDC {
				logger.Infof("===== Test: VMotion across DC with host update before valid VNICs =====")

				// Start migration
				vMotionStart(dstHost, dstDC)
				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

				// Trigger config update
				vmConfigUpdate(dstHost, dstDC, isAcrossDC)

				// Check Migration still in start
				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

				// VCSim will crash if we attempt to query info about the migrating VM. Can't test updating vnics

				// CLEANUP
				vchub.Destroy(false)
				deleteVM(false)
				recreateVM()
				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()
				waitVMReady()
			}
		}

		/** ------------- RESTART CASES --------------- */

		{
			/** ---------- Restart during host config update ----------
			1. VM moves from h1 -> h2
			2. Kill VCHub
			3. VM update host config
			4. VCHub comes back, final sync should start
			5. Kill VCHub
			6. Datapath says completed
			7. VCHub comes back, finish migration called
			8. VM migration completed
			*/
			if !isAcrossDC { // Doesn't work with sim as inventory watches think VM is in the wrong DC when the watch restarts
				logger.Infof("===== Test: Restart during host config update - AcrossDC: %v =====", isAcrossDC)
				vMotionStart(dstHost, dstDC)
				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

				addVnic(dstDC, isAcrossDC)
				vchub.Destroy(false)

				vmConfigUpdate(dstHost, dstDC, isAcrossDC)

				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()

				verifyWorkload(dstHost, penHost1, 1, isAcrossDC) // Verify sync isn't writing any state

				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)

				vchub.Destroy(false)

				updateMigrationStatus(statusDone)

				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()
				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationDone)

				verifyWorkload(dstHost, nil, 2, isAcrossDC)

				// CLEANUP
				vchub.Destroy(false)
				deleteVM(false)
				recreateVM()
				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()
				waitVMReady()
			}
		}

		{
			/** ---------- Restart during data path failure ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. Kill VCHub
			4. Datapath says failure
			5. Start VCHub
			6. VCHub comes back, finish migration called
			8. VM migration completed
			*/
			if !isAcrossDC { // Doesn't work with sim as inventory watches think VM is in the wrong DC when the watch restarts
				logger.Infof("===== Test: Restart during data path failure - AcrossDC: %v =====", isAcrossDC)
				vMotionStart(dstHost, dstDC)
				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

				addVnic(dstDC, isAcrossDC)

				vmConfigUpdate(dstHost, dstDC, isAcrossDC)

				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)
				verifyWorkload(dstHost, penHost1, 1, isAcrossDC) // Verify sync isn't writing any state

				vchub.Destroy(false)

				updateMigrationStatus(statusFailed)

				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()

				checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationDone)

				verifyWorkload(dstHost, nil, 2, isAcrossDC)
				// CLEANUP
				vchub.Destroy(false)
				deleteVM(false)
				recreateVM()
				vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
				waitForWatch()
				waitVMReady()
			}
		}

		{
			/** ---------- VM deleted after VMotion with restart ----------
			1. VM moves from h1 -> h2
			2. VM update host config
			3. Kill VCHub
			4. Datapath says completed
			5. Delete VM in vcenter
			6. VCHub comes back, vm should be deleted
			*/
			logger.Infof("===== Test: VM deleted after VMotion with restart - AcrossDC: %v =====", isAcrossDC)
			vMotionStart(dstHost, dstDC)
			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationStart)

			addVnic(dstDC, isAcrossDC)

			vmConfigUpdate(dstHost, dstDC, isAcrossDC)

			checkMigrationState(t, sm, vchub.OrchID, wlName, stageMigrationFinalSync)
			verifyWorkload(dstHost, penHost1, 1, isAcrossDC)

			vchub.Destroy(false)

			updateMigrationStatus(statusDone)

			deleteVM(false)

			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()

			AssertEventually(t, func() (bool, interface{}) {
				meta := &api.ObjectMeta{
					Name: vchub.createVMWorkloadName("", vm1.Self.Value),
					// TODO: Don't use default tenant
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}

				workload, err := sm.Controller().Workload().Find(meta)
				if err == nil {
					return false, fmt.Errorf("Expected workload to be deleted, found %v", workload)
				}
				return true, nil
			}, "workload should have been deleted")
			// CLEANUP - recreate workload on host1
			vchub.Destroy(false)
			recreateVM()
			vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)
			waitForWatch()
			waitVMReady()
		}
	}

	waitForWatch()

	logger.Infof("===== Start Tests =====")
	testCases(penHost2, dc1)
	// Commenting out since vcsim starts sending incorrect events when across DC
	// testCases(penHost3, dc2)

	vchub.Destroy(false)
}

func checkMigrationState(t *testing.T, sm *smmock.Statemgr, vcID, wlName, stage string) {
	AssertEventually(t, func() (bool, interface{}) {
		meta := &api.ObjectMeta{
			Name: wlName,
			// TODO: Don't use default tenant
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		}

		obj, err := sm.Controller().Workload().Find(meta)
		if err != nil {
			return false, err
		}
		if obj.Status.MigrationStatus == nil || obj.Status.MigrationStatus.Stage != stage {
			foundStage := ""
			if obj.Status.MigrationStatus != nil {
				foundStage = obj.Status.MigrationStatus.Stage
			}

			return false, fmt.Errorf("Expected migration stage %s, but found %s", stage, foundStage)
		}
		return true, nil
	}, "workload not in correct stage")

}

func createVMHostUpdateEvent(vcID, dcName, dcID, vmID, hostID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dcID,
			DcName:     dcName,
			Key:        vmID,
			Originator: vcID,
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "runtime",
					Val: types.VirtualMachineRuntimeInfo{
						Host: &types.ManagedObjectReference{
							Type:  "HostSystem",
							Value: hostID,
						},
					},
				},
			},
		},
	}
}

func createVnicUpdateEvent(vcID, dcName, dcID, vmID string, nicInfo []types.GuestNicInfo, devices ...types.BaseVirtualDevice) defs.Probe2StoreMsg {
	if nicInfo == nil {
		nicInfo = []types.GuestNicInfo{}
	}

	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dcID,
			DcName:     dcName,
			Key:        vmID,
			Originator: vcID,
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "config",
					Val: types.VirtualMachineConfigInfo{
						Hardware: types.VirtualHardware{
							Device: devices,
						},
					},
				},
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "guest",
					Val: types.GuestInfo{
						Net: nicInfo,
					},
				},
			},
		},
	}
}

func createVmotionStartEvent(vmKey, dstHostKey, dcID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCNotification,
		Val: defs.VCNotificationMsg{
			Type: defs.VMotionStart,
			Msg: defs.VMotionStartMsg{
				VMKey:        vmKey,
				DstHostKey:   dstHostKey,
				DcID:         dcID,
				HotMigration: true,
			},
		},
	}
}

func createVmotionAbortEvent(vmKey, dstHostKey, dcID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCNotification,
		Val: defs.VCNotificationMsg{
			Type: defs.VMotionFailed,
			Msg: defs.VMotionFailedMsg{
				VMKey:      vmKey,
				DstHostKey: dstHostKey,
				DcID:       dcID,
				Reason:     "Testing",
			},
		},
	}
}

// RegisterMigrationActions registers local implementations for migration actions
func RegisterMigrationActions(sm *smmock.Statemgr, logger log.Logger) {
	localAction := func(fn actionInput) actionOutput {
		return func(in *workload.Workload) (*workload.Workload, error) {
			// Get existing
			old, err := sm.Controller().Workload().Find(&in.ObjectMeta)
			if err != nil {
				return nil, err
			}
			// Perform action
			obj, err := fn(logger, &old.Workload, in)
			if err != nil {
				return nil, err
			}
			err = sm.Controller().Workload().Update(obj)
			if err != nil {
				logger.Errorf("Failed to update stateMgr, err %s", err)
			}
			return obj, err
		}
	}

	startMigration := localAction(utils.ProcessStartMigration)
	sm.Controller().Workload().RegisterLocalStartMigrationHandler(startMigration)
	sm.Controller().Workload().RegisterLocalSyncStartMigrationHandler(startMigration)

	abortMigration := localAction(utils.ProcessAbortMigration)
	sm.Controller().Workload().RegisterLocalAbortMigrationHandler(abortMigration)
	sm.Controller().Workload().RegisterLocalSyncAbortMigrationHandler(abortMigration)

	finalSync := localAction(utils.ProcessFinalSyncMigration)
	sm.Controller().Workload().RegisterLocalFinalSyncMigrationHandler(finalSync)
	sm.Controller().Workload().RegisterLocalSyncFinalSyncMigrationHandler(finalSync)

	finishMigration := localAction(utils.ProcessFinishMigration)
	sm.Controller().Workload().RegisterLocalFinishMigrationHandler(finishMigration)
	sm.Controller().Workload().RegisterLocalSyncFinishMigrationHandler(finishMigration)
}
