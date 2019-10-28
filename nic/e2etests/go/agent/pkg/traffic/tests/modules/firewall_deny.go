package modules

import (
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/protos/netproto"
	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Tests "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests"
	TestApi "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/api"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
)

type FirewallDeny struct {
	sgPolicies *netproto.NetworkSecurityPolicy // For now just store everything.

}

func (fw FirewallDeny) Run(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {
	cmd := []string{"nc", "-l", "80"}
	if _, err := dstEp.RunCommand(cmd, 0, true); err != nil {
		return err
	}
	cmd = []string{"nc", "-w", "10", dstEp.GetIP(), "80"}
	_, err := srcEp.RunCommand(cmd, 0, false)
	if err != nil {
		return nil
	}
	return errors.New("Connection was successfull, not expected")
}

func (FirewallDeny) FilterEpPairs(epPairs []TestApi.EpPair, agentCfg *Pkg.AgentConfig) []TestApi.EpPair {

	findMatchingEpPair := func(sip string, dip string) *TestApi.EpPair {

		for _, epPair := range epPairs {
			srcEp := epPair.Src
			dstEp := epPair.Dst
			if strings.Split(srcEp.Spec.GetIPv4Addresses()[0], "/")[0] == sip &&
				strings.Split(dstEp.Spec.GetIPv4Addresses()[0], "/")[0] == dip {
				return &epPair
			}
		}
		return nil
	}

	eps := []TestApi.EpPair{}
	for _, policy := range agentCfg.SgPolicies {
		for _, rule := range policy.Spec.Rules {
			if rule.GetAction() == "DENY" && rule.GetSrc() != nil && rule.GetDst() != nil {

				epPair := findMatchingEpPair(rule.GetSrc().GetAddresses()[0],
					rule.GetDst().GetAddresses()[0])
				if epPair != nil {
					eps = append(eps, *epPair)
				}
			}
		}
	}
	return eps
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(FirewallDeny{})
}
