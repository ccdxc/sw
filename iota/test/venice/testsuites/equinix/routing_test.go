package equinix_test

import (
	"encoding/json"
	"fmt"
	"reflect"
	"sort"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Routing Config Tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
	})

	Context("Routing tests", func() {

		It("Update Underlay ASN", func() {
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			testASN := uint32(65001)
			remoteASN := uint32(0) // to save original ASN
			match := false

			for _, v := range rcc.RoutingObjs {
				if match == false {
					for _, n := range v.RoutingObj.Spec.BGPConfig.Neighbors {
						out := strings.Join(n.EnableAddressFamilies, " ")
						if strings.Contains(string(out), "ipv4-unicast") {
							log.Debugf("RoutingConfig %v has ipv4-unicast AF", v.RoutingObj.Name)
							match = true
							remoteASN = n.RemoteAS
							n.RemoteAS = testASN
							log.Infof("RoutingConfig %v Underlay RemoteAS is updated from %v to %v", v.RoutingObj.Name, remoteASN, n.RemoteAS)
							break
						}
					}
				}
			}
			if match {
				// update routing config
				Expect(rcc.Commit()).Should(Succeed())
			} else {
				// no underlay routing config
				return
			}

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)

			// fetch routing config from Naples
			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				cmd := "curl localhost:9007/api/routingconfigs/"
				cmdOut, err := ts.model.RunNaplesCommand(nc, cmd)
				Expect(err).ShouldNot(HaveOccurred())

				RoutingData := []netproto.RoutingConfig{}
				err = json.Unmarshal([]byte(cmdOut[0]), &RoutingData)
				Expect(err).ShouldNot(HaveOccurred())

				for _, v := range RoutingData {
					for _, n := range v.Spec.BGPConfig.Neighbors {
						out := strings.Join(n.EnableAddressFamilies, " ")
						if strings.Contains(string(out), "ipv4-unicast") {
							Expect(n.RemoteAS == testASN).Should(BeTrue())
						}
					}
				}
				return nil
			})

			// fetch routing config from Fake Naples
			ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
				cmd := "curl localhost:9007/api/routingconfigs/"
				cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc, cmd)
				Expect(err).ShouldNot(HaveOccurred())
				cmdResp, _ := cmdOut.([]*iota.Command)
				for _, cmdLine := range cmdResp {
					RoutingData := []netproto.RoutingConfig{}
					err = json.Unmarshal([]byte(cmdLine.Stdout), &RoutingData)
					Expect(err).ShouldNot(HaveOccurred())

					for _, v := range RoutingData {
						for _, n := range v.Spec.BGPConfig.Neighbors {
							out := strings.Join(n.EnableAddressFamilies, " ")
							if strings.Contains(string(out), "ipv4-unicast") {
								Expect(n.RemoteAS == testASN).Should(BeTrue())
							}
						}
					}
				}
				return nil
			})

			// Change to original ASN
			for _, v := range rcc.RoutingObjs {
				for _, n := range v.RoutingObj.Spec.BGPConfig.Neighbors {
					out := strings.Join(n.EnableAddressFamilies, " ")
					if strings.Contains(string(out), "ipv4-unicast") {
						n.RemoteAS = remoteASN
						log.Infof("RoutingConfig %v Underlay RemoteAS is reset from %v to %v", v.RoutingObj.Name, testASN, n.RemoteAS)
						break
					}
				}
			}
			Expect(rcc.Commit()).Should(Succeed())
		})

		It("Change Timer config & verify", func() {
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			orig, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			keepAlive := uint32(70)
			holdTime := uint32(210)
			for _, v := range rcc.RoutingObjs {
				v.RoutingObj.Spec.BGPConfig.KeepaliveInterval = keepAlive
				v.RoutingObj.Spec.BGPConfig.Holdtime = holdTime
			}

			Expect(rcc.Commit()).Should(Succeed())

			Expect(verifyTimerInVenice(keepAlive, holdTime)).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//revert to original
			Expect(orig.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Add, delete ECX Peer in RR", func() {
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			orig, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			for _, v := range rcc.RoutingObjs {
				if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig == false {
					nbrs := v.RoutingObj.Spec.BGPConfig.GetNeighbors()
					log.Infof("1. %v", nbrs)
					newNbr := &network.BGPNeighbor{
						Shutdown:              false,
						IPAddress:             "22.1.1.50",
						RemoteAS:              100,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
						Password:              "",
						DSCAutoConfig:         false,
					}
					nbrs = append(nbrs, newNbr)
					v.RoutingObj.Spec.BGPConfig.Neighbors = nbrs
					break
				}
			}

			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Delete added ECX peer
			for _, v := range rcc.RoutingObjs {
				if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig == false {
					nbrs := v.RoutingObj.Spec.BGPConfig.GetNeighbors()
					v.RoutingObj.Spec.BGPConfig.Neighbors = nbrs[:len(nbrs)-1]
					log.Infof("%+v", v)
					break
				}
			}
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//revert to original
			Expect(orig.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Add, delete DSCAutoConfig for Naples in RR", func() {
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			for _, v := range rcc.RoutingObjs {
				if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig == false {
					nbrs := v.RoutingObj.Spec.BGPConfig.GetNeighbors()
					newNbr := &network.BGPNeighbor{
						Shutdown:              false,
						IPAddress:             "",
						RemoteAS:              100,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
						Password:              "",
						DSCAutoConfig:         true,
					}
					nbrs = append(nbrs, newNbr)
					v.RoutingObj.Spec.BGPConfig.Neighbors = nbrs
					break
				}
			}

			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			log.Infof("Deleting Naples peering template")
			//Delete added Naples template
			for _, v := range rcc.RoutingObjs {
				if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig == false {
					nbrs := v.RoutingObj.Spec.BGPConfig.GetNeighbors()
					v.RoutingObj.Spec.BGPConfig.Neighbors = nbrs[:len(nbrs)-1]
					log.Infof("%+v", v)
					break
				}
			}
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Change overlay password", func() {
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			orig, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			newPassword := "N0isystem$"
			//First change password in RR; verify state is NOT ESTABLISHED (CONNECT or IDLE)
			rcc.SetPasswordOnRR(newPassword)
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_NOT_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Now change password on Naples & verify status is established
			rcc.SetPasswordOnNaples(newPassword)
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//reset passwords
			rcc.SetPasswordOnRR("")
			rcc.SetPasswordOnNaples("")
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Revert to original state (i.e. w/o the DSC autoconfig block in RR)
			log.Infof("Reverting to original config on Naples & RR")
			Expect(orig.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Change overlay password w/ implicit reset", func() {
			/*
				This case is same as above except when we revert to original config,
				we remove DSC config template from RR w/o  explicitly resetting password.
			*/
			// get all existing routing config
			rcc, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			orig, err := ts.model.ListRoutingConfig()
			Expect(err).ShouldNot(HaveOccurred())

			newPassword := "N0isystem$"
			//First change password in RR; verify state is NOT ESTABLISHED (CONNECT or IDLE)
			rcc.SetPasswordOnRR(newPassword)
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_NOT_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Now change password on Naples & verify status is established
			rcc.SetPasswordOnNaples(newPassword)
			Expect(rcc.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Revert to original state (i.e. w/o the DSC autoconfig block in RR)
			//Default config doesn't have Naples config template
			log.Infof("Reverting original config on Naples & RR")
			Expect(orig.Commit()).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Disassociate RR from cluster", func() {
			cfgClient := ts.model.ConfigClient()
			nodes, err := cfgClient.ListClusterNodes()
			Expect(err).ShouldNot(HaveOccurred())

			//To restore nodes
			savedNodes, err := cfgClient.ListClusterNodes()
			Expect(err).ShouldNot(HaveOccurred())

			//Remove RR routing config from cluster nodes
			for _, n := range nodes {
				n.Spec.RoutingConfig = ""
				err = cfgClient.UpdateClusterNode(n)
				Expect(err).ShouldNot(HaveOccurred())
			}
			//overlay should not be formed
			verifyNaplesBgpState(PEER_NOT_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//revert to original routing config association
			for _, n := range savedNodes {
				err = cfgClient.UpdateClusterNode(n)
				Expect(err).ShouldNot(HaveOccurred())
			}

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("RR: Change associated routing cfg", func() {
			cfgClient := ts.model.ConfigClient()

			//copy one node's routing config,  alter it, add it
			//change it in node association & verify
			vnodes := ts.model.VeniceNodes()
			pnodes, err := vnodes.GetVeniceNodeWithService("pen-pegasus")

			//Pick any one RR node
			pnodes = pnodes.Any(1)
			cnode := pnodes.Nodes[0].ClusterNode
			savedRtgCfgName := cnode.Spec.RoutingConfig
			rcfg, err := cfgClient.GetRoutingConfig(cnode.Spec.RoutingConfig)
			Expect(err).ShouldNot(HaveOccurred())

			newRtgCfg := &network.RoutingConfig{}
			_, err = rcfg.Clone(newRtgCfg)
			Expect(err).ShouldNot(HaveOccurred())

			newRtgCfgName := "testRRNodeCfg"

			newRtgCfg.Name = newRtgCfgName

			for i, nbr := range newRtgCfg.Spec.BGPConfig.Neighbors {
				nbr.IPAddress = fmt.Sprintf("33.1.1.%v", i+1)
			}

			Expect(cfgClient.CreateRoutingConfig(newRtgCfg)).Should(Succeed())

			log.Infof("Changing cluster node %s rtg config from %s to %s", cnode.GetName(),
				savedRtgCfgName, newRtgCfgName)

			//Associate newly created cfg to node
			cnode.Spec.RoutingConfig = newRtgCfgName
			err = cfgClient.UpdateClusterNode(cnode)
			Expect(err).ShouldNot(HaveOccurred())

			//Verify overlay is formed
			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Restore original state
			cnode.Spec.RoutingConfig = savedRtgCfgName
			err = cfgClient.UpdateClusterNode(cnode)
			Expect(err).ShouldNot(HaveOccurred())

			//Delete new added routing config
			Expect(cfgClient.DeleteRoutingConfig(newRtgCfg)).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Naples: Disassociate routing config", func() {
			cfgClient := ts.model.ConfigClient()
			dscs, err := cfgClient.ListSmartNIC()
			Expect(err).ShouldNot(HaveOccurred())

			//To restore nodes
			savedDSCs, err := cfgClient.ListSmartNIC()
			Expect(err).ShouldNot(HaveOccurred())

			//Disassociate routing config from DSCs
			for _, d := range dscs {
				d.Spec.RoutingConfig = ""
				err = cfgClient.UpdateSmartNIC(d)
				Expect(err).ShouldNot(HaveOccurred())
			}
			//overlay should not be formed
			verifyNaplesBgpState(PEER_NOT_ESTABLISHED, PEER_NOT_ESTABLISHED)
			verifyRRState()

			//revert to original routing config association
			for _, d := range savedDSCs {
				err = cfgClient.UpdateSmartNIC(d)
				Expect(err).ShouldNot(HaveOccurred())
			}

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})

		It("Naples: Change routing config association", func() {
			cfgClient := ts.model.ConfigClient()

			var dsc *cluster.DistributedServiceCard

			if ts.tb.HasNaplesSim() {
				naples := ts.model.Naples().AnyFakeNodes(1)
				Expect(naples.Refresh()).Should(Succeed())
				dsc = naples.FakeNodes[0].Instances[0].Dsc
			} else {
				naples := ts.model.Naples().Any(1)
				Expect(naples.Refresh()).Should(Succeed())
				dsc = naples.Nodes[0].Instances[0].Dsc
			}

			savedRtgCfgName := dsc.Spec.RoutingConfig

			log.Infof("DSC %s routing config %s", dsc.GetName(), savedRtgCfgName)
			rcfg, err := cfgClient.GetRoutingConfig(dsc.Spec.RoutingConfig)
			Expect(err).ShouldNot(HaveOccurred())

			newRtgCfg := &network.RoutingConfig{}
			_, err = rcfg.Clone(newRtgCfg)
			Expect(err).ShouldNot(HaveOccurred())

			newRtgCfgName := "testNaplesNodeCfg"

			newRtgCfg.Name = newRtgCfgName

			newRtgCfg.Spec.BGPConfig.Holdtime = uint32(210)
			newRtgCfg.Spec.BGPConfig.KeepaliveInterval = uint32(70)

			Expect(cfgClient.CreateRoutingConfig(newRtgCfg)).Should(Succeed())

			log.Infof("Changing DSC node %s rtg config from %s to %s", dsc.GetName(),
				savedRtgCfgName, newRtgCfgName)

			//Associate newly created cfg to node
			dsc.Spec.RoutingConfig = newRtgCfgName
			err = cfgClient.UpdateSmartNIC(dsc)
			Expect(err).ShouldNot(HaveOccurred())

			//Verify overlay is formed
			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()

			//Restore original state
			dsc.Spec.RoutingConfig = savedRtgCfgName
			err = cfgClient.UpdateSmartNIC(dsc)
			Expect(err).ShouldNot(HaveOccurred())

			//Delete new added routing config
			Expect(cfgClient.DeleteRoutingConfig(newRtgCfg)).Should(Succeed())

			verifyNaplesBgpState(PEER_ESTABLISHED, PEER_ESTABLISHED)
			verifyRRState()
		})
	})
})

func verifyTimerInVenice(ka uint32, ht uint32) error {
	//Get updated venice info
	updc, err := ts.model.ListRoutingConfig()
	if err != nil {
		return err
	}
	for _, obj := range updc.RoutingObjs {
		if obj.RoutingObj.Spec.BGPConfig.GetKeepaliveInterval() != ka {
			return fmt.Errorf("KeepAlive Interval verification failed")
		}
		if obj.RoutingObj.Spec.BGPConfig.GetHoldtime() != ht {
			return fmt.Errorf("HoldTime verification failed")
		}
	}
	return nil
}

//Refer utils.ShadowBgpSpec & pds.BGPSpec
type PdsBgpSpec struct {
	LocalASN uint32 `json:"LocalASN"`
	RouterId string `json:"RouterId"`
}

//Refer utils.ShadowBGPPeerAFSpec & pds.BGPPeerAfSpec
type PdsBGPPeerAFSpec struct {
	Spec struct {
		PeerAddr string `json:"PeerAddr"`
		Afi      string `json:"Afi"`
		Safi     string `json:"Safi"`
	} `json:"Spec"`
}

//Refer utils.ShadowBGPPeerSpec & pds.BGPPeerSpec
type PdsBGPPeerSpec struct {
	PeerAddr  string `json:"PeerAddr"`
	HoldTime  uint32 `json:"HoldTime"`
	KeepAlive uint32 `json:"KeepAlive"`
}

//Refer utils.ShadowBGPPeerStatus & pds.BGPPeerStatus
type PdsBGPPeerStatus struct {
	Status string `json:"Status"`
}

//Refer utils.ShadowBGPPeer
type PdsBGPPeer struct {
	Spec   PdsBGPPeerSpec   `json:"Spec"`
	Status PdsBGPPeerStatus `json:"Status"`
}

type pdsBGPNeighbor struct {
	Spec struct {
		IPAddress             string
		RemoteAS              uint32
		Keepalive             uint32
		Holdtime              uint32
		MultiHop              uint32
		EnableAddressFamilies []string
	}
	Status struct {
		Status string
	}
}
type pdsConfigCmp struct {
	RouterId      string
	LocalASNumber uint32
	Neighbors     []*pdsBGPNeighbor
}

func (pdsNbr *pdsBGPNeighbor) populate(r *objects.RoutingConfig,
	nbr *network.BGPNeighbor, ip string, state string) {
	pdsNbr.Spec.IPAddress = ip
	pdsNbr.Spec.Holdtime = r.RoutingObj.Spec.BGPConfig.GetHoldtime()
	pdsNbr.Spec.Keepalive = r.RoutingObj.Spec.BGPConfig.GetKeepaliveInterval()
	pdsNbr.Status.Status = state
}

func (pdsNbr *pdsBGPNeighbor) populateRR(r *objects.RoutingConfig,
	nbr *network.BGPNeighbor, ip string) {
	pdsNbr.Spec.IPAddress = ip
	pdsNbr.Spec.Holdtime = r.RoutingObj.Spec.BGPConfig.GetHoldtime()
	pdsNbr.Spec.Keepalive = r.RoutingObj.Spec.BGPConfig.GetKeepaliveInterval()
}

func convertPDSConfig(peersAf []*PdsBGPPeerAFSpec, peers []*PdsBGPPeer,
	bgpSpec *PdsBgpSpec, getStatus bool) *pdsConfigCmp {

	var cfg pdsConfigCmp

	cfg.LocalASNumber = bgpSpec.LocalASN
	cfg.RouterId = bgpSpec.RouterId

	for _, peer := range peers {
		var pdsNbr pdsBGPNeighbor
		pdsNbr.Spec.IPAddress = peer.Spec.PeerAddr
		pdsNbr.Spec.Keepalive = peer.Spec.KeepAlive
		pdsNbr.Spec.Holdtime = peer.Spec.HoldTime
		if getStatus {
			if peer.Status.Status != PEER_ESTABLISHED {
				pdsNbr.Status.Status = PEER_NOT_ESTABLISHED
			} else {
				pdsNbr.Status.Status = peer.Status.Status
			}
		}
		cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
	}

	//sort neighbors list according to peer ip
	sort.Slice(cfg.Neighbors, func(i, j int) bool {
		return cfg.Neighbors[i].Spec.IPAddress < cfg.Neighbors[j].Spec.IPAddress
	})
	return &cfg
}

func getNaplesState(node *objects.Naples, pdsctlPath string, isHWNode bool) *pdsConfigCmp {

	peersaf_fn := func(cmd string) ([]*PdsBGPPeerAFSpec, error) {
		naples := []*objects.Naples{node}
		var cmdOut []string
		if isHWNode {
			cmdOut, _ = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples}, cmd)
		} else {
			cmdOut, _ = ts.model.RunFakeNaplesCommand(&objects.NaplesCollection{FakeNodes: naples}, cmd)
		}
		peersAf := []*PdsBGPPeerAFSpec{}
		err := json.Unmarshal([]byte(cmdOut[0]), &peersAf)
		return peersAf, err
	}

	peers_fn := func(cmd string) ([]*PdsBGPPeer, error) {
		naples := []*objects.Naples{node}
		var cmdOut []string
		if isHWNode {
			cmdOut, _ = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples}, cmd)
		} else {
			cmdOut, _ = ts.model.RunFakeNaplesCommand(&objects.NaplesCollection{FakeNodes: naples}, cmd)
		}
		peers := []*PdsBGPPeer{}
		err := json.Unmarshal([]byte(cmdOut[0]), &peers)
		return peers, err
	}

	bgp_fn := func(cmd string) (*PdsBgpSpec, error) {
		naples := []*objects.Naples{node}
		var cmdOut []string
		if isHWNode {
			cmdOut, _ = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples}, cmd)
		} else {
			cmdOut, _ = ts.model.RunFakeNaplesCommand(&objects.NaplesCollection{FakeNodes: naples}, cmd)
		}
		bgpSpecs := &PdsBgpSpec{}
		err := json.Unmarshal([]byte(cmdOut[0]), bgpSpecs)
		return bgpSpecs, err
	}

	//get bgp peers, peers-af, show bgp o/p from each naples
	peersAf, _ := peersaf_fn(fmt.Sprintf("%s show bgp peers-af --json", pdsctlPath))
	peers, _ := peers_fn(fmt.Sprintf("%s show bgp peers --json", pdsctlPath))
	bgpSpec, _ := bgp_fn(fmt.Sprintf("%s show bgp --json", pdsctlPath))

	//convert pds config
	return convertPDSConfig(peersAf, peers, bgpSpec, true)
}

func getRRState(pegContainerCollection *objects.VeniceContainerCollection,
	pegasusContainer *objects.VeniceContainer, rtrctlPath string) *pdsConfigCmp {

	peersaf_fn := func(cmd string) ([]*PdsBGPPeerAFSpec, error) {
		cmdOut, _, _, _ := pegContainerCollection.RunCommand(pegasusContainer, cmd)
		peersAf := []*PdsBGPPeerAFSpec{}
		err := json.Unmarshal([]byte(cmdOut), &peersAf)
		return peersAf, err
	}

	peers_fn := func(cmd string) ([]*PdsBGPPeer, error) {
		cmdOut, _, _, _ := pegContainerCollection.RunCommand(pegasusContainer, cmd)
		peers := []*PdsBGPPeer{}
		err := json.Unmarshal([]byte(cmdOut), &peers)
		return peers, err
	}

	bgp_fn := func(cmd string) (*PdsBgpSpec, error) {
		cmdOut, _, _, _ := pegContainerCollection.RunCommand(pegasusContainer, cmd)
		bgpSpecs := &PdsBgpSpec{}
		err := json.Unmarshal([]byte(cmdOut), bgpSpecs)
		return bgpSpecs, err
	}

	//get bgp peers, peers-af, show bgp o/p from each naples
	peersAf, _ := peersaf_fn(fmt.Sprintf("%s show bgp peers-af --json", rtrctlPath))
	peers, _ := peers_fn(fmt.Sprintf("%s show bgp peers --json", rtrctlPath))
	bgpSpec, _ := bgp_fn(fmt.Sprintf("%s show bgp --json", rtrctlPath))

	//convert pds config
	return convertPDSConfig(peersAf, peers, bgpSpec, false)
}

func getNaplesCfgTemplate(node *objects.Naples) (*objects.RoutingConfig, error) {
	dscName := node.Instances[0].Dsc.GetName()
	dsc, err := ts.model.ConfigClient().GetSmartNIC(dscName)
	if err != nil {
		return nil, err
	}

	if dsc.Spec.RoutingConfig == "" {
		return nil, nil
	}

	updc, err := ts.model.ListRoutingConfig()
	if err != nil {
		return nil, err
	}
	for _, r := range updc.RoutingObjs {
		if r.RoutingObj.Spec.BGPConfig.GetDSCAutoConfig() &&
			r.RoutingObj.GetName() == dsc.Spec.RoutingConfig {
			return r, nil
		}
	}
	return nil, fmt.Errorf("Naples routing config not found")
}

func getRRCfgTemplate(nodeName string) (*objects.RoutingConfig, error) {

	node, err := ts.model.ConfigClient().GetClusterNode(nodeName)
	if err != nil {
		log.Infof("Node %s: Error retrieving node config",
			nodeName)
		return nil, err
	}

	if node.Spec.GetRoutingConfig() == "" {
		log.Infof("Node %s: Routing config not associated",
			nodeName)
		return nil, nil
	}

	updc, err := ts.model.ListRoutingConfig()
	if err != nil {
		return nil, err
	}

	//Routing config for RRs is one for each RR; so need to match routerid
	for _, r := range updc.RoutingObjs {
		if r.RoutingObj.Spec.BGPConfig.GetDSCAutoConfig() == false &&
			r.RoutingObj.GetName() == node.Spec.GetRoutingConfig() {
			return r, nil
		}
	}
	return nil, fmt.Errorf("RR : routing config not found for %s", nodeName)
}

func getUnderlayPeers(node *objects.Naples) ([]string, error) {
	naples := []*objects.Naples{node}
	var cmdOut []string
	cmdOut, _ = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples},
		"curl localhost:9007/api/mode/")

	dscStatus := types.DistributedServiceCardStatus{}
	err := json.Unmarshal([]byte(cmdOut[0]), &dscStatus)

	var ip []string

	for _, intf := range dscStatus.DSCInterfaceIPs {
		ip = append(ip, intf.GatewayIP)
	}
	return ip, err
}

const (
	PEER_ESTABLISHED     = "ESTABLISHED"
	PEER_NOT_ESTABLISHED = "NOT ESTABLISHED"
)

func getDefaultNaplesPeerState() (map[string]string, error) {
	m := make(map[string]string)
	var err error

	getOverlayState := func() error {
		vnodes := ts.model.VeniceNodes()
		for _, venicenode := range vnodes.Nodes {
			m[venicenode.IP()] = PEER_NOT_ESTABLISHED
		}

		pnodes, err := vnodes.GetVeniceNodeWithService("pen-pegasus")
		if err != nil {
			return err
		}
		for _, pnode := range pnodes.Nodes {
			m[pnode.IP()] = PEER_ESTABLISHED
		}
		return err
	}

	nodes := ts.model.Naples().Nodes

	for _, node := range nodes {
		//get underlay
		ipList, err := getUnderlayPeers(node)
		if err != nil {
			return nil, err
		}
		for _, ip := range ipList {
			m[ip] = PEER_ESTABLISHED
		}
	}

	//get overlay ; overlay peers are same for all DSCs
	err = getOverlayState()
	if err != nil {
		return nil, err
	}

	return m, nil
}

func getExpectedNaplesState(r *objects.RoutingConfig, node *objects.Naples, isHwNode bool,
	overlayState string, underlayState string) (*pdsConfigCmp, error) {
	var cfg pdsConfigCmp

	if r == nil {
		//Will be nil if there's no routing config associated with this node
		cfg.RouterId = "0.0.0.0"
		return &cfg, nil
	}

	cfg.LocalASNumber = r.RoutingObj.Spec.BGPConfig.GetASNumber()

	if isHwNode {
		cfg.RouterId = strings.Split(node.Instances[0].LoopbackIP, "/")[0]
	} else {
		cfg.RouterId = strings.Split(node.IP(), "/")[0]
	}

	//Add all Venice nodes running pegasus (RR)
	vnodes := ts.model.VeniceNodes()
	pnodes, err := vnodes.GetVeniceNodeWithService("pen-pegasus")

	if err != nil {
		return nil, err
	}
	for _, nbr := range r.RoutingObj.Spec.BGPConfig.Neighbors {
		af := nbr.EnableAddressFamilies[0]
		if strings.Contains(af, "l2vpn-evpn") {
			pnodeMap := make(map[string]bool)
			for _, pnode := range pnodes.Nodes {
				var pdsNbr pdsBGPNeighbor
				pdsNbr.populate(r, nbr, pnode.IP(), overlayState)
				cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
				pnodeMap[pnode.IP()] = true
			}
			//Add other venice nodes which should be in not-established (CONNECT/IDLE) state
			for _, veniceNode := range vnodes.Nodes {
				if pnodeMap[veniceNode.IP()] {
					continue
				}
				var pdsNbr pdsBGPNeighbor
				pdsNbr.populate(r, nbr, veniceNode.IP(), PEER_NOT_ESTABLISHED)
				cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
			}
			break
		}
	}

	//Add all underlay peers -- only for HW nodes
	if isHwNode {
		peerIPs, _ := getUnderlayPeers(node)
		for _, nbr := range r.RoutingObj.Spec.BGPConfig.Neighbors {
			af := nbr.EnableAddressFamilies[0]
			if strings.Contains(af, "ipv4-unicast") {
				for _, ip := range peerIPs {
					var pdsNbr pdsBGPNeighbor
					pdsNbr.populate(r, nbr, ip, underlayState)
					cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
				}
				break
			}
		}
	}

	//sort neighbors list according to peer ip
	sort.Slice(cfg.Neighbors, func(i, j int) bool {
		return cfg.Neighbors[i].Spec.IPAddress < cfg.Neighbors[j].Spec.IPAddress
	})

	return &cfg, err
}

func getExpectedRRState(r *objects.RoutingConfig) *pdsConfigCmp {
	var cfg pdsConfigCmp

	if r == nil {
		//In case when routing config is disassociated from cluster node, it will be nil
		cfg.RouterId = "0.0.0.0"
		return &cfg
	}

	cfg.LocalASNumber = r.RoutingObj.Spec.BGPConfig.GetASNumber()
	cfg.RouterId = r.RoutingObj.Spec.BGPConfig.GetRouterId()

	//Add all non auto config nodes (ECX and any such DSCs) first
	for _, nbr := range r.RoutingObj.Spec.BGPConfig.Neighbors {
		var pdsNbr pdsBGPNeighbor

		if nbr.GetDSCAutoConfig() {
			//skip DSC neighbors with auto config here
			continue
		}
		//ECX or non-autoconfig DSC nodes should have valid IP
		pdsNbr.populateRR(r, nbr, nbr.GetIPAddress())
		cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
	}

	//Add all DSCs
	dscs := ts.model.Naples()

	for _, dsc := range dscs.FakeNodes {
		var pdsNbr pdsBGPNeighbor

		pdsNbr.populateRR(r, nil, strings.Split(dsc.IP(), "/")[0])
		cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
	}
	for _, dsc := range dscs.Nodes {
		var pdsNbr pdsBGPNeighbor
		pdsNbr.populateRR(r, nil, strings.Split(dsc.Instances[0].LoopbackIP, "/")[0])
		cfg.Neighbors = append(cfg.Neighbors, &pdsNbr)
	}

	//sort neighbors list according to peer ip
	sort.Slice(cfg.Neighbors, func(i, j int) bool {
		return cfg.Neighbors[i].Spec.IPAddress < cfg.Neighbors[j].Spec.IPAddress
	})

	return &cfg
}

func verifyNaplesBgpState(overlayState string, underlayState string) {

	//Get pdsagent o/p from Naples and compare
	fakeNodes := ts.model.Naples().FakeNodes
	for _, node := range fakeNodes {
		EventuallyWithOffset(1, func() error {
			log.Infof("DSC %s: trying to match naples bgp state...", node.IP())
			//get naples rtg config template
			r, err := getNaplesCfgTemplate(node)
			if err != nil {
				return err
			}

			expected, _ := getExpectedNaplesState(r, node, false, overlayState, underlayState)

			existing := getNaplesState(node, "/naples/nic/bin/pdsctl", false)

			logStr := fmt.Sprintf("Expected bgp state %+v\n", expected)
			logStr += fmt.Sprintf("Existing bgp state %+v\n", existing)

			for _, n := range expected.Neighbors {
				logStr += fmt.Sprintf("Expected nbr %+v\n", n)
			}
			for _, n := range existing.Neighbors {
				logStr += fmt.Sprintf("Existing nbr %+v\n", n)
			}

			if reflect.DeepEqual(expected, existing) == false {
				return fmt.Errorf("Naples: BGP state verify failed for node %s\nError logs:\n%s",
					node.IP(), logStr)
			}
			return nil
		}).Should(Succeed())
	}

	nodes := ts.model.Naples().Nodes
	for _, node := range nodes {
		EventuallyWithOffset(1, func() error {
			log.Infof("DSC %s: trying to match naples bgp state...", node.IP())
			//get naples rtg config template
			r, err := getNaplesCfgTemplate(node)
			if err != nil {
				return err
			}

			expected, _ := getExpectedNaplesState(r, node, true, overlayState, underlayState)

			existing := getNaplesState(node, "/nic/bin/pdsctl", true)

			logStr := fmt.Sprintf("Expected bgp state %+v\n", expected)
			logStr += fmt.Sprintf("Existing bgp state %+v\n", existing)

			for _, n := range expected.Neighbors {
				logStr += fmt.Sprintf("Expected nbr %+v\n", n)
			}
			for _, n := range existing.Neighbors {
				logStr += fmt.Sprintf("Existing nbr %+v\n", n)
			}

			if reflect.DeepEqual(expected, existing) == false {
				return fmt.Errorf("Naples: BGP state verify failed for node %s\nError logs:\n%s",
					node.IP(), logStr)
			}
			return nil
		}).Should(Succeed())
	}
}

func verifyRRState() {
	//Get pdsagent o/p from RR nodes and compare
	pegContainerCollection, err := ts.model.VeniceNodes().GetVeniceContainersWithService("pen-pegasus", true)

	ExpectWithOffset(1, err).ShouldNot(HaveOccurred())

	for _, pegContainer := range pegContainerCollection.Containers {
		EventuallyWithOffset(1, func() error {
			nodeName := pegContainer.Node.ClusterNode.GetName()
			log.Infof("Cluster node %s: trying to match RR state...", nodeName)
			//get RR rtg config template
			r, err := getRRCfgTemplate(nodeName)

			if err != nil {
				return err
			}
			//populate expected o/p
			expected := getExpectedRRState(r)

			existing := getRRState(pegContainerCollection, pegContainer, "/bin/rtrctl")

			logStr := fmt.Sprintf("Expected bgp state %+v \n", expected)
			logStr += fmt.Sprintf("Existing bgp state %+v\n", existing)

			for _, n := range expected.Neighbors {
				logStr += fmt.Sprintf("Expected nbr %+v\n", n)
			}
			for _, n := range existing.Neighbors {
				logStr += fmt.Sprintf("Existing nbr %+v\n", n)
			}

			if reflect.DeepEqual(expected, existing) == false {
				return fmt.Errorf("RR : BGP state verify failed for RR %s \nError logs:\n%s",
					pegContainer.Node.IP(), logStr)
			}
			return nil
		}).Should(Succeed())
	}
}
