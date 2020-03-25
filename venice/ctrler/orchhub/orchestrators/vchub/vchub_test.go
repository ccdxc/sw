package vchub

import (
	"context"
	"fmt"
	"net"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/simulator"
	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/usegvlanmgr"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	// create mock events recorder
	eventRecorder = mockevtsrecorder.NewRecorder("vchub_test",
		log.GetNewLogger(log.GetDefaultConfig("vchub_test")))
	_ = recorder.Override(eventRecorder)
)

var defaultTestParams = &testutils.TestParams{
	// TestHostName: "barun-vc.pensando.io",
	// TestUser:     "administrator@pensando.io",
	// TestPassword: "N0isystem$",
	TestHostName: "127.0.0.1:8989",
	TestUser:     "user",
	TestPassword: "pass",

	TestDCName:             "PenTestDC",
	TestDVSName:            CreateDVSName("PenTestDC"),
	TestPGNameBase:         defs.DefaultPGPrefix,
	TestMaxPorts:           4096,
	TestNumStandalonePorts: 512,
	TestNumPVLANPair:       5,
	StartPVLAN:             500,
	TestNumPG:              5,
	TestNumPortsPerPG:      20,
}

// Tests creation of internal DC state, creationg of DVS
// and creation/deletion of networks in venice
// trigger respective events in VC
func TestVCWrite(t *testing.T) {
	// venice network -> PG
	// create/delete
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("vchub_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	_, err = s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	dc2 := "DC2"
	_, err = s.AddDC(dc2)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		s.Destroy()
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	clusterConfig := &cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
		},
	}

	err = sm.Controller().Cluster().Create(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")
	clusterItems, err := sm.Controller().Cluster().List(context.Background(), &api.ListWatchOptions{})
	AssertOk(t, err, "failed to get cluster config")

	clusterID := defs.CreateClusterID(clusterItems[0].Cluster)

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "failed to create orch config")

	vchub := LaunchVCHub(sm, orchConfig, logger)

	// Give time for VCHub to come up
	time.Sleep(2 * time.Second)

	defer func() {
		vchub.Destroy(false)
		defer s.Destroy()
	}()

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	n1, err := smmock.CreateNetwork(sm, "default", "n1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	AssertOk(t, err, "Failed to create network")
	orchInfo2 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
		{
			Name:      orchConfig.Name,
			Namespace: dc2,
		},
	}
	_, err = smmock.CreateNetwork(sm, "default", "n2", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo2)
	AssertOk(t, err, "Failed to create network")

	c, err := govmomi.NewClient(context.Background(), u, true)
	AssertOk(t, err, "Failed to create govmomi client")
	restCl := rest.NewClient(c.Client)
	tagClient := tags.NewManager(restCl)
	err = tagClient.Login(context.Background(), u.User)
	AssertOk(t, err, "Failed to create tags client")

	verifyPg := func(dcPgMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, pgNames := range dcPgMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					err := fmt.Errorf("Failed to find DC %s", name)
					logger.Errorf("%s", err)
					return false, err
				}

				attachedTags, err := tagClient.GetAttachedTags(context.Background(), dc.dcRef)
				AssertOk(t, err, "failed to get tags")
				if len(attachedTags) != 1 {
					return false, fmt.Errorf("DC didn't have expected tags, had %v", attachedTags)
				}
				AssertEquals(t, defs.CreateVCTagManagedTag(clusterID), attachedTags[0].Name, "DC didn't have managed tag")
				AssertEquals(t, defs.VCTagManagedDescription, attachedTags[0].Description, "DC didn't have managed tag")

				dvs := dc.GetPenDVS(CreateDVSName(name))
				if dvs == nil {
					err := fmt.Errorf("Failed to find dvs in DC %s", name)
					logger.Errorf("%s", err)
					return false, err
				}

				attachedTags, err = tagClient.GetAttachedTags(context.Background(), dvs.DvsRef)
				AssertOk(t, err, "failed to get tags")
				if len(attachedTags) != 1 {
					return false, fmt.Errorf("DVS didn't have expected tags, had %v", attachedTags)
				}
				AssertEquals(t, defs.CreateVCTagManagedTag(clusterID), attachedTags[0].Name, "DVS didn't have managed tag")

				for _, pgName := range pgNames {
					pgObj := dvs.GetPenPG(pgName)
					if pgObj == nil {
						err := fmt.Errorf("Failed to find %s in DC %s", pgName, name)
						logger.Errorf("%s", err)
						return false, err
					}
					attachedTags, err := tagClient.GetAttachedTags(context.Background(), pgObj.PgRef)
					AssertOk(t, err, "failed to get tags")
					if len(attachedTags) != 2 {
						return false, fmt.Errorf("PG didn't have expected tags, had %v", attachedTags)
					}
					expTags := []string{
						fmt.Sprintf("%s", defs.CreateVCTagManagedTag(clusterID)),
						fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 100),
					}
					for _, tag := range attachedTags {
						AssertOneOf(t, tag.Name, expTags)
					}
				}
				dvs.Lock()
				if len(dvs.Pgs) != len(pgNames) {
					err := fmt.Errorf("PG length didn't match: exp %v, actual %v", pgNames, dvs.Pgs)
					dvs.Unlock()
					logger.Errorf("%s", err)
					return false, err
				}
				dvs.Unlock()
			}
			return true, nil
		}, "Failed to find PGs")
	}

	// n1 should only be in defaultDC
	// n2 should be in both
	pg1 := CreatePGName("n1")
	pg2 := CreatePGName("n2")

	dcPgMap := map[string][]string{
		defaultTestParams.TestDCName: []string{pg1, pg2},
		dc2:                          []string{pg2},
	}

	verifyPg(dcPgMap)

	// Update n1 to be in dc2 only
	n1.Spec.Orchestrators = []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: dc2,
		},
	}
	err = sm.Controller().Network().Update(n1)
	AssertOk(t, err, "Failed to update host")

	dcPgMap = map[string][]string{
		defaultTestParams.TestDCName: []string{pg2},
		dc2:                          []string{pg1, pg2},
	}

	verifyPg(dcPgMap)

	// Delete n2
	n2 := &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "n2",
			Namespace: "default",
			Tenant:    "default",
		},
	}
	sm.Controller().Network().Delete(n2)

	dcPgMap = map[string][]string{
		defaultTestParams.TestDCName: []string{},
		dc2:                          []string{pg1},
	}

	verifyPg(dcPgMap)

	// Delete n1
	n1 = &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "n1",
			Namespace: "default",
			Tenant:    "default",
		},
	}
	sm.Controller().Network().Delete(n1)

	dcPgMap = map[string][]string{
		defaultTestParams.TestDCName: []string{},
		dc2:                          []string{},
	}
	verifyPg(dcPgMap)

}

func TestVCHub(t *testing.T) {
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("vchub_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	_, err = s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)

	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(defaultTestParams.TestDCName)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", defaultTestParams.TestDCName)
		}
		dvs := dc.GetPenDVS(defaultTestParams.TestDVSName)
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", defaultTestParams.TestDCName)
		}
		return true, nil
	}, "failed to find DVS")

	// Update with new config
	orchConfig.Spec.URI = "127.0.0.1:9999"

	vchub.UpdateConfig(orchConfig)

	// Wait for vchub to restart
	time.Sleep(3 * time.Second)

	vchub.Destroy(false)
}

func TestVCHubDestroy1(t *testing.T) {
	// Destroy while vcenter can't be reached
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("vchub_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	probe := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = probe.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, retryCount)

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

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)

	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 1 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

	// Update with new config
	orchConfig.Spec.URI = "127.0.0.1:9999"

	vchub.UpdateConfig(orchConfig)

	// Wait for vchub to restart
	time.Sleep(3 * time.Second)

	vchub.Destroy(true)

	// Verify hosts are removed from statemgr
	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 0 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

}

func TestVCHubDestroy2(t *testing.T) {
	// Destroy while vcenter can't be reached
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("vchub_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	probe := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = probe.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, retryCount)

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

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)

	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 1 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

	vchub.Destroy(true)

	// Verify hosts are removed from statemgr
	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 0 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

}

func TestVCHubDestroy3(t *testing.T) {
	// Destroy while sync happens
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("vchub_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	probe := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = probe.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, retryCount)
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

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)

	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 1 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

	vchub.Sync()
	vchub.Destroy(true)

	// Verify hosts are removed from statemgr
	AssertEventually(t, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		hosts, err := sm.Controller().Host().List(context.Background(), opts)
		if err != nil {
			return false, err
		}
		if len(hosts) != 0 {
			return false, fmt.Errorf("Incorrect number of hosts %d, %v", len(hosts), hosts)
		}
		return true, nil
	}, "failed to find correct number of hosts")

}

func createProbe(ctx context.Context, uri, user, pass string) *mock.ProbeMock {
	u := &url.URL{
		Scheme: "https",
		Host:   uri,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, pass)

	state := defs.State{
		VcURL: u,
		VcID:  "VCProbe",
		Ctx:   ctx,
		Log:   logger.WithContext("submodule", "vcprobe"),
		Wg:    &sync.WaitGroup{},
	}
	vcp := vcprobe.NewVCProbe(nil, nil, &state)
	mockProbe := mock.NewProbeMock(vcp)
	mockProbe.Started = true
	mockProbe.Wg.Add(1)
	go mockProbe.PeriodicSessionCheck(mockProbe.Wg)

	return mockProbe
}

// Test thrashing DC api
func TestDCWatchers(t *testing.T) {
	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	config := log.GetDefaultConfig("vcprobe_testDcWatcher")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	// Give time for VCHub to come up
	time.Sleep(2 * time.Second)

	defer func() {
		vchub.Destroy(false)
		defer s.Destroy()
	}()

	vcp := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready")

	var wg sync.WaitGroup
	for i := 0; i < 5; i++ {
		dcName := fmt.Sprintf("DC%d", i)
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := 0; j < 6; j++ {
				if j%2 == 0 {
					logger.Infof("--------- Create %s ---------", dcName)
					err := vcp.AddPenDC(dcName, 5)
					AssertOk(t, err, "failed dc create")
					// vcp.StartWatchForDC(dcName, dcName)
				} else {
					logger.Infof("--------- Delete %s ---------", dcName)
					err := vcp.RemovePenDC(dcName, 5)
					AssertOk(t, err, "failed dc create")
					// vcp.StopWatchForDC(dcName, dcName)
				}
			}
		}()
	}
	wg.Wait()
	AssertEquals(t, 0, len(vchub.DcMap), "DC map length did not match")
}

func TestUsegVlanLimit(t *testing.T) {
	// Lower the limit for testing
	usegvlanmgr.VlanMax = useg.FirstUsegVlan + 100

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	config := log.GetDefaultConfig("vcprobe_testVlanLimit")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	probe := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !probe.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = probe.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, retryCount)
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

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	smmock.CreateNetwork(sm, "default", "pg1", "11.1.1.0/24", "11.1.1.1", 500, nil, orchInfo1)

	vchub := LaunchVCHub(sm, orchConfig, logger, WithMockProbe)

	// Give time for VCHub to come up
	time.Sleep(2 * time.Second)

	defer func() {
		vchub.Destroy(false)
		defer s.Destroy()
	}()

	pgName := CreatePGName("pg1")

	pgID := ""
	// Verify PG is created
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(defaultTestParams.TestDCName)
		dvs := dc.GetPenDVS(CreateDVSName(defaultTestParams.TestDCName))
		if dvs == nil {
			err := fmt.Errorf("Failed to find dvs in DC")
			logger.Errorf("%s", err)
			return false, err
		}
		pgObj := dvs.GetPenPG(pgName)
		if pgObj == nil {
			err := fmt.Errorf("Failed to find %s in DC", pgName)
			logger.Errorf("%s", err)
			return false, err
		}
		pgID = pgObj.PgRef.Value
		return true, nil
	}, "failled to find PG")

	toChar := func(i int) string {
		return string(rune('a' + i))
	}

	// Send events on the channel to simulate the VMs
	vmCount := 25
	for i := 0; i < vmCount; i++ {
		// 4 vnics per vm
		evt := defs.Probe2StoreMsg{
			MsgType: defs.VCEvent,
			Val: defs.VCEventMsg{
				VcObject:   defs.VirtualMachine,
				DcID:       dc1.Obj.Self.Value,
				DcName:     dc1.Obj.Name,
				Key:        fmt.Sprintf("vm-%d", i),
				Originator: vchub.OrchID,
				Changes: []types.PropertyChange{
					types.PropertyChange{
						Op:   types.PropertyChangeOpAdd,
						Name: "config",
						Val: types.VirtualMachineConfigInfo{
							Hardware: types.VirtualHardware{
								Device: []types.BaseVirtualDevice{
									generateVNIC(fmt.Sprintf("a%s:f%s:ff:ff:ff:ff", toChar(i/5), toChar(i%5)), "10", pgID, "E1000e"),
									generateVNIC(fmt.Sprintf("a%s:ff:f%s:ff:ff:ff", toChar(i/5), toChar(i%5)), "10", pgID, "E1000e"),
									generateVNIC(fmt.Sprintf("a%s:ff:ff:f%s:ff:ff", toChar(i/5), toChar(i%5)), "10", pgID, "E1000e"),
									generateVNIC(fmt.Sprintf("a%s:ff:ff:ff:f%s:ff", toChar(i/5), toChar(i%5)), "10", pgID, "E1000e"),
								},
							},
						},
					},
					types.PropertyChange{
						Op:   types.PropertyChangeOpAdd,
						Name: "runtime",
						Val: types.VirtualMachineRuntimeInfo{
							Host: &types.ManagedObjectReference{
								Type:  "HostSystem",
								Value: hostSystem1.Obj.Self.Value,
							},
						},
					},
				},
			},
		}
		vchub.vcReadCh <- evt
	}

	dc := vchub.GetDC(dc1.Obj.Name)
	penDVS := dc.GetPenDVS(CreateDVSName(dc1.Obj.Name))
	AssertEventually(t, func() (bool, interface{}) {
		count, err := penDVS.UsegMgr.GetRemainingVnicCount("orch0--host-18")
		if err != nil {
			return false, err
		}
		if count != 0 {
			return false, fmt.Errorf("Remaining vlan count %d", count)
		}
		return count == 0, err
	}, "vlan remaining count never reached 0", "1s", "20s")

	// Should be at capacity now. Next vnic should fail
	eventRecorder.ClearEvents()
	evt := defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dc1.Obj.Self.Value,
			DcName:     dc1.Obj.Name,
			Key:        fmt.Sprintf("vm-%d", vmCount+1),
			Originator: vchub.OrchID,
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "config",
					Val: types.VirtualMachineConfigInfo{
						Hardware: types.VirtualHardware{
							Device: []types.BaseVirtualDevice{
								generateVNIC("ff:ff:ff:ff:ff:ff", "10", pgID, "E1000e"),
							},
						},
					},
				},
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "runtime",
					Val: types.VirtualMachineRuntimeInfo{
						Host: &types.ManagedObjectReference{
							Type:  "HostSystem",
							Value: hostSystem1.Obj.Self.Value,
						},
					},
				},
			},
		},
	}
	vchub.vcReadCh <- evt

	// Should have gotten vlan limit failure
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_CONFIG_PUSH_FAILURE.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find vlan failure event", "1s", "10s")
}

func TestRapidEvents(t *testing.T) {
	numDCs := 1
	numHosts := 10 // Hosts per DC
	numVMs := 10   // VMs per Host
	numVNICs := 4  // VNICs per VM

	// STARTING SIM
	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	config := log.GetDefaultConfig("vcprobe_testRapidEvents")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger, WithMockProbe)

	defer func() {
		vchub.Destroy(false)
		defer s.Destroy()
	}()

	// Give time for VCHub to come up
	time.Sleep(2 * time.Second)
	vcsimLock := sync.Mutex{}

	addDC := func(dcName string) *sim.Datacenter {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		dc, err := s.AddDC(dcName)
		dc.Obj.Name = dcName
		AssertOk(t, err, "failed to create DC %s", dcName)
		vchub.vcReadCh <- createDCEvent(dcName, dc.Obj.Self.Value)
		return dc
	}

	removeDC := func(dc *sim.Datacenter) {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		dc.Destroy()
		vchub.vcReadCh <- deleteDCEvent(dc.Obj.Self.Value)
	}

	addHost := func(dc *sim.Datacenter, hostName string, pnic net.HardwareAddr) *sim.Host {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		hostSystem, err := dc.AddHost("host1")
		AssertOk(t, err, "failed host create")
		dvs, ok := dc.GetDVS(CreateDVSName(dc.Obj.Name))
		Assert(t, ok, "failed to get dvs for DC %s", dc.Obj.Name)

		err = dvs.AddHost(hostSystem)
		AssertOk(t, err, "failed to add Host to DVS")

		macStr := conv.MacString(pnic)
		err = hostSystem.AddNic("vmnic0", macStr)
		// Generate host create event
		vchub.vcReadCh <- createHostEvent(dc.Obj.Name, dc.Obj.Self.Value, hostName, hostSystem.Obj.Self.Value, dvs.Obj.Name, macStr)
		return hostSystem
	}

	removeHost := func(dc *sim.Datacenter, host *sim.Host) {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		err := host.Destroy()
		AssertOk(t, err, "Failed to delete host")
		vchub.vcReadCh <- deleteHostEvent(dc.Obj.Name, dc.Obj.Self.Value, host.Obj.Self.Value)
	}

	addVM := func(dc *sim.Datacenter, vmName string, host *sim.Host, vnics []sim.VNIC) *simulator.VirtualMachine {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		vm, err := dc.AddVM(vmName, host.Obj.Name, vnics)
		AssertOk(t, err, "failed to create VM")
		vchub.vcReadCh <- createVMEvent(dc.Obj.Name, dc.Obj.Self.Value, vmName, vm.Self.Value, host.Obj.Self.Value, vnics)
		return vm
	}

	removeVM := func(dc *sim.Datacenter, vm *simulator.VirtualMachine) {
		vcsimLock.Lock()
		defer vcsimLock.Unlock()
		err := dc.DeleteVM(vm)
		AssertOk(t, err, "failed to delete VM")
		vchub.vcReadCh <- deleteVMEvent(dc.Obj.Name, dc.Obj.Self.Value, vm.Self.Value)
	}

	vcp := createProbe(context.Background(), defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready")

	wg := sync.WaitGroup{}
	for i := 0; i < numDCs; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			dcName := fmt.Sprintf("DC%d", i)

			orchInfo1 := []*network.OrchestratorInfo{
				{
					Name:      orchConfig.Name,
					Namespace: dcName,
				},
			}
			nwName := fmt.Sprintf("PG-%d", i)
			nw, err := smmock.CreateNetwork(sm, "default", nwName, "11.1.1.0/24", "11.1.1.1", 500, nil, orchInfo1)
			AssertOk(t, err, "Faield to create network")

			dc := addDC(dcName)

			pgName := CreatePGName(nw.Name)
			pgID := ""
			// Verify PG is created
			AssertEventually(t, func() (bool, interface{}) {
				dc := vchub.GetDC(dcName)
				if dc == nil {
					return false, fmt.Errorf("DC does not exist")
				}
				dvs := dc.GetPenDVS(CreateDVSName(dcName))
				if dvs == nil {
					err := fmt.Errorf("Failed to find dvs in DC %s", dcName)
					logger.Errorf("%s", err)
					return false, err
				}
				pgObj := dvs.GetPenPG(pgName)
				if pgObj == nil {
					err := fmt.Errorf("Failed to find %s in DC", pgName)
					logger.Errorf("%s", err)
					return false, err
				}
				pgID = pgObj.PgRef.Value
				return true, nil
			}, "failled to find PG")

			for j := 0; j < numHosts; j++ {
				pNicMac := append(createPenPnicBase(), 0xaa, byte(i/256), byte(j%256))
				hostName := fmt.Sprintf("host-%d-%d", i, j)
				host := addHost(dc, hostName, pNicMac)
				for k := 0; k < numVMs; k++ {
					vnics := []sim.VNIC{}
					for l := 0; l < numVNICs; l++ {
						mac := append(net.HardwareAddr{}, 0xaa, byte(i/256), byte(j%256), 0xaa, byte(k/256), byte(l%256))
						vnic := sim.VNIC{
							PortKey:      "10",
							PortgroupKey: pgID,
							MacAddress:   conv.MacString(mac),
						}
						vnics = append(vnics, vnic)
					}
					vm := addVM(dc, fmt.Sprintf("vm-%d-%d-%d", i, j, k), host, vnics)
					// Immediately delete VM
					removeVM(dc, vm)
				}
				// Delete host
				removeHost(dc, host)
			}
			removeDC(dc)
		}(i)
	}

	wg.Wait()

	// Verify vchub state
	AssertEventually(t, func() (bool, interface{}) {
		vchub.DcMapLock.Lock()
		defer vchub.DcMapLock.Unlock()
		if len(vchub.DcMap) != 0 {
			return false, fmt.Errorf("found %d DCs", len(vchub.DcMap))
		}

		workloads, err := sm.Controller().Workload().List(context.Background(), &api.ListWatchOptions{})
		if err != nil {
			return false, err
		}
		if len(workloads) != 0 {
			return false, fmt.Errorf("Found %d workloads", len(workloads))
		}

		hosts, err := sm.Controller().Host().List(context.Background(), &api.ListWatchOptions{})
		if err != nil {
			return false, err
		}
		if len(hosts) != 0 {
			return false, fmt.Errorf("Found %d hosts", len(hosts))
		}
		return true, nil
	}, "state wasn't cleaned up")

}

func createVMEvent(dcName, dcID, vmName, vmID, hostID string, vnics []sim.VNIC) defs.Probe2StoreMsg {
	devices := []types.BaseVirtualDevice{}
	for _, vnic := range vnics {
		devices = append(devices, generateVNIC(vnic.MacAddress, vnic.PortKey, vnic.PortgroupKey, "E1000e"))
	}
	// Generate host create event
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dcID,
			DcName:     dcName,
			Key:        vmID,
			Originator: "127.0.0.1:8990",
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

func deleteVMEvent(dcName, dcID, vmID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dcID,
			DcName:     dcName,
			Key:        vmID,
			Originator: "127.0.0.1:8990",
			Changes:    []types.PropertyChange{},
			UpdateType: types.ObjectUpdateKindLeave,
		},
	}
}

func createHostEvent(dcName, dcID, hostName, hostID, dvsName, macStr string) defs.Probe2StoreMsg {
	// Generate host create event
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.HostSystem,
			DcID:       dcID,
			DcName:     dcName,
			Key:        hostID,
			Originator: "127.0.0.1:8990",
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "name",
					Val:  hostName,
				},
				types.PropertyChange{
					Op:   types.PropertyChangeOpAdd,
					Name: "config",
					Val: types.HostConfigInfo{
						Network: &types.HostNetworkInfo{
							Pnic: []types.PhysicalNic{
								types.PhysicalNic{
									Mac: macStr,
								},
							},
							ProxySwitch: []types.HostProxySwitch{
								types.HostProxySwitch{
									DvsName: dvsName,
								},
							},
						},
					},
				},
			},
		},
	}
}

func deleteHostEvent(dcName, dcID, hostID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.VirtualMachine,
			DcID:       dcID,
			DcName:     dcName,
			Key:        hostID,
			Originator: "127.0.0.1:8990",
			Changes:    []types.PropertyChange{},
			UpdateType: types.ObjectUpdateKindLeave,
		},
	}
}

func createDCEvent(dcName, dcID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.Datacenter,
			Key:        dcID,
			Originator: "127.0.0.1:8990",
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:  types.PropertyChangeOpAdd,
					Val: dcName,
				},
			},
		},
	}
}

func deleteDCEvent(dcID string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.Datacenter,
			Key:        dcID,
			Originator: "127.0.0.1:8990",
			Changes:    []types.PropertyChange{},
			UpdateType: types.ObjectUpdateKindLeave,
		},
	}
}

// Test update URL
func TestUpdateUrl(t *testing.T) {
	listener := netutils.TestListenAddr{}

	err := listener.GetAvailablePort()
	AssertOk(t, err, "Failed to get available port")
	newURL := listener.ListenURL.String()

	err = listener.GetAvailablePort()
	AssertOk(t, err, "Failed to get available port")
	badURL := listener.ListenURL.String()

	config := log.GetDefaultConfig("vchub_testUpdateUrl")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	// vcsim based on old URL
	s1, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s1.Destroy()

	// vcsim based on new URL
	u.Host = newURL
	s2, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s2.Destroy()

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)
	defer vchub.Destroy(false)

	AssertEventually(t, func() (bool, interface{}) {
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}

		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

	// Update with new URL, expect to see the connection established succesfully
	orchConfig.Spec.URI = newURL
	vchub.UpdateConfig(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}

		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

	// Update with bad URL, expect to see connection failure
	orchConfig.Spec.URI = badURL
	vchub.UpdateConfig(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "5s")
}

// Test update Orchestrator config credential information
func TestUpdateOrchConfigCredential(t *testing.T) {
	config := log.GetDefaultConfig("vchub_testUpdateOrchConfigCredential")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)
	defer vchub.Destroy(false)

	AssertEventually(t, func() (bool, interface{}) {
		// Check if the first connection got established successfully
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

	// Update orchestrator config with incorrect password, expect to see connection failure
	orchConfig.Spec.Credentials.Password = "badpw"
	vchub.UpdateConfig(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "5s")

	// Update orchestrator with correct password, expect to see connection established succesfully
	orchConfig.Spec.Credentials.Password = defaultTestParams.TestPassword
	vchub.UpdateConfig(orchConfig)
	AssertEventually(t, func() (bool, interface{}) {
		o, err := vchub.StateMgr.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")
}

// Test multiple VC connections(vcsim connections)
func TestMultipleVcs(t *testing.T) {
	numConn := 4
	listener := netutils.TestListenAddr{}
	urls := make([]string, numConn)
	vcsims := make([]*sim.VcSim, numConn)
	orchConfigs := make([]*orchestration.Orchestrator, numConn)
	vchubs := make([]*VCHub, numConn)

	config := log.GetDefaultConfig("vchub_testMultipleVcs")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	u := &url.URL{
		Scheme: "https",
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)
	for i := 0; i < len(urls); i++ {
		err := listener.GetAvailablePort()
		AssertOk(t, err, "Failed to get available port")
		urls[i] = listener.ListenURL.String()
		u.Host = urls[i]
		vcsims[i], err = sim.NewVcSim(sim.Config{Addr: u.String()})
		AssertOk(t, err, "Failed to create vcsim")
		defer vcsims[i].Destroy()
	}

	time.Sleep(2 * time.Second)

	for i := 0; i < len(urls); i++ {
		orchConfigs[i] = smmock.GetOrchestratorConfig(urls[i], defaultTestParams.TestUser, defaultTestParams.TestPassword)
		orchConfigs[i].Status.OrchID = int32(i)
		err = sm.Controller().Orchestrator().Create(orchConfigs[i])
		vchubs[i] = LaunchVCHub(sm, orchConfigs[i], logger)
		defer vchubs[i].Destroy(false)
	}

	AssertEventually(t, func() (bool, interface{}) {
		// Validate connection status
		for i := 0; i < len(urls); i++ {
			o, err := vchubs[i].StateMgr.Controller().Orchestrator().Find(&vchubs[i].OrchConfig.ObjectMeta)
			if err != nil {
				return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
			}
			if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
				return false, fmt.Errorf("Connection(%d) status was %s", i, o.Orchestrator.Status.Status)
			}
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")
}

func TestManageGivenNamespaces(t *testing.T) {
	config := log.GetDefaultConfig("vchub_testManageNamespaces")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dcName1 := "PenDC1"
	dcName2 := "PenDC2"
	dcName3 := "PenDC3"

	_, err = s.AddDC(dcName1)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName2)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName3)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	// We just want to manage dcName1 and dcName2
	orchConfig.Spec.ManageNamespaces = []string{dcName1, dcName2}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)
	defer vchub.Destroy(false)

	// Check if dcName1 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName1)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", dcName1)
		}
		dvs := dc.GetPenDVS(CreateDVSName(dcName1))
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", dcName1)
		}
		return true, nil
	}, "failed to find DVS")

	// Check if dcName2 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName2)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", dcName2)
		}
		dvs := dc.GetPenDVS(CreateDVSName(dcName2))
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", dcName2)
		}
		return true, nil
	}, "failed to find DVS")

	// Since dcName3 is not managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName3)
		if dc != nil {
			return false, fmt.Errorf("Found DC %s, the expectation is NO DC is found", dcName3)
		}
		return true, nil
	}, "found unexpected DC")
}

func TestManageAllNamespaces(t *testing.T) {
	config := log.GetDefaultConfig("vchub_testManageAllNamespaces")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dcName1 := "PenDC1"
	dcName2 := "PenDC2"
	dcName3 := "PenDC3"

	_, err = s.AddDC(dcName1)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName2)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName3)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	// We just want to manage dcName1 and dcName2
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)
	defer vchub.Destroy(false)

	// Check if dcName1 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName1)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", dcName1)
		}
		dvs := dc.GetPenDVS(CreateDVSName(dcName1))
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", dcName1)
		}
		return true, nil
	}, "failed to find DVS")

	// Check if dcName2 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName2)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", dcName2)
		}
		dvs := dc.GetPenDVS(CreateDVSName(dcName2))
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", dcName2)
		}
		return true, nil
	}, "failed to find DVS")

	// Check if dcName3 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName3)
		if dc == nil {
			return false, fmt.Errorf("Failed to find DC %s", dcName3)
		}
		dvs := dc.GetPenDVS(CreateDVSName(dcName3))
		if dvs == nil {
			return false, fmt.Errorf("Failed to find dvs in DC %s", dcName3)
		}
		return true, nil
	}, "failed to find DVS")
}

func TestManageNoNamespaces(t *testing.T) {
	config := log.GetDefaultConfig("vchub_testManageNoNamespaces")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "https",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dcName1 := "PenDC1"
	dcName2 := "PenDC2"
	dcName3 := "PenDC3"

	_, err = s.AddDC(dcName1)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName2)
	AssertOk(t, err, "failed dc create")

	_, err = s.AddDC(dcName3)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	// We just want to manage dcName1 and dcName2
	orchConfig.Spec.ManageNamespaces = []string{}

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub := LaunchVCHub(sm, orchConfig, logger)
	defer vchub.Destroy(false)

	// Check if dcName1 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName1)
		if dc != nil {
			return false, fmt.Errorf("Found DC %s, the expectation is NO DC is found", dcName1)
		}
		return true, nil
	}, "found unexpected DC")

	// Check if dcName2 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName2)
		if dc != nil {
			return false, fmt.Errorf("Found DC %s, the expectation is NO DC is found", dcName2)
		}
		return true, nil
	}, "found unexpected DC")

	// Check if dcName3 is managed by our orchestrator
	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcName3)
		if dc != nil {
			return false, fmt.Errorf("Found DC %s, the expectation is NO DC is found", dcName3)
		}
		return true, nil
	}, "found unexpected DC")
}
