// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package equinix_test

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

func getNetworkCollection() (*objects.NetworkCollection, error) {

	// add permit rules for workload pairs
	ten, err := ts.model.ConfigClient().ListTenant()
	if err != nil {
		return nil, err
	}

	if len(ten) == 0 {
		return nil, fmt.Errorf("Not enough tenants to list networks")
	}

	nws, err := ts.model.ConfigClient().ListNetwork(ten[0].Name)

	nwc := objects.NewNetworkCollectionFromNetworks(ts.model.ConfigClient(), nws)

	return nwc, nil

}

var _ = Describe("firewall whitelist tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
		nwc, err := getNetworkCollection()
		Expect(err).Should(Succeed())

		//Now attach ingress and egres
		nwc.SetIngressSecurityPolicy(nil)
		nwc.SetEgressSecurityPolicy(nil)
		// delete test policy if its left over. we can ignore the error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("tags:type=basic;datapath=true;duration=short basic whitelist tests", func() {

		It("tags:sanity=true Should allow TCP connections with specific permit rules", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			Skip("Disabling until datapath issue is debugged")

			nwc, err := getNetworkCollection()
			Expect(err).Should(Succeed())

			selNetwork := nwc.Any(1)
			workloadPairs := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)

			spc := ts.model.NewNetworkSecurityPolicy("test-policy").AddRulesForWorkloadPairs(workloadPairs, "tcp/8000", "PERMIT")
			Expect(spc.Commit()).Should(Succeed())

			//Now attach ingress and egress
			selNetwork.SetIngressSecurityPolicy(spc)
			selNetwork.SetEgressSecurityPolicy(spc)

			// verify policy was propagated correctly
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(spc)
			}).Should(Succeed())

			// verify TCP connection works between workload pairs
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairs, 8000)
			}).Should(Succeed())

			// verify ping fails
			Eventually(func() error {
				return ts.model.PingFails(workloadPairs)
			}).Should(Succeed())

			// verify connections in reverse direction fail
			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairs.ReversePairs(), 8000)
			}).Should(Succeed())
		})

	})
})
