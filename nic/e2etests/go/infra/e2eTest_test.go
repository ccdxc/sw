package infra

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	Tests "github.com/pensando/sw/nic/e2etests/go/tests"
	_ "github.com/pensando/sw/nic/e2etests/go/tests"
	_ "github.com/pensando/sw/nic/e2etests/go/tests/modules"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func _fakeWriteToHntap() {

	d1 := []byte("listening on\n")
	for true {
		if _, err := os.Stat(Common.HntapLogFile); err == nil {
			time.Sleep(3 * time.Second)
			ioutil.WriteFile(Common.HntapLogFile, d1, 0644)
			return
		}
		time.Sleep(2 * time.Second)
	}
}

func TestE2eTest(t *testing.T) {
	fmt.Println(os.Getwd())
	Common.HntapCfgFile = "../../../conf/hntap.conf"
	Common.HntapLogFile = "../../../hntap.log"
	//Fake Hntap command run
	HntapCmd = "ls"
	testSpecs := ReadTestSpecs("../tests/utest/specs")
	go _fakeWriteToHntap()
	fmt.Println("Running tests..")
	fmt.Println("Reading Agent configuration...")
	agentCfg := cfg.GetAgentConfig("./test_naples_cfg/e2e.cfg")
	if agentCfg == nil {
		log.Fatalln("Unable to get agent configuration!")
	}

	tm := Tests.GetTestManager()

	for _, testSpec := range testSpecs {
		fmt.Println("Running Test Spec : ", testSpec.Name)
		e2eTest := NewE2ETest(agentCfg, testSpec, nil, nil, nil)
		ret := e2eTest.Run(false, tm)
		e2eTest.Teardown()
		if !ret {
			fmt.Println("Test Failed :", e2eTest)
		} else {
			fmt.Println("Test Passed :", e2eTest)
		}
	}

}

func TestE2eScale(t *testing.T) {
	agentCfg := cfg.GetScaleAgentConfig("./test_naples_cfg/scale.topo")
	if agentCfg == nil {
		log.Fatalln("Unable to get agent configuration!")
	}
	fmt.Println("Number of namespace ", len(agentCfg.NamespacesInfo.Namespaces))
	TestUtils.Assert(t,
		len(agentCfg.NamespacesInfo.Namespaces) == 4, "Namespace match failed")
	TestUtils.Assert(t,
		len(agentCfg.NetworksInfo.Networks) == 64, "Namespace match failed")
	TestUtils.Assert(t,
		len(agentCfg.EndpointsInfo.Endpoints) == 8192, "Endpoints count failed")
}
