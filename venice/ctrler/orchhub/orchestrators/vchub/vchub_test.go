package vchub

import (
	"context"
	"fmt"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
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
