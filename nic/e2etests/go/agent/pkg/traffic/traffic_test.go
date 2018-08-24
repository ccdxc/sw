package traffic

import (
	"fmt"
	"testing"

	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestUplinkToUplinkTraffic(t *testing.T) {

	if true {
		return
	}
	fmt.Println("Generating configs...")
	configs, err := Pkg.GetObjectsFromManifest("../../configs/golden/golden_cfg.yml")
	TestUtils.Assert(t, err == nil, "Generate failed")

	agentCfg, err := Pkg.GetAgentConfig(configs, "../../configs/golden/golden_cfg.yml")
	fmt.Println(err)
	TestUtils.Assert(t, err == nil, "Get Config failed")

	trafficHelper, err1 := GetTrafficHelper(TrafficUplinkToUplink, "./utest/uplink_map.cfg")
	TestUtils.Assert(t, err1 == nil, "Get Traffic helper failed")
	err = RunTests("utest", "sanity", trafficHelper, agentCfg)
	TestUtils.Assert(t, err == nil, "Environment or suite not present")
}

func TestHostToHostTraffic(t *testing.T) {

	fmt.Println("Generating configs...")
	configs, err := Pkg.GetObjectsFromManifest("../../configs/golden/golden_cfg.yml")
	TestUtils.Assert(t, err == nil, "Generate failed")

	agentCfg, err := Pkg.GetAgentConfig(configs, "../../configs/golden/golden_cfg.yml")
	fmt.Println(err)
	TestUtils.Assert(t, err == nil, "Get Config failed")

	getIntfMatchingMac = func(macaddr string) string {
		return "eth0"
	}

	trafficHelper, err1 := GetTrafficHelper(TrafficHostToHost, "./utest/sim_device.json")
	TestUtils.Assert(t, err1 == nil, "Get Traffic helper failed")
	err = RunTests("utest", "sanity", trafficHelper, agentCfg)
	TestUtils.Assert(t, err == nil, "Environment or suite not present")
}
