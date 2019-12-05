package vcprobe

import (
	"fmt"
	"net/url"
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/mo"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestMessages(t *testing.T) {
	vcID := "127.0.0.1:8990"
	user := "user"
	password := "pass"

	url := fmt.Sprintf("%s:%s@%s/sdk", user, password, vcID)
	expectedMsgs := map[defs.VCObject][]defs.Probe2StoreMsg{
		// The changes property is not checked currently
		defs.VirtualMachine: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "vm-19",
				Originator: vcID,
			},
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "vm-21",
				Originator: vcID,
			},
		},
		defs.HostSystem: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.HostSystem,
				Key:        "host-14",
				Originator: vcID,
			},
		},
	}
	config := log.GetDefaultConfig("vcprobe_test")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	s, err := sim.NewVcSim(sim.Config{Addr: url})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "Failed to create DC")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "failed to create host")
	_, err = dc.AddVM("vm1", "host1")
	AssertOk(t, err, "failed to create vm")
	_, err = dc.AddVM("vm2", "host1")
	AssertOk(t, err, "failed to create vm")

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)
	orchConfig := getOrchestratorConfig(vcID, user, password)

	vcp := NewVCProbe(orchConfig, storeCh, probeCh, nil, logger, "http")
	vcp.Start()
	defer vcp.Stop()

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 2 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages. ")
		t.Logf("Expected: ")
		for k, v := range expectedMsgs {
			t.Logf("%d %s but got %d %s", len(v), k, len(eventMap[k]), k)
		}
		t.FailNow()
	}
	for objType, events := range expectedMsgs {
		recvEvents := eventMap[objType]
		for _, expE := range events {
			foundMatch := false
			for _, recvE := range recvEvents {
				if expE.Key == recvE.Key &&
					expE.Originator == recvE.Originator {
					foundMatch = true
					break
				}
			}
			Assert(t, foundMatch, "could not find matching event for %v", expE)
		}
	}
}

func TestReconnect(t *testing.T) {
	vcID := "user:pass@127.0.0.1:8990"
	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "failed dc create")
	dc.AddHost("host1")
	dc.AddVM("vm1", "host1")

	fmt.Printf("starting on %s\n", s.GetURL())

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)

	config := log.GetDefaultConfig("vcprobe_test")
	config.Filter = log.AllowAllFilter
	config.LogToStdout = true
	logger := log.SetConfig(config)

	orchConfig := getOrchestratorConfig("127.0.0.1:8990", "user", "pass")
	vcp := NewVCProbe(orchConfig, storeCh, probeCh, nil, logger, "http")
	err = vcp.Start()
	AssertOk(t, err, "Failed to start probe")
	defer vcp.Stop()

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.Logf("host events: %v", eventMap[defs.HostSystem])
		t.Logf("vm events: %v", eventMap[defs.VirtualMachine])
		t.FailNow()
	}

	// Reset values and break connection
	eventMap = make(map[defs.VCObject][]defs.Probe2StoreMsg)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()

	s.Server.CloseClientConnections()

	select {
	case <-doneCh:
	case <-time.After(10 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.Logf("host events: %v", eventMap[defs.HostSystem])
		t.Logf("vm events: %v", eventMap[defs.VirtualMachine])
		t.FailNow()
	}
}

func TestLoginRetry(t *testing.T) {
	t.Skip("Watch currently does not process events for objects that have been created after the watch has started")

	// If the probe is started before the vcenter instance is up, it should
	// keep retrying to create the client

	vcID := "user:pass@127.0.0.1:8990"

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)
	config := log.GetDefaultConfig("vcprobe_test")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	orchConfig := getOrchestratorConfig("127.0.0.1:8990/sdk", "user", "pass")
	vcp := NewVCProbe(orchConfig, storeCh, probeCh, nil, logger, "http")
	err := vcp.Start()
	AssertOk(t, err, "Failed to start probe")
	defer vcp.Stop()

	time.Sleep(time.Second)

	fmt.Printf("starting sim %s\n", vcID)

	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "failed dc create")
	dc.AddHost("host1")
	dc.AddVM("vm1", "host1")

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.FailNow()
	}
}

func TestDVS(t *testing.T) {
	testParams := &vcprobeTestParams{
		testHostName:           "127.0.0.1:8990",
		testUser:               "user",
		testPassword:           "pass",
		testDCName:             "PenTestDC",
		testDVSName:            "PenTestDVS",
		testPGNameBase:         "PenTestPG",
		testMaxPorts:           4096,
		testNumStandalonePorts: 512,
		testNumPVLANPair:       5,
		startPVLAN:             500,
		testNumPG:              5,
		testNumPortsPerPG:      20,
	}

	err := TestValidateParams(testParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	u := &url.URL{
		Scheme: "http",
		Host:   testParams.testHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(testParams.testUser, testParams.testPassword)

	config := log.GetDefaultConfig("vcprobe_testDVS")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	_, err = s.AddDC(testParams.testDCName)
	AssertOk(t, err, "failed dc create")

	orchConfig := getOrchestratorConfig(testParams.testHostName, testParams.testUser, testParams.testPassword)
	vcp := NewVCProbe(orchConfig, storeCh, probeCh, nil, logger, u.Scheme)
	err = vcp.Start()
	AssertOk(t, err, "Failed to start probe")
	defer vcp.Stop()

	time.Sleep(time.Second * 3)

	// Trigger the test
	//var mapPortsSetting *PenDVSPortSettings
	penPGArray := make([]*PenPG, testParams.testNumPG)
	pvlanConfigSpecArray := GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	penDVS, err := vcp.AddPenDVS(testParams.testDCName, dvsCreateSpec)
	AssertOk(t, err, "Failed to add DVS")

	pgConfigSpecArray := GenPGConfigSpecArray(testParams, pvlanConfigSpecArray)
	//startMicroSegVlanID := testParams.startPVLAN + int32(testParams.testNumPVLANPair*2)
	var isPassed bool
	var numPG int
	var mapPGNamesWithIndex *map[string]int
	var dvs *mo.DistributedVirtualSwitch

	for i := 0; i < testParams.testNumPG; i++ {
		penPGArray[i], err = penDVS.AddPenPG(&pgConfigSpecArray[i])
		if err != nil {
			t.Logf("Failed at adding new portgroup, err: %s", err)
			isPassed = false
			goto exitDvsTest
		}
	}
	/*
		for i := 0; i < testParams.testNumPG; i++ {
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
	dvs, err = penDVS.getMoDVSRef()
	if err != nil {
		t.Logf("Failed at converting to mo.DistributedVirtualSwitch, err: %s", err)
		isPassed = false
		goto exitDvsTest
	}

	numPG = len(dvs.Summary.PortgroupName)
	if numPG != (testParams.testNumPG + 1) {
		t.Logf("Incorrect number of portgroup: %d, expected value is: %d", numPG, testParams.testNumPG+1)
		isPassed = false
		goto exitDvsTest
	}

	mapPGNamesWithIndex = GenPGNamesForComp(testParams)

	for i := 0; i < numPG; i++ {
		delete(*mapPGNamesWithIndex, dvs.Summary.PortgroupName[i])
	}

	if len(*mapPGNamesWithIndex) == 0 {
		isPassed = true
	} else {
		isPassed = false
	}

exitDvsTest:
	// Teardown the configuration
	err = vcp.RemovePenDVS(testParams.testDVSName)
	if err != nil {
		t.Logf("Failed at tearing down DVS: %s from datacenter: %s, err: %s", penDVS.DvsName, penDVS.DcName, err)
		isPassed = false
	}

	if !isPassed {
		t.Fatal("Failed at validating test results")
	}
}
