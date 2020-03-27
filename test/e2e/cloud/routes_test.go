package cloud

import (
	"context"
	"encoding/json"
	"fmt"
	net2 "net"
	"strings"

	"github.com/deckarep/golang-set"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

type nodeDetails struct {
	nodeName string
	nodeIP   string
}

type bgpPeerSpec struct {
	LocalAddr string
	PeerAddr  string
	RemoteASN int
	HoldTime  int
	KeepAlive int
}

type bgpPeerStatus struct {
	Status string
}

type bgpPeer struct {
	Spec   bgpPeerSpec
	Status bgpPeerStatus
}

type ConfigCache struct {
	Tenants  map[string]*cluster.Tenant
	VPCs     map[string]*network.VirtualRouter
	Networks map[string]*network.Network
	IPAMPols map[string]*network.IPAMPolicy
	SecPols  map[string]*security.NetworkSecurityPolicy
}

func (c *ConfigCache) Init() {
	c.Tenants = make(map[string]*cluster.Tenant)
	c.VPCs = make(map[string]*network.VirtualRouter)
	c.Networks = make(map[string]*network.Network)
	c.IPAMPols = make(map[string]*network.IPAMPolicy)
	c.SecPols = make(map[string]*security.NetworkSecurityPolicy)
}

const (
	naplesVPCPath      = "/api/vrfs/"
	naplesNetworksPath = "/api/networks/"
	naplesIPAMPath     = "/api/ipam-policies/"
	naplesSecPolPath   = "/api/security/policies/"
)

func (c *ConfigCache) Create(ctx context.Context, client apiclient.Services, obj runtime.Object) error {
	kind := obj.GetObjectKind()
	switch kind {
	case string(cluster.KindTenant):
		t := obj.(*cluster.Tenant)
		_, err := client.ClusterV1().Tenant().Create(ctx, t)
		if err != nil {
			return err
		}
		c.Tenants[t.Name] = t
	case string(network.KindVirtualRouter):
		v := obj.(*network.VirtualRouter)
		_, err := client.NetworkV1().VirtualRouter().Create(ctx, v)
		if err != nil {
			return err
		}
		c.VPCs[v.Tenant+"."+v.Name] = v
	case string(network.KindNetwork):
		n := obj.(*network.Network)
		_, err := client.NetworkV1().Network().Create(ctx, n)
		if err != nil {
			return err
		}
		c.Networks[n.Tenant+"."+n.Name] = n
	case string(network.KindIPAMPolicy):
		i := obj.(*network.IPAMPolicy)
		_, err := client.NetworkV1().IPAMPolicy().Create(ctx, i)
		if err != nil {
			return err
		}
		c.IPAMPols[i.Tenant+"."+i.Name] = i
	case string(security.KindNetworkSecurityPolicy):
		s := obj.(*security.NetworkSecurityPolicy)
		_, err := client.SecurityV1().NetworkSecurityPolicy().Create(ctx, s)
		if err != nil {
			return err
		}
		c.SecPols[s.Tenant+"."+s.Name] = s
	default:
		return fmt.Errorf("unknown kind")
	}
	return nil
}

func (c *ConfigCache) Update(ctx context.Context, client apiclient.Services, obj runtime.Object) error {
	kind := obj.GetObjectKind()
	switch kind {
	case string(cluster.KindTenant):
		t := obj.(*cluster.Tenant)
		_, err := client.ClusterV1().Tenant().Update(ctx, t)
		if err != nil {
			return err
		}
		c.Tenants[t.Name] = t
	case string(network.KindVirtualRouter):
		v := obj.(*network.VirtualRouter)
		_, err := client.NetworkV1().VirtualRouter().Update(ctx, v)
		if err != nil {
			return err
		}
		c.VPCs[v.Tenant+"."+v.Name] = v
	case string(network.KindNetwork):
		n := obj.(*network.Network)
		_, err := client.NetworkV1().Network().Update(ctx, n)
		if err != nil {
			return err
		}
		c.Networks[n.Tenant+"."+n.Name] = n
	case string(network.KindIPAMPolicy):
		i := obj.(*network.IPAMPolicy)
		_, err := client.NetworkV1().IPAMPolicy().Update(ctx, i)
		if err != nil {
			return err
		}
		c.IPAMPols[i.Tenant+"."+i.Name] = i
	case string(security.KindNetworkSecurityPolicy):
		s := obj.(*security.NetworkSecurityPolicy)
		_, err := client.SecurityV1().NetworkSecurityPolicy().Update(ctx, s)
		if err != nil {
			return err
		}
		c.SecPols[s.Tenant+"."+s.Name] = s
	default:
		return fmt.Errorf("unknown kind")
	}
	return nil
}

func (c *ConfigCache) Delete(ctx context.Context, client apiclient.Services, obj runtime.Object) error {
	kind := obj.GetObjectKind()
	switch kind {
	case string(cluster.KindTenant):
		t := obj.(*cluster.Tenant)
		_, err := client.ClusterV1().Tenant().Delete(ctx, &t.ObjectMeta)
		if err != nil {
			return err
		}
		delete(c.Tenants, t.Name)
	case string(network.KindVirtualRouter):
		v := obj.(*network.VirtualRouter)
		_, err := client.NetworkV1().VirtualRouter().Delete(ctx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		delete(c.VPCs, v.Tenant+"."+v.Name)
	case string(network.KindNetwork):
		n := obj.(*network.Network)
		_, err := client.NetworkV1().Network().Delete(ctx, &n.ObjectMeta)
		if err != nil {
			return err
		}
		delete(c.Networks, n.Tenant+"."+n.Name)
	case string(network.KindIPAMPolicy):
		i := obj.(*network.IPAMPolicy)
		_, err := client.NetworkV1().IPAMPolicy().Delete(ctx, &i.ObjectMeta)
		if err != nil {
			return err
		}
		delete(c.IPAMPols, i.Tenant+"."+i.Name)
	case string(security.KindNetworkSecurityPolicy):
		s := obj.(*security.NetworkSecurityPolicy)
		_, err := client.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &s.ObjectMeta)
		if err != nil {
			return err
		}
		delete(c.SecPols, s.Tenant+"."+s.Name)
	default:
		return fmt.Errorf("unknown kind")
	}
	return nil
}

func (c *ConfigCache) Get(ctx context.Context, client apiclient.Services, kind string, obj *api.ObjectMeta) (runtime.Object, error) {
	switch kind {
	case string(cluster.KindTenant):
		return client.ClusterV1().Tenant().Get(ctx, obj)
	case string(network.KindVirtualRouter):
		return client.NetworkV1().VirtualRouter().Get(ctx, obj)
	case string(network.KindNetwork):
		return client.NetworkV1().Network().Get(ctx, obj)
	case string(network.KindIPAMPolicy):
		return client.NetworkV1().IPAMPolicy().Get(ctx, obj)
	case string(security.KindNetworkSecurityPolicy):
		return client.SecurityV1().NetworkSecurityPolicy().Get(ctx, obj)
	}
	return nil, fmt.Errorf("unknown kind")
}

func (c *ConfigCache) Verify(tenant, kind, naples string) error {
	By(fmt.Sprintf("verifying [%v] on [%v]", kind, naples))
	verifyRd := func(r1 *network.RDSpec, r2 *netproto.RDSpec) bool {
		if r1.AddressFamily != r2.AddressFamily {
			return false
		}
		if len(r1.ImportRTs) != len(r2.ImportRTs) || len(r1.ExportRTs) != len(r2.ExportRTs) {
			return false
		}
		if fmt.Sprintf("%v", r1) != fmt.Sprintf("%v", r2) {
			return false
		}
		return true
	}
	switch kind {
	case string(cluster.KindTenant):
		return fmt.Errorf("not supported")
	case string(network.KindVirtualRouter):
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s curl -sS http://127.0.0.1:9007%s", naples, naplesVPCPath))
		var vpcs, nvpcs []*netproto.Vrf
		err := json.Unmarshal([]byte(out), &nvpcs)
		if err != nil {
			return err
		}
		// filter on Tenant
		for _, v := range nvpcs {
			if v.Tenant == tenant {
				if v.Name == "default" {
					continue
				}
				vpcs = append(vpcs, v)
			}
		}
		// no filtering for now
		if len(vpcs) != len(c.VPCs) {
			return fmt.Errorf("expecting %d vpcs got %d", len(c.VPCs), len(vpcs))
		}
		for _, v := range vpcs {
			cv, ok := c.VPCs[v.Tenant+"."+v.Name]
			if !ok {
				return fmt.Errorf("found unexpected VPC [%s.%s]", v.Tenant, v.Name)
			}
			if cv.Spec.VxLanVNI != v.Spec.VxLANVNI || cv.Spec.DefaultIPAMPolicy != v.Spec.IPAMPolicy {
				return fmt.Errorf("did not match [%v/%v][%v]/[%v]", cv.Name, v.Name, cv.Spec, v.Spec)
			}
			if !verifyRd(cv.Spec.RouteImportExport, v.Spec.RouteImportExport) {
				return fmt.Errorf("RD does not match")
			}
		}
	case string(network.KindNetwork):
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s curl -sS http://127.0.0.1:9007%s", naples, naplesNetworksPath))
		var nnws, nws []*netproto.Network
		err := json.Unmarshal([]byte(out), &nnws)
		if err != nil {
			return err
		}
		// filtering on tenant
		for _, v := range nnws {
			if v.Tenant == tenant {
				nws = append(nws, v)
			}
		}
		if len(nws) != len(c.Networks) {
			return fmt.Errorf("expecting %d networks got %d", len(c.Networks), len(nws))
		}
		for _, v := range nws {
			cv, ok := c.Networks[v.Tenant+"."+v.Name]
			if !ok {
				return fmt.Errorf("found unexpected Network [%s.%s]", v.Tenant, v.Name)
			}
			// TODO validate all other fields.
			if cv.Spec.VirtualRouter != v.Spec.VrfName {
				return fmt.Errorf("spec did not match")
			}
			if !verifyRd(cv.Spec.RouteImportExport, v.Spec.RouteImportExport) {
				return fmt.Errorf("RD does not match")
			}
		}
	case string(network.KindIPAMPolicy):
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s curl -sS http://127.0.0.1:9007%s", naples, naplesIPAMPath))
		var nipams, ipams []*netproto.IPAMPolicy
		err := json.Unmarshal([]byte(out), &nipams)
		if err != nil {
			return err
		}
		// filter on tenant
		for _, v := range nipams {
			if v.Tenant == tenant {
				ipams = append(ipams, v)
			}
		}
		if len(ipams) != len(c.IPAMPols) {
			return fmt.Errorf("expecting %d IPAM Polcies got %d", len(c.IPAMPols), len(ipams))
		}
		for _, v := range ipams {
			cv, ok := c.IPAMPols[v.Tenant+"."+v.Name]
			if !ok {
				return fmt.Errorf("found unexpected Network [%s.%s]", v.Tenant, v.Name)
			}
			if len(cv.Spec.DHCPRelay.Servers) != len(v.Spec.DHCPRelay.Servers) {
				return fmt.Errorf("spec did not match")
			}
			if fmt.Sprintf("%v", cv.Spec.DHCPRelay) != fmt.Sprintf("%v", v.Spec.DHCPRelay) {
				return fmt.Errorf("spec did not match")
			}
		}
	case string(security.KindNetworkSecurityPolicy):
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s curl -sS http://127.0.0.1:9007%s", naples, naplesSecPolPath))
		var secPols, nsecPols []*security.NetworkSecurityPolicy
		err := json.Unmarshal([]byte(out), &nsecPols)
		if err != nil {
			return err
		}
		// filter on tenant
		for _, v := range nsecPols {
			if v.Tenant == tenant {
				secPols = append(secPols, v)
			}
		}
		if len(secPols) != len(c.SecPols) {
			return fmt.Errorf("expecting %d Security Polcies got %d", len(c.SecPols), len(secPols))
		}
		for _, v := range secPols {
			_, ok := c.SecPols[v.Tenant+"."+v.Name]
			if !ok {
				return fmt.Errorf("found unexpected SecurityPolicy [%s.%s]", v.Tenant, v.Name)
			}
			// TODO: validate rules
		}
	}
	return nil
}

var _ = Describe("Cloud E2E", func() {
	var restClient apiclient.Services

	getPegasusNodes := func() []nodeDetails {
		ret := []nodeDetails{}
		out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods -o wide --no-headers | grep pegasus"), "\n")
		for _, line := range out {
			fields := strings.Fields(line)
			ret = append(ret, nodeDetails{nodeName: fields[6], nodeIP: fields[5]})
		}
		return ret
	}

	getPeerStatus := func(node string, cmd string) []bgpPeer {
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s show bgp peers --json", node, cmd))
		ret := []bgpPeer{}
		err := json.Unmarshal([]byte(out), &ret)
		if err != nil {
			By(fmt.Sprintf("failed to parse BGP peer status [%v]", out))
			return []bgpPeer{}
		}
		return ret
	}

	Context("Routing configurationTests", func() {

		BeforeEach(func() {
			var err error
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restClient, err = apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			pegNodes := getPegasusNodes()
			Expect(len(pegNodes)).Should(Equal(2), "did not find 2 Pegasus nodes got [%v]", pegNodes)
		})

		It("Configure BGP", func() {
			lctx, cancel := context.WithCancel(ts.tu.MustGetLoggedInContext(context.Background()))
			opts := api.ListWatchOptions{
				FieldSelector: "spec.type=loopback-tep",
			}
			defer cancel()
			Expect(ts).NotTo(BeNil(), "Rest client is nil")

			var lbIfs []*network.NetworkInterface
			var err error
			Eventually(func() error {
				lbIfs, err = restClient.NetworkV1().NetworkInterface().List(lctx, &opts)
				Expect(err).Should(BeNil(), fmt.Sprintf("failed to list loopback interfaces %s)", err))
				if len(ts.tu.NaplesNodes) != len(lbIfs) {
					By(fmt.Sprintf("expecting %d Loopbacks got %d", len(ts.tu.NaplesNodes), len(lbIfs)))
					return fmt.Errorf("expecting %d Loopbacks got %d", len(ts.tu.NaplesNodes), len(lbIfs))
				}
				return nil
			}, 30, 1).Should(BeNil(), "did not find all loopback interfaces")

			dscs, err := restClient.ClusterV1().DistributedServiceCard().List(lctx, &api.ListWatchOptions{})
			Expect(err).Should(BeNil(), fmt.Sprintf("failed to list DSCs %s)", err))
			mactoIPMap := make(map[string]string)
			for _, dsc := range dscs {
				Expect(dsc.Status.IPConfig).ShouldNot(BeNil(), "IPConfig is nil")
				ip, _, err := net2.ParseCIDR(dsc.Status.IPConfig.IPAddress)
				Expect(err).Should(BeNil(), fmt.Sprintf("paprse IP failed [%v](%s)", dsc.Status.IPConfig.IPAddress, err))
				mactoIPMap[dsc.Status.PrimaryMAC] = ip.String()
			}
			for _, lb := range lbIfs {
				By(fmt.Sprintf("Got DSC [%+v]", lb))
				lb.Spec.IPConfig = &cluster.IPConfig{
					IPAddress: mactoIPMap[lb.Status.DSC] + "/32",
				}
				By(fmt.Sprintf("setting naples [%v] loopback IP to [%v]", lb.Name, lb.Spec.IPConfig.IPAddress))
				_, err = ts.restSvc.NetworkV1().NetworkInterface().Update(lctx, lb)
				Expect(err).Should(BeNil(), fmt.Sprintf("failed to update loopback interface IP (%s)", err))
			}

			By("Configure Routing Config for NAPLES")
			rtCfg := network.RoutingConfig{
				ObjectMeta: api.ObjectMeta{
					Name: "NaplesBGP",
				},
				Spec: network.RoutingConfigSpec{
					BGPConfig: &network.BGPConfig{
						DSCAutoConfig:     true,
						ASNumber:          65000,
						KeepaliveInterval: 30,
						Holdtime:          90,
						Neighbors: []*network.BGPNeighbor{
							{
								DSCAutoConfig:         true,
								RemoteAS:              65514,
								MultiHop:              10,
								EnableAddressFamilies: []string{network.BGPAddressFamily_IPv4Unicast.String()},
							},
							{
								DSCAutoConfig:         true,
								RemoteAS:              65000,
								MultiHop:              10,
								EnableAddressFamilies: []string{network.BGPAddressFamily_L2vpnEvpn.String()},
							},
						},
					},
				},
			}

			_, err = restClient.NetworkV1().RoutingConfig().Create(lctx, &rtCfg)
			if err != nil {
				Expect(strings.Contains(err.Error(), "409")).Should(BeTrue(), "got error while creating rouging config (%s)", err)
			}

			for _, nip := range ts.tu.VeniceNodeIPs {
				nodeName := ts.tu.IPToNameMap[nip]
				By(fmt.Sprintf("configuring Routing config for node [%v]", nodeName))
				nrtCfg := network.RoutingConfig{
					ObjectMeta: api.ObjectMeta{
						Name: fmt.Sprintf("%s-RR-BGP", nodeName),
					},
					Spec: network.RoutingConfigSpec{
						BGPConfig: &network.BGPConfig{
							RouterId:          nip,
							ASNumber:          65000,
							KeepaliveInterval: 30,
							Holdtime:          90,
							Neighbors: []*network.BGPNeighbor{
								{
									IPAddress:             "192.168.30.9",
									RemoteAS:              65500,
									MultiHop:              10,
									EnableAddressFamilies: []string{network.BGPAddressFamily_L2vpnEvpn.String()},
								},
							},
						},
					},
				}
				// Add neighbors to other nodes
				for _, noip := range ts.tu.VeniceNodeIPs {
					if nip == noip {
						continue
					}
					peer := &network.BGPNeighbor{
						IPAddress:             noip,
						RemoteAS:              65500,
						MultiHop:              10,
						EnableAddressFamilies: []string{network.BGPAddressFamily_L2vpnEvpn.String()},
					}
					nrtCfg.Spec.BGPConfig.Neighbors = append(nrtCfg.Spec.BGPConfig.Neighbors, peer)
				}
				_, err = restClient.NetworkV1().RoutingConfig().Create(lctx, &nrtCfg)
				if err != nil {
					Expect(strings.Contains(err.Error(), "409")).Should(BeTrue(), "got error while creating rouging config (%s)", err)
				}

				node, err := restClient.ClusterV1().Node().Get(lctx, &api.ObjectMeta{Name: nodeName})
				Expect(err).Should(BeNil(), "get node [%v] failed (%s)", nodeName, err)
				node.Spec.RoutingConfig = fmt.Sprintf("%s-RR-BGP", nodeName)

				_, err = restClient.ClusterV1().Node().Update(lctx, node)
				Expect(err).Should(BeNil(), "failed to update node [%v](%s)", nodeName, err)
			}

			By("validating BGP Peering Sessions")
			Eventually(func() error {
				pnodes := getPegasusNodes()
				pnodeSet := mapset.NewSet()
				for _, p := range pnodes {
					pnodeSet.Add(p.nodeIP)
				}

				for _, n := range ts.tu.NaplesNodes {
					estabSet := mapset.NewSet()
					peers := getPeerStatus(n, "/naples//nic/bin/pdsctl")
					for _, p := range peers {
						if p.Status.Status == "ESTABLISHED" {
							estabSet.Add(p.Spec.PeerAddr)
						}
					}
					if !pnodeSet.Equal(estabSet) {
						By(fmt.Sprintf("Peer Sessions did not match on naples [%v] got [%+v]", n, peers))
						return fmt.Errorf("did not match [%v] got [%v]", pnodeSet.String(), estabSet.String())
					}
				}
				return nil
			}, 180, 10).Should(BeNil(), "BGP Peer sessions did not match on naples")
		})
	})

	Context("Overlay Obejct tests", func() {
		var restClient apiclient.Services
		tenantName := "cloude2e"

		BeforeEach(func() {
			var err error
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restClient, err = apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			pegNodes := getPegasusNodes()
			Expect(len(pegNodes)).Should(Equal(2), "did not find 2 Pegasus nodes got [%v]", pegNodes)
		})

		cleanUpObjects := func(ctx context.Context) {
			subnList, err := restClient.NetworkV1().Network().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenantName}})
			Expect(err).To(BeNil(), "listing network failed (%s)", err)

			for _, n := range subnList {
				if n.Tenant == tenantName {
					_, err := restClient.NetworkV1().Network().Delete(ctx, &n.ObjectMeta)
					Expect(err).To(BeNil(), "failed to delete network [%v](%s)", n.Name, err)
				}
			}

			nspList, err := restClient.SecurityV1().NetworkSecurityPolicy().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenantName}})
			Expect(err).To(BeNil(), "listing security policies failed (%s)", err)

			for _, n := range nspList {
				if n.Tenant == tenantName {
					_, err := restClient.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &n.ObjectMeta)
					Expect(err).To(BeNil(), "failed to delete security policy [%v](%s)", n.Name, err)
				}
			}

			vpcList, err := restClient.NetworkV1().VirtualRouter().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenantName}})
			Expect(err).To(BeNil(), "error getting list (%s)", err)
			for _, v := range vpcList {
				if v.Tenant == tenantName {
					_, err := restClient.NetworkV1().VirtualRouter().Delete(ctx, &v.ObjectMeta)
					Expect(err).To(BeNil(), "failed to delete vpc [%v](%s)", v.Name, err)
				}
			}

			_, err = restClient.ClusterV1().Tenant().Get(ctx, &api.ObjectMeta{Name: tenantName})
			if err == nil {
				_, err = restClient.ClusterV1().Tenant().Delete(ctx, &api.ObjectMeta{Name: tenantName})
				Expect(err).To(BeNil(), "failed to delete tenant[%v](%s)", tenantName, err)
			}
		}

		It("CRUD tests", func() {
			lctx, cancel := context.WithCancel(ts.tu.MustGetLoggedInContext(context.Background()))
			defer cancel()

			cleanUpObjects(lctx)
			cache := ConfigCache{}
			cache.Init()

			By(fmt.Sprintf("Creating Tenant"))
			// Create Tenant
			err := cache.Create(lctx, restClient, &cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: "Tenant"},
				ObjectMeta: api.ObjectMeta{
					Name: tenantName,
				},
			})

			Expect(err).To(BeNil(), "Expecting tenant create to succeed")

			// Create VPCs
			vpcs := []*network.VirtualRouter{
				{
					TypeMeta: api.TypeMeta{Kind: "VirtualRouter"},
					ObjectMeta: api.ObjectMeta{
						Name:   "e2eVpc1",
						Tenant: tenantName,
					},
					Spec: network.VirtualRouterSpec{
						Type:             network.VirtualRouterSpec_Tenant.String(),
						RouterMACAddress: "0000.1111.0011",
						VxLanVNI:         90001,
						RouteImportExport: &network.RDSpec{
							AddressFamily: network.BGPAddressFamily_L2vpnEvpn.String(),
							ExportRTs: []*network.RouteDistinguisher{
								{
									Type:          network.RouteDistinguisher_Type2.String(),
									AdminValue:    1000,
									AssignedValue: 1000,
								},
							},
							ImportRTs: []*network.RouteDistinguisher{
								{
									Type:          network.RouteDistinguisher_Type2.String(),
									AdminValue:    1000,
									AssignedValue: 1000,
								},
							},
						},
					},
				},
				{
					TypeMeta: api.TypeMeta{Kind: "VirtualRouter"},
					ObjectMeta: api.ObjectMeta{
						Name:   "e2eVpc2",
						Tenant: tenantName,
					},
					Spec: network.VirtualRouterSpec{
						Type:             network.VirtualRouterSpec_Tenant.String(),
						RouterMACAddress: "0000.1111.0022",
						VxLanVNI:         90002,
						RouteImportExport: &network.RDSpec{
							AddressFamily: network.BGPAddressFamily_L2vpnEvpn.String(),
							ExportRTs: []*network.RouteDistinguisher{
								{
									Type:          network.RouteDistinguisher_Type2.String(),
									AdminValue:    2000,
									AssignedValue: 2000,
								},
							},
							ImportRTs: []*network.RouteDistinguisher{
								{
									Type:          network.RouteDistinguisher_Type2.String(),
									AdminValue:    2000,
									AssignedValue: 2000,
								},
							},
						},
					},
				},
			}

			for i, v := range vpcs {
				By(fmt.Sprintf("Creating VPC %s", v.Name))
				err = cache.Create(lctx, restClient, v)
				Expect(err).To(BeNil(), "VPC create failed (%s)", err)

				// Create Subnets
				for j := 0; i < 3; i++ {
					subnetName := fmt.Sprintf("%s.subnet%d", v.Name, i)
					subn := &network.Network{
						TypeMeta: api.TypeMeta{Kind: "Network"},
						ObjectMeta: api.ObjectMeta{
							Name:   subnetName,
							Tenant: v.Tenant,
						},
						Spec: network.NetworkSpec{
							Type:          network.NetworkType_Routed.String(),
							VirtualRouter: v.Name,
							IPv4Subnet:    fmt.Sprintf("10.%d.%d.1/24", i, j),
							IPv4Gateway:   fmt.Sprintf("10.%d.%d.1", i, j),
							VxlanVNI:      uint32(10000*i + j),
							RouteImportExport: &network.RDSpec{
								AddressFamily: network.BGPAddressFamily_L2vpnEvpn.String(),
								ExportRTs: []*network.RouteDistinguisher{
									{
										Type:          network.RouteDistinguisher_Type2.String(),
										AdminValue:    uint32(10000*i + j),
										AssignedValue: uint32(1000 + j),
									},
								},
								ImportRTs: []*network.RouteDistinguisher{
									{
										Type:          network.RouteDistinguisher_Type2.String(),
										AdminValue:    uint32(10000*i + j),
										AssignedValue: uint32(1000 + j),
									},
								},
							},
						},
					}
					sging1 := &security.NetworkSecurityPolicy{
						TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
						ObjectMeta: api.ObjectMeta{
							Name:   subn.Name + ".ing1",
							Tenant: v.Tenant,
						},
						Spec: security.NetworkSecurityPolicySpec{
							AttachTenant: true,
							Rules: []security.SGRule{
								{
									ProtoPorts:      []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{"10.1.1.0/24"},
									ToIPAddresses:   []string{"10.1.2.0/24"},
									Action:          security.SGRule_PERMIT.String(),
								},
							},
						},
					}
					sging2 := &security.NetworkSecurityPolicy{
						TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
						ObjectMeta: api.ObjectMeta{
							Name:   subn.Name + ".ing2",
							Tenant: v.Tenant,
						},
						Spec: security.NetworkSecurityPolicySpec{
							AttachTenant: true,
							Rules: []security.SGRule{
								{
									ProtoPorts:      []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{"10.2.1.0/24"},
									ToIPAddresses:   []string{"10.2.2.0/24"},
									Action:          security.SGRule_PERMIT.String(),
								},
							},
						},
					}
					sgeg1 := &security.NetworkSecurityPolicy{
						TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
						ObjectMeta: api.ObjectMeta{
							Name:   subn.Name + ".eg1",
							Tenant: v.Tenant,
						},
						Spec: security.NetworkSecurityPolicySpec{
							AttachTenant: true,
							Rules: []security.SGRule{
								{
									ProtoPorts:      []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{"10.3.1.0/24"},
									ToIPAddresses:   []string{"10.3.2.0/24"},
									Action:          security.SGRule_PERMIT.String(),
								},
							},
						},
					}
					sgeg2 := &security.NetworkSecurityPolicy{
						TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
						ObjectMeta: api.ObjectMeta{
							Name:   subn.Name + ".eg2",
							Tenant: v.Tenant,
						},
						Spec: security.NetworkSecurityPolicySpec{
							AttachTenant: true,
							Rules: []security.SGRule{
								{
									ProtoPorts:      []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{"10.4.1.0/24"},
									ToIPAddresses:   []string{"10.4.2.0/24"},
									Action:          security.SGRule_PERMIT.String(),
								},
							},
						},
					}
					err = cache.Create(lctx, restClient, sging1)
					Expect(err).To(BeNil(), "NSP ing1 create did not succeed (%s)", err)
					err = cache.Create(lctx, restClient, sging2)
					Expect(err).To(BeNil(), "NSP ing2 create did not succeed (%s)", err)
					err = cache.Create(lctx, restClient, sgeg1)
					Expect(err).To(BeNil(), "NSP eg1 create did not succeed (%s)", err)
					err = cache.Create(lctx, restClient, sgeg2)
					Expect(err).To(BeNil(), "NSP egw create did not succeed (%s)", err)

					subn.Spec.IngressSecurityPolicy = []string{sging1.Name, sging2.Name}
					subn.Spec.EgressSecurityPolicy = []string{sgeg1.Name, sgeg2.Name}
					By(fmt.Sprintf("Creating network %s", subn.Name))
					err = cache.Create(lctx, restClient, subn)
					Expect(err).To(BeNil(), "subnet create [%v] failed 9%s)", subn.Name, err)

				}
			}
			By(fmt.Sprintf("Verify start on [%v]", ts.tu.NaplesNodes))
			Eventually(func() error {
				for _, n := range ts.tu.NaplesNodes {
					err := cache.Verify(tenantName, string(network.KindVirtualRouter), n)
					if err != nil {
						return err
					}
					err = cache.Verify(tenantName, string(network.KindNetwork), n)
					if err != nil {
						return err
					}
					err = cache.Verify(tenantName, string(security.KindNetworkSecurityPolicy), n)
					if err != nil {
						return err
					}
				}
				By(fmt.Sprintf("veriyfy okay on %d nodes", len(ts.tu.NaplesNodes)))
				return nil
			}, 60, 10).Should(BeNil(), "Failed to validate on naples (%s)", err)

			// Cleanup all objects
			By(fmt.Sprintf("cleaning up created objectd"))
			cleanUpObjects(lctx)
			cache.Init()
		})

	})
})
