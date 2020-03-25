package vchub

import (
	"context"
	"encoding/json"
	"net/url"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestDebug(t *testing.T) {
	err := testutils.ValidateParams(defaultTestParams)
	if err != nil {
		t.Fatalf("Failed at validating test parameters")
	}

	config := log.GetDefaultConfig("debug_test")
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
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	_, err = dc1.AddHost("host1")
	AssertOk(t, err, "Failed to create host")
	vm, err := dc1.AddVM("vm1", "host1", nil)
	AssertOk(t, err, "Failed to create vm")

	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		s.Destroy()
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Status.OrchID = 1
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)

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
	smmock.CreateNetwork(sm, "default", "n1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := vchub.probe.GetPenPG(defaultTestParams.TestDCName, CreatePGName("n1"), 1)
		if err != nil {
			return false, err
		}
		return true, nil
	}, "failed to find PG")

	testDebug := func(action string, params map[string]string, exp string) {
		ret, err := vchub.debugHandler(action, params)
		AssertOk(t, err, "Failed to get debug info")
		retJSON, err := json.Marshal(ret)
		AssertOk(t, err, "Failed to marshal debug info")
		AssertEquals(t, exp, string(retJSON), "debug strings were not equal")
	}

	params := map[string]string{
		"dc": defaultTestParams.TestDCName,
	}
	testDebug(DebugUseg, params, `{"PG":{"#Pen-PG-n1-primary":2,"#Pen-PG-n1-secondary":3},"Hosts":{}}`)

	params = map[string]string{}
	debugString := `{"Workload":{"default/default/` + vchub.createHostName(defaultTestParams.TestDCName, vm.Summary.Vm.Value) + `":{"kind":"Workload","api-version":"v1","meta":{"name":"` + vchub.createHostName(defaultTestParams.TestDCName, vm.Summary.Vm.Value) + `","tenant":"default","namespace":"default","generation-id":"","labels":{"io.pensando.namespace":"","io.pensando.orch-name":"127.0.0.1:8989","io.pensando.vcenter.display-name":"vm1"},"creation-time":"","mod-time":""},"spec":{"host-name":"` + vchub.createHostName(defaultTestParams.TestDCName, vm.Runtime.Host.Value) + `"},"status":{"propagation-status":{"generation-id":"","updated":0,"pending":0,"min-version":"","status":"","pending-dscs":null}}}}}`

	testDebug(DebugCache, params, debugString)
	params = map[string]string{}
	testDebug(DebugState, params, `{"PenTestDC":{"ID":"datacenter-2","DVS":{"#Pen-DVS-PenTestDC":{"ID":"dvs-21","PGs":{"#Pen-PG-n1":{"ID":"dvportgroup-25","Network":"n1"}}}}}}`)

	params = map[string]string{}
	testDebug(DebugSync, params, "null")

	// negative testing
	params = map[string]string{}
	_, err = vchub.debugHandler("unknown action", params)
	Assert(t, err != nil, "expected failure for unknown action")

	params = map[string]string{}
	_, err = vchub.debugHandler(DebugUseg, params)
	Assert(t, err != nil, "expected failure since dc is not supplied")

}
