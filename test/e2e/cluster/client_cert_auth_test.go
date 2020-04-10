// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"fmt"
	"net/http"
	"time"

	"github.com/pensando/sw/venice/globals"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/nic/agent/protos/nmd"

	"github.com/pensando/sw/test/utils"
)

const testMac = "random.test.mac"

func waitForDSCsToBeAdmitted(apiGwAddr string) {
	Eventually(func() bool {
		httpClient, err := utils.GetNodeAuthTokenHTTPClient(ts.loggedInCtx, apiGwAddr, []string{"*"})
		if err != nil {
			By(fmt.Sprintf("ts:%s Unable to create http client attached with client certificate fetched from Venice, error: %s", time.Now().String(), err.Error()))
			return false
		}
		for _, naples := range ts.tu.NaplesNodes {
			var naplesStatus nmd.DistributedServiceCard
			status, err := httpClient.Req("GET", fmt.Sprintf("https://%s:8888/api/v1/naples/", ts.tu.NameToIPMap[naples]), nil, &naplesStatus)
			if err == nil {
				if naplesStatus.Status.AdmissionPhase == "admitted" {
					By(fmt.Sprintf("ts:%s DSC has been admitted", time.Now().String()))
					continue
				}
				By(fmt.Sprintf("ts: %s DSC has not been admitted yet", time.Now().String()))
				return false
			}
			if status != http.StatusUnauthorized {
				By(fmt.Sprintf("ts: %s error fetching DSC info, error: %s", time.Now().String(), err.Error()))
				return false
			}
			By(fmt.Sprintf("ts:%s wild-card audience is rejected, naples node %s must have been admitted", time.Now().String(), naples))
		}
		return true
	}, 1200, 5).Should(BeTrue(), "Unable to verify all DSCs have been admitted")
}

var _ = Describe("Client certificates", func() {

	var apiGwAddr string
	BeforeEach(func() {
		Skip("Skipping client cert auth tests")
		apiGwAddr = ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		waitForDSCsToBeAdmitted(apiGwAddr)
	})

	Context("with wild-card audience", func() {
		It("should be allowed", func() {
			Skip("Skipping client cert auth tests")
			Eventually(func() bool {
				// get token with * audience from Venice/api-gw
				httpClient, err := utils.GetNodeAuthTokenHTTPClient(ts.loggedInCtx, apiGwAddr, []string{"*"})
				if err != nil {
					By(fmt.Sprintf("ts:%s Unable to create http client attached with client certificate fetched from Venice", time.Now().String()))
					return false
				}
				// invoke NMD command using the above generated token file on all naples node
				By(fmt.Sprintf("ts:%s check if the auth-token is allowed by all Naples cards", time.Now().String()))
				for _, naples := range ts.tu.NaplesNodes {
					var naplesStatus nmd.DistributedServiceCard
					status, err := httpClient.Req("GET", fmt.Sprintf("https://%s:8888/api/v1/naples/", ts.tu.NameToIPMap[naples]), nil, &naplesStatus)
					Expect(err).ShouldNot(HaveOccurred(), fmt.Sprintf("Got error: %v", err))
					Expect(status).Should(Equal(http.StatusOK), fmt.Sprintf("Got status: %d", status))
				}
				return true
			}, 300, 5).Should(BeTrue(), "Unable to verify mac-based authorization with wild-card audience")
		})
	})

	Context("with specific mac-address audience", func() {
		It("should be allowed only when mac-address in token audience matches request serving device mac", func() {
			Skip("Skipping client cert auth tests")
			Eventually(func() bool {
				// get mac-address to allow
				allowedNaplesNode := ts.tu.NaplesNodes[0]
				allowedMACAddress, ok := ts.naplesNameToMACMap[allowedNaplesNode]
				Expect(ok).Should(BeTrue(), fmt.Sprintf("No mac address found for a naples node: %s", allowedNaplesNode))

				// get token with specific audience from Venice/api-gw
				httpClient, err := utils.GetNodeAuthTokenHTTPClient(ts.loggedInCtx, apiGwAddr, []string{allowedMACAddress})
				if err != nil {
					By(fmt.Sprintf("ts:%s Unable to create http client attached with client certificate fetched from Venice", time.Now().String()))
					return false
				}

				// invoke NMD command using the above generated token file on all naples
				By(fmt.Sprintf("ts:%s Verify mac-based token authorization by Naples cards", time.Now().String()))
				for _, naples := range ts.tu.NaplesNodes {
					nodeIP := ts.tu.NameToIPMap[naples]
					var naplesStatus nmd.DistributedServiceCard
					status, err := httpClient.Req("GET", fmt.Sprintf("https://%s:8888/api/v1/naples/", nodeIP), nil, &naplesStatus)
					if naples == allowedNaplesNode {
						Expect(err).ShouldNot(HaveOccurred(), fmt.Sprintf("Got error: %v", err))
						Expect(status).Should(Equal(http.StatusOK), fmt.Sprintf("Got status: %d", status))
					} else {
						Expect(err).Should(HaveOccurred(), fmt.Sprintf("Expected to get an error from naples node: %s", naples))
						By(fmt.Sprintf("ts:%s Got error: %s", time.Now().String(), err.Error()))
						if status != http.StatusUnauthorized {
							By(fmt.Sprintf("ts:%s Got %d status but expected %d", time.Now().String(), status, http.StatusUnauthorized))
							return false
						}
						By(fmt.Sprintf("ts:%s Response status: %d, from naples node: %s", time.Now().String(), status, naples))
					}
				}
				return true
			}, 300, 5).Should(BeTrue(), "Unable to verify mac based authorization with specific audience")
		})

		It("should be rejected when mac-address in token audience does not match request serving device mac", func() {
			Skip("Skipping client cert auth tests")
			Eventually(func() bool {
				// get token with specific audience from Venice/api-gw
				httpClient, err := utils.GetNodeAuthTokenHTTPClient(ts.loggedInCtx, apiGwAddr, []string{testMac})
				if err != nil {
					By(fmt.Sprintf("ts:%s Unable to create http client attached with client certificate fetched from Venice", time.Now().String()))
					return false
				}

				// invoke NMD command using the above generated token file on all naples
				By(fmt.Sprintf("ts:%s Verify mac-based token authorization by Naples cards", time.Now().String()))
				for _, naples := range ts.tu.NaplesNodes {
					nodeIP := ts.tu.NameToIPMap[naples]
					var naplesStatus nmd.DistributedServiceCard
					status, err := httpClient.Req("GET", fmt.Sprintf("https://%s:8888/api/v1/naples/", nodeIP), nil, naplesStatus)
					Expect(err).Should(HaveOccurred(), fmt.Sprintf("Expected to get an error from naples node: %s", naples))
					By(fmt.Sprintf("ts:%s Got error: %s", time.Now().String(), err.Error()))
					if status != http.StatusUnauthorized {
						By(fmt.Sprintf("ts:%s Got %d status but expected %d", time.Now().String(), status, http.StatusUnauthorized))
						return false
					}
					By(fmt.Sprintf("ts:%s Response status: %d, from naples node: %s", time.Now().String(), status, naples))
				}
				return true
			}, 300, 5).Should(BeTrue(), "Unable to verify client cert rejection")
		})
	})
})
