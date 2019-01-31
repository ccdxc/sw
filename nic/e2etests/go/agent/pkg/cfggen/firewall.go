package cfggen

import (
	"fmt"
	"net"
	"strings"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"

	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateFirewallPolicies() error {
	var cfg IOTAConfig
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
		log.Info("Skipping SG Policy Generation")
		return nil
	}

	rulesPerPolicy := sgRuleManifest.Count / sgPolicyManifest.Count

	log.Infof("Generating %v sg policies with %v rules in each.", sgPolicyManifest.Count, rulesPerPolicy)

	// Generate EPPairs

	for i := 0; i < sgPolicyManifest.Count; i++ {
		sgPolicyName := fmt.Sprintf("%s-%d", sgPolicyManifest.Name, i)
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		nsIdx := i % len(ns)
		namespace := ns[nsIdx]

		// Generate Node EP Pairs in the current namespace
		for _, nodeUUID := range c.NodeUUIDs {
			c.NodeEPLUT[nodeUUID] = c.GenerateEPPairs(namespace.Name, nodeUUID, sgRuleManifest.Count)
		}

		policyRules := c.generatePolicyRules(namespace.Name, rulesPerPolicy)[:rulesPerPolicy]

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

	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/security/policies/"
	cfg.Objects = sgPolicies
	c.SGPolicies = cfg
	return nil
}

func (c *CfgGen) GenerateEPPairs(namespace, nodeUUID string, count int) (epPairs NodeEPPairs) {
	var localEPs, remoteEPs []string
	// Get the endpoints object
	eps, ok := c.Endpoints.Objects.([]*netproto.Endpoint)
	if !ok {
		log.Errorf("Failed to cast the object %v to endpoints.", c.Endpoints.Objects)
	}

	for _, ep := range eps {
		if ep.Namespace == namespace {
			if ep.Spec.NodeUUID == nodeUUID {
				localEPs = append(localEPs, ep.Spec.IPv4Address)
			} else {
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
			sIP, _, _ := net.ParseCIDR(c[0])
			dIP, _, _ := net.ParseCIDR(c[1])
			epPair := EPPair{
				SrcEP: sIP.String(),
				DstEP: dIP.String(),
			}
			epPairs = append(epPairs, epPair)
		}
		return
	}
	localCombinations := libs.GenPairs(localEPs, count)
	remoteCombinations := libs.GenPairs(remoteEPs, count)

	for _, l := range localCombinations {
		for _, r := range remoteCombinations {
			sIP1, _, _ := net.ParseCIDR(l[0])
			sIP2, _, _ := net.ParseCIDR(l[1])
			dIP1, _, _ := net.ParseCIDR(r[0])
			dIP2, _, _ := net.ParseCIDR(r[1])
			epPair := []EPPair{
				{
					SrcEP: sIP1.String(),
					DstEP: dIP1.String(),
				},
				{
					SrcEP: sIP2.String(),
					DstEP: dIP2.String(),
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

func (c *CfgGen) generatePolicyRules(namespace string, count int) (policyRules []netproto.PolicyRule) {
	for j := 0; j < count; j++ {
		for _, nodeUUID := range c.NodeUUIDs {
			// Get the apps object
			apps, ok := c.Apps.Objects.([]*netproto.App)
			if !ok {
				log.Errorf("Failed to cast the object %v to apps.", c.Apps.Objects)
			}

			// ensure that the apps are in the same namespace as the policy
			var curApps []*netproto.App
			for _, a := range apps {
				if a.Namespace == namespace {
					curApps = append(curApps, a)
				}
			}
			var localEPPair, remoteEPPair EPPair

			//Pick a local and remote EP Pair for the rule
			if len(c.NodeEPLUT[nodeUUID].LocalEPPairs) == 0 || len(c.NodeEPLUT[nodeUUID].RemoteEPPairs) == 0 {
				c.NodeEPLUT[nodeUUID] = NodeEPPairs{
					RemoteEPPairs: c.genEPPairs(c.EpCache[nodeUUID], c.EpCache[defaultRemoteUUIDName], count),
				}
			}

			localEPPairs := c.NodeEPLUT[nodeUUID].LocalEPPairs
			remoteEPPairs := c.NodeEPLUT[nodeUUID].RemoteEPPairs

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
			app := curApps[j%len(curApps)]

			appConfig := c.generateL4Match(j % len(c.Template.FirewallPolicyRules))
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


func (c *CfgGen) generateL4Match(offset int) []*netproto.AppConfig{
	var appConfigs []*netproto.AppConfig
	matches := c.Template.FirewallPolicyRules[offset:]

	if len(matches) < c.Template.L4MatchPerRule{
		matches = append(matches, c.Template.FirewallPolicyRules[:offset]...)
	}
	matches = matches[:c.Template.L4MatchPerRule]

	for _, l4Match := range matches {
		var proto, port string
		components := strings.Split(l4Match, "/")
		if components[0] == "*" {
			proto = "any"
			port = "1-65535"
		} else {
			proto = components[0]
			port = components[1]
		}

		appConfig := &netproto.AppConfig{
			Protocol:proto,
			Port: port,
		}
		appConfigs = append(appConfigs, appConfig)

	}
	return appConfigs
}