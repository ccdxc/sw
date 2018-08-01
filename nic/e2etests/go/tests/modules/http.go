package modules

import (
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	InfraApi "github.com/pensando/sw/nic/e2etests/go/infra/api"
	Tests "github.com/pensando/sw/nic/e2etests/go/tests"
)

//HTTP test structure
type HTTP struct {
	HTTPServerHandle Common.CmdHandle
}

//Setup function
func (t *HTTP) Setup(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
	cmd := []string{"python", "-m", "SimpleHTTPServer", "8000"}
	srcEp.RunCommand(cmd, 0, true)
}

//Run function
func (t *HTTP) Run(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) bool {
	cmd := []string{"wget", srcEp.GetIP() + ":8000"}
	_, err := dstEp.RunCommand(cmd, 0, false)
	if err != nil {
		return false
	}

	return true
}

//Teardown function
func (t *HTTP) Teardown(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(HTTP{})
}
