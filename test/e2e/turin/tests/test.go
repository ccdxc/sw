package Tests

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"

	yaml "gopkg.in/yaml.v2"

	Common "github.com/pensando/sw/nic/e2etests/go/common"
	TestApi "github.com/pensando/sw/test/e2e/turin/tests/api"
)

var testSpecDir string

//ModuleSpec module spec
type ModuleSpec struct {
	Name    string `yaml:"name"`
	Enabled bool   `yaml:"enabled"`
	Tests   []struct {
		Test struct {
			Name    string `yaml:"name"`
			Program string `yaml:"program"`
		} `yaml:"test"`
	} `yaml:"tests"`
}

//ReadModuleSpecs read test specs from given directory
func ReadModuleSpecs() []ModuleSpec {
	dir := testSpecDir
	files, err := filepath.Glob(dir + "/*.spec")
	if err != nil {
		panic(err)
	}
	moduleCfgs := []ModuleSpec{}
	for _, f := range files {
		yamlFile, err := os.Open(f)
		if err != nil {
			fmt.Println(err)
		}
		byteValue, _ := ioutil.ReadAll(yamlFile)
		var moduleCfg ModuleSpec
		yaml.Unmarshal(byteValue, &moduleCfg)
		moduleCfgs = append(moduleCfgs, moduleCfg)
		yamlFile.Close()
	}
	return moduleCfgs
}

//TestManager struct
type TestManager struct {
	ModuleMap map[string]reflect.Type
}

//Register Resister a test with test manager
func (tm *TestManager) Register(obj interface{}) {
	tm.ModuleMap[Common.GetTypeName(obj)] = reflect.TypeOf(obj)
}

//GetInstance Get instance from name of test
func (tm *TestManager) GetInstance(moduleName string) TestApi.TestInterface {
	v := reflect.New(tm.ModuleMap[moduleName])
	return v.Interface().(TestApi.TestInterface)
}

var _TM = TestManager{}

//GetTestManager get test manager
func GetTestManager() *TestManager {
	return &_TM
}

func init() {
	_TM.ModuleMap = make(map[string]reflect.Type)
	testSpecDir = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/e2e/turin/tests/specs"
}
