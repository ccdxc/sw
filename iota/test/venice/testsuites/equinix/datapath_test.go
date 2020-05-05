package equinix_test

import (
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Datapath", func() {

	var tenant string = "customer0"
	var sn *objects.NetworkCollection
	var naples []string
	var allWc *objects.WorkloadCollection

	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// get networks
		var err error
		sn, err = objects.DefaultNetworkCollection(ts.model.ConfigClient())
		if err != nil {
			return
		}

		// get naples info
		naples = ts.model.Naples().Names()
		allWc = ts.model.Workloads()
	})
	AfterEach(func() {
	})

	Context("Datapath Tests", func() {

		It("L2 Remote Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			wfilter := objects.WorkloadFilter{}
			wfilter.SetNaplesCollection(ts.model.Naples())
			wfilter.SetNetworkCollection(sn.Any(1))
			wc := allWc.Filter(&wfilter)

			// L2 remote ping
			Expect(ts.model.PingPairs(wc.RemotePairsWithinNetwork())).Should(Succeed())

		})

		It("L3 Local Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			wfilter := objects.WorkloadFilter{}
			wfilter.SetNaplesCollection(ts.model.Naples())
			wfilter.SetNetworkCollection(sn.Pop(1))
			wc1 := allWc.Filter(&wfilter)

			wfilter = objects.WorkloadFilter{}
			wfilter.SetNaplesCollection(ts.model.Naples())
			wfilter.SetNetworkCollection(sn.Pop(1))
			wc2 := allWc.Filter(&wfilter)

			subnetWorkloads := wc1.MeshPairsWithOther(wc2)

			// L3 local ping
			Expect(ts.model.PingPairs(subnetWorkloads.LocalPairs()))

		})

		It("L3 Remote with Local Subnet Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			wfilter := objects.WorkloadFilter{}
			wfilter.SetNaplesCollection(ts.model.Naples())
			wfilter.SetNetworkCollection(sn.Pop(1))
			wc1 := allWc.Filter(&wfilter)

			wfilter = objects.WorkloadFilter{}
			wfilter.SetNaplesCollection(ts.model.Naples())
			wfilter.SetNetworkCollection(sn.Pop(1))
			wc2 := allWc.Filter(&wfilter)

			subnetWorkloads := wc1.MeshPairsWithOther(wc2)

			// L3 local ping
			Expect(ts.model.PingPairs(subnetWorkloads.RemotePairs()))

		})

		It("L3 Remote with non-local Subnet ping", func() {

			//TODO: this test should be modified to remove ref from topology
			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			sn = sn.Any(2)
			if len(naples) < 2 {
				Skip("not enough naples to L3 remote with non-local subnet ping")
			}

			// DSC1: get host-pf attached with network S1
			dscIntf1, err := objects.GetNetworkInterfaceBySubnet(naples[0], sn.Subnets()[0].Name, ts.model.ConfigClient(), ts.model.Testbed())
			Expect(err).Should(Succeed())

			// DSC2: get host-pf attached with network S2

			dscIntf2, err := objects.GetNetworkInterfaceBySubnet(naples[1], sn.Subnets()[1].Name, ts.model.ConfigClient(), ts.model.Testbed())
			Expect(err).Should(Succeed())

			// detach network interfaces
			dscIntf1.AttachNetwork("", "") //DSC1-S1
			dscIntf2.AttachNetwork("", "") //DSC2-S2

			// ping between DSC1-S1 and DSC2-S1 should fail
			Expect(dataPathPingTest(naples[0], naples[1], sn.Subnets()[0], sn.Subnets()[0], false)).Should(Succeed())

			//Ping between DSC1-S2 and DSC2-S2 should fail
			Expect(dataPathPingTest(naples[0], naples[1], sn.Subnets()[1], sn.Subnets()[1], false)).Should(Succeed())

			// Ping pair DSC1-S2 vs DSC2-S1 should succeed
			Expect(dataPathPingTest(naples[0], naples[1], sn.Subnets()[1], sn.Subnets()[0], true)).Should(Succeed())

			// restore network interfaces
			dscIntf1.AttachNetwork(tenant, sn.Subnets()[0].Name) // DSC1-S1
			dscIntf2.AttachNetwork(tenant, sn.Subnets()[1].Name) // DSC2-S2

			// verify that cluster status is not messed up
			Eventually(func() error {
				return ts.model.VerifyClusterStatus()
			}).Should(Succeed())
		})
	})
})

func dataPathPingTest(naples1, naples2 string, nw1, nw2 *objects.Network, pass bool) error {

	logStr := fmt.Sprintf("ping between %v-%v and %v-%v", naples1, nw1.VeniceNetwork.Name, naples2, nw2.VeniceNetwork.Name)

	wpc := ts.model.WorkloadPairs().SpecificPair(naples1, naples2, nw1, nw2)
	if len(wpc.Pairs) == 0 {
		log.Infof("No workload combination to %v", logStr)
		return fmt.Errorf("No workload combination to %v", logStr)
	}

	if pass {
		log.Infof("%v should succeed", logStr)
		Eventually(func() error {
			return ts.model.PingPairs(wpc)
		}).Should(Succeed())
	} else {
		log.Infof("%v should fail", logStr)
		Eventually(func() error {
			return ts.model.PingFails(wpc)
		}).Should(Succeed())
	}
	return nil
}
