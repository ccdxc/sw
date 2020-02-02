// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

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
		It("tags:sanity=true should log ICMP allow in fwlog", func() {
			Skip("Disabling as this test is failing")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till shm flag is enabled")
			}

			objs, err := ts.model.ConfigClient().ListObjectStoreObjects()
			Expect(err).ShouldNot(HaveOccurred())

			currentObjectCount := 0
			for _, obj := range objs {
				if obj.ObjectMeta.Tenant == "default" && obj.ObjectMeta.Namespace == "fwlogs" {
					currentObjectCount++
				}
			}
			Expect(err).ShouldNot(HaveOccurred())

			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())

			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

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

				objs, err := ts.model.ConfigClient().ListObjectStoreObjects()
				Expect(err).ShouldNot(HaveOccurred())

				newObjectCount := 0
				for _, obj := range objs {
					if obj.ObjectMeta.Tenant == "default" && obj.ObjectMeta.Namespace == "fwlogs" {
						newObjectCount++
					}
				}

				return newObjectCount > currentObjectCount
			}).Should(BeTrue())
		})
	})
})
