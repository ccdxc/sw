// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package vchub_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Vc hub workload tests", func() {
	BeforeEach(func() {
		 //verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Bring up teardown workloads", func() {

		It("tags:sanity=true Bring up workload and test traffic", func() {

			workloads := ts.model.Workloads()
			err := ts.model.TeardownWorkloads(workloads)
			Expect(err == nil)

			workloads = ts.model.BringUpNewWorkloads(ts.model.Hosts(), ts.model.Networks().Any(1), 2)
			Expect(workloads.Error()).ShouldNot(HaveOccurred())

			// verify workload status is good, Put all check w.r.t to venice here.
			Eventually(func() error {
				return ts.model.VerifyWorkloadStatus(workloads)
			}).Should(Succeed())

			// check ping between new workloads
			Eventually(func() error {
				return ts.model.TCPSession(workloads.LocalPairs().Any(4), 8000)
			}).ShouldNot(HaveOccurred())

			/*
				// check ping between new workloads Once Datapath is ready
				Eventually(func() error {
						return ts.model.TCPSession(workloads.RemotePairs().Any(4), 8000)
				}).ShouldNot(HaveOccurred())
			*/

			//Teardown the new workloads
			//err := workloads.Teardown()
			//Expect(err == nil)

			// verify workload status is good, Put all check w.r.t to venice here.
			Eventually(func() error {
				return ts.model.VerifyWorkloadStatus(workloads)
			}).Should(Succeed())
		})

		runTraffic := func(workloadPairs *objects.WorkloadPairCollection, rerr chan error) {
			options := &objects.ConnectionOptions{
				Duration:          "180s",
				Port:              "8000",
				Proto:             "tcp",
				ReconnectAttempts: 100,
			}
			err := ts.model.ConnectionWithOptions(workloadPairs, options)
			log.Infof("TCP SESSION TEST COMPLETE")
			rerr <- err
			return
		}

		It("tags:sanity=true Vmotion basic test", func() {

			//Skip("Skipping move tests..")
			workloads := ts.model.BringUpNewWorkloads(ts.model.Hosts(), ts.model.Networks().Any(1), 1)
			Expect(workloads.Error()).ShouldNot(HaveOccurred())

			//Get All possible Host & Corresponding workload combination
			hostWorkloads := ts.model.HostWorkloads()

			if len(hostWorkloads) < 2 {
				Skip("Skipping vmotion tests as there are not enough workloads")
			}

			log.Infof("Foudn sufficient worklaods")
			srcHost := hostWorkloads[0].Host()
			srcWorkloads := hostWorkloads[0].Workloads().Any(1)
			dstHost := hostWorkloads[1].Host()
			dstWorkloads := hostWorkloads[1].Workloads().Any(1)

			wPairs := srcWorkloads.MeshPairsWithOther(dstWorkloads)

			terr := make(chan error)

			go runTraffic(wPairs, terr)

			err := ts.model.MoveWorkloads(srcWorkloads, dstHost)
			Expect(err == nil)
			err = ts.model.MoveWorkloads(dstWorkloads, srcHost)
			Expect(err == nil)

			Eventually(func() error {
				return ts.model.VerifyWorkloadStatus(srcWorkloads)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.VerifyWorkloadStatus(dstWorkloads)
			}).Should(Succeed())

			Expect(terr == nil)
		})

	})

})
