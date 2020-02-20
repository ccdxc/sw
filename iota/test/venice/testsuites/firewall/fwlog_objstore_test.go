package firewall_test

import (
	"fmt"
	"strings"
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
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			t := time.Now()
			y, m, dt := t.Date()
			h, _, _ := t.Clock()
			timestamp := time.Date(y, m, dt, h, 0, 0, 0, time.UTC)

			// Get the naples id from the workload
			workloadA := workloadPairs.Pairs[0].First
			workloadB := workloadPairs.Pairs[0].Second
			naplesAMac := workloadA.NaplesMAC()
			naplesBMac := workloadB.NaplesMAC()

			bucketAName :=
				naplesAMac + "-" +
					strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1)
			bucketBName :=
				naplesBMac + "-" +
					strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1)

			By(fmt.Sprintf("bucketAName %s bucketBName %s", bucketPrefix+"."+bucketAName, bucketPrefix+"."+bucketBName))

			currentObjectCountBucketA, err := ts.model.GetFwLogObjectCount(bucketPrefix, bucketAName)
			Expect(err).ShouldNot(HaveOccurred())
			currentObjectCountBucketB, err := ts.model.GetFwLogObjectCount(bucketPrefix, bucketBName)
			Expect(err).ShouldNot(HaveOccurred())

			By(fmt.Sprintf("currentObjectCountBucketA %d, currentObjectCountBucketB %d",
				currentObjectCountBucketA, currentObjectCountBucketB))

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
				newObjectCountBucketA, err := ts.model.GetFwLogObjectCount(bucketPrefix, bucketAName)
				Expect(err).ShouldNot(HaveOccurred())
				newObjectCountBucketB, err := ts.model.GetFwLogObjectCount(bucketPrefix, bucketBName)
				Expect(err).ShouldNot(HaveOccurred())
				By(fmt.Sprintf("newObjectCountBucketA %d, newObjectCountBucketB %d",
					newObjectCountBucketA, newObjectCountBucketB))
				return newObjectCountBucketA > currentObjectCountBucketA && newObjectCountBucketB > currentObjectCountBucketB
			}, time.Second*100).Should(BeTrue())
		})
	})
})
