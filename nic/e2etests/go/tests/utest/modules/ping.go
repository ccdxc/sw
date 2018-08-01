package modules

import (
	"fmt"

	InfraApi "github.com/pensando/sw/nic/e2etests/go/infra/api"
	TestApi "github.com/pensando/sw/nic/e2etests/go/tests"
)

//Ping Ping test structure
type Ping struct {
}

//Setup Setup environment for test
func (t *Ping) Setup(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
}

//Run Run test
func (t *Ping) Run(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) bool {
	fmt.Println("Called Test Set Run")
	cmd := []string{"ping", "-c", "5", dstEp.GetIP()}
	_, err := srcEp.RunCommand(cmd, 0, false)
	if err != nil {
		return false
	}
	cmd[len(cmd)-1] = srcEp.GetIP()
	_, err = dstEp.RunCommand(cmd, 0, false)
	if err != nil {
		return false
	}

	return true
}

//Teardown Teardown environment
func (t *Ping) Teardown(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) {
}

func init() {
	tm := TestApi.GetTestManager()
	tm.Register(Ping{})
}
