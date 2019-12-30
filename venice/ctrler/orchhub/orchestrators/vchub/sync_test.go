package vchub

import (
	"context"
	"fmt"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	. "github.com/pensando/sw/venice/utils/testutils"

	// need tsdb
	// _ "github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

// Tests creation of internal DC state, creationg of DVS
// and creation/deletion of networks in venice
// trigger respective events in VC
func TestVCSyncPG(t *testing.T) {
	t.Skipf("Test does not work with vcsim since vcsim returns different object types for PG and DVS objects")
	// Stale PGs should be deleted
	// New PGs should be created
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("sync_test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	u := &url.URL{
		Scheme: "http",
		Host:   defaultTestParams.TestHostName,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(defaultTestParams.TestUser, defaultTestParams.TestPassword)

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	err = sm.Controller().Orchestrator().Create(orchConfig)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	dc2Name := "DC2"
	dc2, err := s.AddDC(dc2Name)
	AssertOk(t, err, "failed dc create")

	state := &defs.State{
		VcURL:      u,
		VcID:       "vcenter",
		Ctx:        context.Background(),
		Log:        logger,
		StateMgr:   sm,
		OrchConfig: orchConfig,
		Wg:         &sync.WaitGroup{},
	}
	storeCh := make(chan defs.Probe2StoreMsg, 24)
	vcp := vcprobe.NewVCProbe(storeCh, state)
	vcp.Start()

	pvlanConfigSpecArray := testutils.GenPVLANConfigSpecArray(defaultTestParams, "add")
	dvsCreateSpec := testutils.GenDVSCreateSpec(defaultTestParams, pvlanConfigSpecArray)
	_, err = dc1.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")

	_, err = dc2.AddDVS(dvsCreateSpec)
	AssertOk(t, err, "failed dvs create")
	// 1 stale, 1 correct pg, 1 missing PG
	pgConfigSpec0 := []types.DVPortgroupConfigSpec{
		types.DVPortgroupConfigSpec{
			Name:     createPGName("dvs1-stalePG1"),
			NumPorts: 10,
		},
		types.DVPortgroupConfigSpec{
			Name:     createPGName("pg1"),
			NumPorts: 10,
		},
	}

	vcp.AddPenPG(defaultTestParams.TestDCName, defaultTestParams.TestDVSName, &pgConfigSpec0[0])
	vcp.AddPenPG(defaultTestParams.TestDCName, defaultTestParams.TestDVSName, &pgConfigSpec0[1])

	// _, err = dvs1.AddPortgroup(pgConfigSpec0)
	AssertOk(t, err, "failed pg create")

	// All stale

	pgConfigSpec1 := []types.DVPortgroupConfigSpec{
		types.DVPortgroupConfigSpec{
			Name:     createPGName("dvs2-stalePG1"),
			NumPorts: 10,
		},
		types.DVPortgroupConfigSpec{
			Name:     createPGName("dvs2-stalePG2"),
			NumPorts: 10,
		},
	}
	vcp.AddPenPG(dc2Name, defaultTestParams.TestDVSName, &pgConfigSpec1[0])
	vcp.AddPenPG(defaultTestParams.TestDCName, defaultTestParams.TestDVSName, &pgConfigSpec1[1])

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	smmock.CreateNetwork(sm, "default", "pg1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	smmock.CreateNetwork(sm, "default", "pg2", "10.1.2.0/24", "10.1.1.2", 101, nil, orchInfo1)

	time.Sleep(1 * time.Second)
	vchub := LaunchVCHub(sm, orchConfig, logger, WithScheme("http"))

	defer vchub.Destroy()

	verifyPg := func(dcPgMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, pgNames := range dcPgMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					return false, fmt.Errorf("Failed to find DC %s", name)
				}
				dvs := dc.GetPenDVS(defs.DefaultDVSName)
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
		dc2Name:                      []string{},
	}

	verifyPg(dcPgMap)

}
