package modules

import (
	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Tests "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests"
	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
)

type Utest struct {
}

func (Utest) Run(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {
	return nil
}

func (Utest) FilterEpPairs(epPairs []TestApi.EpPair, agentCfg *Pkg.AgentConfig) []TestApi.EpPair {
	return epPairs
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(Utest{})
}
