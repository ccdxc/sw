package cfggen

import (
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateFlowMonitorRules() error {
	var flowExportPolicies []*monitoring.FlowExportPolicy
	var flowExportManifest, flowMonManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "FlowExportPolicy" {
			flowExportManifest = &o
		}
		if o.Kind == "FlowMonitorRule" {
			flowMonManifest = &o
		}

	}
	if flowExportManifest == nil || flowMonManifest == nil {
		log.Debug("Flow Export Manifest missing.")
	}

	rulesPerPolicy := flowMonManifest.Count / flowExportManifest.Count
	log.Infof("Generating %v Flow Export Policies with %v Flow Monitor Rules.", flowExportManifest.Count, rulesPerPolicy)

	for i := 0; i < flowExportManifest.Count; i++ {
		nsIdx := i % len(c.Namespaces)
		namespace := c.Namespaces[nsIdx]
		policyName := fmt.Sprintf("%s-%d", flowExportManifest.Name, i)

		policyRules := c.generateFlowMonRules(rulesPerPolicy)[:rulesPerPolicy]
		exportConfigs := c.generateExportConfigs(namespace.Name)

		fe := monitoring.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      policyName,
			},
			Spec: monitoring.FlowExportPolicySpec{
				Interval:   c.Template.FlowMonInterval,
				Format:     "IPFIX",
				MatchRules: policyRules,
				Exports:    exportConfigs,
			},
		}
		flowExportPolicies = append(flowExportPolicies, &fe)
	}
	c.FlowExportPolicies = flowExportPolicies
	return nil
}

func (c *CfgGen) generateFlowMonRules(count int) (rules []monitoring.MatchRule) {
	sgpRules := c.generatePolicyRules(count)

	for _, s := range sgpRules {
		var protoPorts []string
		// Pick up from APP Object
		if len(s.Dst.AppConfigs) == 0 {
			appName := s.AppName
			for _, a := range c.Apps {
				if a.Name == appName {
					protoPorts = a.Spec.ProtoPorts
				}
			}
		} else {
			protoPorts = convertProtoPort(s.Dst.AppConfigs[0])
		}
		r := monitoring.MatchRule{
			Src: &monitoring.MatchSelector{
				IPAddresses: s.Src.Addresses,
			},
			Dst: &monitoring.MatchSelector{
				IPAddresses: s.Dst.Addresses,
			},
			AppProtoSel: &monitoring.AppProtoSelector{
				Ports: protoPorts,
			},
		}
		rules = append(rules, r)
	}

	return
}

func (c *CfgGen) generateExportConfigs(namespace string) (exportCfg []monitoring.ExportConfig) {
	var dst string
	for _, ep := range c.Endpoints {
		if ep.Namespace == namespace && ep.Spec.NodeUUID == defaultRemoteUUIDName {
			dst = ep.Spec.IPv4Address
			break
		}
	}
	e := monitoring.ExportConfig{
		Destination: dst,
		Transport:   "UDP/2055",
	}
	exportCfg = append(exportCfg, e)
	return
}

func convertProtoPort(app *netproto.AppConfig) []string {
	var proto, port string
	if len(app.Protocol) == 0 {
		proto = "any"
	} else {
		proto = app.Protocol
	}
	if len(app.Port) == 0 {
		port = "any"
	} else {
		port = app.Port
	}
	return []string{fmt.Sprintf("%s/%s", proto, port)}
}
