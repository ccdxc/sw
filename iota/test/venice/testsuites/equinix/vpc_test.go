package equinix_test

import (
	"encoding/json"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("VPC", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("VPC tests", func() {
		It("Add a VPC & verify config", func() {
			//Create VPC config

			tenantName := "tenant0"

			//Create tenant
			tenant := ts.model.NewTenant(tenantName)

			Expect(tenant.Commit()).Should(Succeed())

			//Create VPC
			vpcName := "testVPC"
			vpc := ts.model.NewVPC(tenantName, vpcName, "0001.0102.0202", 100, "")

			Expect(vpc.Commit()).Should(Succeed())

			//Validate using Get command

			veniceVpc, err := ts.model.GetVPC(vpcName, tenantName)

			Expect(err).ShouldNot(HaveOccurred())
			Expect(veniceVpc.Obj.Name == vpcName).Should(BeTrue())

			//Verify state in Naples

			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim")
			}

			uuid := veniceVpc.Obj.GetUUID()

			log.Infof("UUID %s", uuid)

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)

			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				cmdout, err := ts.model.RunNaplesCommand(nc, "/nic/bin/pdsctl show vpc")
				Expect(err).ShouldNot(HaveOccurred())

				log.Infof("pdsctl output: \n %s", cmdout)

				Expect(strings.Contains(cmdout[0], uuid)).Should(BeTrue())
				return nil
			})

			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				cmd := "curl localhost:9007/api/vrfs/"
				cmdout, err := ts.model.RunNaplesCommand(nc, cmd)
				Expect(err).ShouldNot(HaveOccurred())

				log.Infof("PDSCTL output: \n %s", cmdout)

				vrfData := []netproto.Vrf{}

				err = json.Unmarshal([]byte(cmdout[0]), &vrfData)

				Expect(err).ShouldNot(HaveOccurred())

				match := false
				for _, v := range vrfData {
					if v.Name == vpcName {
						match = true
						break
					}
				}
				Expect(match).To(BeTrue())
				return nil
			})

			//Delete VPC
			Expect(vpc.Delete()).Should(Succeed())

			_, err = ts.model.GetVPC(vpcName, tenantName)
			log.Infof("Error for VPC delete %s", err.Error())
			Expect(strings.Contains(err.Error(), "object not found")).Should(BeTrue())

			//Delete tenant
			Expect(tenant.Delete()).Should(Succeed())

			_, err = ts.model.GetTenant(tenantName)
			log.Infof("Error for Tenant delete %s", err.Error())

			Expect(strings.Contains(err.Error(), "object not found")).Should(BeTrue())
		})
	})
})
