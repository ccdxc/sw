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
	var cfg IOTAConfig
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
		log.Info("Skipping Flow Export Policy Generation")
		return nil
	}

	rulesPerPolicy := flowMonManifest.Count / flowExportManifest.Count
	log.Infof("Generating %v Flow Export Policies with %v Flow Monitor Rules.", flowExportManifest.Count, rulesPerPolicy)

	for i := 0; i < flowExportManifest.Count; i++ {
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}

		nsIdx := i % len(ns)
		namespace := ns[nsIdx]
		policyName := fmt.Sprintf("%s-%d", flowExportManifest.Name, i)

		policyRules := c.generateFlowMonRules(namespace.Name, rulesPerPolicy)[:rulesPerPolicy]
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
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/telemetry/flowexports"
	cfg.Objects = flowExportPolicies
	c.FlowExportPolicies = cfg
	return nil
}

func (c *CfgGen) generateFlowMonRules(namespace string, count int) (rules []monitoring.MatchRule) {
	sgpRules := c.generatePolicyRules(namespace, count)

	for _, s := range sgpRules {
		var protoPorts []string
		// Pick up from APP Object
		if len(s.Dst.AppConfigs) == 0 {
			appName := s.AppName
			// Get the apps object
			apps, ok := c.Apps.Objects.([]*netproto.App)
			if !ok {
				log.Errorf("Failed to cast the object %v to apps.", c.Apps.Objects)
			}
			for _, a := range apps {
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
	// Get the endpoints object
	eps, ok := c.Endpoints.Objects.([]*netproto.Endpoint)
	if !ok {
		log.Errorf("Failed to cast the object %v to endpoints.", c.Endpoints.Objects)
	}
	for _, ep := range eps {
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
