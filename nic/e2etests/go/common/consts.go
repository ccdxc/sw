package common

import (
	"log"
	"os"
)

//NicDir NIC directory
var NicDir = _getCwd()

//E2eTestDir E2E test direcotry
var E2eTestDir = NicDir + "/e2etests/go/"

//HntapCfgFile Hntap config file
var HntapCfgFile = NicDir + "/conf/hntap.conf"

//HntapLogFile Hntap log file
var HntapLogFile = NicDir + "/hntap.log"

//HntapContainerLogFile Hntap logfile in container
var HntapContainerLogFile = os.Getenv("HOME") + "/naples/data/logs/hntap.log"

//E2eCfgFile E2e cfg file
var E2eCfgFile = E2eTestDir + "/naples_cfg/e2e.cfg"

//E2eAppConfigFile App config file
var E2eAppConfigFile = E2eTestDir + "/e2e.json"

//TestDir Test Dir
var TestDir = E2eTestDir + "/tests"

//TestSpecDir Test Spec directory
var TestSpecDir = TestDir + "/specs"

//TestModulesDir Test modules directory
var TestModulesDir = TestDir + "/modules"

func _getCwd() string {
	dir, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}
	return dir
}
