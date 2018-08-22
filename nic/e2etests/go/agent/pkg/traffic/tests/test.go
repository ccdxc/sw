package Tests

import (
	"reflect"

	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
)

//TestManager struct
type TestManager struct {
	ModuleMap map[string]reflect.Type
}

//Register Resister a test with test manager
func (tm *TestManager) Register(obj TestApi.TestInterface) {
	tm.ModuleMap[Common.GetTypeName(obj)] = reflect.TypeOf(obj)
}

//GetInstances Get instance from name of test
func (tm *TestManager) GetInstances(moduleName string) []TestApi.TestInterface {
	tests := []TestApi.TestInterface{}
	if moduleName != "" {
		v := reflect.New(tm.ModuleMap[moduleName])
		tests = append(tests, v.Interface().(TestApi.TestInterface))
	} else {
		for _, value := range tm.ModuleMap {
			v := reflect.New(value)
			tests = append(tests, v.Interface().(TestApi.TestInterface))
		}
	}

	return tests
}

var _TM = TestManager{}

//GetTestManager get test manager
func GetTestManager() *TestManager {
	return &_TM
}

func init() {
	_TM.ModuleMap = make(map[string]reflect.Type)
}
