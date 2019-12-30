package vcprobe

import (
	"context"
	"fmt"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/mo"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

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
	}

	err := testutils.ValidateParams(testParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	u := &url.URL{
		Scheme: "http",
		Host:   testParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(testParams.TestUser, testParams.TestPassword)

	config := log.GetDefaultConfig("vcprobe_testDVS")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	storeCh := make(chan defs.Probe2StoreMsg, 24)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC(testParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "failed host create")
	_, err = dc.AddVM("vm1", "host1")
	AssertOk(t, err, "failed vm create")

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
	}
	vcp := NewVCProbe(storeCh, state)
	vcp.Start()

	defer func() {
		cancel()
		state.Wg.Wait()
	}()

	// Start test
	state.Wg.Add(1)
	go vcp.StartWatchers()

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)
	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				item := m.Val.(defs.VCEventMsg)
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

	// Create DVS for listing
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(testParams.TestDCName, dvsCreateSpec)
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
		err = vcp.AddPenPG(testParams.TestDCName, testParams.TestDVSName, &pgConfigSpecArray[i])
		AssertOk(t, err, "Failed to add new PG")
	}

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
	}

	err := testutils.ValidateParams(testParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	u := &url.URL{
		Scheme: "http",
		Host:   testParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(testParams.TestUser, testParams.TestPassword)

	config := log.GetDefaultConfig("vcprobe_testDVS")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	storeCh := make(chan defs.Probe2StoreMsg, 24)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	_, err = s.AddDC(testParams.TestDCName)
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
	}
	vcp := NewVCProbe(storeCh, state)
	vcp.Start()

	defer func() {

		cancel()
		state.Wg.Wait()
	}()

	time.Sleep(time.Second * 3)

	// Trigger the test
	//var mapPortsSetting *PenDVSPortSettings
	// penPGArray := make([]*object.DistributedVirtualPortgroup, testParams.TestNumPG)
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(testParams.TestDCName, dvsCreateSpec)
	AssertOk(t, err, "Failed to add DVS")

	pgConfigSpecArray := testutils.GenPGConfigSpecArray(testParams, pvlanConfigSpecArray)
	// startMicroSegVlanID := testParams.StartPVLAN + int32(testParams.TestNumPVLANPair*2)
	var numPG int
	var mapPGNamesWithIndex *map[string]int

	for i := 0; i < testParams.TestNumPG; i++ {
		err = vcp.AddPenPG(testParams.TestDCName, testParams.TestDVSName, &pgConfigSpecArray[i])
		AssertOk(t, err, "Failed to add new PG")

		pgName := fmt.Sprint(testParams.TestPGNameBase, i)
		pgObj, err := vcp.GetPenPG(testParams.TestDCName, pgName)
		AssertOk(t, err, "Failed to find created PG %s", pgName)
		Assert(t, pgObj != nil, "Couldn't find created PG %s", pgName)
	}
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
	dvsObj, err := vcp.GetPenDVS(testParams.TestDCName, testParams.TestDVSName)
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
		err = vcp.RemovePenPG(testParams.TestDCName, pgName)
		AssertOk(t, err, "Failed to delete PG")

		_, err := vcp.GetPenPG(testParams.TestDCName, pgName)
		Assert(t, err != nil, "Found deleted PG %s", pgName)
	}

	// Delete DVS
	err = vcp.RemovePenDVS(testParams.TestDCName, testParams.TestDVSName)
	AssertOk(t, err, "Failed to delete DVS")

	_, err = vcp.GetPenDVS(testParams.TestDCName, testParams.TestDVSName)
	Assert(t, err != nil, "Found deleted DVS %s", testParams.TestDVSName)

}
