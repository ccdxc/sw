package orchhub

import (
	"context"
	"fmt"
	"net"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/globals"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

func TestOrchestrationConnectionStatus(t *testing.T) {
	// Create orch config  to vcenter that isn't up
	//  - status should be not connected
	// Bring up vcsim
	//  - connection status updates
	// Teardown vcsim
	//  - connection updates to not connected
	// Update orch object without supplying credentials
	tinfo.eventRecorder.ClearEvents()

	vcInfo := tinfo.vcConfig
	// Only use sim for this test, so we ignore user config's uri
	uri := "127.0.0.1:8989"

	orchConfig, err := createOrchConfig("vc1", uri, vcInfo.user, "badPass", "")
	AssertOk(t, err, "Error creating orch config")
	defer deleteOrchConfig("vc1")

	ctx := context.Background()

	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "10s")

	// Should have gotten connection failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range tinfo.eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_CONNECTION_ERROR.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find connection error event", "1s", "10s")

	tinfo.eventRecorder.ClearEvents()

	// bring up sim
	vcSim, err := startVCSim(uri, vcInfo.user, vcInfo.pass)
	AssertOk(t, err, "Failed to create vcsim")

	time.Sleep(10 * time.Second)

	// Should have gotten login failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range tinfo.eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_LOGIN_FAILURE.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find login error event", "100ms", "10s")

	tinfo.eventRecorder.ClearEvents()

	// update password
	orchConfig, err = updateOrchConfig("vc1", uri, vcInfo.user, vcInfo.pass)
	AssertOk(t, err, "Error updating orch config")

	// Should connect to vc
	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "10s")

	tinfo.eventRecorder.ClearEvents()

	// Bring down vcsim
	// Sim server will block while trying to wait for connections to terminate
	go func() {
		vcSim.Destroy()
	}()

	// Status should be failure
	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "10s")

	// Should have gotten connection failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range tinfo.eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_CONNECTION_ERROR.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find connection error event", "100ms", "10s")
	tinfo.eventRecorder.ClearEvents()

	orchConfig, err = tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
	AssertOk(t, err, "Failed to get object: %v", apierrors.FromError(err))

	orchConfig.Spec.URI = "test.url"
	orchConfig.Spec.Credentials = nil
	obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Update(context.Background(), orchConfig)
	AssertOk(t, err, "Failed to update object without credentials: %v", apierrors.FromError(err))
	AssertEquals(t, "test.url", obj.Spec.URI, "orch object was not updated")
}

func TestNetworks(t *testing.T) {
	// This test takes a really long time to run, especially if sync gets
	// triggered during the test execution
	// Not worthwhile to have it constantly running in CI
	t.Skip("Disabled in CI due to time required to run.")
	// Create network to non-existent datacenter X
	//   - Network create should succeed
	// Create datacenter X
	//   - DVS + PG should appear
	// Exhaust venice network limit (500). Generate event
	// Delete old networks, new networks should be added now that we are below limit.
	// Current hard limit is 500.
	// Testing with lower limit since it takes too long to run with 500
	networkCount := 128

	networkExhaust := false
	if networkCount == (useg.FirstUsegVlan-useg.FirstPGVlan)/2 {
		networkExhaust = true
	}

	vcInfo := tinfo.vcConfig

	// bring up sim
	var vcSim *sim.VcSim
	if !vcInfo.useRealVC {
		var err error
		vcSim, err = startVCSim(vcInfo.uri, vcInfo.user, vcInfo.pass)
		AssertOk(t, err, "Failed to create vcsim")
	}

	dcName := "TestNetworks-DC1"

	ctx, cancel := context.WithCancel(context.Background())
	probe := createProbe(ctx, vcInfo.uri, vcInfo.user, vcInfo.pass)
	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	err := probe.AddPenDC(dcName, 5)
	AssertOk(t, err, "Failed to create DC")

	defer func() {
		logger.Infof("----- Teardown ----")
		cancel()
		probe.Wg.Wait()
		cleanup()
		if vcSim != nil {
			go func() {
				vcSim.Destroy()
			}()
		}
	}()

	orchConfig, err := createOrchConfig("vc1", vcInfo.uri, vcInfo.user, vcInfo.pass, dcName)
	AssertOk(t, err, "Error creating orch config")

	// Create Network
	orchInfo := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: dcName,
		},
	}
	nw, err := createNetwork("network0", 1000, orchInfo)
	AssertOk(t, err, "Error creating network")

	AssertEventually(t, func() (bool, interface{}) {
		dvs, err := probe.GetPenDVS(dcName, vchub.CreateDVSName(dcName), 1)
		if err != nil {
			return false, err
		}
		if dvs == nil {
			return false, fmt.Errorf("Couldn't find DVS")
		}

		pg, err := probe.GetPenPG(dcName, vchub.CreatePGName(nw.Name), 2)
		if err != nil {
			return false, err
		}
		if pg == nil {
			return false, fmt.Errorf("Couldn't find PG")
		}
		return true, nil
	}, "Failed to find DVS and PG", "1s", "10s")

	for i := 1; i < networkCount; i++ {
		_, err := createNetwork(fmt.Sprintf("network%d", i), 1000+i, orchInfo)
		AssertOk(t, err, "Error creating network")
	}

	// Verify all PGs show up
	foundArr := make([]bool, networkCount)
	foundArr[0] = true
	AssertEventually(t, func() (bool, interface{}) {
		foundAll := true
		for i := 1; i < networkCount; i++ {
			if foundArr[i] {
				continue
			}
			nwName := fmt.Sprintf("network%d", i)
			pg, err := probe.GetPenPG(dcName, vchub.CreatePGName(nwName), 2)
			if err != nil {
				foundAll = false
				continue
			}
			if pg == nil {
				foundAll = false
				continue
			}
			foundArr[i] = true
		}
		if !foundAll {
			// Return list of missing entries
			missingNetworks := []string{}
			for i, found := range foundArr {
				if !found {
					nwName := vchub.CreatePGName(fmt.Sprintf("network%d", i))
					missingNetworks = append(missingNetworks, nwName)
				}
			}
			return false, fmt.Errorf("Missing %d entries, %v", len(missingNetworks), missingNetworks)
		}
		return true, nil
	}, "Failed to find PGs", "30s", "360s")

	tinfo.eventRecorder.ClearEvents()

	if networkExhaust {
		// Next creation should exceed limit
		nw, err = createNetwork(fmt.Sprintf("network-fail"), 2000, orchInfo)
		AssertOk(t, err, "Error creating network")

		// Check config failure event was generated
		AssertEventually(t, func() (bool, interface{}) {
			foundEvent := false
			for _, evt := range tinfo.eventRecorder.GetEvents() {
				if evt.EventType == eventtypes.ORCH_CONFIG_PUSH_FAILURE.String() {
					if strings.Contains(evt.Message, nw.Name) {
						foundEvent = true
					}
				}
			}
			return foundEvent, nil
		}, "Failed to find config push error event", "1s", "30s")

		err = deleteNetwork(nw.Name)
		AssertOk(t, err, "failed to delete network")
	}

	err = deleteNetwork(fmt.Sprintf("network%d", networkCount-1))
	AssertOk(t, err, "failed to delete network")

	nw, err = createNetwork(fmt.Sprintf("network-new"), 3000, orchInfo)
	AssertOk(t, err, "Error creating network")

	// Verify last network was created
	AssertEventually(t, func() (bool, interface{}) {
		pg, err := probe.GetPenPG(dcName, vchub.CreatePGName(nw.Name), 2)
		if err != nil {
			return false, err
		}
		if pg == nil {
			return false, fmt.Errorf("Couldn't find PG")
		}
		return true, nil
	}, "Failed to find PG", "1s", "30s")

}

func TestAPIServerRestart(t *testing.T) {
	testParams := &testutils.TestParams{
		TestHostName: "127.0.0.1:8989",
		TestUser:     "user",
		TestPassword: "pass",

		TestDCName:             "PenTestDC",
		TestDVSName:            vchub.CreateDVSName("PenTestDC"),
		TestPGNameBase:         defs.DefaultPGPrefix,
		TestMaxPorts:           4096,
		TestNumStandalonePorts: 512,
		TestNumPVLANPair:       5,
		StartPVLAN:             500,
		TestNumPG:              5,
		TestNumPortsPerPG:      20,
		TestOrchName:           "test-orchestrator",
	}
	// Create host 1
	// Disconnect from apiserver
	// Create host 2, workload 1 on host1
	// Bring up APIserver
	// Workload 1 and host 1, 2 should now exist in apiserver

	vcInfo := tinfo.vcConfig

	if vcInfo.useRealVC {
		t.Skip("Real VC is not supported for this test")
	}

	// bring up sim
	vcSim, err := startVCSim(vcInfo.uri, vcInfo.user, vcInfo.pass)
	AssertOk(t, err, "Failed to create vcsim")

	ctx, cancel := context.WithCancel(context.Background())
	probe := createProbe(ctx, vcInfo.uri, vcInfo.user, vcInfo.pass)
	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	defer func() {
		logger.Infof("----- Teardown ----")
		cancel()
		probe.Wg.Wait()
		cleanup()
		if vcSim != nil {
			go func() {
				vcSim.Destroy()
			}()
		}
	}()

	dcName := testParams.TestDCName
	dc1, err := vcSim.AddDC(dcName)
	AssertOk(t, err, "failed dc create")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(testParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(testParams, pvlanConfigSpecArray)

	err = probe.AddPenDVS(dcName, dvsCreateSpec, nil, 3)
	dvsName := vchub.CreateDVSName(testParams.TestDCName)
	dvs, ok := dc1.GetDVS(dvsName)
	Assert(t, ok, "failed dvs create")

	hostSystem1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed host1 create")
	err = dvs.AddHost(hostSystem1)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac := net.HardwareAddr{}
	pNicMac = append(pNicMac, globals.PensandoOUI[0], globals.PensandoOUI[1], globals.PensandoOUI[2])
	pNicMac = append(pNicMac, 0xaa, 0x00, 0x00)
	// Make it Pensando host
	err = hostSystem1.AddNic("vmnic0", conv.MacString(pNicMac))

	// Trigger vchub
	_, err = createOrchConfig("vc1", vcInfo.uri, vcInfo.user, vcInfo.pass, dcName)
	AssertOk(t, err, "Error creating orch config")

	// Take down apiserver
	tinfo.apiServer.Stop()

	tinfo.l.Infof("API Server shutdown")

	// Create new host while apiserver is down
	hostSystem2, err := dc1.AddHost("host2")
	AssertOk(t, err, "failed host2 create")
	err = dvs.AddHost(hostSystem2)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac2 := net.HardwareAddr{}
	pNicMac2 = append(pNicMac2, globals.PensandoOUI[0], globals.PensandoOUI[1], globals.PensandoOUI[2])
	pNicMac2 = append(pNicMac2, 0xaa, 0x00, 0x11)
	// Make it Pensando host
	err = hostSystem2.AddNic("vmnic0", conv.MacString(pNicMac2))

	// Reconnect apiserver
	tinfo.apiServer, tinfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", "OrchhubIntegTest", tinfo.l)
	AssertOk(t, err, "Failed to start apiserver")

	tinfo.updateResolver(globals.APIServer, tinfo.apiServerAddr)

	AssertEventually(t, func() (bool, interface{}) {
		apicl, err := apiclient.NewGrpcAPIClient("OrchhubIntegTest", tinfo.apiServerAddr, tinfo.l)
		if err != nil {
			tinfo.l.Errorf("cannot create grpc client, Err: %v", err)
			return false, err
		}
		tinfo.apicl = apicl
		return true, nil
	}, "Failed to create apicl", "15s", "3m")

	AssertEventually(t, func() (bool, interface{}) {
		hosts, err := tinfo.apicl.ClusterV1().Host().List(ctx, &api.ListWatchOptions{})
		if err != nil {
			return false, err
		}
		if len(hosts) != 2 {
			return false, fmt.Errorf("Found %d hosts, expected 2", len(hosts))
		}

		return true, nil
	}, "Failed to find workloads and hosts", "5s", "30s")

}
