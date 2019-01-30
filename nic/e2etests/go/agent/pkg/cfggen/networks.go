package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
)

func (c *CfgGen) GenerateNetworks() error {
	var cfg IOTAConfig
	var networks []*netproto.Network
	var networkManifest *pkg.Object
	for _, o := range c.Config.Objects {
		if o.Kind == "Network" {
			networkManifest = &o
			break
		}
	}
	if networkManifest == nil {
		log.Debug("Network Manifest missing.")
	}

	log.Infof("Generating %v networks.", networkManifest.Count)

	subnets, err := libs.GenSubnets(c.Template.ParentCIDR, networkManifest.Count)
	if err != nil {
		log.Errorf("Failed to generate the required subnets. Err: %v", err)
	}

	for i := 0; i < networkManifest.Count; i++ {
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		nsIdx := i % len(ns)
		namespace := ns[nsIdx]
		subnet := subnets[i]
		addrs, gateway, err := libs.GenIPAddress(subnet, 0, true)
		if err != nil {
			log.Errorf("Failed to calculate default gateway.Err: %v", err)
		}
		networkName := fmt.Sprintf("%s-%d", networkManifest.Name, i)
		// Populate network ip address cache.
		c.SubnetIPLUT[networkName] = addrs
		net := netproto.Network{
			TypeMeta: api.TypeMeta{
				Kind: "Namespace",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      networkName,
			},
			Spec: netproto.NetworkSpec{
				IPv4Subnet:  subnet,
				IPv4Gateway: gateway,
				VlanID:      c.Template.VlanOffset + uint32(i),
			},
		}
		networks = append(networks, &net)
	}
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/networks/"
	cfg.Objects = networks
	c.Networks = cfg
	return nil
}
