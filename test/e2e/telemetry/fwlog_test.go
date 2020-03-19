package telemetry

import (
	"context"
	"flag"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	//"github.com/pensando/sw/venice/utils/log"
)

var ts *teleSuite
var _ = BeforeSuite(func() {
	flag.Parse()
	var err error
	ts, err = newTS()
	Expect(err).Should(BeNil())

	Eventually(func() error {
		err := ts.broker.ClusterCheck()
		return err
	}, 20, 1).Should(BeNil(), "cluster is not ready, %s", err)

	err = ts.CreateDB(context.Background(), "FWLogs")
	Expect(err).Should(BeNil())

})

var _ = AfterSuite(func() {
	err := ts.DeleteDB(context.Background(), "FWLogs")
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
