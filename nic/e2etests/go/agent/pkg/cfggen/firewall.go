package cfggen

import (
	"fmt"
	"strings"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"

	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateFirewallPolicies() error {
	var sgPolicies []*netproto.SGPolicy

	var sgPolicyManifest, sgRuleManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "SGPolicy" {
			sgPolicyManifest = &o
		}
		if o.Kind == "SGRule" {
			sgRuleManifest = &o
		}
	}

	if sgPolicyManifest == nil || sgRuleManifest == nil {
		log.Debug("SG Policy manifest missing.")
	}

	rulesPerPolicy := sgRuleManifest.Count / sgPolicyManifest.Count

	log.Infof("Generating %v sg policies with %v rules in each.", sgPolicyManifest.Count, rulesPerPolicy)

	// Generate EPPairs

	for i := 0; i < sgPolicyManifest.Count; i++ {
		sgPolicyName := fmt.Sprintf("%s-%d", sgPolicyManifest.Name, i)
		nsIdx := i % len(c.Namespaces)
		namespace := c.Namespaces[nsIdx]

		// Generate Node EP Pairs in the current namespace
		for _, nodeUUID := range c.NodeUUIDs {
			c.NodeEPLUT[nodeUUID] = c.GenerateEPPairs(namespace.Name, nodeUUID, sgRuleManifest.Count)
		}

		policyRules := c.generatePolicyRules(rulesPerPolicy)[:rulesPerPolicy]

		sgPolicy := netproto.SGPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "SGPolicy",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      sgPolicyName,
			},
			Spec: netproto.SGPolicySpec{
				AttachTenant: true,
				Rules:        policyRules,
			},
		}
		sgPolicies = append(sgPolicies, &sgPolicy)
	}
	c.SGPolicies = sgPolicies
	return nil
}

func (c *CfgGen) GenerateEPPairs(namespace, nodeUUID string, count int) (epPairs NodeEPPairs) {
	var localEPs, remoteEPs []string
	for _, ep := range c.Endpoints {
		if ep.Namespace == namespace {
			if ep.Spec.NodeUUID == nodeUUID {
				localEPs = append(localEPs, ep.Spec.IPv4Address)
			} else if ep.Spec.NodeUUID != defaultRemoteUUIDName {
				remoteEPs = append(remoteEPs, ep.Spec.IPv4Address)
			}
		}
	}

	//remoteEPs = append(remoteEPs, gwEPs...)

	epPairs.LocalEPPairs = c.genEPPairs(localEPs, nil, count)
	epPairs.RemoteEPPairs = c.genEPPairs(localEPs, remoteEPs, count)
	return
}

func (c *CfgGen) genEPPairs(localEPs, remoteEPs []string, count int) (epPairs []EPPair) {
	if remoteEPs == nil {
		combinations := libs.GenPairs(localEPs, count)
		for _, c := range combinations {
			epPair := EPPair{
				SrcEP: c[0],
				DstEP: c[1],
			}
			epPairs = append(epPairs, epPair)
		}
		return
	}
	localCombinations := libs.GenPairs(localEPs, count)
	remoteCombinations := libs.GenPairs(remoteEPs, count)

	for _, l := range localCombinations {
		for _, r := range remoteCombinations {
			epPair := []EPPair{
				{
					SrcEP: l[0],
					DstEP: r[0],
				},
				{
					SrcEP: l[1],
					DstEP: r[1],
				},
			}
			epPairs = append(epPairs, epPair...)
			if len(epPairs) == count {
				return
			}
		}
	}
	return
}

func (c *CfgGen) getRemoteEPs(nodeUUID string) []string {
	var remoteEps []string
	for nodeID, endpoints := range c.EpCache {
		if nodeID != defaultRemoteUUIDName {
			remoteEps = append(remoteEps, endpoints...)
		}
	}
	// Add other remote EPs which are not on naples node
	remoteEps = append(remoteEps, c.EpCache[defaultRemoteUUIDName]...)
	return remoteEps
}

func (c *CfgGen) generatePolicyRules(count int) (policyRules []netproto.PolicyRule) {
	for j := 0; j < count; j++ {
		for _, nodeUUID := range c.NodeUUIDs {
			var proto, port string
			var localEPPair, remoteEPPair EPPair
			localEPPairs := c.NodeEPLUT[nodeUUID].LocalEPPairs
			remoteEPPairs := c.NodeEPLUT[nodeUUID].RemoteEPPairs

			//Pick a local and remote EP Pair for the rule
			if len(c.NodeEPLUT[nodeUUID].LocalEPPairs) != 0 {
				localEPPair, localEPPairs = localEPPairs[0], localEPPairs[1:]
			} else {
				localEPPair, localEPPairs = remoteEPPairs[0], remoteEPPairs[1:]
			}
			remoteEPPair, remoteEPPairs = remoteEPPairs[0], remoteEPPairs[1:]

			c.NodeEPLUT[nodeUUID] = NodeEPPairs{
				LocalEPPairs:  localEPPairs,
				RemoteEPPairs: remoteEPPairs,
			}
			app := c.Apps[j%len(c.Apps)]
			l4Match := c.Template.FirewallPolicyRules[j%len(c.Template.FirewallPolicyRules)]
			components := strings.Split(l4Match, "/")

			if components[0] == "*" {
				proto = "any"
			} else {
				proto = components[0]
				port = components[1]
			}

			appConfig := []*netproto.AppConfig{
				{
					Protocol: proto,
					Port:     port,
				},
			}

			rules := []netproto.PolicyRule{
				{
					Action:  "PERMIT",
					AppName: app.Name,
					Src: &netproto.MatchSelector{
						Addresses: []string{localEPPair.SrcEP},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{localEPPair.DstEP},
					},
				},
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{localEPPair.SrcEP},
					},
					Dst: &netproto.MatchSelector{
						Addresses:  []string{localEPPair.DstEP},
						AppConfigs: appConfig,
					},
				},
				{
					Action:  "DENY",
					AppName: app.Name,
					Src: &netproto.MatchSelector{
						Addresses: []string{remoteEPPair.SrcEP},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{remoteEPPair.DstEP},
					},
				},
			}

			policyRules = append(policyRules, rules...)
			if len(policyRules) > count {
				return
			}
		}
	}
	return
}
