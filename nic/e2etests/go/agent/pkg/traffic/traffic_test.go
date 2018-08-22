package traffic

import (
	"fmt"
	"testing"

	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestTraffic(t *testing.T) {

	fmt.Println("Generating configs...")
	configs, err := Pkg.GetObjectsFromManifest("../../configs/golden/golden_cfg.yml")
	TestUtils.Assert(t, err == nil, "Generate failed")

	agentCfg, err := Pkg.GetAgentConfig(configs, "../../configs/golden/golden_cfg.yml")
	fmt.Println(err)
	TestUtils.Assert(t, err == nil, "Get Config failed")

	err = RunTests("utest", "sanity", "./utest/uplink_map.cfg", agentCfg)
	TestUtils.Assert(t, err == nil, "Environment or suite not present")
}
