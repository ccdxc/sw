package vchub

import (
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/vmware/govmomi/vim25/types"
	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils/pcache"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	logConfig = log.GetDefaultConfig("vcstore_integ_test")
	logger    = log.SetConfig(logConfig)

	evRecorder = mockevtsrecorder.NewRecorder("vcstore_integ_test",
		log.GetNewLogger(log.GetDefaultConfig("vcstore_integ_test")))
	_ = recorder.Override(evRecorder)
)

type storeTC struct {
	name   string
	events []defs.Probe2StoreMsg
	setup  func(*VCHub, *gomock.Controller)
	verify func(*VCHub)
}

func runStoreTC(t *testing.T, testCases []storeTC) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	// If set, logger will output to console
	debugMode := true
	if debugMode {
		logConfig.LogToStdout = true
		logConfig.Filter = log.AllowAllFilter
		logger = log.SetConfig(logConfig)
	}

	tsdb.Init(context.Background(), &tsdb.Opts{})

	for _, tc := range testCases {
		ctx, cancelFn := context.WithCancel(context.Background())

		if len(forceTestName) != 0 && tc.name != forceTestName {
			continue
		}
		t.Logf("running %s", tc.name)
		logger.Infof("<==== RUNNING %s ====>", tc.name)
		sm, _, err := statemgr.NewMockStateManager()
		if err != nil {
			t.Fatalf("Failed to create state manager. Err : %v", err)
			return
		}

		orchConfig := statemgr.GetOrchestratorConfig("127.0.0.1:8990", "user", "pass")
		orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

		err = sm.Controller().Orchestrator().Create(orchConfig)

		pCache := pcache.NewPCache(sm, logger)
		AssertOk(t, err, "failed to create useg mgr")
		state := &defs.State{
			VcID:         "127.0.0.1:8990",
			Ctx:          ctx,
			Log:          logger,
			StateMgr:     sm,
			OrchConfig:   orchConfig,
			Wg:           &sync.WaitGroup{},
			ForceDCNames: map[string]bool{utils.ManageAllDcs: true},
			DcIDMap:      map[string]types.ManagedObjectReference{},
			DvsIDMap:     map[string]types.ManagedObjectReference{},
		}

		vchub := &VCHub{
			State:        state,
			pCache:       pCache,
			DcMap:        map[string]*PenDC{},
			DcID2NameMap: map[string]string{},
		}
		pCache.SetValidator("Workload", vchub.validateWorkload)
		pCache.SetValidator(workloadVnicKind, validateWorkloadVnics)

		vchub.StateMgr.SetAPIClient(nil)
		inbox := make(chan defs.Probe2StoreMsg)
		vchub.vcReadCh = inbox
		var mockCtrl *gomock.Controller
		if tc.setup != nil {
			mockCtrl = gomock.NewController(t)
			tc.setup(vchub, mockCtrl)
		}

		vchub.Wg.Add(1)
		go vchub.startEventsListener()

		// Push events

		// Process any statemgr events first
		time.Sleep(100 * time.Millisecond)
		for _, e := range tc.events {
			inbox <- e
		}
		// Time for events to process
		tc.verify(vchub)

		// Terminating store instance
		cancelFn()
		doneCh := make(chan bool)
		go func() {
			vchub.Wg.Wait()
			doneCh <- true
		}()
		select {
		case <-doneCh:
		case <-time.After(1 * time.Second):
			t.Fatalf("Store failed to shutdown within timeout")
		}
		if mockCtrl != nil {
			mockCtrl.Finish()
		}
	}

	Assert(t, len(forceTestName) == 0, "focus test flag should not be checked in")
}

func generateVNIC(macAddress, portKey, portgroupKey, vnicType string) types.BaseVirtualDevice {
	ethCard := types.VirtualEthernetCard{
		VirtualDevice: types.VirtualDevice{
			Backing: &types.VirtualEthernetCardDistributedVirtualPortBackingInfo{
				Port: types.DistributedVirtualSwitchPortConnection{
					PortKey:      portKey,
					PortgroupKey: portgroupKey,
				},
			},
		},
		MacAddress: macAddress,
	}
	switch vnicType {
	case "Vmxnet3":
		return &types.VirtualVmxnet3{
			VirtualVmxnet: types.VirtualVmxnet{
				VirtualEthernetCard: ethCard,
			},
		}
	case "Vmxnet2":
		return &types.VirtualVmxnet2{
			VirtualVmxnet: types.VirtualVmxnet{
				VirtualEthernetCard: ethCard,
			},
		}
	case "Vmxnet":
		return &types.VirtualVmxnet{
			VirtualEthernetCard: ethCard,
		}
	case "E1000e":
		return &types.VirtualE1000e{
			VirtualEthernetCard: ethCard,
		}
	case "E1000":
		return &types.VirtualE1000{
			VirtualEthernetCard: ethCard,
		}
	default:
		return &types.VirtualE1000{
			VirtualEthernetCard: ethCard,
		}
	}
}

func addDCState(t *testing.T, vchub *VCHub, dcName string) {
	dvsName := CreateDVSName(dcName)
	useg, err := useg.NewUsegAllocator()
	AssertOk(t, err, "Failed to create useg")
	penDVS := &PenDVS{
		State:   vchub.State,
		DcName:  dcName,
		DvsName: dvsName,
		DvsRef: types.ManagedObjectReference{
			Type:  string(defs.VmwareDistributedVirtualSwitch),
			Value: "dvs-1",
		},
		UsegMgr: useg,
		Pgs:     map[string]*PenPG{},
		pgIDMap: map[string]*PenPG{},
		probe:   vchub.probe,
	}
	vchub.DcMap[dcName] = &PenDC{
		State: vchub.State,
		Name:  dcName,
		dcRef: types.ManagedObjectReference{
			Type:  string(defs.Datacenter),
			Value: "DC1",
		},
		DvsMap: map[string]*PenDVS{
			dvsName: penDVS,
		},
		HostName2Key: map[string]string{},
		probe:        vchub.probe,
	}
	vchub.DcID2NameMap["DC1"] = dcName
}

func addPGState(t *testing.T, vchub *VCHub, dcName, pgName, pgID, networkName string) {
	penDC := vchub.GetDC(dcName)
	penDVS := penDC.GetPenDVS(CreateDVSName(dcName))
	penPG := &PenPG{
		State:  vchub.State,
		probe:  vchub.probe,
		PgName: pgName,
		PgRef: types.ManagedObjectReference{
			Type:  string(defs.DistributedVirtualPortgroup),
			Value: pgID,
		},
		NetworkMeta: api.ObjectMeta{
			Name:      networkName,
			Tenant:    "default",
			Namespace: "default",
		},
	}
	penDVS.UsegMgr.AssignVlansForPG(pgName)
	penDVS.Pgs[pgName] = penPG
	penDVS.pgIDMap[pgID] = penPG
}

func TestDCs(t *testing.T) {

	dcName := "DC1"
	dcID := "DC1"

	testCases := []storeTC{
		{
			name: "DC rename",
			events: []defs.Probe2StoreMsg{
				{
					MsgType: defs.VCEvent,
					Val: defs.VCEventMsg{
						VcObject:   defs.Datacenter,
						Key:        dcID,
						Originator: "127.0.0.1:8990",
						Changes: []types.PropertyChange{
							types.PropertyChange{
								Op:   types.PropertyChangeOpAdd,
								Name: "name",
								Val:  "RandomName",
							},
						},
					},
				},
			},
			setup: func(vchub *VCHub, mockCtrl *gomock.Controller) {
				mockProbe := mock.NewMockProbeInf(mockCtrl)
				vchub.probe = mockProbe
				mockProbe.EXPECT().TagObjAsManaged(gomock.Any()).Return(nil).AnyTimes()

				mockProbe.EXPECT().RenameDC("RandomName", dcName, gomock.Any()).Return(nil).Times(1)

				// Setup state for DC1
				addDCState(t, vchub, dcName)

				eventRecorder.ClearEvents()

			},
			verify: func(v *VCHub) {
				// Verification is mockprobe RenamePG getting called
				AssertEventually(t, func() (bool, interface{}) {
					evts := eventRecorder.GetEvents()
					found := false
					for _, evt := range evts {
						if evt.EventType == eventtypes.ORCH_INVALID_ACTION.String() && strings.Contains(evt.Message, "DC") {
							found = true
						}
					}
					return found, nil
				}, "Failed to find orch invalid event")
			},
		},
	}

	runStoreTC(t, testCases)
}
