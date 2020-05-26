package vchub

import (
	"context"
	"encoding/json"
	"net"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/testutils"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	conv "github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
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

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	host, err := dc1.AddHost("host1")
	AssertOk(t, err, "Failed to create host")
	pNicMac := net.HardwareAddr{}
	pNicMac = append(pNicMac, globals.PensandoOUI[0], globals.PensandoOUI[1], globals.PensandoOUI[2])
	pNicMac = append(pNicMac, 0xaa, 0x00, 0x00)
	// Make it Pensando host
	err = host.AddNic("vmnic0", conv.MacString(pNicMac))
	AssertOk(t, err, "failed to add nic")
	host2, err := dc1.AddHost("host2")
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

	AssertEventually(t, func() (bool, interface{}) {
		return vchub.watchStarted, nil
	}, "VCHub sync never finished")

	dvsName := CreateDVSName(defaultTestParams.TestDCName)
	dvs, ok := dc1.GetDVS(dvsName)
	Assert(t, ok, "failed dvs create")
	err = dvs.AddHost(host)
	AssertOk(t, err, "failed to add Host to DVS")
	time.Sleep(100 * time.Millisecond)

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

		Assert(t, strings.Contains(string(retJSON), exp), "debug strings were not equal, got:  %v, expected to contain: %v", string(retJSON), exp)
	}

	params := map[string]string{
		"dc": defaultTestParams.TestDCName,
	}
	testDebug(DebugUseg, params, `{"PG":{"#Pen-PG-n1-primary":2,"#Pen-PG-n1-secondary":3},"Hosts":{}}`)

	params = map[string]string{}
	debugString := `{"Host":{"//` + vchub.createHostName(defaultTestParams.TestDCName, host2.Obj.Self.Value) + `":{"kind":"Host","api-version":"v1","meta":{"name":"` + vchub.createHostName(defaultTestParams.TestDCName, host2.Obj.Self.Value) + `","generation-id":"","labels":{"io.pensando.namespace":"PenTestDC","io.pensando.orch-name":"127.0.0.1:8989","io.pensando.vcenter.display-name":"host2"},"creation-time":"","mod-time":""},"spec":{},"status":{}}},` + `"Workload":{"default/default/` + vchub.createHostName(defaultTestParams.TestDCName, vm.Summary.Vm.Value) + `":{"kind":"Workload","api-version":"v1","meta":{"name":"` + vchub.createHostName(defaultTestParams.TestDCName, vm.Summary.Vm.Value) + `","tenant":"default","namespace":"default","generation-id":"","labels":{"io.pensando.namespace":"` + defaultTestParams.TestDCName + `","io.pensando.orch-name":"127.0.0.1:8989","io.pensando.vcenter.display-name":"vm1"},"creation-time":"","mod-time":""},"spec":{"host-name":"` + vchub.createHostName(defaultTestParams.TestDCName, vm.Runtime.Host.Value) + `"},"status":{"propagation-status":{"generation-id":"","updated":0,"pending":0,"min-version":"","status":"","pending-dscs":null}}}}}`

	testDebug(DebugCache, params, debugString)
	params = map[string]string{}
	testDebug(DebugState, params, `{"PenTestDC":{"ID":"datacenter-2","DVS":{"#Pen-DVS-PenTestDC":{"ID":"`+dvs.Obj.Self.Value+`","PGs":{"#Pen-PG-n1":{"ID":"dvportgroup-34","Network":"n1"}}}}}}`)

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
