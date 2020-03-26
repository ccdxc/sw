package equinix_test

import (
	"encoding/json"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"strings"
	"time"
)

var _ = Describe("Network", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Network tests", func() {
		It("Add a subnet & verify config", func() {

			tenantName := "tenant0"

			//Create tenant
			tenant := ts.model.NewTenant(tenantName)

			Expect(tenant.Commit()).Should(Succeed())

			//Create VPC config
			vpcName := "testVPC"
			vpcVni := uint32(700)

			vpc := ts.model.NewVPC(tenantName, vpcName, "0001.0102.0202", vpcVni)

			Expect(vpc.Commit()).Should(Succeed())

			//Validate using Get command
			veniceVpc, err := ts.model.GetVPC(vpcName, tenantName)

			Expect(err).ShouldNot(HaveOccurred())
			Expect(veniceVpc.Obj.Name == vpcName).Should(BeTrue())

			//Create a subnet
			nwName := "Network0"

			nwVni := 0x80000 | vpcVni

			log.Infof("NW Vni %v", nwVni)

			nwp := &base.NetworkParams{
				nwName,
				"10.1.2.0/24",
				"10.1.2.1",
				nwVni,
				vpcName,
				tenantName,
			}

			nwc := ts.model.NewNetwork(nwp)

			Expect(nwc.Commit()).Should(Succeed())

			//Verify network config in Venice

			nw, err := ts.model.GetNetwork(tenantName, nwName)

			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Name == nwName).Should(BeTrue())

			//Verify state in Naples

			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim")
			}

			uuid := nw.VeniceNetwork.GetUUID()

			log.Infof("Network object UUID %s", uuid)

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)

			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				cmdout, err := ts.model.RunNaplesCommand(nc, "/nic/bin/pdsctl show subnet")
				Expect(err).ShouldNot(HaveOccurred())

				log.Infof("pdsctl output: \n %s", cmdout)

				Expect(strings.Contains(cmdout[0], uuid)).Should(BeTrue())
				return nil
			})

			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {

				cmd := "curl localhost:9007/api/networks/"

				cmdout, err := ts.model.RunNaplesCommand(nc, cmd)

				Expect(err).ShouldNot(HaveOccurred())

				log.Infof("PDSCTL output: \n %s", cmdout)

				nwData := []netproto.Network{}

				err = json.Unmarshal([]byte(cmdout[0]), &nwData)

				Expect(err).ShouldNot(HaveOccurred())

				match := false
				for _, v := range nwData {
					if v.Name == nwName {
						match = true
						break
					}
				}

				Expect(match).To(BeTrue())
				return nil
			})

			Expect(nwc.Delete()).Should(Succeed())

			_, err = ts.model.GetNetwork(tenantName, nwName)
			log.Infof("Error for Network delete %s", err.Error())
			Expect(strings.Contains(err.Error(), "object not found")).Should(BeTrue())

			Expect(vpc.Delete()).Should(Succeed())
			Expect(tenant.Delete()).Should(Succeed())
		})
	})
})
