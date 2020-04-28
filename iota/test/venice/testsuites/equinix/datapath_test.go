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
	var nwc *objects.NetworkCollection
	var naples []string

	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// get default tenant VRF
		vpcc, err := objects.TenantVPCCollection(tenant, ts.model.ConfigClient(), ts.model.Testbed())
		Expect(err).Should(Succeed())
		vpcName := vpcc.Objs[0].Obj.Name

		// get network from vpc
		nwc, err = GetNetworkCollectionFromVPC(vpcName, tenant)
		Expect(err).Should(Succeed())

		// get naples info
		naples = ts.model.Naples().Names()
	})
	AfterEach(func() {
		ts.model.AfterTestCommon()
	})

	Context("Datapath Tests", func() {

		It("L2 Remote Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			// Ping pair DSC1-S1 vs DSC2-S1 should succeed
			sn := nwc.Any(1)
			Expect(dataPathPingTest(naples[0], naples[1], sn.Subnets()[0], sn.Subnets()[0], true)).Should(Succeed())
		})

		It("L3 Local Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			// Ping pair DSC1-S1 vs DSC1-S2 should succeed
			sn := nwc.Any(2)
			Expect(dataPathPingTest(naples[0], naples[0], sn.Subnets()[0], sn.Subnets()[1], true)).Should(Succeed())
		})

		It("L3 Remote with Local Subnet Ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			// Ping pair DSC1-S2 vs DSC2-S1 should succeed
			sn := nwc.Any(2)
			Expect(dataPathPingTest(naples[0], naples[1], sn.Subnets()[0], sn.Subnets()[1], true)).Should(Succeed())

		})

		It("L3 Remote with non-local Subnet ping", func() {

			if !ts.tb.HasNaplesHW() {
				Skip("Ping test cases are enabled only for HW naples")
			}

			sn := nwc.Any(2)
			naples := ts.model.Naples().Names()

			// DSC1: get host-pf attached with network S1
			dscIntf1, err := getSubnetHostPfNetworkIntfOnNaples(naples[0], sn.Subnets()[0].Name)
			Expect(err).Should(Succeed())

			// DSC2: get host-pf attached with network S2
			dscIntf2, err := getSubnetHostPfNetworkIntfOnNaples(naples[1], sn.Subnets()[1].Name)
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

func getSubnetHostPfNetworkIntfOnNaples(naples, subnet string) (*objects.NetworkInterfaceCollection, error) {

	filter := fmt.Sprintf("spec.type=host-pf,spec.attach-network=%v,status.dsc=%v", subnet, naples)
	hostNwIntfs, err := ts.model.ListNetworkInterfacesByFilter(filter)
	if err != nil {
		return nil, err
	}
	if len(hostNwIntfs.Interfaces) == 0 {
		err = fmt.Errorf("no host-pf on %v attached to subnet %v", naples, subnet)
		log.Errorf("%v", err)
		return nil, err
	}
	return hostNwIntfs, nil
}

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
