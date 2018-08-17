package infra

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"testing"

	"github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestEndpoint(t *testing.T) {
	Common.E2eCfgFile = "./test_naples_cfg/e2e.cfg"
	jsonFile, err := os.Open(Common.E2eCfgFile)
	if err != nil {
		panic(err)
	}
	fmt.Println("Successfully Opened e2e.json")
	// defer the closing of our jsonFile so that we can parse it later on
	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)
	var e2ecfg cfg.E2eCfg
	json.Unmarshal(byteValue, &e2ecfg)
	fmt.Println("Number of configs", len(e2ecfg.EndpointsInfo.Endpoints))
	for _, ep := range e2ecfg.EndpointsInfo.Endpoints {
		newEp := NewEndpoint(ep.EndpointMeta.Name, ep, &e2ecfg)
		newEp.Init(true)
		TestUtils.AssertEquals(t, newEp.Name, ep.EndpointMeta.Name, "EP NAME CHECK")
		TestUtils.AssertEquals(t, newEp.Interface.IPAddress,
			strings.Split(ep.EndpointSpec.Ipv4Address, "/")[0], "EP IP CHECK")
		TestUtils.AssertEquals(t, newEp.Interface.MacAddress, ep.EndpointSpec.MacAddresss, "EP MAC CHECK")
		intfs := newEp.AppEngine.GetInterfaces()
		TestUtils.AssertEquals(t, len(intfs), 1, "Interface count check")
		TestUtils.AssertEquals(t, len(intfs[0]._VlanIntfs), 1, "Sub Interface count check")
		if ep.EndpointSpec.InterfaceType == "lif" {
			_, ok := intfs[0]._VlanIntfs[ep.EndpointSpec.UsegVlan]
			TestUtils.Assert(t, ok, "Encap vlan check")
		}
		newEp.PrintInformation()
		newEp.Delete()
	}

}
