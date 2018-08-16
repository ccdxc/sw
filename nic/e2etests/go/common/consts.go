package common

import (
	"log"
	"os"
	"time"
)

//NicDir NIC directory
//var NicDir = _getCwd()
var NicDir = os.Getenv("HAL_CONFIG_PATH")

//E2eTestDir E2E test direcotry
var E2eTestDir = NicDir + "/test_driver"

//HntapCfgFile Hntap config file
var HntapCfgFile = NicDir + "/conf/hntap.conf"

//HntapLogFile Hntap log file
var HntapLogFile = NicDir + "/hntap.log"

//HntapContainerLogFile Hntap logfile in container
var HntapContainerLogFile = os.Getenv("HOME") + "/naples/data/logs/hntap.log"

//E2eCfgFile E2e cfg file
var E2eCfgFile = E2eTestDir + "/e2e.cfg"

//E2eScaleTopo E2e cfg file
var E2eScaleTopo = E2eTestDir + "/scale.topo"

//E2eAppConfigFile App config file
var E2eAppConfigFile = E2eTestDir + "/e2e.json"

//TestDir Test Dir
var TestDir = E2eTestDir + "/tests"

//TestSpecDir Test Spec directory
var TestSpecDir = TestDir + "/specs"

//ScaleTestSpecDir Test Spec directory
var ScaleTestSpecDir = TestDir + "/scale/specs"

//TestModulesDir Test modules directory
var TestModulesDir = TestDir + "/modules"

//NaplesContainerImageDir naples container image directory
var NaplesContainerImageDir = NicDir + "/obj/images"

//NaplesContainerImage naples container image
var NaplesContainerImage = "naples-release-v1.tgz"

//NaplesContainerImageName naples container image name
var NaplesContainerImageName = "pensando/naples:v1"

//NaplesContainerName  naples container name
var NaplesContainerName = "naples-v1"

//NaplesContainerStartUpScript start up script
var NaplesContainerStartUpScript = NicDir + "/sim/naples/start-naples-docker.sh"

//NaplesAgentTimeout agent timeout
var NaplesAgentTimeout = 600 * time.Second

//NaplesHalTimeout agent timeout
var NaplesHalTimeout = 600 * time.Second

//NaplesSimimeout Sim timeout
var NaplesSimimeout = 600 * time.Second

func _getCwd() string {
	dir, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}
	return dir
}
