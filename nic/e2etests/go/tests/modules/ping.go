package modules

import (
	InfraApi "github.com/pensando/sw/nic/e2etests/go/infra/api"
	Tests "github.com/pensando/sw/nic/e2etests/go/tests"
)

//Ping Structure
type Ping struct {
}

//Setup Function
func (t *Ping) Setup(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
}

//Run Function
func (t *Ping) Run(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) bool {
	cmd := []string{"ping", "-c", "5", dstEp.GetIP()}
	if _, err := srcEp.RunCommand(cmd, 0, false); err != nil {
		return false
	}
	cmd[len(cmd)-1] = srcEp.GetIP()
	if _, err := dstEp.RunCommand(cmd, 0, false); err != nil {
		return false
	}
	return true
}

//Teardown function
func (t *Ping) Teardown(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(Ping{})
}
