package vcprobe

import (
	"context"
	"fmt"
	"net"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var defaultTestParams = &testutils.TestParams{
	// TestHostName: "barun-vc.pensando.io",
	// TestUser:     "administrator@pensando.io",
	// TestPassword: "N0isystem$",
	TestHostName: "127.0.0.1:8989",
	TestUser:     "user",
	TestPassword: "pass",

	TestDCName:             "PenTestDC",
	TestDVSName:            "#Pen-DVS-PenTestDC",
	TestPGNameBase:         defs.DefaultPGPrefix,
	TestMaxPorts:           4096,
	TestNumStandalonePorts: 512,
	TestNumPVLANPair:       5,
	StartPVLAN:             500,
	TestNumPG:              5,
	TestNumPortsPerPG:      20,
	TestOrchName:           "test-orchestrator",
}

var retryCount = 1

func TestListAndWatch(t *testing.T) {
	testParams := &testutils.TestParams{
		TestHostName: "127.0.0.1:8989",
		TestUser:     "user",
		TestPassword: "pass",

		TestDCName:             "PenTestDC",
		TestDVSName:            "PenTestDVS",
		TestPGNameBase:         "PenTestPG",
		TestMaxPorts:           4096,
		TestNumStandalonePorts: 512,
		TestNumPVLANPair:       5,
		StartPVLAN:             500,
		TestNumPG:              5,
		TestNumPortsPerPG:      20,
		TestOrchName:           "test-orchestrator",
	}

	err := testutils.ValidateParams(testParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	u := &url.URL{
		Scheme: "https",
		Host:   testParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(testParams.TestUser, testParams.TestPassword)

	config := log.GetDefaultConfig("vcprobe_testDVS")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	eventCh := make(chan defs.Probe2StoreMsg, 24)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC(testParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "failed host create")
	_, err = dc.AddVM("vm1", "host1", []sim.VNIC{})
	AssertOk(t, err, "failed vm create")

	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}
	ctx, cancel := context.WithCancel(context.Background())

	orchConfig := smmock.GetOrchestratorConfig(testParams.TestHostName, testParams.TestUser, testParams.TestPassword)
	err = sm.Controller().Orchestrator().Create(orchConfig)
	state := &defs.State{
		VcURL:      u,
		VcID:       "vcenter",
		Ctx:        ctx,
		Log:        logger,
		StateMgr:   sm,
		OrchConfig: orchConfig,
		Wg:         &sync.WaitGroup{},
		DcIDMap:    map[string]types.ManagedObjectReference{},
		DvsIDMap:   map[string]types.ManagedObjectReference{},
	}
	vcp := NewVCProbe(storeCh, eventCh, state)
	vcp.Start(false)

	defer func() {
		cancel()
		state.Wg.Wait()
	}()
	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready")

	// Start test
	vcp.StartWatchers()
	vcp.StartWatchForDC(testParams.TestDCName, dc.Obj.Reference().Value)

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)
	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				item, ok := m.Val.(defs.VCEventMsg)
				if !ok {
					continue
				}
				eventMap[item.VcObject] = append(eventMap[item.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.Datacenter]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()

	select {
	case <-doneCh:
	case <-time.After(10 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.Logf("%+v", eventMap)
		t.FailNow()
	}

	// Testing list
	s.AddDC("DC2")

	dcs := vcp.ListDC()
	AssertEquals(t, 2, len(dcs), "List DC response length did not match exp")
	vms := vcp.ListVM(nil)
	AssertEquals(t, 1, len(vms), "List VM response length did not match exp")
	// Listing in DC2 should be 0
	for _, dc := range dcs {
		ref := dc.Reference()
		if dc.Name == "DC2" {
			vms := vcp.ListVM(&ref)
			AssertEquals(t, 0, len(vms), "List VM response length did not match exp")
		} else {
			vms := vcp.ListVM(&ref)
			AssertEquals(t, 1, len(vms), "List VM response length did not match exp")
		}
	}
	vm1, err := vcp.GetVM(vms[0].Self.Value, 1)
	AssertOk(t, err, "failed to get vm")
	AssertEquals(t, vm1, vms[0], "VMs were not equal")

	// Test host list
	hosts := vcp.ListHosts(nil)
	AssertEquals(t, 1, len(hosts), "List Host response length did not match exp")

	// Create DVS for listing
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(testParams.TestDCName, dvsCreateSpec, nil, retryCount)
	AssertOk(t, err, "Failed to add DVS")

	time.Sleep(1 * time.Second)

	// There is a bug with the simulator where the type of the object
	// is distributedVirtualSwitch when with a real VCenter it is
	// VmwareDistributedVirtualSwitch
	// // List DVS
	// dvsObjs := vcp.ListDVS(nil)
	// AssertEquals(t, 1, len(dvsObjs), "List DVS response length did not match exp")

	// Create PG for listing

	pgConfigSpecArray := testutils.GenPGConfigSpecArray(testParams, pvlanConfigSpecArray)

	for i := 0; i < testParams.TestNumPG; i++ {
		err = vcp.AddPenPG(testParams.TestDCName, testParams.TestDVSName, &pgConfigSpecArray[i], nil, retryCount)
		AssertOk(t, err, "Failed to add new PG")
	}

	ports := PenDVSPortSettings{
		"1": &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(10),
		},
	}
	// Call will fail, added to increase coverage
	err = vcp.UpdateDVSPortsVlan(testParams.TestDCName, testParams.TestDVSName, ports, retryCount)
	// Don't check err since it is
	// ServerFaultCode: DistributedVirtualSwitch:dvs-28 does not implement: ReconfigureDVPort_Task

	_, err = vcp.GetPenDVSPorts(testParams.TestDCName, testParams.TestDVSName, &types.DistributedVirtualSwitchPortCriteria{}, retryCount)
	AssertOk(t, err, "Failed to get DVS Ports")

	// List PGs

	pgs := vcp.ListPG(nil)
	// 1 extra PG for the uplink PG
	AssertEquals(t, testParams.TestNumPG+1, len(pgs), "List PG response length did not match exp")

}

func TestDVSAndPG(t *testing.T) {
	// TestHostName: "jingyiz-vcsa67new.pensando.io",
	// TestUser:     "administrator@vsphere.local",
	// TestPassword: "N0isystem$",

	testParams := &testutils.TestParams{
		TestHostName: "127.0.0.1:8989",
		TestUser:     "user",
		TestPassword: "pass",

		TestDCName:             "PenTestDC",
		TestDVSName:            "PenTestDVS",
		TestPGNameBase:         "PenTestPG",
		TestMaxPorts:           4096,
		TestNumStandalonePorts: 512,
		TestNumPVLANPair:       5,
		StartPVLAN:             500,
		TestNumPG:              5,
		TestNumPortsPerPG:      20,
		TestOrchName:           "test-orchestrator",
	}

	err := testutils.ValidateParams(testParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	u := &url.URL{
		Scheme: "https",
		Host:   testParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(testParams.TestUser, testParams.TestPassword)

	config := log.GetDefaultConfig("vcprobe_testDVS")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	eventCh := make(chan defs.Probe2StoreMsg, 24)

	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(testParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}
	ctx, cancel := context.WithCancel(context.Background())

	orchConfig := smmock.GetOrchestratorConfig(testParams.TestHostName, testParams.TestUser, testParams.TestPassword)
	err = sm.Controller().Orchestrator().Create(orchConfig)
	state := &defs.State{
		VcURL:      u,
		VcID:       "vcenter",
		Ctx:        ctx,
		Log:        logger,
		StateMgr:   sm,
		OrchConfig: orchConfig,
		Wg:         &sync.WaitGroup{},
		DcIDMap:    map[string]types.ManagedObjectReference{},
		DvsIDMap:   map[string]types.ManagedObjectReference{},
	}
	vcp := NewVCProbe(storeCh, eventCh, state)
	vcp.Start(false)

	defer func() {

		cancel()
		state.Wg.Wait()
	}()

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Trigger the test
	//var mapPortsSetting *PenDVSPortSettings
	// penPGArray := make([]*object.DistributedVirtualPortgroup, testParams.TestNumPG)
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(testParams.TestDCName, dvsCreateSpec, nil, retryCount)
	AssertOk(t, err, "Failed to add DVS")

	// Reconfigure should succeed
	err = vcp.AddPenDVS(testParams.TestDCName, dvsCreateSpec, nil, retryCount)
	AssertOk(t, err, "Failed to add DVS")

	dvsName := testParams.TestDVSName
	dvsObj, err := vcp.GetPenDVS(testParams.TestDCName, dvsName, retryCount)
	AssertOk(t, err, "Failed to get DVS")

	pgConfigSpecArray := testutils.GenPGConfigSpecArray(testParams, pvlanConfigSpecArray)
	// startMicroSegVlanID := testParams.StartPVLAN + int32(testParams.TestNumPVLANPair*2)
	var numPG int
	var mapPGNamesWithIndex *map[string]int

	for i := 0; i < testParams.TestNumPG; i++ {
		err = vcp.AddPenPG(testParams.TestDCName, testParams.TestDVSName, &pgConfigSpecArray[i], nil, retryCount)
		AssertOk(t, err, "Failed to add new PG")

		// Second add will result in reconfigure operation
		err = vcp.AddPenPG(testParams.TestDCName, testParams.TestDVSName, &pgConfigSpecArray[i], nil, retryCount)
		AssertOk(t, err, "Failed to add DVS")

		// Rename objects
		// VCSim doesn't support rename, so we don't check the rror
		vcp.RenamePG(testParams.TestDCName, pgConfigSpecArray[i].Name, "TestPG", retryCount)
		vcp.RenameDVS(testParams.TestDCName, testParams.TestDVSName, "TestDVS", retryCount)
		vcp.RenameDC(testParams.TestDCName, "TestDC", retryCount)

		pgName := fmt.Sprint(testParams.TestPGNameBase, i)
		pgObj, err := vcp.GetPenPG(testParams.TestDCName, pgName, retryCount)
		AssertOk(t, err, "Failed to find created PG %s", pgName)
		Assert(t, pgObj != nil, "Couldn't find created PG %s", pgName)
		pgMo, err := vcp.GetPGConfig(testParams.TestDCName, pgName, []string{"config"}, retryCount)
		AssertOk(t, err, "Failed to find created PG %s as mo ref", pgName)
		Assert(t, pgMo != nil, "Couldn't find created PG %s as mo ref", pgName)
	}

	// Verify state cache behavior
	// Incorrectly populate state. Future opertaions should happen on the dc1 ref
	vcp.State.DcIDMap["DC2"] = dc1.Obj.Self
	vcp.State.DvsIDMap["DVS2"] = dvsObj.Reference()

	dvsObj, err = vcp.GetPenDVS("DC2", "DVS2", retryCount)
	AssertOk(t, err, "Failed to get DVS")
	/*
		for i := 0; i < testParams.TestNumPG; i++ {
			mapPortsSetting, err = GenMicroSegVlanMappingPerPG(testParams, penPGArray[i], &startMicroSegVlanID)
			if err != nil {
				t.Logf("Failed at generating useg vlans, err: %s", err)
				isPassed = false
				goto exitDvsTest
			}
			_, err = penDVS.UpdatePorts(mapPortsSetting)
			if err != nil {
				t.Logf("Failed at updating ports on DVS, err: %s", err)
				isPassed = false
				goto exitDvsTest
			}
		}
	*/

	// Verify the results
	dvsObj, err = vcp.GetPenDVS(testParams.TestDCName, testParams.TestDVSName, retryCount)
	AssertOk(t, err, "Failed to get DVS %s", testParams.TestDVSName)

	var dvs mo.DistributedVirtualSwitch
	err = dvsObj.Properties(ctx, dvsObj.Reference(), nil, &dvs)
	AssertOk(t, err, "Failed to get properties for DVS")

	numPG = len(dvs.Summary.PortgroupName)
	AssertEquals(t, testParams.TestNumPG+1, numPG, "Incorrect number of portgroups")

	mapPGNamesWithIndex = testutils.GenPGNamesForComp(testParams)

	for i := 0; i < numPG; i++ {
		delete(*mapPGNamesWithIndex, dvs.Summary.PortgroupName[i])
	}

	AssertEquals(t, 0, len(*mapPGNamesWithIndex), "No entries should remain")

	// List and Delete all PGs
	for i := 0; i < testParams.TestNumPG; i++ {
		pgName := fmt.Sprint(testParams.TestPGNameBase, i)
		err = vcp.RemovePenPG(testParams.TestDCName, pgName, retryCount)
		AssertOk(t, err, "Failed to delete PG")

		_, err := vcp.GetPenPG(testParams.TestDCName, pgName, retryCount)
		Assert(t, err != nil, "Found deleted PG %s", pgName)
	}

	// Delete DVS
	err = vcp.RemovePenDVS(testParams.TestDCName, testParams.TestDVSName, retryCount)
	AssertOk(t, err, "Failed to delete DVS")

	_, err = vcp.GetPenDVS(testParams.TestDCName, testParams.TestDVSName, retryCount)
	Assert(t, err != nil, "Found deleted DVS %s", testParams.TestDVSName)

	err = vcp.AddPenDC("DC2", 5)
	AssertOk(t, err, "Failed to create DC")

	err = vcp.RemovePenDC("DC2", 5)
	AssertOk(t, err, "Failed to remove DC")
}

func TestEventReceiver(t *testing.T) {

	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("vmotion_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

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

	dc, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	// Create Host1 (pensando)
	penHost1, err := dc.AddHost("host1")
	AssertOk(t, err, "failed to add Host to DC")

	pNicMac := net.HardwareAddr{}
	pNicMac = append(pNicMac, globals.PensandoOUI[0])
	pNicMac = append(pNicMac, globals.PensandoOUI[1])
	pNicMac = append(pNicMac, globals.PensandoOUI[2])
	pNicMac = append(pNicMac, 0xbb)
	pNicMac = append(pNicMac, 0x00)
	pNicMac = append(pNicMac, 0x00)
	// Make it Pensando host
	err = penHost1.AddNic("vmnic0", conv.MacString(pNicMac))
	AssertOk(t, err, "failed to add pNic")
	// Create a VM on host1
	vm1, err := dc.AddVM("vm1", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aaaa.bbbb.ddde",
			PortgroupKey: "pg-1",
			PortKey:      "11",
		},
	})
	AssertOk(t, err, "Failed to create vm1")

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	eventCh := make(chan defs.Probe2StoreMsg, 24)
	ctx, cancel := context.WithCancel(context.Background())
	state := &defs.State{
		VcURL:    vcURL,
		VcID:     "vcenter",
		Ctx:      ctx,
		Log:      logger,
		Wg:       &sync.WaitGroup{},
		DcIDMap:  map[string]types.ManagedObjectReference{},
		DvsIDMap: map[string]types.ManagedObjectReference{},
	}
	defer func() {
		cancel()
		state.Wg.Wait()
	}()
	vmEventArg := types.VmEventArgument{Vm: vm1.Reference()}
	destHost := types.HostEventArgument{Host: penHost1.Obj.Reference()}
	vcp := NewVCProbe(storeCh, eventCh, state)

	// build a bunch of events and call event receiver
	events1 := []types.BaseEvent{
		&types.VmBeingHotMigratedEvent{
			VmEvent:  types.VmEvent{Event: types.Event{Key: 1, Vm: &vmEventArg}},
			DestHost: destHost,
		},
		&types.VmBeingMigratedEvent{
			VmEvent:  types.VmEvent{Event: types.Event{Key: 2, Vm: &vmEventArg}},
			DestHost: destHost,
		},
		&types.VmBeingRelocatedEvent{
			VmRelocateSpecEvent: types.VmRelocateSpecEvent{
				VmEvent: types.VmEvent{Event: types.Event{Key: 3, Vm: &vmEventArg}},
			},
			DestHost: destHost,
		},
		&types.VmEmigratingEvent{
			VmEvent: types.VmEvent{Event: types.Event{Key: 4, Vm: &vmEventArg}},
		},
		&types.EventEx{
			EventTypeId: "com.vmware.vc.vm.VmHotMigratingWithEncryptionEvent",
			Arguments: []types.KeyAnyValue{
				{
					Key:   "destHost",
					Value: "host1",
				},
				{
					Key:   "destDatacenter",
					Value: "DC1",
				},
			},
		},
	}
	events2 := []types.BaseEvent{
		&types.VmMigratedEvent{
			VmEvent:    types.VmEvent{Event: types.Event{Key: 11, Vm: &vmEventArg}},
			SourceHost: destHost,
		},
		&types.VmRelocatedEvent{
			VmRelocateSpecEvent: types.VmRelocateSpecEvent{
				VmEvent: types.VmEvent{Event: types.Event{Key: 12, Vm: &vmEventArg}},
			},
			SourceHost: destHost,
		},
	}
	events3 := []types.BaseEvent{
		&types.VmRelocateFailedEvent{
			VmRelocateSpecEvent: types.VmRelocateSpecEvent{
				VmEvent: types.VmEvent{Event: types.Event{Key: 21, Vm: &vmEventArg}},
			},
			DestHost: destHost,
		},
		&types.VmFailedMigrateEvent{
			VmEvent:  types.VmEvent{Event: types.Event{Key: 22, Vm: &vmEventArg}},
			DestHost: destHost,
		},
		&types.MigrationEvent{
			VmEvent: types.VmEvent{Event: types.Event{Key: 23, Vm: &vmEventArg}},
		},
	}
	vcp.initEventTracker(dc.Obj.Reference())
	vcp.receiveEvents(dc.Obj.Reference(), events1)
	vcp.receiveEvents(dc.Obj.Reference(), events2)
	vcp.receiveEvents(dc.Obj.Reference(), events3)
	vcp.deleteEventTracker(dc.Obj.Reference())
}
