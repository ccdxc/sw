package cluster

import (
	"context"
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("mirror session tests", func() {
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
					PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
					Collectors: []monitoring.MirrorCollector{
						{
							Type: "ERSPAN",
							ExportCfg: &monitoring.MirrorExportConfig{
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
								ProtoPorts: []string{"TCP/5555"},
							},
						},
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"10.2.2.20"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"UDP/5555"},
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

		It("Check max mirror sessions", func() {
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			ms := testMirrorSessions[0]

			for i := 0; i < statemgr.MaxMirrorSessions+1; i++ {
				ms.Name = fmt.Sprintf("max-mirror-%d", i+1)
				ms.Spec.Collectors = []monitoring.MirrorCollector{
					{
						Type: "ERSPAN",
						ExportCfg: &monitoring.MirrorExportConfig{
							Destination: fmt.Sprintf("192.168.30.1"),
						},
					},
				}

				By(fmt.Sprintf("Creating MirrorSession %v", ms.Name))
				_, err := mirrorRestIf.Create(ctx, &ms)
				if i < statemgr.MaxMirrorSessions {
					Expect(err).ShouldNot(HaveOccurred())
				} else {
					Expect(err).Should(HaveOccurred())
				}
			}
			s, err := mirrorRestIf.List(ctx, &api.ListWatchOptions{})
			Expect(err).ShouldNot(HaveOccurred())
			for _, i := range s {
				_, err := mirrorRestIf.Delete(ctx, i.GetObjectMeta())
				Expect(err).ShouldNot(HaveOccurred())
			}
		})

		It("Should run mirror sessions Test 1", func() {
			// create mirror session with exp duration of 5s
			// check that it starts active and then stops after 5s
			// delete mirror session

			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			ms := testMirrorSessions[0]
			By("Creating MirrorSession ------")
			_, err := mirrorRestIf.Create(ctx, &ms)
			Expect(err).ShouldNot(HaveOccurred())

			By("Checking that MirrorSession has started------")
			Eventually(func() bool {
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil {
					By(fmt.Sprintf("GET err:%s", err))
					return false
				}
				if tms.Status.ScheduleState != monitoring.MirrorSessionState_ACTIVE.String() {
					By(fmt.Sprintf("mirror state: %v", tms.Status.ScheduleState))
					return false
				}
				return true
			}, 5, 1).Should(BeTrue(), fmt.Sprintf("Failed to start %s", ms.Name))

			/* XXX Reenable after session expiry is added in NAPLES
			By("Checking that MirrorSession has stopped using expriy time------")
			Eventually(func() bool {
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil {
					By(fmt.Sprintf("GET err:%s", err))
					return false
				}
				if tms.Status.ScheduleState != monitoring.MirrorSessionState_STOPPED.String() {
					By(fmt.Sprintf("mirror state: %v", tms.Status.ScheduleState))
					return false
				}
				return true
			}, 10, 1).Should(BeTrue(), fmt.Sprintf("Failed to stop %s", ms.Name)) */
			By("Deleting MirrorSession ------")
			_, err = mirrorRestIf.Delete(ctx, &ms.ObjectMeta)
			Expect(err).ShouldNot(HaveOccurred())
		})

		It("Should run mirror sessions after shcedule time", func() {
			// create mirror session with exp duration of 5s
			// check that it starts active and then stops after 5s
			// delete mirror session
			scheduledTestCase := monitoring.MirrorSession{
				ObjectMeta: api.ObjectMeta{
					Name:   "TestMirrorSession2",
					Tenant: "default",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "MirrorSession",
					APIVersion: "v1",
				},
				Spec: monitoring.MirrorSessionSpec{
					PacketSize:    128,
					PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
					Collectors: []monitoring.MirrorCollector{
						{
							Type: "ERSPAN",
							ExportCfg: &monitoring.MirrorExportConfig{
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
								ProtoPorts: []string{"TCP/5555"},
							},
						},
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"10.2.2.20"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"UDP/5555"},
							},
						},
					},
					// for this test case
					// add schedule time after 30 seconds
					// test mirror session state
					StartConditions: monitoring.MirrorStartConditions{
						ScheduleTime: &api.Timestamp{
							Timestamp: types.Timestamp{
								Seconds: time.Now().Unix() + 60,
							},
						},
					},
				},
			}

			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			ms := scheduledTestCase
			By("Creating MirrorSession ------")
			_, err := mirrorRestIf.Create(ctx, &ms)
			Expect(err).ShouldNot(HaveOccurred())

			By("Checking whether MirrorSession has started after scheduled time or not------")
			Eventually(func() bool {
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil {
					By(fmt.Sprintf("GET err:%s", err))
					return false
				}

				// raise error if session start before scheduled time
				// the expected state should be SCHEDULED
				if tms.Status.ScheduleState != monitoring.MirrorSessionState_SCHEDULED.String() {
					By(fmt.Sprintf("unexpected mirror session state: %v before scheduled time", tms.Status.ScheduleState))
					return false
				}
				return true
			}, 30, 5).Should(BeTrue(), fmt.Sprintf("Failed to get SCHEDULED state for scheduled mirror session %s", ms.Name))

			Eventually(func() bool {
				// get the new state
				tms, err := mirrorRestIf.Get(ctx, &ms.ObjectMeta)
				if err != nil {
					By(fmt.Sprintf("GET err:%s", err))
					return false
				}

				// raise error if session doesn't start after scheduled time
				// the expected state should be ACTIVE
				if tms.Status.ScheduleState != monitoring.MirrorSessionState_ACTIVE.String() {
					By(fmt.Sprintf("unexpected mirror session state: %v after scheduled time", tms.Status.ScheduleState))
					return false
				}
				return true
			}, 180, 30).Should(BeTrue(), fmt.Sprintf("Failed to get mirror session active state after scheduled time %s", ms.Name))

			// Delete mirror session anyway to avoid the aftermath on the other tests
			By("Deleting MirrorSession ------")
			Eventually(func() bool {
				_, err = mirrorRestIf.Delete(ctx, &ms.ObjectMeta)
				if err != nil {
					By(fmt.Sprintf("GET err:%s", err))
					return false
				}
				return true
			}, 30, 5).Should(BeTrue(), fmt.Sprintf("Failed to delete mirror session %s after testing process", ms.Name))
		})
	})
})
