package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) generateMirrorRules(namespace string, count int) (rules []netproto.MatchRule) {
	sgpRules := c.generatePolicyRules(namespace, count)

	for _, s := range sgpRules {
		r := netproto.MatchRule{
			Src: &netproto.MatchSelector{
				Addresses: s.Src.Addresses,
			},
			Dst: &netproto.MatchSelector{
				Addresses: s.Dst.Addresses,
				ProtoPorts: s.Dst.ProtoPorts,
			},
		}

		rules = append(rules, r)
	}

	return
}

func (c *CfgGen) GenerateMirrorSessions() error {
	var cfg pkg.IOTAConfig
	var mirrors []*netproto.MirrorSession
	var mirrorManifest, mirrorRuleMaifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "MirrorSession" {
			mirrorManifest = &o
		}
		if o.Kind == "MirrorSessionRule" {
			mirrorRuleMaifest = &o
		}
	}
	if mirrorManifest == nil {
		log.Debug("Mirrors missing.")
		log.Info("Skipping Mirror Session Generation")
		return nil
	}

	log.Infof("Generating %v Mirror Sessions.", mirrorManifest.Count)

	ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
	if !ok {
		log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
		return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
	}

	rulesPerPolicy := mirrorRuleMaifest.Count / mirrorManifest.Count
	namespace := ns[0]
	policyRules := c.generateMirrorRules(namespace.Name, 0)
	for i := 0; i < mirrorManifest.Count; i++ {
		// Get the namespaces object
		mirrorSessionName := fmt.Sprintf("%s-%d", mirrorManifest.Name, i)

		tuns, ok := c.Tunnels.Objects.([]*netproto.Tunnel)
		if !ok {
			log.Errorf("Failed to cast the object %v to tunnels.", c.Tunnels.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Tunnels.Objects)
		}
		t := tuns[i%len(tuns)]

		newPolicyRules := policyRules[:rulesPerPolicy]
		policyRules = policyRules[rulesPerPolicy:]
		ms := netproto.MirrorSession{
			TypeMeta: api.TypeMeta{
				Kind: "MirrorSession",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      mirrorSessionName,
			},
			Spec: netproto.MirrorSessionSpec{
				Collectors: []netproto.MirrorCollector{
					{
						ExportCfg: netproto.MirrorExportConfig{
							Destination: t.Spec.Dst,
						},
					},
				},
				MatchRules: newPolicyRules,
			},
		}
		mirrors = append(mirrors, &ms)
	}
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/mirror/sessions/"
	cfg.Objects = mirrors
	c.MirrorSessions = cfg
	return nil
}
