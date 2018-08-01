package infra

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"time"

	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/e2etests/go/cfg"
	"github.com/pensando/sw/nic/e2etests/go/common"
	TestApi "github.com/pensando/sw/nic/e2etests/go/tests/api"
)

//E2ETestCfg e2e test cfg struct
type E2ETestCfg struct {
	Name      string `yaml:"name"`
	Enabled   bool   `yaml:"enabled"`
	EpPairCfg struct {
		DstEpType string `yaml:"dst"`
		SrcEpType string `yaml:"src"`
	} `yaml:"endpoint"`
	Modules []struct {
		Module struct {
			Name    string `yaml:"name"`
			Program string `yaml:"program"`
		} `yaml:"module"`
	} `yaml:"modules"`
}

//E2ETest struct
type E2ETest struct {
	E2eCfg          *cfg.E2eCfg
	TestCfg         E2ETestCfg
	SrcEp           *Endpoint
	DstEp           *Endpoint
	NaplesContainer AppEngine
	Hntap           Hntap
}

//ReadTestSpecs read test specs from given directory
func ReadTestSpecs(dir string) []E2ETestCfg {
	files, err := filepath.Glob(dir + "/*.spec")
	if err != nil {
		panic(err)
	}
	e2eTestCfgs := []E2ETestCfg{}
	for _, f := range files {
		yamlFile, err := os.Open(f)
		if err != nil {
			fmt.Println(err)
		}
		byteValue, _ := ioutil.ReadAll(yamlFile)
		var testCfg E2ETestCfg
		yaml.Unmarshal(byteValue, &testCfg)
		e2eTestCfgs = append(e2eTestCfgs, testCfg)
		yamlFile.Close()
	}
	return e2eTestCfgs
}

func (e2e *E2ETest) String() string {
	return "E2E_TEST: " + e2e.TestCfg.Name
}

func (e2e *E2ETest) _GenerateEpPairCfg() (*Endpoint, *Endpoint) {

	epPairType := e2e.TestCfg.EpPairCfg.SrcEpType + "-" +
		e2e.TestCfg.EpPairCfg.DstEpType

	var HostEps = make([]*cfg.Endpoint,
		len(e2e.E2eCfg.EndpointsInfo.Endpoints))
	var RemoteEps = make([]*cfg.Endpoint,
		len(e2e.E2eCfg.EndpointsInfo.Endpoints))
	var hostIndex, remoteIndex int
	var srcEp, dstEp *cfg.Endpoint

	hostIndex = 0
	remoteIndex = 0
	for i := range e2e.E2eCfg.EndpointsInfo.Endpoints {
		ep := &e2e.E2eCfg.EndpointsInfo.Endpoints[i]
		if ep.EndpointSpec.InterfaceType == "lif" {
			HostEps[hostIndex] = ep
			hostIndex++
		} else {
			RemoteEps[remoteIndex] = ep
			remoteIndex++
		}
	}

	if epPairType == "host-host" {
		srcEp = HostEps[0]
		dstEp = HostEps[1]
	} else if epPairType == "host-remote" || epPairType == "remote-host" {
		srcEp = HostEps[0]
		dstEp = RemoteEps[0]
	} else if epPairType == "remote-remote" {
		srcEp = RemoteEps[0]
		dstEp = RemoteEps[1]
	} else {
		println(epPairType)
		panic("Invalid EP Pair Type")
	}
	return NewEndpoint(srcEp.EndpointMeta.Name,
			*srcEp, e2e.E2eCfg),
		NewEndpoint(dstEp.EndpointMeta.Name,
			*dstEp, e2e.E2eCfg)
}

//HntapCfg hntap config structure
type HntapCfg struct {
	Name  string `json:"name"`
	Local bool   `json:"local"`
	Port  int    `json:"port"`
	LifID int    `json:"lif_id"`
}

func _GetHntapCfgForEp(ep *Endpoint) *HntapCfg {
	return &HntapCfg{
		Name:  ep.Name,
		Local: !ep.Remote,
		Port:  ep.Interface.ID,
		LifID: ep.Interface.ID,
	}
}

func (e2e *E2ETest) _GenerateHntapConfig() {
	hntapCfgs := [...]*HntapCfg{_GetHntapCfgForEp(e2e.SrcEp),
		_GetHntapCfgForEp(e2e.DstEp)}
	hntapJSON, _ := json.Marshal(hntapCfgs)
	fmt.Println(common.HntapCfgFile)
	err := ioutil.WriteFile(common.HntapCfgFile, hntapJSON, 0644)
	if err != nil {
		log.Fatal("Error writing Hntap Config file")
	}
}

//BringUp bring up E2E environment
func (e2e *E2ETest) BringUp(noModel bool) {
	e2e.SrcEp, e2e.DstEp = e2e._GenerateEpPairCfg()
	if e2e.SrcEp == nil || e2e.DstEp == nil {
		panic("Failed to generate EP pair configuration")
	}
	e2e._GenerateHntapConfig()
	hntapType := HntapTypeLocal
	if e2e.NaplesContainer != nil {
		hntapType = HntapTypeContainer
	}
	e2e.Hntap = HntapGet(hntapType,
		common.HntapCfgFile, e2e.NaplesContainer)
	e2e.Hntap.Run(noModel)
	e2e._ConfigureEndpoints()
}

//Run Run Testcases with the environment.
func (e2e *E2ETest) Run(noModel bool,
	TestManager TestApi.TestManagerInterface) bool {
	if !e2e.TestCfg.Enabled {
		fmt.Println("E2E Test skipped!")
		return true
	}
	retCode := true
	for _, module := range e2e.TestCfg.Modules {
		test := TestManager.GetInstance(module.Module.Program)
		e2e.BringUp(noModel)
		e2e.PrintEnvironmentSummary()
		fmt.Println("Setting up Test : ", module.Module.Name,
			module.Module.Program)
		test.Setup(e2e.SrcEp, e2e.DstEp)
		fmt.Println("Running Test : ", module.Module.Name,
			module.Module.Program)
		testResult := test.Run(e2e.SrcEp, e2e.DstEp)
		fmt.Println("Teardown Test : ", module.Module.Name,
			module.Module.Program)
		test.Teardown(e2e.SrcEp, e2e.DstEp)
		if testResult {
			println("Test Passed")
		} else {
			println("Test Failed")
			retCode = false
		}
		e2e.Teardown()
	}
	return retCode
}

func (e2e *E2ETest) _ConfigureEndpoints() {
	e2e.SrcEp.Init()
	e2e.DstEp.Init()

	//Add Arp entries for now
	e2e.SrcEp.AppEngine.AddArpEntry(e2e.DstEp.IPAddress, e2e.DstEp.MacAddress)
	e2e.DstEp.AppEngine.AddArpEntry(e2e.SrcEp.IPAddress, e2e.SrcEp.MacAddress)
}

func (e2e *E2ETest) _CleanUpEndpoints() {
	e2e.SrcEp.Delete()
	e2e.DstEp.Delete()
}

//PrintEnvironmentSummary Print summary of environment.
func (e2e *E2ETest) PrintEnvironmentSummary() {
	fmt.Println("***********  Source EP information ********")
	e2e.SrcEp.PrintInformation()
	fmt.Println("************ Destination EP information *******")
	e2e.DstEp.PrintInformation()

}

//Teardown Teardown environment
func (e2e *E2ETest) Teardown() {
	e2e._CleanUpEndpoints()
	e2e.Hntap.Stop()
	time.Sleep(2 * time.Second)
}

//NewE2ETest Create Instance of E2E environment
func NewE2ETest(e2eCfg *cfg.E2eCfg,
	testcfg E2ETestCfg, naplesContainer AppEngine) *E2ETest {
	return &E2ETest{
		E2eCfg:          e2eCfg,
		TestCfg:         testcfg,
		NaplesContainer: naplesContainer,
	}
}
