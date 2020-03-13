package firewall_test

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

const timeFormat = "2006-01-02T15:04:05"
const bucketPrefix = "fwlogs"

var _ = Describe("tests for storing firewall logs in object store", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))

		// delete test policy if its left over. we can ignore thes error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Commit()).ShouldNot(HaveOccurred())

		// verify policy was propagated correctly
		Eventually(func() error {
			return ts.model.VerifyPolicyStatus(ts.model.DefaultNetworkSecurityPolicy())
		}).Should(Succeed())
	})

	// TODO: Checking exact losg needs much more backend work before tests can be written for it.
	// MinIO's SQL queries would have to get exposed through API Gateway for querying exact logs from a bucket.
	// ObjectStore's client interface would have to get extended to included SQL queries.
	// For now, just checking that number of objects in the bucket are increasing when logs are getting uploaded
	// to the bucket.
	Context("tags:type=basic;datapath=true;duration=short;store=objectstore verify fwlog on traffic ", func() {
		It("tags:sanity=true should push fwlog to objectstore", func() {
			Skip("reenable test case after local testing")

			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			// Get the naples id from the workload
			workloadA := workloadPairs.Pairs[0].First
			workloadB := workloadPairs.Pairs[0].Second
			naplesAMac := workloadA.NaplesMAC()
			naplesBMac := workloadB.NaplesMAC()

			currentObjectCountNaplesA, err :=
				ts.model.GetFwLogObjectCount("fwlogs", "fwlogs", naplesAMac)
			Expect(err).ShouldNot(HaveOccurred())
			currentObjectCountNaplesB, err :=
				ts.model.GetFwLogObjectCount("fwlogs", "fwlogs", naplesBMac)
			Expect(err).ShouldNot(HaveOccurred())

			By(fmt.Sprintf("currentObjectCountNaplesA %d, currentObjectCountNaplesB %d",
				currentObjectCountNaplesA, currentObjectCountNaplesB))

			policy := ts.model.NewNetworkSecurityPolicy("test-policy").AddRule("any", "any", "icmp", "PERMIT")
			Expect(policy.Commit()).ShouldNot(HaveOccurred())

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ts.model.NetworkSecurityPolicy("test-policy"))
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.PingPairs(workloadPairs)
			}).Should(Succeed())

			By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

			// check object count
			Eventually(func() bool {
				newObjectCountNaplesA, err :=
					ts.model.GetFwLogObjectCount("fwlogs", "fwlogs", naplesAMac)
				Expect(err).ShouldNot(HaveOccurred())
				newObjectCountNaplesB, err :=
					ts.model.GetFwLogObjectCount("fwlogs", "fwlogs", naplesBMac)
				Expect(err).ShouldNot(HaveOccurred())

				By(fmt.Sprintf("newObjectCountNaplesA %d, newObjectCountNaplesB %d",
					newObjectCountNaplesA, newObjectCountNaplesB))
				return newObjectCountNaplesA > currentObjectCountNaplesA && newObjectCountNaplesB > currentObjectCountNaplesB
			}, time.Second*100).Should(BeTrue())
		})
	})
})
