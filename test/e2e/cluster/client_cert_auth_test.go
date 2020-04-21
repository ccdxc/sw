// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"fmt"
	"net"
	"net/http"
	"time"

	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pensando/sw/api/generated/cluster"

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
				} else if naplesStatus.Status.AdmissionPhase == "" && naplesStatus.Status.Mode == "HOST" {
					By(fmt.Sprintf("ts:%s DSC found in HOST mode, trying to switch to NETWORK mode", time.Now().String()))
					// switch to network mode
					switchToNetworkMode(naplesStatus, naples, httpClient)
					return false
				}
				By(fmt.Sprintf("ts: %s DSC has not been admitted yet, it is in admission-phase: %s, mode: %s", time.Now().String(), naplesStatus.Status.AdmissionPhase, naplesStatus.Status.Mode))
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

func switchToNetworkMode(naplesStatus nmd.DistributedServiceCard, naples string, httpClient *netutils.HTTPClient) {
	agIP := net.ParseIP(ts.tu.FirstNaplesIP).To4()
	gw := net.ParseIP(ts.tu.NameToIPMap[naples])
	naplesStatus.Spec.Mode = nmd.MgmtMode_NETWORK.String()
	naplesStatus.Spec.Controllers = []string{ts.tu.ClusterVIP}
	naplesStatus.Spec.ID = naples
	naplesStatus.Spec.NetworkMode = nmd.NetworkMode_OOB.String()
	// Ensure that a random static IP is given
	naplesStatus.Spec.IPConfig = &cluster.IPConfig{
		IPAddress: agIP.String() + "/24",
		DefaultGW: gw.String(),
	}
	By(fmt.Sprintf("Switching Naples %+v to managed mode", naples))
	_, err := httpClient.Req("POST", fmt.Sprintf("https://%s:8888/api/v1/naples/", ts.tu.NameToIPMap[naples]), naplesStatus, nil)
	if err != nil {
		By(fmt.Sprintf("ts:%s received error: %s, when switching to NETWORK mode", time.Now().String(), err.Error()))
	}
}

var _ = Describe("Client certificates", func() {

	var apiGwAddr string
	BeforeEach(func() {
		apiGwAddr = ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		waitForDSCsToBeAdmitted(apiGwAddr)
	})

	Context("with wild-card audience", func() {
		It("should be allowed", func() {
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
