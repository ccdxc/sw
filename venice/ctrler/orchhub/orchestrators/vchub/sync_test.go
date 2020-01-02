package vchub

import (
	"context"
	"fmt"
	"net/url"
	"sync"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	. "github.com/pensando/sw/venice/utils/testutils"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
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
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockProbe := mock.NewMockProbeInf(mockCtrl)

	vchub := setupVCHub(vcURL, sm, orchConfig, logger)
	vchub.probe = mockProbe
	defer vchub.Destroy()

	// Real probe that will be used my mock probe when possible
	vcp := vcprobe.NewVCProbe(vchub.vcReadCh, vchub.State)
	vcp.Start()

	mockProbe.EXPECT().IsSessionReady().DoAndReturn(func() bool {
		return vcp.IsSessionReady()
	}).AnyTimes()

	mockProbe.EXPECT().ClearState().Return()

	var ref types.ManagedObjectReference

	mockProbe.EXPECT().ListDC().DoAndReturn(func() []mo.Datacenter {
		dcs := vcp.ListDC()
		ref = dcs[0].Reference()
		return dcs
	})

	mockProbe.EXPECT().AddPenDVS(defaultTestParams.TestDCName, gomock.Any()).DoAndReturn(
		func(dcName, dvsCreateSpec interface{}) error {
			return vcp.AddPenDVS(dcName.(string), dvsCreateSpec.(*types.DVSCreateSpec))
		})

	// Overriding default probe behavior to work correctly with vcsim
	mockProbe.EXPECT().ListDVS(gomock.Any()).DoAndReturn(func(_ interface{}) []mo.VmwareDistributedVirtualSwitch {
		var dvsObjs []mo.DistributedVirtualSwitch
		var ret []mo.VmwareDistributedVirtualSwitch
		vcp.ListObj("DistributedVirtualSwitch", []string{"name"}, &dvsObjs, &ref)
		for _, obj := range dvsObjs {
			ret = append(ret, mo.VmwareDistributedVirtualSwitch{
				DistributedVirtualSwitch: obj,
			})
		}
		return ret
	})

	// Overriding probe to return a PG with PVLAN config since vcsim doesn't support it currently
	mockProbe.EXPECT().ListPG(gomock.Any()).DoAndReturn(
		func(dcRef interface{}) []mo.DistributedVirtualPortgroup {
			// stale pg
			pg1 := mo.DistributedVirtualPortgroup{
				Network: mo.Network{
					Name: createPGName("stalePG1"),
				},
				Config: types.DVPortgroupConfigInfo{
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: 10,
						},
					},
				},
			}
			pg2 := mo.DistributedVirtualPortgroup{
				Network: mo.Network{
					Name: createPGName("pg1"),
				},
				Config: types.DVPortgroupConfigInfo{
					DefaultPortConfig: &types.VMwareDVSPortSetting{
						Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
							PvlanId: 12,
						},
					},
				},
			}
			return []mo.DistributedVirtualPortgroup{pg1, pg2}
		})

	mockProbe.EXPECT().AddPenPG(defaultTestParams.TestDCName, gomock.Any(), gomock.Any()).Return(nil).AnyTimes()
	mockProbe.EXPECT().RemovePenPG(defaultTestParams.TestDCName, gomock.Any()).Return(nil).AnyTimes()

	vchub.Sync()

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
	}

	verifyPg(dcPgMap)

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
