package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateMirrorSessions() error {
	var portOffset = 8000
	var cfg pkg.IOTAConfig
	var mirrors []*tsproto.MirrorSession
	var mirrorManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "MirrorSession" {
			mirrorManifest = &o
			break
		}
	}
	if mirrorManifest == nil {
		log.Debug("Mirrors missing.")
		log.Info("Skipping Mirror Session Generation")
		return nil
	}

	log.Infof("Generating %v Mirror Sessions.", mirrorManifest.Count)

	for i := 0; i < mirrorManifest.Count; i++ {
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		namespace := ns[i%len(ns)]
		mirrorSessionName := fmt.Sprintf("%s-%d", mirrorManifest.Name, i)

		tuns, ok := c.Tunnels.Objects.([]*netproto.Tunnel)
		if !ok {
			log.Errorf("Failed to cast the object %v to tunnels.", c.Tunnels.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Tunnels.Objects)
		}
		t := tuns[i%len(tuns)]

		ms := tsproto.MirrorSession{
			TypeMeta: api.TypeMeta{
				Kind: "MirrorSession",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      mirrorSessionName,
			},
			Spec: tsproto.MirrorSessionSpec{
				Enable:     true,
				PacketSize: 128,
				Collectors: []tsproto.MirrorCollector{
					{
						Type: "ERSPAN",
						ExportCfg: monitoring.ExportConfig{
							Destination: t.Spec.Dst,
							Transport:   fmt.Sprintf("TCP/%d", portOffset+i),
						},
					},
				},
				MatchRules: []tsproto.MatchRule{
					{
						Src: &tsproto.MatchSelector{
							IPAddresses: []string{"0.0.0.0/0"},
						},
						Dst: &tsproto.MatchSelector{
							IPAddresses: []string{"0.0.0.0/0"},
						},
						AppProtoSel: &tsproto.AppProtoSelector{
							Ports: []string{"ICMP/0/0"},
						},
					},
					{
						Src: &tsproto.MatchSelector{
							IPAddresses: []string{"0.0.0.0/0"},
						},
						Dst: &tsproto.MatchSelector{
							IPAddresses: []string{"0.0.0.0/0"},
						},
						AppProtoSel: &tsproto.AppProtoSelector{
							Ports: []string{"TCP/0/0"},
						},
					},
				},
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
