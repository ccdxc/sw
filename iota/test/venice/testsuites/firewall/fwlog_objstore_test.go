package firewall_test

import (
	"fmt"
	"os"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

const (
	timeFormat = "2006-01-02T15:04:05"
	bucketName = "fwlogs"
	tenantName = "default"
)

var _ = Describe("tests for storing firewall logs in object store and elastic", func() {
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

		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())

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
	Context("tags:type=basic;datapath=true;duration=short;store=verify fwlog on traffic in objectstore and elastic", func() {
		It("tags:sanity=true should push fwlog to objectstore", func() {
			if !ts.tb.HasNaplesHW() || os.Getenv("REGRESSION") == "" {
				Skip("runs only on hardware naples and in regression")
			}
			ts.model.WorkloadPairs().WithinNetwork().Permit(ts.model.DefaultNetworkSecurityPolicy(), "icmp")
			pushLogsAndVerify()
		})

		It("tags:sanity=true venice isolate APIServer node, should not affect reporting of fwlogs", func() {
			if !ts.tb.HasNaplesHW() || os.Getenv("REGRESSION") == "" {
				Skip("runs only on hardware naples and in regression")
			}
			ts.model.WorkloadPairs().WithinNetwork().Permit(ts.model.DefaultNetworkSecurityPolicy(), "icmp")

			naples := ts.model.Naples()
			apiServerNode, err := ts.model.VeniceNodes().GetVeniceNodeWithService("pen-apiserver")
			Expect(err).ShouldNot(HaveOccurred())
			err = ts.model.DisconnectVeniceNodesFromCluster(apiServerNode, naples)
			Expect(err).ShouldNot(HaveOccurred())

			// Sleep for 60 seconds to make sure we detect partitioning.
			time.Sleep(800 * time.Second)
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			// Push logs and verify
			pushLogsAndVerify()

			//Connect Back and make sure cluster is good
			ts.model.ConnectVeniceNodesToCluster(apiServerNode, naples)
			time.Sleep(60 * time.Second)
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())

			// Again verify
			pushLogsAndVerify()
		})

		It("tags:sanity=true venice isolate nodes in a loop, should not affect reporting of fwlogs", func() {
			Skip("reenable this test after testing")
			if !ts.tb.HasNaplesHW() || os.Getenv("REGRESSION") == "" {
				Skip("runs only on hardware naples and in regression")
			}
			ts.model.WorkloadPairs().WithinNetwork().Permit(ts.model.DefaultNetworkSecurityPolicy(), "icmp")

			ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
				naples := ts.model.Naples()
				err := ts.model.DisconnectVeniceNodesFromCluster(vnc, naples)
				Expect(err).ShouldNot(HaveOccurred())

				// Sleep for 60 seconds to make sure we detect partitioning.
				time.Sleep(800 * time.Second)
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				// Push logs and verify
				pushLogsAndVerify()

				//Connect Back and make sure cluster is good
				ts.model.ConnectVeniceNodesToCluster(vnc, naples)
				time.Sleep(60 * time.Second)
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				// Again verify
				pushLogsAndVerify()

				return nil
			})
		})

		It("tags:sanity=true reloading venice nodes should not affect fwlogs", func() {
			if !ts.tb.HasNaplesHW() || os.Getenv("REGRESSION") == "" {
				Skip("runs only on hardware naples and in regression")
			}
			ts.model.WorkloadPairs().WithinNetwork().Permit(ts.model.DefaultNetworkSecurityPolicy(), "icmp")

			// reload each host
			ts.model.ForEachVeniceNode(func(vnc *objects.VeniceNodeCollection) error {
				Expect(ts.model.ReloadVeniceNodes(vnc)).Should(Succeed())

				// Again verify
				pushLogsAndVerify()

				// wait for cluster to be back in good state
				Eventually(func() error {
					return ts.model.VerifyClusterStatus()
				}).Should(Succeed())

				return nil
			})
		})
	})
})

func pushLogsAndVerify() {
	workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

	// Get the naples id from the workload
	workloadA := workloadPairs.Pairs[0].First
	workloadB := workloadPairs.Pairs[0].Second
	naplesAMac := workloadA.NaplesMAC()
	naplesBMac := workloadB.NaplesMAC()
	currentObjectCountNaplesA, currentObjectCountNaplesB := 0, 0
	var err error
	Eventually(func() error {
		currentObjectCountNaplesA, err =
			ts.model.GetFwLogObjectCount(tenantName, bucketName, naplesAMac)
		return err
	}).Should(Succeed())

	Eventually(func() error {
		currentObjectCountNaplesB, err =
			ts.model.GetFwLogObjectCount(tenantName, bucketName, naplesBMac)
		return err
	}).Should(Succeed())

	By(fmt.Sprintf("currentObjectCountNaplesA %d, currentObjectCountNaplesB %d",
		currentObjectCountNaplesA, currentObjectCountNaplesB))

	// Do it in a loop, and verify that the object count keeps going up in object store
	for i := 0; i < 3; i++ {
		Eventually(func() error {
			return ts.model.PingPairs(workloadPairs)
		}).Should(Succeed())

		By(fmt.Sprintf("workload ip address %+v", workloadPairs.ListIPAddr()))

		// check object count
		Eventually(func() bool {
			newObjectCountNaplesA, newObjectCountNaplesB := 0, 0
			var err error

			Eventually(func() error {
				newObjectCountNaplesA, err =
					ts.model.GetFwLogObjectCount(tenantName, bucketName, naplesAMac)
				return err
			}).Should(Succeed())

			Eventually(func() error {
				newObjectCountNaplesB, err =
					ts.model.GetFwLogObjectCount(tenantName, bucketName, naplesBMac)
				return err
			}).Should(Succeed())

			By(fmt.Sprintf("newObjectCountNaplesA %d, newObjectCountNaplesB %d",
				newObjectCountNaplesA, newObjectCountNaplesB))
			if newObjectCountNaplesA > currentObjectCountNaplesA && newObjectCountNaplesB > currentObjectCountNaplesB {
				currentObjectCountNaplesA = newObjectCountNaplesA
				currentObjectCountNaplesB = newObjectCountNaplesB
				return true
			}
			return false
		}, time.Second*100).Should(BeTrue())
	}
}
