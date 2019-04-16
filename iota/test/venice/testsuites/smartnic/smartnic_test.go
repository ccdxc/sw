// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package smartnic_test

import (
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit"
)

var _ = Describe("smartnic tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Basic smartnic tests", func() {
		It("Should be able to reload hosts and smartnic should come back up", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disable on Sim setups till we debug Naples sim issues with reload")
			}

			for i := 0; i < 2; i++ {
				// reload each host
				ts.model.ForEachHost(func(hc *iotakit.HostCollection) error {
					Expect(ts.model.Action().ReloadHosts(hc)).Should(Succeed())

					// wait for cluster to be back in good state
					Eventually(func() error {
						return ts.model.Action().VerifyClusterStatus()
					}).Should(Succeed())

					// verify ping between all workloads
					Eventually(func() error {
						return ts.model.Action().TCPSession(ts.model.WorkloadPairs().WithinNetwork().Any(4), 8000)
					}).Should(Succeed())

					return nil
				})
			}
		})

		It("Should be able to disconnect and connect smartnics", func() {
			if ts.tb.HasNaplesHW() {
				Skip("Skipping connect/disconnect tests on HW setups till inband mgmt is enabled")
			}
			for i := 0; i < 2; i++ {
				// reload each host
				ts.model.ForEachNaples(func(nc *iotakit.NaplesCollection) error {
					Expect(ts.model.Action().DisconnectNaples(nc)).Should(Succeed())
					time.Sleep(time.Second * 30)
					Expect(ts.model.Action().ConnectNaples(nc)).Should(Succeed())

					// wait for cluster to be back in good state
					Eventually(func() error {
						return ts.model.Action().VerifyClusterStatus()
					}).Should(Succeed())

					// verify ping between all workloads
					Eventually(func() error {
						return ts.model.Action().TCPSession(ts.model.WorkloadPairs().WithinNetwork().Any(4), 8000)
					}).Should(Succeed())

					return nil
				})
			}
		})
	})
})
