package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
)

func (c *CfgGen) GenerateEndpoints() error {
	var cfg pkg.IOTAConfig
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
		log.Debugf("Endpoint Manifest missing. Local: %v, Remote: %v", localEPManifest, remoteEPManifest)
		log.Info("Skipping Endpoints Generation")
		return nil
	}

	log.Infof("Generating %v Local Endpoints.", localEPManifest.Count)

	// Generate MAC Addresses
	macAddrs, err := libs.GenMACAddresses(localEPManifest.Count + remoteEPManifest.Count)
	if err != nil {
		log.Errorf("Could not generate %v MAC Addresses. Err: %v", localEPManifest.Count+remoteEPManifest.Count, err)
	}

	for i := 0; i < localEPManifest.Count; i++ {
		var curNetworks []*netproto.Network
		var epIP string
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		nsIdx := i % len(ns)
		namespace := ns[nsIdx]

		// Get the networks object
		nets, ok := c.Networks.Objects.([]*netproto.Network)
		if !ok {
			log.Errorf("Failed to cast the object %v to networks.", c.Networks.Objects)
			return fmt.Errorf("failed to cast the object %v to networks", c.Networks.Objects)
		}

		for _, n := range nets {
			if n.Namespace == namespace.Name {
				curNetworks = append(curNetworks, n)
			}
		}

		netIdx := i % len(curNetworks)

		network := curNetworks[netIdx]
		// Generate local ep for every naples.
		for idx, nodeUUID := range c.NodeUUIDs {
			nodeUUID := nodeUUID
			var epMac string
			// Pop elements here instead of maintaining a complex index
			epIP, c.SubnetIPLUT[network.Name] = c.SubnetIPLUT[network.Name][0], c.SubnetIPLUT[network.Name][1:]
			epIP = fmt.Sprintf("%s/32", epIP)

			epMac, macAddrs = macAddrs[0], macAddrs[1:]
			epName := fmt.Sprintf("%s-%d-node-%d", localEPManifest.Name, i, idx)

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
					MacAddress:  epMac,
					IPv4Addresses: []string{epIP},
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
		var epIP, epMac string
		var curNetworks []*netproto.Network
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
		namespace := ns[nsIdx]
		for _, n := range nets {
			if n.Namespace == namespace.Name {
				curNetworks = append(curNetworks, n)
			}
		}
		netIdx := i % len(curNetworks)
		network := curNetworks[netIdx]
		epName := fmt.Sprintf("%s-%d", remoteEPManifest.Name, i)
		// Pop elements here instead of maintaining a complex index
		epIP, c.SubnetIPLUT[network.Name] = c.SubnetIPLUT[network.Name][0], c.SubnetIPLUT[network.Name][1:]
		epIP = fmt.Sprintf("%s/32", epIP)

		epMac, macAddrs = macAddrs[0], macAddrs[1:]

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
				MacAddress:  epMac,
				IPv4Addresses: []string{epIP},
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

//GetRemoteEndpointsIPs get remote IPs
func (c *CfgGen) GetRemoteEndpointsIPs() []string {
	eps, ok := c.Endpoints.Objects.([]*netproto.Endpoint)
	if !ok {
		log.Panicf("Failed to cast the object %v to endpoints.", c.Endpoints.Objects)

	}
	remoteEPs := []string{}
	for _, ep := range eps {
		if ep.Spec.NodeUUID == defaultRemoteUUIDName {
			for _, address := range ep.Spec.IPv4Addresses {
				remoteEPs = append(remoteEPs, address)
			}
		}
	}

	return remoteEPs
}
