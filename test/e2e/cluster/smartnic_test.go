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
					By(fmt.Sprintf("Expected %v, Found %v, nics: %+v", ts.tu.NumNaplesHosts, len(snics), snics))
					return false
				}
				By(fmt.Sprintf("ts:%s SmartNIC creation validated for [%d] nics", time.Now().String(), len(snics)))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC object creation failed")

			// Validate nic-admission status
			Eventually(func() bool {
				numAdmittedNICs := 0
				for _, snic := range snics {
					if snic.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
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

		It("CMD should receive SmartNIC health updates and flag unresponsive NICs", func() {
			Expect(err).ShouldNot(HaveOccurred())
			// Validate nic is healthy
			Eventually(func() bool {
				snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
				numHealthySmartNICs := 0
				for _, snic := range snics {
					for _, cond := range snic.Status.Conditions {
						if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
							cond.Status == cmd.ConditionStatus_TRUE.String() {
							numHealthySmartNICs++
							health[snic.Name] = &cond
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

			// Pause the NIC containers, verify that CMD marks health as "unknown"
			for _, nicContainer := range ts.tu.NaplesNodes {
				By(fmt.Sprintf("Pausing NIC container %s", nicContainer))
				ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause %s", nicContainer))
			}
			// Validate nic health gets updated
			Eventually(func() bool {
				snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
				numUnresponsiveSmartNICs := 0
				for _, snic := range snics {
					for _, cond := range snic.Status.Conditions {
						if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
							cond.Status == cmd.ConditionStatus_UNKNOWN.String() {
							numUnresponsiveSmartNICs++
							health[snic.Name] = &cond
							By(fmt.Sprintf("SmartNIC [%s] health is unknown", snic.Name))
						}
					}
				}
				if numUnresponsiveSmartNICs != ts.tu.NumNaplesHosts {
					return false
				}
				By(fmt.Sprintf("ts:%s SmartNIC health status update validated for [%d] nics", time.Now().String(), numUnresponsiveSmartNICs))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC health status update failed")

			// CMD should update health of NIC after it comes back
			wakeTime := time.Now()

			// LastTransitionTime granularity is seconds, so we need to wait at least 1 sec to make
			// sure that next health update has a timestamp that is in the future w.r.t. to time.Now()
			time.Sleep(time.Second)
			// Unpause the NIC containers, verify that NICs goes back to "healthy"
			// Validate periodic health updates
			for _, nicContainer := range ts.tu.NaplesNodes {
				By(fmt.Sprintf("Unpausing NIC container %s", nicContainer))
				ts.tu.LocalCommandOutput(fmt.Sprintf("docker unpause %s", nicContainer))
			}
			Eventually(func() bool {
				snics, err = snIf.List(context.Background(), &api.ListWatchOptions{})
				numHealthySmartNICs := 0
				for _, snic := range snics {
					for _, cond := range snic.Status.Conditions {
						if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
							cond.Status == cmd.ConditionStatus_TRUE.String() {
							curr, err := time.Parse(time.RFC3339, cond.LastTransitionTime)
							if err != nil {
								By(fmt.Sprintf("ts:%s SmartNIC [%s] invalid LastTransitionTime %v", time.Now().String(), snic.Name, cond.LastTransitionTime))
								return false
							}
							if curr.Sub(wakeTime) <= 0 {
								By(fmt.Sprintf("ts:%s SmartNIC [%s] invalid LastTransitionTime %v, ref: %v", time.Now().String(), snic.Name, curr, wakeTime))
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
