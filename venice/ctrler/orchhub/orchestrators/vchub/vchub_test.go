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
	TestOrchName:           "test-orchestrator",
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

	logger := setupLogger("vchub_test_vc_write")

	var vchub *VCHub
	var s *sim.VcSim

	defer func() {
		logger.Infof("Tearing Down")
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
	_, err = s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	dc2 := "DC2"
	_, err = s.AddDC(dc2)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "failed to create orch config")

	vchub = LaunchVCHub(sm, orchConfig, logger, WithTagSyncDelay(2*time.Second))

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

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

	logger := setupLogger("vchub_test")

	var vchub *VCHub
	var s *sim.VcSim

	defer func() {
		logger.Infof("Tearing Down")
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
	_, err = s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	vchub = nil
}

func TestVCHubDestroy1(t *testing.T) {
	// Destroy while vcenter can't be reached
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	logger := setupLogger("vchub_test_destory")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, nil, retryCount)

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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	vchub = nil

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

	logger := setupLogger("vchub_test_destroy_2")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()
	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, nil, retryCount)

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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	vchub = nil

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

	logger := setupLogger("vchub_test_destroy_3")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, nil, retryCount)
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	vchub = nil

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
		VcURL:    u,
		VcID:     "VCProbe",
		Ctx:      ctx,
		Log:      logger.WithContext("submodule", "vcprobe"),
		Wg:       &sync.WaitGroup{},
		DcIDMap:  map[string]types.ManagedObjectReference{},
		DvsIDMap: map[string]types.ManagedObjectReference{},
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
	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	logger := setupLogger("vchub_test_dc_watchers")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

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
	eventRecorder := mockevtsrecorder.NewRecorder("vchub_test",
		log.GetNewLogger(log.GetDefaultConfig("vchub_test")))
	_ = recorder.Override(eventRecorder)
	// Lower the limit for testing
	usegvlanmgr.VlanMax = useg.FirstUsegVlan + 100

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	logger := setupLogger("vchub_test_vlan_limit")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, nil, retryCount)
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	smmock.CreateNetwork(sm, "default", "pg1", "11.1.1.0/24", "11.1.1.1", 500, nil, orchInfo1)

	vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

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
	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	logger := setupLogger("vchub_test_rapid_events")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var err error
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")
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

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
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
									Key: "pnic-2",
								},
							},
							ProxySwitch: []types.HostProxySwitch{
								types.HostProxySwitch{
									DvsName: dvsName,
									Pnic:    []string{"pnic-1", "pnic-2"},
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

func renameDCEvent(dcID string, newName string) defs.Probe2StoreMsg {
	return defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject:   defs.Datacenter,
			Key:        dcID,
			Originator: "127.0.0.1:8990",
			Changes: []types.PropertyChange{
				types.PropertyChange{
					Op:  types.PropertyChangeOpAdd,
					Val: newName,
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

	logger := setupLogger("vchub_test_update_url")

	var vchub *VCHub
	var s1 *sim.VcSim
	var s2 *sim.VcSim

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}
		if s1 != nil {
			s1.Destroy()
		}
		if s2 != nil {
			s2.Destroy()
		}
	}()

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	// vcsim based on old URL
	s1, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	// vcsim based on new URL
	u.Host = newURL
	s2, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	orchConfig.Spec.URI = "bad-url"
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
	logger := setupLogger("vchub_testUpdateOrchConfigCredential")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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

	logger := setupLogger("vchub_testMultipleVcs")

	defer func() {
		logger.Infof("Tearing Down")
		for _, vchub := range vchubs {
			vchub.Destroy(false)
		}
		for _, vc := range vcsims {
			vc.Destroy()
		}
	}()

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
	}

	time.Sleep(2 * time.Second)

	for i := 0; i < len(urls); i++ {
		orchConfigs[i] = smmock.GetOrchestratorConfig(urls[i], defaultTestParams.TestUser, defaultTestParams.TestPassword)
		orchConfigs[i].Status.OrchID = int32(i)
		err = sm.Controller().Orchestrator().Create(orchConfigs[i])
		vchubs[i] = LaunchVCHub(sm, orchConfigs[i], logger)
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
	logger := setupLogger("vchub_testManageNamespaces")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	// We just want to manage dcName1 and dcName2
	orchConfig.Spec.ManageNamespaces = []string{dcName1, dcName2}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	logger := setupLogger("vchub_testManageAllNamespaces")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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
	logger := setupLogger("vchub_testManageNoNamespaces")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

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

func TestOrchRemoveManagedDC(t *testing.T) {
	dcCount := 4
	dcs := []*sim.Datacenter{}
	dcNames := []string{}

	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	logger := setupLogger("vchub_testRemoveManagedDC")

	var vchub *VCHub
	var s *sim.VcSim

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}
		if s != nil {
			s.Destroy()
		}
	}()

	logger.Infof("Place holder")
	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")

	for i := 0; i < dcCount; i++ {
		dcNames = append(dcNames, fmt.Sprintf("%s-%d", defaultTestParams.TestDCName, i))
		dc, err := s.AddDC(dcNames[i])
		AssertOk(t, err, "failed dc create")

		dcs = append(dcs, dc)
	}

	for i := 0; i < dcCount; i++ {
		_, ok := dcs[i].GetDVS(CreateDVSName(dcNames[i]))
		Assert(t, !ok, "No Pensando DVS should be found")
	}

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	// ADD DC-0
	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestOrchName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{dcNames[0]}
	orchConfig.Spec.URI = defaultTestParams.TestHostName

	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "failed to create orch config")

	vchub = LaunchVCHub(sm, orchConfig, logger, WithTagSyncDelay(2*time.Second))

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcNames[0])
		if dc == nil {
			return false, fmt.Errorf("Did not find DC %v", dcNames[0])
		}

		_, ok := dcs[0].GetDVS(CreateDVSName(dcNames[0]))
		if !ok {
			return false, fmt.Errorf("Pensando DVS not found in DC %v", dcNames[0])
		}

		return true, nil
	}, "Did not find DC")

	AssertEventually(t, func() (bool, interface{}) {
		for i := 1; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc != nil {
				return false, fmt.Errorf("Found unexpected DC %v", dcNames[i])
			}
		}
		return true, nil
	}, "found unexpected DC")

	AssertEventually(t, func() (bool, interface{}) {
		for i := 1; i < dcCount; i++ {
			_, ok := dcs[i].GetDVS(CreateDVSName(dcNames[i]))
			if ok {
				return false, fmt.Errorf("unexpected pensando DVS found in DC %s", dcNames[i])
			}
		}

		return true, nil
	}, "Unexpected Pensando DVS found in DC")

	// REMOVE all DCs
	orchConfig.Spec.ManageNamespaces = []string{}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		for i := 0; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc == nil {
				return true, nil
			}

			dvs := dc.GetDVS(CreateDVSName(dcNames[i]))
			if dvs != nil {
				return false, fmt.Errorf("unexpected pensando DVS found in DC %s", dcNames[i])
			}
		}

		if len(vchub.DcMap) > 0 {
			return false, fmt.Errorf("VCHub in-memory state not cleared")
		}

		return true, nil
	}, "Unexpected Pensando DVS found in DC")

	// Add DC-1
	orchConfig.Spec.ManageNamespaces = []string{dcNames[1]}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		dc := vchub.GetDC(dcNames[1])
		if dc == nil {
			return false, fmt.Errorf("Did not find DC %v", dcNames[1])
		}

		if len(vchub.DcMap) != 1 {
			return false, fmt.Errorf("VCHub in-memory state not cleared")
		}
		return true, nil
	}, "Did not find DC")

	dc := vchub.GetDC(dcNames[0])
	Assert(t, dc == nil, fmt.Sprintf("Found unexpected DC %v", dcNames[0]))

	// ADD ALL DCs
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		for i := 0; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc == nil {
				return false, fmt.Errorf("Did not find DC %v", dcNames[i])
			}

			_, ok := dcs[i].GetDVS(CreateDVSName(dcNames[i]))
			if !ok {
				return false, fmt.Errorf("Pensando DVS not found in DC %s", dcNames[i])
			}

			if len(vchub.DcMap) != dcCount {
				return false, fmt.Errorf("VCHub in-memory state not cleared")
			}
		}
		return true, nil
	}, "Did not find DC")

	// Create a random host with the orchname and namespace key
	testHost := "another-host"
	prodMap := make(map[string]string)
	prodMap[utils.OrchNameKey] = "another-orchestrator"
	prodMap[utils.NamespaceKey] = dcNames[0]

	np := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:      testHost,
			Namespace: "default",
			Labels:    prodMap,
		},
		Spec:   cluster.HostSpec{},
		Status: cluster.HostStatus{},
	}

	// create a Host
	err = sm.Controller().Host().Create(&np)
	Assert(t, (err == nil), "Host could not be created")

	// REMOVE ALL DCs - Ensure the above host is not deleted
	orchConfig.Spec.ManageNamespaces = []string{}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		for i := 1; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc != nil {
				return false, fmt.Errorf("DC %v should not have been found", dcNames[i])
			}

			if len(vchub.DcMap) > 0 {
				return false, fmt.Errorf("VCHub in-memory state not cleared")
			}
		}

		h, err := sm.Controller().Host().List(context.Background(), &api.ListWatchOptions{})
		if len(h) != 1 || err != nil {
			return false, fmt.Errorf("found %v hosts instead of 1. Err : %v", len(h), err)
		}

		return true, nil
	}, "Unexpected DC found")

	meta := api.ObjectMeta{
		Name:      testHost,
		Namespace: "default",
	}

	h, err := sm.Controller().Host().Find(&meta)
	AssertOk(t, err, "did not find the Host")

	AssertEquals(t, testHost, h.Host.Name, "Wrong host found")

	// Add non-existing DC
	orchConfig.Spec.ManageNamespaces = []string{"non-existing-dc"}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		for i := 1; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc != nil {
				return false, fmt.Errorf("DC %v should not have been found", dcNames[i])
			}

			if len(vchub.DcMap) > 0 {
				return false, fmt.Errorf("VCHub in-memory state not cleared")
			}
		}
		dc := vchub.GetDC("non-existing-dc")
		if dc != nil {
			return false, fmt.Errorf("non-existing-dc found")
		}
		return true, nil
	}, "non existing dc test failed")

	// REMOVE ALL DCs - Ensure the above host is not deleted
	orchConfig.Spec.ManageNamespaces = []string{}
	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		for i := 1; i < dcCount; i++ {
			dc := vchub.GetDC(dcNames[i])
			if dc != nil {
				return false, fmt.Errorf("DC %v should not have been found", dcNames[i])
			}

			if len(vchub.DcMap) > 0 {
				return false, fmt.Errorf("VCHub in-memory state not cleared")
			}
		}
		return true, nil
	}, "Unexpected DC found")
}

func TestDiscoveredDCs(t *testing.T) {
	logger := setupLogger("vchub_test_discovered_dc")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	dcName1 := "PenDC1"
	dcName2 := "PenDC2"
	dcName3 := "PenDC3"
	dcName4 := "PenDC12"
	dcName5 := "PenDC31"

	_, err = s.AddDC(dcName1)
	AssertOk(t, err, "failed dc create")

	dc2, err := s.AddDC(dcName2)
	AssertOk(t, err, "failed dc create")

	dc3, err := s.AddDC(dcName3)
	AssertOk(t, err, "failed dc create")

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{dcName1}

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger)

	AssertEventually(t, func() (bool, interface{}) {
		o, err := sm.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		act := o.Orchestrator.Status.DiscoveredNamespaces
		exp := []string{dcName1, dcName2, dcName3}
		if len(act) != len(exp) {
			return false, fmt.Errorf("discovered namespaces were %v, expected %v", o.Orchestrator.Status.DiscoveredNamespaces, exp)
		}

		for i := range act {
			if act[i] != exp[i] {
				return false, fmt.Errorf("discovered namespaces were %v, expected %v", o.Orchestrator.Status.DiscoveredNamespaces, exp)
			}
		}
		return true, nil
	}, "Orch status didn't have all the discovered namespaces ", "100ms", "5s")

	addDC := func(dcName string) *sim.Datacenter {
		dc, err := s.AddDC(dcName)
		dc.Obj.Name = dcName
		AssertOk(t, err, "failed to create DC %s", dcName)
		vchub.vcReadCh <- createDCEvent(dcName, dc.Obj.Self.Value)
		return dc
	}

	renameDC := func(dc *sim.Datacenter, newName string) {
		vchub.vcReadCh <- renameDCEvent(dc.Obj.Self.Value, newName)
	}

	removeDC := func(dc *sim.Datacenter) {
		dc.Destroy()
		vchub.vcReadCh <- deleteDCEvent(dc.Obj.Self.Value)
	}

	addDC(dcName4)
	renameDC(dc3, dcName5)
	removeDC(dc2)

	AssertEventually(t, func() (bool, interface{}) {
		o, err := sm.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		act := o.Orchestrator.Status.DiscoveredNamespaces
		exp := []string{dcName1, dcName4, dcName5}
		if len(act) != len(exp) {
			return false, fmt.Errorf("discovered namespaces were %v, expected %v", o.Orchestrator.Status.DiscoveredNamespaces, exp)
		}

		for i := range act {
			if act[i] != exp[i] {
				return false, fmt.Errorf("discovered namespaces were %v, expected %v", o.Orchestrator.Status.DiscoveredNamespaces, exp)
			}
		}
		if len(vchub.DcID2NameMap) != 3 {
			return false, fmt.Errorf("Number of entries in DcID2NameMap was incorrect %v", vchub.DcID2NameMap)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

	orchConfig.Spec.Credentials.Password = "badPassword"
	err = sm.Controller().Orchestrator().Update(orchConfig)
	AssertOk(t, err, "Failed to update orchestrator")

	vchub.UpdateConfig(orchConfig)

	AssertEventually(t, func() (bool, interface{}) {
		o, err := sm.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}
		act := o.Orchestrator.Status.DiscoveredNamespaces
		if len(act) != 0 {
			return false, fmt.Errorf("discovered namespaces were %v, expected none", o.Orchestrator.Status.DiscoveredNamespaces)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

}

func TestDegradedConn(t *testing.T) {
	// TODO: enable once this change is committed to the govmomi repo.
	t.Skipf("This test requires changes in the govmomi REST simulator to check the credentials of the client.")
	logger := setupLogger("vchub_test_discovered_dc")

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
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

	dcName1 := "PenDC1"

	_, err = s.AddDC(dcName1)
	AssertOk(t, err, "failed dc create")

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{dcName1}

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. Err : %v", err)

	err = sm.Controller().Orchestrator().Create(orchConfig)

	vchub = LaunchVCHub(sm, orchConfig, logger, WithTagSyncDelay(1*time.Second), WithMockProbe)

	AssertEventually(t, func() (bool, interface{}) {
		o, err := sm.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}

		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("status was %v", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

	// Simulate tags server becoming unauthenticated by changing credentials and forcing logout
	u1 := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, "changePass")
	s.Service.Listen = u1
	vchub.State.VcURL.User = url.UserPassword("random", "pass")
	probe := vchub.probe.(*mock.ProbeMock)
	tc := probe.GetTagClientWithRLock()
	tc.Logout(context.Background())
	probe.ReleaseClientsRLock()

	AssertEventually(t, func() (bool, interface{}) {
		o, err := sm.Controller().Orchestrator().Find(&vchub.OrchConfig.ObjectMeta)
		if err != nil {
			return false, fmt.Errorf("Failed to find orchestrator object. Err : %v", err)
		}

		if o.Orchestrator.Status.Status != orchestration.OrchestratorStatus_Degraded.String() {
			return false, fmt.Errorf("status was %v", o.Orchestrator.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "5s")

}

func TestVCHubStalePG(t *testing.T) {
	// Test resyncing stale PG
	// Scenario: vCenter already has PenDVS, PenPG, and VM attached
	// user tries to have another venice manage this DC.
	// User creates the network after initial sync has already run.
	// Orchhub should now take control of the PenPG and take any VMs
	// on it and create workloads
	logger := setupLogger("vchub_test_stale_PG")

	ctx, cancel := context.WithCancel(context.Background())

	var vchub *VCHub
	var s *sim.VcSim
	var err error
	var vcp *mock.ProbeMock

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}

		cancel()
		vcp.Wg.Wait()

		if s != nil {
			s.Destroy()
		}
	}()
	// VChub comes up with PG in use on vcenter
	// Create network comes after
	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")

	vcp = createProbe(ctx, defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	AssertEventually(t, func() (bool, interface{}) {
		if !vcp.IsSessionReady() {
			return false, fmt.Errorf("Session not ready")
		}
		return true, nil
	}, "Session is not Ready", "1s", "10s")

	// Create DVS
	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)

	err = vcp.AddPenDVS(defaultTestParams.TestDCName, dvsCreateSpec, nil, retryCount)
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

	vchub = LaunchVCHub(sm, orchConfig, logger, WithMockProbe)

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

	// Create PG it doesn't know about (simulate stale PG in use before we connected)
	spec := testutils.GenPGConfigSpec(CreatePGName("pg1"), 2, 3)
	err = vcp.AddPenPG(dc1.Obj.Name, dvs.Obj.Name, &spec, nil, retryCount)
	AssertOk(t, err, "failed to create pg")
	pg, err := vcp.GetPenPG(dc1.Obj.Name, CreatePGName("pg1"), retryCount)
	AssertOk(t, err, "failed to get pg")

	// Create VM on this PG
	_, err = dc1.AddVM("vm1", "host1", []sim.VNIC{
		sim.VNIC{
			MacAddress:   "aa:aa:bb:bb:dd:dd",
			PortgroupKey: pg.Reference().Value,
			PortKey:      "11",
		},
	})
	// if err != nil {
	// 	t.Skipf("Skipping test since VM could not be added, %s", err)
	// }

	// Add network, workload should appear
	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	smmock.CreateNetwork(sm, "default", "pg1", "11.1.1.0/24", "11.1.1.1", 500, nil, orchInfo1)

	AssertEventually(t, func() (bool, interface{}) {
		wl, err := sm.Controller().Workload().List(context.Background(), &api.ListWatchOptions{})
		if err != nil {
			return false, err
		}
		if len(wl) != 1 {
			return false, fmt.Errorf("Found %d workloads", len(wl))
		}

		mac := wl[0].Spec.Interfaces[0].MACAddress
		if mac != "aaaa.bbbb.dddd" {
			return false, fmt.Errorf("mac inf did not match, got %s", mac)
		}
		return true, nil
	}, "Failed to get wl")

}

func setupLogger(logName string) log.Logger {
	config := log.GetDefaultConfig(logName)
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)
	return logger
}

func createURL(host, user, pass string) *url.URL {
	u := &url.URL{
		Scheme: "https",
		Host:   host,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, pass)
	return u
}
