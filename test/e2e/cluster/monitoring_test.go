package cluster

import (
	"context"
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"

	apiclient "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("MirrorSession Tests", func() {
	Context("MirrorSession CRUD Tests", func() {
		var testMirrorSessions = []monitoring.MirrorSession{
			{
				ObjectMeta: api.ObjectMeta{
					Name:   "TestMirrorSession1",
					Tenant: "default",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "MirrorSession",
					APIVersion: "v1",
				},
				Spec: monitoring.MirrorSessionSpec{
					PacketSize:    128,
					PacketFilters: []string{"ALL_PKTS"},
					StopConditions: monitoring.MirrorStopConditions{
						MaxPacketCount: 1000,
						ExpiryDuration: "5s",
					},
					Collectors: []monitoring.MirrorCollector{
						{
							Type: "ERSPAN",
							ExportCfg: api.ExportConfig{
								Destination: "100.1.1.1",
							},
						},
					},
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"10.1.1.10"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								Ports: []string{"TCP"},
							},
						},
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"10.2.2.20"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								Ports: []string{"UDP"},
							},
						},
					},
				},
			},
		}
		// XXX:Need to create other objects - VRF/EPs/routes etc for testing with packets

		var (
			mirrorRestIf monitoring.MonitoringV1MirrorSessionInterface
		)

		BeforeEach(func() {
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restSvc, err := apiclient.NewRestAPIClient(apiGwAddr)
			if err == nil {
				mirrorRestIf = restSvc.MonitoringV1().MirrorSession()
			}
			Expect(err).ShouldNot(HaveOccurred())
			Expect(mirrorRestIf).ShouldNot(Equal(nil))
		})
		It("Should run mirror sessions Test 1", func() {
			// create mirror session with exp duration of 5s
			// check that it starts running and then stops after 5s
			// delete mirror session

			ctx := ts.tu.NewLoggedInContext(context.Background())
			ms := testMirrorSessions[0]
			By("Creating MirrorSession ------")
			_, err := mirrorRestIf.Create(ctx, &ms)
			Expect(err).ShouldNot(HaveOccurred())

			By("Checking that MirrorSession has started------")
			Eventually(func() bool {
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil || tms.Status.State != monitoring.MirrorSessionState_RUNNING.String() {
					By(".")
					return false
				}
				return true
			}, 2, 1).Should(BeTrue(), fmt.Sprintf("Failed to start %s", ms.Name))

			By("Checking that MirrorSession has stopped using expriy time------")
			Eventually(func() bool {
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil || tms.Status.State != monitoring.MirrorSessionState_STOPPED.String() {
					return false
				}
				return true
			}, 10, 1).Should(BeTrue(), fmt.Sprintf("Failed to stop %s", ms.Name))
			By("Deleting MirrorSession ------")
			_, err = mirrorRestIf.Delete(ctx, &ms.ObjectMeta)
			Expect(err).ShouldNot(HaveOccurred())
		})
	})
})
