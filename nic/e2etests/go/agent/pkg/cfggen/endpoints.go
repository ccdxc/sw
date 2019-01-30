package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
)

func (c *CfgGen) GenerateEndpoints() error {
	var cfg IOTAConfig
	var endpoints []*netproto.Endpoint
	var localEPManifest, remoteEPManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "LocalEndpoint" {
			localEPManifest = &o
		} else if o.Kind == "RemoteEndpoint" {
			remoteEPManifest = &o
		}
	}

	if localEPManifest == nil || remoteEPManifest == nil {
		log.Debug("Endpoint Manifest missing. Local: %v, Remote: %v", localEPManifest, remoteEPManifest)
	}

	log.Infof("Generating %v Local Endpoints.", localEPManifest.Count)

	// Generate MAC Addresses
	macAddrs, err := libs.GenMACAddresses(localEPManifest.Count + remoteEPManifest.Count)
	if err != nil {
		log.Errorf("Could not generate %v MAC Addresses. Err: %v", localEPManifest.Count+remoteEPManifest.Count, err)
	}

	for i := 0; i < localEPManifest.Count; i++ {
		var epIP string
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}

		// Get the networks object
		nets, ok := c.Networks.Objects.([]*netproto.Network)
		if !ok {
			log.Errorf("Failed to cast the object %v to networks.", c.Networks.Objects)
			return fmt.Errorf("failed to cast the object %v to networks", c.Networks.Objects)
		}

		nsIdx := i % len(ns)
		netIdx := i % len(nets)
		namespace := ns[nsIdx]
		network := nets[netIdx]
		epName := fmt.Sprintf("%s-%d", localEPManifest.Name, i)
		// Generate local ep for every naples.
		for _, nodeUUID := range c.NodeUUIDs {
			nodeUUID := nodeUUID
			// Pop elements here instead of maintaining a complex index
			epIP, c.SubnetIPLUT[network.Name] = c.SubnetIPLUT[network.Name][0], c.SubnetIPLUT[network.Name][1:]
			ep := netproto.Endpoint{
				TypeMeta: api.TypeMeta{
					Kind: "Endpoint",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: namespace.Name,
					Name:      epName,
				},
				Spec: netproto.EndpointSpec{
					NetworkName: network.Name,
					UsegVlan:    c.Template.USegVlanOffset + uint32(i),
					MacAddress:  macAddrs[i],
					IPv4Address: epIP,
					NodeUUID:    nodeUUID,
				},
			}
			c.EpCache[nodeUUID] = append(c.EpCache[nodeUUID], epIP)
			endpoints = append(endpoints, &ep)
		}
	}

	log.Infof("Generating %v Remote Endpoints.", remoteEPManifest.Count)
	remoteEPCount := remoteEPManifest.Count - (len(c.NodeUUIDs)-1)*localEPManifest.Count

	for i := 0; i < remoteEPCount; i++ {
		var epIP string
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}

		// Get the networks object
		nets, ok := c.Networks.Objects.([]*netproto.Network)
		if !ok {
			log.Errorf("Failed to cast the object %v to networks.", c.Networks.Objects)
			return fmt.Errorf("failed to cast the object %v to networks", c.Networks.Objects)
		}

		nsIdx := i % len(ns)
		netIdx := i % len(nets)
		namespace := ns[nsIdx]
		network := nets[netIdx]
		epName := fmt.Sprintf("%s-%d", localEPManifest.Name, i)
		// Pop elements here instead of maintaining a complex index
		epIP, c.SubnetIPLUT[network.Name] = c.SubnetIPLUT[network.Name][0], c.SubnetIPLUT[network.Name][1:]

		ep := netproto.Endpoint{
			TypeMeta: api.TypeMeta{
				Kind: "Endpoint",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      epName,
			},
			Spec: netproto.EndpointSpec{
				NetworkName: network.Name,
				MacAddress:  macAddrs[i],
				IPv4Address: epIP,
				NodeUUID:    defaultRemoteUUIDName,
			},
		}
		c.EpCache[defaultRemoteUUIDName] = append(c.EpCache[defaultRemoteUUIDName], epIP)
		endpoints = append(endpoints, &ep)
	}
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/endpoints/"
	cfg.Objects = endpoints
	c.Endpoints = cfg
	return nil
}
