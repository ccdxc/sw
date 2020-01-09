package vchub

import (
	"context"
	"fmt"
	"net/url"
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
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Tests creation of internal DC state, creationg of DVS
// and creation/deletion of networks in venice
// trigger respective events in VC
func TestVCSyncPG(t *testing.T) {
	// Stale PGs should be deleted
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
		Scheme: "http",
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
	_, err = dc1.AddDVS(dvsCreateSpec)
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

	time.Sleep(1 * time.Second)

	// SETTING UP MOCK
	// Real probe that will be used by mock probe when possible
	vchub := setupVCHub(vcURL, sm, orchConfig, logger)
	vcp := vcprobe.NewVCProbe(vchub.vcReadCh, vchub.State)
	mockProbe := mock.NewProbeMock(vcp)
	vchub.probe = mockProbe
	mockProbe.Start()

	defer vchub.Destroy()

	vchub.Sync()

	verifyPg := func(dcPgMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, pgNames := range dcPgMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
				}
				dvs := dc.GetPenDVS(createDVSName(name))
				if dvs == nil {
					return false, fmt.Errorf("Failed to find dvs in DC %s", name)
				}

				for _, pgName := range pgNames {
					pgObj := dvs.GetPenPG(pgName)
					if pgObj == nil {
						return false, fmt.Errorf("Failed to find %s in DC %s", pgName, name)
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
	pg1 := createPGName("pg1")
	pg2 := createPGName("pg2")

	dcPgMap := map[string][]string{
		defaultTestParams.TestDCName: []string{pg1, pg2},
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
		Scheme: "http",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	vcURL.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: vcURL.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	hostSystem1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed host1 create")
	hostSystem2, err := dc1.AddHost("host2")
	AssertOk(t, err, "failed host2 create")

	// CREATING HOSTS
	staleHost := cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:      utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, "hostsystem-00000"),
			Namespace: "default",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Host",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				cluster.DistributedServiceCardID{
					ID: "test",
				},
			},
		},
	}
	sm.Controller().Host().Create(&staleHost)
	host1 := cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:      utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, hostSystem1.Obj.Self.Value),
			Namespace: "default",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Host",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				cluster.DistributedServiceCardID{
					ID: "test1",
				},
			},
		},
	}
	sm.Controller().Host().Create(&host1)

	time.Sleep(1 * time.Second)

	// SETTING UP MOCK
	// Real probe that will be used by mock probe when possible
	vchub := setupVCHub(vcURL, sm, orchConfig, logger)
	vcp := vcprobe.NewVCProbe(vchub.vcReadCh, vchub.State)
	mockProbe := mock.NewProbeMock(vcp)
	vchub.probe = mockProbe
	mockProbe.Start()

	defer vchub.Destroy()

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
					return false, fmt.Errorf("expected %d hosts but only got %d", len(hostnames), len(hosts))
				}
			}
			return true, nil
		}, "Failed to find hosts")
	}

	dcHostMap := map[string][]string{
		defaultTestParams.TestDCName: []string{host1.Name, utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, hostSystem2.Obj.Self.Value)},
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
		Scheme: "http",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	vcURL.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: vcURL.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	hostSystem1, err := dc1.AddHost("host1")
	AssertOk(t, err, "failed host1 create")

	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)
	dvs, err := dc1.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")

	// SETTING UP MOCK
	// Real probe that will be used by mock probe when possible
	vchub := setupVCHub(vcURL, sm, orchConfig, logger)
	vcp := vcprobe.NewVCProbe(vchub.vcReadCh, vchub.State)
	mockProbe := mock.NewProbeMock(vcp)
	vchub.probe = mockProbe
	mockProbe.Start()

	time.Sleep(3 * time.Second)

	spec := testutils.GenPGConfigSpec(createPGName("pg1"), 2, 3)
	err = mockProbe.AddPenPG(dc1.Obj.Name, dvs.Obj.Name, &spec)
	AssertOk(t, err, "failed to create pg")
	pg1, err := mockProbe.GetPenPG(dc1.Obj.Name, createPGName("pg1"))
	AssertOk(t, err, "failed to get pg")

	defer vchub.Destroy()

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
	vnNew, err := dc1.AddVM("vnNew", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aaaa.bbbb.ddde",
			PortgroupKey: pg1.Reference().Value,
			PortKey:      "11",
		},
	})
	AssertOk(t, err, "Failed to create vnNew")

	// CREATING HOSTS
	host1 := cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:      utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, hostSystem1.Obj.Self.Value),
			Namespace: "default",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Host",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				cluster.DistributedServiceCardID{
					ID: "test1",
				},
			},
		},
	}
	sm.Controller().Host().Create(&host1)

	// CREATING WORKLOADS
	staleWorkload := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, "staleWorkload"),
			Namespace: "default",
			Tenant:    "default",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Workload",
		},
		Spec: workload.WorkloadSpec{
			HostName: host1.Name,
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					MACAddress:   "aaaa.bbbb.cccc",
					MicroSegVlan: 2000,
				},
			},
		},
	}
	sm.Controller().Workload().Create(&staleWorkload)

	workloadExisting := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, vmExisting.Self.Value),
			Namespace: "default",
			Tenant:    "default",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Workload",
		},
		Spec: workload.WorkloadSpec{
			HostName: host1.Name,
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					MACAddress:   vmExistingMac,
					MicroSegVlan: 3000,
				},
			},
		},
	}
	sm.Controller().Workload().Create(&workloadExisting)

	portUpdate := vcprobe.PenDVSPortSettings{
		vmExistingPort: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
			VlanId: int32(3000),
		},
	}
	mockProbe.UpdateDVSPortsVlan(dc1.Obj.Name, dvs.Obj.Name, portUpdate)

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: dc1.Obj.Name,
		},
	}
	smmock.CreateNetwork(sm, "default", "pg1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)

	time.Sleep(1 * time.Second)

	vchub.Sync()

	verifyWorkloads := func(dcWorkloadMap map[string][]string) {
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
						if inf.ExternalVlan != 100 {
							return false, fmt.Errorf("interface did not have external valn set correctly")
						}
					}
				}
				opts := api.ListWatchOptions{}
				workloadObjs, err := sm.Controller().Workload().List(context.Background(), &opts)
				AssertOk(t, err, "failed to get workloads")
				if len(wlNames) != len(workloadObjs) {
					return false, fmt.Errorf("expected %d workloads but only got %d", len(wlNames), len(workloadObjs))
				}
			}
			return true, nil
		}, "Failed to find hosts")
	}

	dcWorkloadMap := map[string][]string{
		defaultTestParams.TestDCName: []string{workloadExisting.Name, utils.CreateGlobalKey(orchConfig.Name, dc1.Obj.Self.Value, vnNew.Self.Value)},
	}

	verifyWorkloads(dcWorkloadMap)

	dvsPorts, err := mockProbe.GetPenDVSPorts(dc1.Obj.Name, dvs.Obj.Name, &types.DistributedVirtualSwitchPortCriteria{})
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
	mockProbe.UpdateDVSPortsVlan(dc1.Obj.Name, dvs.Obj.Name, portUpdate)

	vchub.Sync()

	AssertEventually(t, func() (bool, interface{}) {
		dvsPorts, err := mockProbe.GetPenDVSPorts(dc1.Obj.Name, dvs.Obj.Name, &types.DistributedVirtualSwitchPortCriteria{})
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

func setupVCHub(vcURL *url.URL, stateMgr *statemgr.Statemgr, config *orchestration.Orchestrator, logger log.Logger, opts ...Option) *VCHub {
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
	vchub.setupPCache()

	return vchub
}
