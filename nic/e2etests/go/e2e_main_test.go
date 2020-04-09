package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	. "github.com/pensando/sw/nic/e2etests/go/infra"
	_ "github.com/pensando/sw/nic/e2etests/go/tests"
	_ "github.com/pensando/sw/nic/e2etests/go/tests/modules"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func DummyPost(string, string, io.Reader) (*http.Response, error) {
	return &http.Response{
		Status:     "200 OK",
		StatusCode: 200,
	}, nil
}

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

func TestMain(t *testing.T) {
	cfg.PostMethod = DummyPost
	dir, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(dir)

	Common.NicDir = Common.NicDir + "/../../"
	HntapExecDir = "../../../nic/build/x86_64/iris/capri/bin"
	Common.HntapCfgFile = Common.NicDir + "/conf/hntap.conf"
	Common.HntapLogFile = Common.NicDir + "/hntap.log"

	HntapCmd = HntapExecDir + "/nic_infra_hntap"
	ignoreTestErrors := false
	if _, err := os.Stat(HntapCmd); err != nil {
		//If No Hntap run fake for now
		HntapCmd = "ls"
		ignoreTestErrors = true
	}
	Common.E2eTestDir = Common.NicDir + "/e2etests/go/"
	Common.TestDir = Common.E2eTestDir + "/tests"
	Common.TestSpecDir = Common.TestDir + "/utest/specs"
	Common.TestModulesDir = Common.TestDir + "utest/modules"
	Common.E2eAppConfigFile = Common.E2eTestDir + "/e2e.json"
	Common.E2eCfgFile = Common.E2eTestDir + "/naples_cfg/e2e.cfg"
	Common.NaplesContainerImageDir = Common.NicDir + "/obj/images"
	Common.NaplesContainerStartUpScript = Common.NicDir + "/sim/naples/start-naples-docker.sh"

	go _fakeWriteToHntap()
	*_CmdArgs.NoModel = true
	*_CmdArgs.SkipNaples = true
	*_CmdArgs.SkipCfg = true
	*_CmdArgs.CfgFile = Common.E2eCfgFile
	ret := _RunMain()
	TestUtils.Assert(t, ignoreTestErrors || ret == 0, "Test failed")
}
