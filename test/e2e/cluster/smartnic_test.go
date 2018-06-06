package cluster

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
)

var _ = Describe("SmartNIC tests", func() {

	Context("SmartNIC object creation & nic-admission validation test", func() {
		var (
			snics []*cmd.SmartNIC
			err   error
			snIf  cmd.ClusterV1SmartNICInterface
		)
		BeforeEach(func() {
			snIf = ts.tu.APIClient.ClusterV1().SmartNIC()
		})

		It("SmartNIC should be created and admitted", func() {
			Expect(err).ShouldNot(HaveOccurred())

			// Validate smartNIC object creation
			Eventually(func() bool {
				snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
				if len(snics) != ts.tu.NumNaplesHosts {
					return false
				}
				By(fmt.Sprintf("ts:%s SmartNIC creation validated for [%d] nics", time.Now().String(), len(snics)))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC object creation failed")

			// Validate nic-admission status
			Eventually(func() bool {

				numAdmittedNICs := 0
				for _, snic := range snics {
					if snic.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
						return false
					}
					numAdmittedNICs++
					By(fmt.Sprintf("SmartNIC [%s] is created & admitted", snic.Name))
				}

				if numAdmittedNICs != ts.tu.NumNaplesHosts {
					return false
				}

				By(fmt.Sprintf("ts:%s SmartNIC admission validated for [%d] nics", time.Now().String(), numAdmittedNICs))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC nic-admission check failed")
		})
	})

	Context("SmartNIC health status and periodic updates test", func() {
		var (
			snics  []*cmd.SmartNIC
			health map[string]*cmd.SmartNICCondition
			err    error
			snIf   cmd.ClusterV1SmartNICInterface
		)
		BeforeEach(func() {
			snIf = ts.tu.APIClient.ClusterV1().SmartNIC()
			health = make(map[string](*cmd.SmartNICCondition))
		})

		It("SmartNIC should be in healthy state", func() {
			snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
			Expect(err).ShouldNot(HaveOccurred())

			// Validate nic is healthy
			Eventually(func() bool {

				numHealthySmartNICs := 0
				for _, snic := range snics {
					for _, cond := range snic.Status.Conditions {
						if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
							cond.Status == cmd.ConditionStatus_TRUE.String() {
							numHealthySmartNICs++
							health[snic.Name] = cond
							By(fmt.Sprintf("SmartNIC [%s] is healthy", snic.Name))
						}
					}
				}

				if numHealthySmartNICs != ts.tu.NumNaplesHosts {
					return false
				}

				By(fmt.Sprintf("ts:%s SmartNIC health status check validated for [%d] nics", time.Now().String(), numHealthySmartNICs))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC health status check failed")

			// Wait for nic-update interval and get the updated object
			// Wait for 5 sec plus a buffer of 200ms to accomadate load/stress in testbed
			time.Sleep(5*time.Second + 200*time.Millisecond)

			// Validate periodic health updates
			Eventually(func() bool {
				snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
				numHealthySmartNICs := 0
				for _, snic := range snics {
					for _, cond := range snic.Status.Conditions {
						if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
							cond.Status == cmd.ConditionStatus_TRUE.String() {
							curr, err := time.Parse(time.RFC3339, cond.LastTransitionTime)
							if err != nil {
								return false
							}
							old, err := time.Parse(time.RFC3339, health[snic.Name].GetLastTransitionTime())
							if err != nil {
								return false
							}

							if curr.Sub(old) < (5 * time.Second) {
								return false
							}
							numHealthySmartNICs++
							By(fmt.Sprintf("ts:%s SmartNIC [%s] reported new health update", time.Now().String(), snic.Name))
						}
					}
				}

				if numHealthySmartNICs != ts.tu.NumNaplesHosts {
					return false
				}

				By(fmt.Sprintf("ts: %s SmartNIC periodic health update validated for [%d] nics", time.Now().String(), numHealthySmartNICs))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC periodic health update check failed")
		})

	})
})
