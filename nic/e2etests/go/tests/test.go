package Tests

import (
	"reflect"

	Common "github.com/pensando/sw/nic/e2etests/go/common"
	TestApi "github.com/pensando/sw/nic/e2etests/go/tests/api"
)

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
}
