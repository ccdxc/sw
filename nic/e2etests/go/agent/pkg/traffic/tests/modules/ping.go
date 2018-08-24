package modules

import (
	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Tests "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests"
	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
	"github.com/pkg/errors"
)

type Ping struct {
}

func (Ping) Run(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {
	/* If differet Network, setup routes */

	cmd := []string{"ping", "-c", "5", dstEp.GetIP()}

	if _, err := srcEp.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Ping test failed")
	}
	cmd[3] = srcEp.GetIP()
	if _, err := dstEp.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Ping test failed")
	}

	return nil
}

func (Ping) FilterEpPairs(epPairs []TestApi.EpPair, agentCfg *Pkg.AgentConfig) []TestApi.EpPair {
	return epPairs
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(Ping{})
}
