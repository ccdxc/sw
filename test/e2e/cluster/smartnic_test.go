package cluster

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/netutils"
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
					By(fmt.Sprintf("SmartNIC [%s] is created & admitted, MAC: %s", snic.Name, snic.Status.PrimaryMAC))
				}
				if numAdmittedNICs != ts.tu.NumNaplesHosts {
					return false
				}
				By(fmt.Sprintf("ts:%s SmartNIC admission validated for [%d] nics", time.Now().String(), numAdmittedNICs))
				return true
			}, 90, 1).Should(BeTrue(), "SmartNIC nic-admission check failed")

			// Validate MAC Addresses. Each NIC should have a valid, unique Pensando MAC address
			nicMACMap := make(map[string]string)
			for _, snic := range snics {
				mac := snic.Status.PrimaryMAC
				Expect(netutils.IsPensandoMACAddress(mac)).Should(BeTrue(), fmt.Sprintf("Invalid MAC Address: %s", mac))
				_, ok := nicMACMap[mac]
				Expect(ok).Should(BeFalse(), fmt.Sprintf("SmartNIC %s had duplicate MAC Address: %s, NIC with same MAC: %s", snic.Name, mac, nicMACMap[mac]))
				nicMACMap[mac] = snic.Name
			}
		})
	})

	Context("SmartNIC health status and periodic updates test", func() {
		var (
			err  error
			snIf cmd.ClusterV1SmartNICInterface
		)
		BeforeEach(func() {
			snIf = ts.tu.APIClient.ClusterV1().SmartNIC()
		})

		It("CMD should receive SmartNIC health updates and flag unresponsive NICs", func() {
			Expect(err).ShouldNot(HaveOccurred())
			ctx := context.Background()
			// Validate nic is healthy
			validateNICHealth(ctx, snIf, ts.tu.NumNaplesHosts, cmd.ConditionStatus_TRUE)

			// Pause the NIC containers, verify that CMD marks health as "unknown"
			for _, nicContainer := range ts.tu.NaplesNodes {
				By(fmt.Sprintf("Pausing NIC container %s", nicContainer))
				ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause %s", nicContainer))
			}

			// Validate nic health gets updated
			validateNICHealth(ctx, snIf, ts.tu.NumNaplesHosts, cmd.ConditionStatus_UNKNOWN)

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
			validateNICHealth(ctx, snIf, ts.tu.NumNaplesHosts, cmd.ConditionStatus_TRUE)

			// Check that LastTransitionTime has been updated
			Eventually(func() bool {
				snics, err := snIf.List(context.Background(), &api.ListWatchOptions{})
				if err != nil {
					By(fmt.Sprintf("Error getting list of NICs: %v", err))
					return false
				}
				for _, snic := range snics {
					for _, condition := range snic.Status.Conditions {
						curr, err := time.Parse(time.RFC3339, condition.LastTransitionTime)
						if err != nil {
							By(fmt.Sprintf("ts:%s SmartNIC [%s] invalid LastTransitionTime %v", time.Now().String(), snic.Name, condition.LastTransitionTime))
							return false
						}
						if curr.Sub(wakeTime) <= 0 {
							By(fmt.Sprintf("ts:%s SmartNIC [%s] invalid LastTransitionTime %v, ref: %v", time.Now().String(), snic.Name, curr, wakeTime))
							return false
						}
					}
				}
				return true
			}, 15, 1).Should(BeTrue(), "SmartNIC condition condition.LastTransitionTime check failed")
		})
	})
})

func validateNICHealth(ctx context.Context, snIf cmd.ClusterV1SmartNICInterface, expectedNumNICS int, status cmd.ConditionStatus) {
	Eventually(func() bool {
		snics, err := snIf.List(ctx, &api.ListWatchOptions{})
		if err != nil {
			By(fmt.Sprintf("Error getting list of NICs: %v", err))
			return false
		}
		numMatchingNICs := 0
		for _, snic := range snics {
			for _, cond := range snic.Status.Conditions {
				if cond.Type == cmd.SmartNICCondition_HEALTHY.String() &&
					cond.Status == status.String() {
					numMatchingNICs++
					By(fmt.Sprintf("SmartNIC [%s] is %s", snic.Name, status.String()))
				}
			}
		}
		if numMatchingNICs != expectedNumNICS {
			By(fmt.Sprintf("Found %d NICS with expected health status %s, want: %d", numMatchingNICs, status.String(), expectedNumNICS))
			return false
		}
		By(fmt.Sprintf("ts:%s SmartNIC health status check validated for [%d] nics", time.Now().String(), numMatchingNICs))
		return true
	}, 90, 1).Should(BeTrue(), "SmartNIC health status check failed")
}
