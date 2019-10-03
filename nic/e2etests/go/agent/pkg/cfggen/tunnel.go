package cfggen

import (
	"fmt"
	"net"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateTunnels() error {
	var cfg pkg.IOTAConfig
	var tunnels []*netproto.Tunnel
	var tunnelManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "Tunnel" {
			tunnelManifest = &o
			break
		}
	}
	if tunnelManifest == nil {
		log.Debug("Tunnel missing.")
		log.Info("Skipping Tunnel Generation")
		return nil
	}

	log.Infof("Generating %v Tunnels.", tunnelManifest.Count)

	remoteEPs := c.GetRemoteEndpointsIPs()
	remoteEPs = remoteEPs[:tunnelManifest.Count]
	for i := 0; i < tunnelManifest.Count; i++ {
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		nsIdx := i % len(ns)
		namespace := ns[nsIdx]
		tunnelName := fmt.Sprintf("%s-%d", tunnelManifest.Name, i)
		tep, _, err := net.ParseCIDR(remoteEPs[i])
		if err != nil {
			log.Errorf("Err: %v", err)
		}

		t := netproto.Tunnel{
			TypeMeta: api.TypeMeta{
				Kind: "Tunnel",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      tunnelName,
			},
			Spec: netproto.TunnelSpec{
				Type:        "GRE",
				AdminStatus: "UP",
				Src:         "100.100.100.100",
				Dst:         tep.String(),
			},
		}
		tunnels = append(tunnels, &t)
	}
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/tunnels/"
	cfg.Objects = tunnels
	c.Tunnels = cfg
	return nil
}
