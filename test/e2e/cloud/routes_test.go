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
	"github.com/pensando/sw/venice/globals"
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

var _ = Describe("Cloud E2E", func() {

	Context("Routing configurationTests", func() {
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
			By(fmt.Sprintf("Get Peer status from [%s][%s]", node, cmd))
			out := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s show bgp peers --json", node, cmd))
			ret := []bgpPeer{}
			err := json.Unmarshal([]byte(out), &ret)
			Expect(err).Should(BeNil(), "failed to unmarshal json data (%s)", err)
			return ret
		}

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
			dscs, err := restClient.ClusterV1().DistributedServiceCard().List(lctx, &api.ListWatchOptions{})
			Expect(err).Should(BeNil(), fmt.Sprintf("failed to list DSCs %s)", err))
			mactoIPMap := make(map[string]string)
			for _, dsc := range dscs {
				Expect(dsc.Status.IPConfig).ShouldNot(BeNil(), "IPConfig is nil")
				ip, _, err := net2.ParseCIDR(dsc.Status.IPConfig.IPAddress)
				Expect(err).Should(BeNil(), fmt.Sprintf("aprse IP failed [%v](%s)", dsc.Status.IPConfig.IPAddress, err))
				mactoIPMap[dsc.Status.PrimaryMAC] = ip.String()
			}

			var lbIfs []*network.NetworkInterface
			Eventually(func() error {
				lbIfs, err = restClient.NetworkV1().NetworkInterface().List(lctx, &opts)
				Expect(err).Should(BeNil(), fmt.Sprintf("failed to list loopback interfaces %s)", err))
				if len(ts.tu.NaplesNodes) != len(lbIfs) {
					By(fmt.Sprintf("expecting %d Loopbacks got %d", len(ts.tu.NaplesNodes), len(lbIfs)))
					return fmt.Errorf("expecting %d Loopbacks got %d", len(ts.tu.NaplesNodes), len(lbIfs))
				}
				return nil
			}, 30, 1).Should(BeNil(), "did not find all loopback interfaces")

			for _, lb := range lbIfs {
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
						RouterId:          "0.0.0.0",
						ASNumber:          65000,
						KeepaliveInterval: 30,
						Holdtime:          90,
						Neighbors: []*network.BGPNeighbor{
							{
								IPAddress:             "0.0.0.0",
								RemoteAS:              65514,
								MultiHop:              10,
								EnableAddressFamilies: []string{network.BGPAddressFamily_IPv4Unicast.String()},
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
					peers := getPeerStatus(n, "/nic/bin/pdsctl")
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
			}, 30, 1).Should(BeNil(), "BGP Peer sessions did not match on naples")
		})
	})
})
