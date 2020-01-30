package cluster

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
)

var _ = Describe("tests for storing firewall logs in object store", func() {
	Context("push firewall logs in object store for default tenant", func() {
		BeforeEach(func() {
			validateCluster()
		})
		It("push fwlogs", func() {
			// get the current list of objects in objectstore
			objects, err := ts.tu.FwLogClient.ListObjects(time.Now().Format("2006-01-02"))
			Expect(err).NotTo(HaveOccurred())
			currentCount := len(objects)

			for _, naple := range ts.tu.NaplesNodes {
				// Run the loop 3 times to make sure that the pipeline is continously transferring data from card to object store.
				for i := 0; i < 3; i++ {
					By(fmt.Sprintf("trigger fwlog in NIC container %s", naple))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naple, fwevent.Cmd(1000, 65)))
					Expect(st == "null").Should(BeTrue())

					time.Sleep(time.Second)

					objects, err := ts.tu.FwLogClient.ListObjects(time.Now().Format("2006-01-02"))
					Expect(err).NotTo(HaveOccurred())
					Expect(len(objects) > currentCount)
					currentCount = len(objects)
				}

				// TODO: Add number of logs in the meta of the object and also Assert on that number here.
				// Not sure how to Assert on the name of the object since its dependent on time.
				// Just do for one naples. Thats enough for e2e tests.
				break
			}
		})
		AfterEach(func() {
			validateCluster()
		})
	})
})
