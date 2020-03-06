package vchub

import (
	"context"
	"fmt"
	"net"
	"net/url"
	"os"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("sync_test",
		log.GetNewLogger(log.GetDefaultConfig("sync_test"))))
	retryCount = 1
)

// Tests creation of internal DC state, creationg of DVS
// and creation/deletion of networks in venice
// trigger respective events in VC
// Test PG with modified config gets reset on sync
func TestVCSyncPG(t *testing.T) {
	// Stale PGs should be deleted
	// Modified PGs should be have config reset
	// New PGs should be created
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("sync_test-pg")
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
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)
	dvs, err := dc1.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}

	// CREATING VENICE NETWORKS
	smmock.CreateNetwork(sm, "default", "pg1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	smmock.CreateNetwork(sm, "default", "pg2", "10.1.2.0/24", "10.1.1.2", 101, nil, orchInfo1)
	smmock.CreateNetwork(sm, "default", "pgModified", "10.1.2.0/24", "10.1.1.2", 102, nil, orchInfo1)

	time.Sleep(1 * time.Second)

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

	spec := testutils.GenPGConfigSpec(CreatePGName("pgStale"), 2, 3)
	err = mockProbe.AddPenPG(dc1.Obj.Name, dvs.Obj.Name, &spec, retryCount)
	AssertOk(t, err, "failed to create pg")

	spec1 := testutils.GenPGConfigSpec(CreatePGName("pgModified"), 4, 5)
	spec1.DefaultPortConfig.(*types.VMwareDVSPortSetting).Vlan = &types.VmwareDistributedVirtualSwitchVlanIdSpec{
		VlanId: 4,
	}
	err = mockProbe.AddPenPG(dc1.Obj.Name, dvs.Obj.Name, &spec1, retryCount)
	AssertOk(t, err, "failed to create pg")

	defer vchub.Destroy(true)

	vchub.Sync()

	verifyPg := func(dcPgMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, pgNames := range dcPgMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
				}
				dvs := dc.GetPenDVS(CreateDVSName(name))
				if dvs == nil {
					return false, fmt.Errorf("Failed to find dvs in DC %s", name)
				}

				for _, pgName := range pgNames {
					penPG := dvs.GetPenPG(pgName)
					if penPG == nil {
						return false, fmt.Errorf("Failed to find %s in DC %s", pgName, name)
					}
					pgObj, err := mockProbe.GetPGConfig(dc1.Obj.Name, pgName, nil, retryCount)
					AssertOk(t, err, "Failed to get PG")

					vlanSpec := pgObj.Config.DefaultPortConfig.(*types.VMwareDVSPortSetting).Vlan
					pvlanSpec, ok := vlanSpec.(*types.VmwareDistributedVirtualSwitchPvlanSpec)
					if !ok {
						return false, fmt.Errorf("PG %s was not in pvlan mode", pgName)
					}
					if !useg.IsPGVlanSecondary(int(pvlanSpec.PvlanId)) {
						return false, fmt.Errorf("PG should be in pvlan mode with odd vlan")
					}
				}
				dvs.Lock()
				if len(dvs.Pgs) != len(pgNames) {
					err := fmt.Errorf("PG length didn't match: exp %v, actual %v", pgNames, dvs.Pgs)
					dvs.Unlock()
					return false, err
				}
				dvs.Unlock()
			}
			return true, nil
		}, "Failed to find PGs")
	}

	// n1 should only be in defaultDC
	// n2 should be in both
	pg1 := CreatePGName("pg1")
	pg2 := CreatePGName("pg2")
	pg3 := CreatePGName("pgModified")

	dcPgMap := map[string][]string{
		defaultTestParams.TestDCName: []string{pg1, pg2, pg3},
	}

	verifyPg(dcPgMap)
}

func TestVCSyncHost(t *testing.T) {
	// Stale hosts should be deleted
	// New hosts should be created
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("sync_test-Host")
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

	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	logger.Infof("Creating PenDC for %s\n", dc1.Obj.Reference().Value)
	_, err = vchub.NewPenDC(defaultTestParams.TestDCName, dc1.Obj.Self.Value)
	// Add DVS
	dvsName := CreateDVSName(defaultTestParams.TestDCName)
	dvs, ok := dc1.GetDVS(dvsName)
	Assert(t, ok, "failed dvs create")

	hostSystem1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed host1 create")
	err = dvs.AddHost(hostSystem1)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac := append(createPenPnicBase(), 0xaa, 0x00, 0x00)
	// Make it Pensando host
	err = hostSystem1.AddNic("vmnic0", conv.MacString(pNicMac))

	hostSystem2, err := dc1.AddHost("host2")
	AssertOk(t, err, "failed host2 create")
	err = dvs.AddHost(hostSystem2)
	AssertOk(t, err, "failed to add Host to DVS")
	pNicMac2 := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	// Make it Pensando host
	err = hostSystem2.AddNic("vmnic0", conv.MacString(pNicMac2))

	// CREATING HOSTS
	staleHost2 := createHostObj(
		vchub.createHostName(dc1.Obj.Self.Value, "hostsystem-00001"),
		"test",
		conv.MacString(pNicMac),
	)
	utils.AddOrchNameLabel(staleHost2.Labels, "AnotherVC")
	utils.AddOrchNamespaceLabel(staleHost2.Labels, "AnotherDC")
	err = sm.Controller().Host().Create(&staleHost2)
	AssertOk(t, err, "failed to create host")

	// Create a another stale host with same mac addr but different VC id
	staleHost := createHostObj(
		vchub.createHostName(dc1.Obj.Self.Value, "hostsystem-00000"),
		"test",
		conv.MacString(pNicMac),
	)
	utils.AddOrchNameLabel(staleHost.Labels, orchConfig.Name)
	utils.AddOrchNamespaceLabel(staleHost.Labels, dc1.Obj.Name)
	// replace stale host with same mac but different VC by this one
	vchub.fixStaleHost(&staleHost)

	// Stale host that will have a stale workload referring to it
	staleHost3 := createHostObj(
		vchub.createHostName(dc1.Obj.Self.Value, "hostsystem-00002"),
		"test",
		conv.MacString(pNicMac),
	)
	utils.AddOrchNameLabel(staleHost3.Labels, orchConfig.Name)
	utils.AddOrchNamespaceLabel(staleHost3.Labels, dc1.Obj.Name)
	err = sm.Controller().Host().Create(&staleHost3)
	AssertOk(t, err, "failed to create host")

	// Create stale workload
	staleWorkload := createWorkloadObj(
		vchub.createVMWorkloadName(dc1.Obj.Self.Value, "staleWorkload"),
		staleHost3.Name,
		[]workload.WorkloadIntfSpec{
			workload.WorkloadIntfSpec{
				MACAddress:   "aaaa.bbbb.cccc",
				MicroSegVlan: 2000,
			},
		},
	)

	vchub.addWorkloadLabels(&staleWorkload, "staleWorkload", dc1.Obj.Name)
	err = sm.Controller().Workload().Create(&staleWorkload)
	AssertOk(t, err, "failed to create workload")

	host1 := createHostObj(
		vchub.createHostName(dc1.Obj.Self.Value, hostSystem1.Obj.Self.Value),
		"test1",
		"",
	)
	utils.AddOrchNameLabel(host1.Labels, orchConfig.Name)
	utils.AddOrchNamespaceLabel(host1.Labels, dc1.Obj.Name)

	time.Sleep(1 * time.Second)

	vchub.Sync()

	verifyHosts := func(dcHostMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, hostnames := range dcHostMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
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
				opts := api.ListWatchOptions{}
				hosts, err := sm.Controller().Host().List(context.Background(), &opts)
				AssertOk(t, err, "failed to get hosts")
				if len(hostnames) != len(hosts) {
					return false, fmt.Errorf("expected %d hosts but got %d", len(hostnames), len(hosts))
				}
			}
			return true, nil
		}, "Failed to find hosts")
	}

	dcHostMap := map[string][]string{
		defaultTestParams.TestDCName: []string{host1.Name, vchub.createHostName(dc1.Obj.Self.Value, hostSystem2.Obj.Self.Value)},
	}

	verifyHosts(dcHostMap)

}

func TestVCSyncVM(t *testing.T) {
	// Stale VMs should be deleted
	// New VMs should be assigned Vlans
	// VMs with an already assigned override should keep it
	// VM with a different override than we assigned should be set back to what we have
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("sync_test-Host")
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

	defer vchub.Destroy(true)
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	logger.Infof("Creating PenDC for %s\n", dc1.Obj.Reference().Value)
	_, err = vchub.NewPenDC(defaultTestParams.TestDCName, dc1.Obj.Self.Value)
	// Add DVS
	dvsName := CreateDVSName(defaultTestParams.TestDCName)
	dvs, ok := dc1.GetDVS(dvsName)
	if !ok {
		logger.Info("GetDVS Failed")
		os.Exit(1)
	}

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}

	// Create network
	smmock.CreateNetwork(sm, "default", "pg1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	hostSystem1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed host1 create")
	err = dvs.AddHost(hostSystem1)
	AssertOk(t, err, "failed to add Host to DVS")
	pNicMac := append(createPenPnicBase(), 0xaa, 0x00, 0x00)
	// Make it Pensando host
	err = hostSystem1.AddNic("vmnic0", conv.MacString(pNicMac))

	time.Sleep(3 * time.Second)

	spec := testutils.GenPGConfigSpec(CreatePGName("pg1"), 2, 3)
	err = mockProbe.AddPenPG(dc1.Obj.Name, dvs.Obj.Name, &spec, retryCount)
	AssertOk(t, err, "failed to create pg")
	pg1, err := mockProbe.GetPenPG(dc1.Obj.Name, CreatePGName("pg1"), retryCount)
	AssertOk(t, err, "failed to get pg")

	// Setting up VMs
	vmExistingPort := "10"
	vmExistingMac := "aaaa.bbbb.dddd"
	vmExisting, err := dc1.AddVM("vmExisting", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   vmExistingMac,
			PortgroupKey: pg1.Reference().Value,
			PortKey:      vmExistingPort,
		},
	})
	AssertOk(t, err, "Failed to create vmExisting")
	vmNew, err := dc1.AddVM("vmNew", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aaaa.bbbb.ddde",
			PortgroupKey: pg1.Reference().Value,
			PortKey:      "11",
		},
	})
	AssertOk(t, err, "Failed to create vmNew")

	// CREATING HOSTS
	host1 := createHostObj(
		vchub.createHostName(dc1.Obj.Self.Value, hostSystem1.Obj.Self.Value),
		"test1",
		"",
	)
	sm.Controller().Host().Create(&host1)

	// CREATING WORKLOADS
	staleWorkload := createWorkloadObj(
		vchub.createVMWorkloadName(dc1.Obj.Self.Value, "staleWorkload"),
		host1.Name,
		[]workload.WorkloadIntfSpec{
			workload.WorkloadIntfSpec{
				MACAddress:   "aaaa.bbbb.cccc",
				MicroSegVlan: 2000,
			},
		},
	)

	vchub.addWorkloadLabels(&staleWorkload, "staleWorkload", dc1.Obj.Name)
	tagMsg := defs.TagMsg{
		Tags: []defs.TagEntry{
			{Name: "tag_a", Category: "Venice"},
		},
	}
	generateLabelsFromTags(staleWorkload.Labels, tagMsg)
	sm.Controller().Workload().Create(&staleWorkload)

	workloadExisting := createWorkloadObj(
		vchub.createVMWorkloadName(dc1.Obj.Self.Value, vmExisting.Self.Value),
		host1.Name,
		[]workload.WorkloadIntfSpec{
			workload.WorkloadIntfSpec{
				MACAddress:   vmExistingMac,
				MicroSegVlan: 3000,
			},
		},
	)
	vchub.addWorkloadLabels(&workloadExisting, "vmExisting", dc1.Obj.Name)
	sm.Controller().Workload().Create(&workloadExisting)

	portUpdate := vcprobe.PenDVSPortSettings{
		vmExistingPort: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(3000),
		},
	}
	mockProbe.UpdateDVSPortsVlan(dc1.Obj.Name, dvs.Obj.Name, portUpdate, retryCount)

	time.Sleep(1 * time.Second)

	vchub.Sync()

	verifyWorkloads := func(dcWorkloadMap map[string][]string, errMsg string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, wlNames := range dcWorkloadMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
				}
				for _, wlName := range wlNames {
					meta := &api.ObjectMeta{
						Name:      wlName,
						Tenant:    "default",
						Namespace: "default",
					}
					wl, err := sm.Controller().Workload().Find(meta)
					if err != nil {
						return false, fmt.Errorf("Failed to find workload %s", wlName)
					}
					if wl == nil {
						return false, fmt.Errorf("Returned workload was nil for %s", wlName)
					}
					if len(wl.Spec.Interfaces) == 0 {
						return false, fmt.Errorf("Workload %s had no interfaces", wlName)
					}
					for _, inf := range wl.Spec.Interfaces {
						if inf.Network != "pg1" {
							return false, fmt.Errorf("interface did not have network set correctly - found %s", inf.Network)
						}
						if inf.ExternalVlan != 0 {
							return false, fmt.Errorf("interface's external vlan was not 0, found %d", inf.ExternalVlan)
						}
					}
				}
				opts := api.ListWatchOptions{}
				workloadObjs, err := sm.Controller().Workload().List(context.Background(), &opts)
				AssertOk(t, err, "failed to get workloads")
				if len(wlNames) != len(workloadObjs) {
					return false, fmt.Errorf("expected %d workloads but got %d", len(wlNames), len(workloadObjs))
				}
			}
			return true, nil
		}, errMsg, "1s", "10s")
	}

	dcWorkloadMap := map[string][]string{
		defaultTestParams.TestDCName: []string{workloadExisting.Name, vchub.createVMWorkloadName(dc1.Obj.Self.Value, vmNew.Self.Value)},
	}

	verifyWorkloads(dcWorkloadMap, "Failed to verify workloads")

	dvsPorts, err := mockProbe.GetPenDVSPorts(dc1.Obj.Name, dvs.Obj.Name, &types.DistributedVirtualSwitchPortCriteria{}, retryCount)
	AssertOk(t, err, "Failed to get port info")
	overrides := vchub.extractOverrides(dvsPorts)
	AssertEquals(t, 2, len(overrides), "2 ports should have vlan override, overrideMap: %v", overrides)
	for port, override := range overrides {
		if port == vmExistingPort {
			AssertEquals(t, 3000, override, "vmExisting didn't have override set in workload object")
		}
	}

	// Change one of the overrides
	// Resync should set it back to its correct value
	portUpdate = vcprobe.PenDVSPortSettings{
		vmExistingPort: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(3002),
		},
	}
	mockProbe.UpdateDVSPortsVlan(dc1.Obj.Name, dvs.Obj.Name, portUpdate, retryCount)

	vchub.Sync()

	AssertEventually(t, func() (bool, interface{}) {
		dvsPorts, err := mockProbe.GetPenDVSPorts(dc1.Obj.Name, dvs.Obj.Name, &types.DistributedVirtualSwitchPortCriteria{}, retryCount)
		AssertOk(t, err, "Failed to get port info")
		overrides := vchub.extractOverrides(dvsPorts)
		if len(overrides) != 2 {
			return false, fmt.Errorf("Expected only 2 overrides, %v", overrides)
		}
		for port, override := range overrides {
			if port == vmExistingPort {
				if override != 3000 {
					return false, fmt.Errorf("Override was %d, not 3000", override)
				}
			}
		}
		return true, nil
	}, "Override was not set back")

}

func TestVCSyncVmkNics(t *testing.T) {
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	// SETTING UP LOGGER
	config := log.GetDefaultConfig("sync_test")
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
	mockProbe.AddPenPG(defaultTestParams.TestDCName, dvsName, &pgConfigSpec[0], retryCount)
	pg, err := mockProbe.GetPenPG(defaultTestParams.TestDCName, CreatePGName("vMotion_PG"), retryCount)
	AssertOk(t, err, "failed to add portgroup")

	// Create Host
	host, err := dc.AddHost("host1")
	AssertOk(t, err, "failed to add Host to DC")
	err = dvs.AddHost(host)
	AssertOk(t, err, "failed to add Host to DVS")

	pNicMac := append(createPenPnicBase(), 0xbb, 0x00, 0x00)
	// Make it Pensando host
	err = host.AddNic("vmnic0", conv.MacString(pNicMac))
	AssertOk(t, err, "failed to add pNic")

	type testEP struct {
		mac  string
		vlan uint32
	}

	type WlMap map[string]map[string]testEP
	testWorkloadMap := WlMap{}
	testNICs := map[string]testEP{}

	// Create vmkNIC
	var spec types.HostVirtualNicSpec
	spec.Mac = "0011.2233.0001"
	var dvPort types.DistributedVirtualSwitchPortConnection
	dvPort.PortgroupKey = pg.Reference().Value
	dvPort.PortKey = "10" // use some port number
	spec.DistributedVirtualPort = &dvPort
	err = host.AddVmkNic(&spec, "vmk1")
	testNICs[spec.Mac] = testEP{
		mac:  spec.Mac,
		vlan: 500,
	}
	AssertOk(t, err, "failed to add vmkNic")

	spec.Mac = "0011.2233.0002"
	var dvPort2 types.DistributedVirtualSwitchPortConnection
	dvPort2.PortgroupKey = pg.Reference().Value
	dvPort2.PortKey = "11" // use some port number
	spec.DistributedVirtualPort = &dvPort2
	err = host.AddVmkNic(&spec, "vmk2")
	AssertOk(t, err, "failed to add vmkNic")
	testNICs[spec.Mac] = testEP{
		mac:  spec.Mac,
		vlan: 500,
	}

	logger.Infof("===== Sync1 =====")
	vchub.Sync()

	wlName := vchub.createVmkWorkloadName(dc.Obj.Self.Value, host.Obj.Self.Value)
	testWorkloadMap[wlName] = testNICs

	// Add Validations
	// Check that workload for the host with its vmknics as EPs is created
	verifyVmkworkloads := func(testWlEPMap WlMap, msg string) {
		AssertEventually(t, func() (bool, interface{}) {
			opts := api.ListWatchOptions{}
			wls, err := sm.Controller().Workload().List(context.Background(), &opts)
			if err != nil {
				logger.Infof("Cannot get workloads - err %s", err)
				return false, nil
			}
			if len(wls) != len(testWlEPMap) {
				logger.Infof("Got %d workloads, Expected %d", len(wls), len(testWlEPMap))
				return false, nil
			}
			for wlname, testEPs := range testWlEPMap {
				meta := &api.ObjectMeta{
					Name:      wlname,
					Tenant:    "default",
					Namespace: "default",
				}
				wl, err := sm.Controller().Workload().Find(meta)
				if err != nil {
					logger.Infof("Workload not found %s", wlname)
					return false, nil
				}
				for _, ep := range wl.Workload.Spec.Interfaces {
					_, ok := testEPs[ep.MACAddress]
					if !ok {
						logger.Infof("EP not found %s", ep.MACAddress)
						return false, nil
					}
				}
				if len(wl.Workload.Spec.Interfaces) != len(testEPs) {
					logger.Infof("Got %d interface, Expected %d", len(wl.Workload.Spec.Interfaces), len(testEPs))
					return false, nil
				}
			}
			return true, nil
		}, msg, "1s", "10s")
	}
	verifyVmkworkloads(testWorkloadMap, "WL with 2EPs create failed")
	host.RemoveVmkNic("vmk2")
	delete(testNICs, "0011.2233.0002")
	spec.Mac = "0011.2233.0003"
	var dvPort3 types.DistributedVirtualSwitchPortConnection
	dvPort3.PortgroupKey = pg.Reference().Value
	dvPort3.PortKey = "12" // use some port number
	spec.DistributedVirtualPort = &dvPort3
	err = host.AddVmkNic(&spec, "vmk3")
	AssertOk(t, err, "failed to add vmkNic")
	testNICs[spec.Mac] = testEP{
		mac:  spec.Mac,
		vlan: 500,
	}
	logger.Infof("===== Sync2 =====")
	vchub.Sync()
	verifyVmkworkloads(testWorkloadMap, "WL delete 1 EP failed")

	host.RemoveVmkNic("vmk1")
	host.RemoveVmkNic("vmk3")
	delete(testNICs, "0011.2233.0001")
	delete(testNICs, spec.Mac)
	delete(testWorkloadMap, wlName)

	logger.Infof("===== Sync3 =====")
	vchub.Sync()
	verifyVmkworkloads(testWorkloadMap, "WL delete all EPs failed")

	// Start the watcher, add vmkNic to the host
	// vc sim does not deliver events if objects are created after starting the watch.. so create
	// them before watch is called
	logger.Infof("===== Watch =====")
	err = host.AddVmkNic(&spec, "vmk1")
	testNICs[spec.Mac] = testEP{
		mac:  spec.Mac,
		vlan: 500,
	}
	AssertOk(t, err, "failed to add vmkNic")
	testWorkloadMap[wlName] = testNICs
	vchub.Wg.Add(1)
	go vchub.startEventsListener()
	vchub.probe.StartWatchers()
	verifyVmkworkloads(testWorkloadMap, "VmkWorkload create with EP failed via watch")
}

func setupTestVCHub(vcURL *url.URL, stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger, opts ...Option) *VCHub {
	ctx, cancel := context.WithCancel(context.Background())

	state := defs.State{
		VcURL:      vcURL,
		VcID:       config.GetName(),
		Ctx:        ctx,
		Log:        logger.WithContext("submodule", fmt.Sprintf("VCHub-%s", config.GetName())),
		StateMgr:   stateMgr,
		OrchConfig: config,
		Wg:         &sync.WaitGroup{},
	}
	vchub := &VCHub{}
	vchub.State = &state
	vchub.cancel = cancel
	vchub.DcMap = map[string]*PenDC{}
	vchub.vcReadCh = make(chan defs.Probe2StoreMsg, storeQSize)
	vchub.vcEventCh = make(chan defs.Probe2StoreMsg, storeQSize)
	vchub.setupPCache()

	clusterItems, err := stateMgr.Controller().Cluster().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		logger.Errorf("Failed to get cluster object, %s", err)
	} else if len(clusterItems) == 0 {
		logger.Errorf("Cluster list returned 0 objects, %s", err)
	} else {
		cluster := clusterItems[0]
		state.ClusterID = defs.CreateClusterID(cluster.Cluster)
	}

	return vchub
}

func createHostObj(name, id, macAddress string) cluster.Host {
	host := cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
			Labels: map[string]string{},
		},
		TypeMeta: api.TypeMeta{
			Kind: "Host",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				cluster.DistributedServiceCardID{
					ID:         id,
					MACAddress: macAddress,
				},
			},
		},
	}
	return host
}

func createWorkloadObj(name, host string, infs []workload.WorkloadIntfSpec) workload.Workload {
	workload := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    "default",
			Labels:    map[string]string{},
		},
		TypeMeta: api.TypeMeta{
			Kind: "Workload",
		},
		Spec: workload.WorkloadSpec{
			HostName:   host,
			Interfaces: infs,
		},
	}
	return workload
}

func createPenPnicBase() net.HardwareAddr {
	pNicMac := net.HardwareAddr{}
	pNicMac = append(pNicMac, globals.PensandoOUI[0], globals.PensandoOUI[1], globals.PensandoOUI[2])
	return pNicMac
}
