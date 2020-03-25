package equinix_test

import (
	"encoding/json"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
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
		ts.tb.AfterTestCommon()
	})

	Context("Undelay ASN", func() {
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
	})
})
