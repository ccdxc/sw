package pkg

import (
	"fmt"
	"strings"
	"testing"

	App "github.com/pensando/sw/nic/e2etests/go/infra"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestTraffic(t *testing.T) {

	var trafficCnt = 0
	testhpingRun := func(ns *App.NS, cmd []string) error {
		fmt.Println(strings.Join(cmd, " "))
		trafficCnt++
		return nil
	}

	hpingRun = testhpingRun

	fmt.Println("Generating configs...")
	configs, err := GenerateObjectsFromManifest("../configs/golden/golden_cfg.yml")
	TestUtils.Assert(t, err == nil, "Generate failed")

	agentCfg, err := GetAgentConfig(configs, "../configs/golden/golden_cfg.yml")
	fmt.Println(err)
	TestUtils.Assert(t, err == nil, "Get Config failed")

	err = RunTraffic("./test/uplink_map.cfg", agentCfg, TrafficUplinkToUplink)
	fmt.Println(trafficCnt)
	TestUtils.Assert(t, err == nil && trafficCnt == 32, "Test failed")

}
