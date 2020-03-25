package vchub

import (
	"context"
	"fmt"
	"net/url"
	"testing"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("scale_test",
		log.GetNewLogger(log.GetDefaultConfig("scale_test"))))
	maxHosts = 1000
)

func TestVCScaleHost(t *testing.T) {
	// Create max naples hosts, add vmkNic to each so that we also have as many workloads
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("scale_test-Host")
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
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

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

	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	logger.Infof("Creating PenDC for %s\n", dc1.Obj.Reference().Value)
	_, err = vchub.NewPenDC(defaultTestParams.TestDCName, dc1.Obj.Self.Value)
	// Add DVS
	dvsName := CreateDVSName(defaultTestParams.TestDCName)
	dvs, ok := dc1.GetDVS(dvsName)
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
	mockProbe.AddPenPG(defaultTestParams.TestDCName, dvsName, &pgConfigSpec[0], retryCount)
	pg, err := mockProbe.GetPenPG(defaultTestParams.TestDCName, CreatePGName("vMotion_PG"), retryCount)
	AssertOk(t, err, "failed to add portgroup")

	hosts := []string{}

	// Create hosts and one vmknic on each host
	for i := 0; i < maxHosts; i++ {
		hName := fmt.Sprintf("Host-%d", i)
		hostSystem, err := dc1.AddHost(hName)
		AssertOk(t, err, "failed host1 create")
		err = dvs.AddHost(hostSystem)
		AssertOk(t, err, "failed to add Host to DVS")

		pNicMac := append(createPenPnicBase(), 0xaa, byte(i/256), byte(i%256))
		// Make it Pensando host
		hostSystem.ClearNics()
		err = hostSystem.AddNic("vmnic0", conv.MacString(pNicMac))

		// Create vmkNIC
		var spec types.HostVirtualNicSpec
		vmkMac := append(createPenPnicBase(), 0xcc, byte(i/256), byte(i%256))
		spec.Mac = conv.MacString(vmkMac)
		var dvPort types.DistributedVirtualSwitchPortConnection
		dvPort.PortgroupKey = pg.Reference().Value
		dvPort.PortKey = "10" // use some port number
		spec.DistributedVirtualPort = &dvPort
		err = hostSystem.AddVmkNic(&spec, "vmk1")

		hosts = append(hosts, vchub.createHostName(dc1.Obj.Self.Value, hostSystem.Obj.Self.Value))
	}

	vchub.Sync()

	verifyHosts := func(dcHostMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, hostnames := range dcHostMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
				}
				opts := api.ListWatchOptions{}
				hosts, err := sm.Controller().Host().List(context.Background(), &opts)
				if err != nil {
					return false, err
				}
				if len(hostnames) != len(hosts) {
					return false, fmt.Errorf("expected %d hosts but got %d", len(hostnames), len(hosts))
				}
				for _, hostname := range hostnames {
					meta := &api.ObjectMeta{
						Name: hostname,
					}
					host, err := sm.Controller().Host().Find(meta)
					if err != nil {
						return false, fmt.Errorf("Failed to find host %s", hostname)
					}
					if host == nil {
						return false, fmt.Errorf("Returned host was nil for host %s", hostname)
					}
				}
			}
			return true, nil
		}, "Failed to find hosts", "1s", "100s")
	}

	verifyWorkloadCount := func(expCount int) {
		AssertEventually(t, func() (bool, interface{}) {
			opts := api.ListWatchOptions{}
			workloads, err := sm.Controller().Workload().List(context.Background(), &opts)
			if err != nil {
				return false, err
			}
			if len(workloads) != expCount {
				return false, fmt.Errorf("expected %d workloads but got %d", expCount, len(workloads))
			}
			return true, nil
		}, "Failed to find Workloads", "1s", "100s")
	}

	dcHostMap := map[string][]string{
		defaultTestParams.TestDCName: hosts,
	}

	logger.Infof("Verify hosts start...")
	verifyHosts(dcHostMap)
	verifyWorkloadCount(maxHosts)
	logger.Infof("Verify hosts done...")
}
