package telemetry

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	//"github.com/pensando/sw/venice/utils/log"
)

var ts *teleSuite
var _ = BeforeSuite(func() {
	var err error
	ts, err = newTS()
	Expect(err).Should(BeNil())
	err = ts.CreateDB("FWLogs")
	Expect(err).Should(BeNil())
})

var _ = AfterSuite(func() {
	err := ts.DeleteDB("FWLogs")
	Expect(err).Should(BeNil())
})

var _ = Describe("TeleE2E", func() {

	Describe("FWLog events injection", func() {
		Context("node0->(grpc)fte_sim->(ipc)tmAgent->(grpc)collector->(http)influx", func() {
			It("Verify they are correctly written to Influx", func() {
				By("Adding 1000 logs")
				err := ts.InjectLogs(1000)
				Expect(err).Should(BeNil())
				By("Counting points in InfluxDB")
				Eventually(func() int {
					count, err := ts.CountPoints("FWLogs", "firewall")
					if err == nil {
						return count
					}
					return 0
				}, 60*time.Second, time.Second).Should(Equal(1000))
				By("Verifying each point in InfluxDB")
				err = ts.VerifyQueryAll("FWLogs", "firewall")
				Expect(err).Should(BeNil())
			})

		})
	})
})
