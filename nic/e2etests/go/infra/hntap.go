package infra

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"time"

	"github.com/pensando/sw/nic/e2etests/go/common"
)

//HntapExecDir Exec dir for Hntap
var HntapExecDir = "build/x86_64/iris/capri/bin"

//HntapCmd Hntap Cmd to execute
var HntapCmd = HntapExecDir + "/nic_infra_hntap"

//ContainerHntapCmd Container hntap cmd
const ContainerHntapCmd = "naples/nic/tools/restart-hntap.sh"

//HntapContainerConfPath Hntap container path
const HntapContainerConfPath = "naples/nic/conf/hntap-cfg.json"

//HntapTimeout Hntap timeou
const HntapTimeout time.Duration = time.Duration(10) * time.Second //HNTAP bring up timeout

const (
	//HntapTypeLocal Local type Hntap
	HntapTypeLocal string = "HntapLocal"
	//HntapTypeContainer Remote type Hntap
	HntapTypeContainer string = "HntapContainer"
)

//Hntap Interface
type Hntap interface {
	Run(noModel bool)
	WaitForHntapUp()
	Stop()
}

//HntapBase Struct
type HntapBase struct {
	ConfFile string
	LogFile  *string
	Timeout  time.Duration
}

//WaitForHntapUp Wait for Hntap interface
func (Hntap *HntapBase) WaitForHntapUp() {
	err := common.WaitForLineInLog(*Hntap.LogFile, "listening on", Hntap.Timeout)
	if err != nil {
		panic("Error waiting for Hntap!")
	}
}

//HntapLocal Hntap local struct
type HntapLocal struct {
	HntapBase
	Process *exec.Cmd
}

//Run Hntap local run method
func (hntap *HntapLocal) Run(noModel bool) {
	_, err := os.Create(*hntap.LogFile)
	if err != nil {
		log.Fatal("Error creating Hntap log file : ", err)
	}
	HntapCmd := []string{HntapCmd, "-f", hntap.ConfFile, "-n", "2"}
	if noModel {
		HntapCmd = append(HntapCmd, "-s")
	}

	Process, err := common.Run(HntapCmd, 0, true)
	if err != nil {
		log.Fatal("Hntap process start failed : ", err)
	}
	log.Println("Hntap Process started with pid : ", Process.Process.Pid)
	log.Println(" - Log File : ", *hntap.LogFile)
	hntap.Process = Process
	if !noModel {
		hntap.WaitForHntapUp()
	}

}

//Stop Hntap Stop method
func (hntap *HntapLocal) Stop() {
	hntap.Process.Process.Kill()
}

//HntapInContainer Hntpap container methond
type HntapInContainer struct {
	HntapBase
	Container AppEngine
}

//Run Hntap in continaer run method
func (hntap *HntapInContainer) Run(noModel bool) {
	_, err := os.Create(*hntap.LogFile)
	if err != nil {
		log.Fatal("Error creating Hntap log file : ", err)
	}
	dockerCp := []string{"docker", "cp", hntap.ConfFile,
		hntap.Container.GetName() + ":" + HntapContainerConfPath}
	common.Run(dockerCp, 0, false)
	fmt.Println("Restarting Hntap process in Nic Container.")
	hntap.Container.RunCommand([]string{ContainerHntapCmd}, 0, true)
	hntap.WaitForHntapUp()

	//Move Interface to Nampespace 1

	jsonFile, err := os.Open(hntap.ConfFile)
	if err != nil {
		panic(err)
	}
	// defer the closing of our jsonFile so that we can parse it later on
	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)
	var hntapcfg []HntapCfg
	json.Unmarshal(byteValue, &hntapcfg)
	for _, cfg := range hntapcfg {
		fmt.Println("Moving interface " + cfg.Name + " to ns 1")
		hntap.Container.MoveInterface(cfg.Name, 1)
	}

}

//Stop Hntap Stop method
func (hntap *HntapInContainer) Stop() {
}

//HntapFactory Interface
type HntapFactory interface {
	Create(configFile string, extraArg interface{}) Hntap
}

//HntapLocalFactory struct
type HntapLocalFactory struct{}

//HntapContainerFactory struct
type HntapContainerFactory struct{}

//Create Method
func (factory *HntapLocalFactory) Create(configFile string,
	extraArg interface{}) Hntap {
	return &HntapLocal{
		HntapBase: HntapBase{ConfFile: configFile,
			LogFile: &common.HntapLogFile,
			Timeout: HntapTimeout,
		},
	}
}

//Create Methond
func (factory *HntapContainerFactory) Create(configFile string, extraArg interface{}) Hntap {
	appEngine := extraArg.(AppEngine)
	return &HntapInContainer{
		HntapBase: HntapBase{ConfFile: configFile,
			LogFile: &common.HntapContainerLogFile,
			Timeout: HntapTimeout},
		Container: appEngine,
	}
}

//HntapHub struct
type HntapHub struct {
	HntapFactories map[string]HntapFactory
}

var _hntapHub = HntapHub{}

//HntapGet Hntap Factory method
func HntapGet(hntapType string, configFile string, extraArg interface{}) Hntap {
	hntapFactory := _hntapHub.HntapFactories[hntapType]
	if hntapFactory == nil {
		panic("Hntap type not found!")
	}
	return hntapFactory.Create(configFile, extraArg)
}

func init() {
	_hntapHub.HntapFactories = make(map[string]HntapFactory, 2)
	_hntapHub.HntapFactories[HntapTypeLocal] = &HntapLocalFactory{}
	_hntapHub.HntapFactories[HntapTypeContainer] = &HntapContainerFactory{}
}
