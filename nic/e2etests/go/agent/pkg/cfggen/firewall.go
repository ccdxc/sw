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
	var cfg pkg.IOTAConfig
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

func (c *CfgGen) GenerateEPPairs(namespace, nodeUUID string, count int) (nwEpPairs NwNodeEPPairs) {
	// Get the endpoints object
	eps, ok := c.Endpoints.Objects.([]*netproto.Endpoint)
	if !ok {
		log.Errorf("Failed to cast the object %v to endpoints.", c.Endpoints.Objects)
	}

	nws, ok := c.Networks.Objects.([]*netproto.Network)
	if !ok {
		log.Errorf("Failed to cast the object %v to networks.", c.Networks.Objects)
	}

	nwEpPairs.NwMap = make(map[string]*NodeEPPairs)
	var nodeEpPair *NodeEPPairs

	for _, nw := range nws {
		var localEPs, remoteEPs []string
		nodeEpPair = &NodeEPPairs{}
		nwEpPairs.NwMap[nw.GetObjectMeta().GetName()] = nodeEpPair

		for _, ep := range eps {
			if ep.Namespace == namespace && ep.Spec.NetworkName == nw.GetObjectMeta().GetName() {
				if ep.Spec.NodeUUID == nodeUUID {
					localEPs = append(localEPs, ep.Spec.IPv4Address)
				} else {
					remoteEPs = append(remoteEPs, ep.Spec.IPv4Address)
				}
			}
		}

		//remoteEPs = append(remoteEPs, gwEPs...)
		nodeEpPair.LocalEPPairs = c.genEPPairs(localEPs, nil, 0)
		nodeEpPair.RemoteEPPairs = c.genEPPairs(localEPs, remoteEPs, 0)
		fmt.Printf("Generated EP pairs local : %v , remote : %v \n", len(nodeEpPair.LocalEPPairs), len(nodeEpPair.RemoteEPPairs))
	}
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
	//localCombinations := libs.GenPairs(localEPs, count)
	//remoteCombinations := libs.GenPairs(remoteEPs, count)

	for _, l := range localEPs {
		for _, r := range remoteEPs {
			sIP1, _, _ := net.ParseCIDR(l)
			dIP1, _, _ := net.ParseCIDR(r)
			epPair := []EPPair{
				{
					SrcEP: sIP1.String(),
					DstEP: dIP1.String(),
				},
			}
			epPairs = append(epPairs, epPair...)
			if count > 0 && len(epPairs) >= count {
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
		rulesAdded := false
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
			for nw, nwEpPair := range c.NodeEPLUT[nodeUUID].NwMap {

				app := curApps[j%len(curApps)]

				appConfig := c.generateL4Match(j % len(c.Template.FirewallPolicyRules))

				rulesAdded = true
				for _, localEPPair := range nwEpPair.LocalEPPairs {
					rules := []netproto.PolicyRule{
						{
							Action:  "PERMIT",
							AppName: app.Name,
							Src: &netproto.MatchSelector{
								Addresses: convertIPAddresses(localEPPair.SrcEP),
							},
							Dst: &netproto.MatchSelector{
								Addresses: convertIPAddresses(localEPPair.DstEP),
							},
						},
						{
							Action: "PERMIT",
							Src: &netproto.MatchSelector{
								Addresses: convertIPAddresses(localEPPair.SrcEP),
							},
							Dst: &netproto.MatchSelector{
								Addresses:  convertIPAddresses(localEPPair.DstEP),
								AppConfigs: appConfig,
							},
						},
					}
					policyRules = append(policyRules, rules...)
				}

				for _, remoteEPPair := range nwEpPair.RemoteEPPairs {
					rules := []netproto.PolicyRule{
						{
							Action:  "PERMIT",
							AppName: app.Name,
							Src: &netproto.MatchSelector{
								Addresses: convertIPAddresses(remoteEPPair.SrcEP),
							},
							Dst: &netproto.MatchSelector{
								Addresses: convertIPAddresses(remoteEPPair.DstEP),
							},
						},
						{
							Action: "PERMIT",
							Src: &netproto.MatchSelector{
								Addresses: convertIPAddresses(remoteEPPair.SrcEP),
							},
							Dst: &netproto.MatchSelector{
								Addresses:  convertIPAddresses(remoteEPPair.DstEP),
								AppConfigs: appConfig,
							},
						},
					}
					policyRules = append(policyRules, rules...)
				}

				delete(c.NodeEPLUT[nodeUUID].NwMap, nw)
			}

			if len(policyRules) > count {
				return
			}
		}

		if !rulesAdded {
			log.Panic("Not enough EP pairs to generate all rules")
		}
	}
	return
}

func (c *CfgGen) generateL4Match(offset int) []*netproto.AppConfig {
	var appConfigs []*netproto.AppConfig
	matches := c.Template.FirewallPolicyRules[offset:]

	if len(matches) < c.Template.L4MatchPerRule {
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
			Protocol: proto,
			Port:     port,
		}
		appConfigs = append(appConfigs, appConfig)

	}
	return appConfigs
}

func convertIPAddresses(args ...string) (ipAddresses []string) {
	ipAddresses = append(ipAddresses, args...)
	//ipAddresses = append(ipAddresses, extraIPs...)
	return
}
