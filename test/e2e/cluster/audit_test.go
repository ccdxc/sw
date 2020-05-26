package cluster

import (
	"bytes"
	"fmt"
	"strings"

	"github.com/gogo/protobuf/types"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/bulkedit"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/staging"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("audit tests", func() {
	Context("node auth token", func() {
		BeforeEach(func() {
			Eventually(func() error {
				_, err := testutils.GetNodeAuthToken(ts.loggedInCtx, ts.tu.APIGwAddr, []string{"*"})
				return err
			}, 10, 1).Should(BeNil())

		})
		It("check audit log", func() {
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{strings.Title(auth.Permission_Read.String())},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{audit.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{auth.Permission_TokenAuth.String()},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{audit.Stage_RequestProcessing.String()},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
				Aggregate:  true,
			}
			var events []*testutils.AuditEntry
			Eventually(func() error {
				resp := testutils.AuditSearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs for node auth token")
				}
				events = resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
				return nil
			}, 30, 1).Should(BeNil())
			for _, event := range events {
				Expect(event.Object.Action == auth.Permission_Read.String() &&
					event.Object.Resource.Kind == auth.Permission_TokenAuth.String() &&
					event.Object.Outcome == audit.Outcome_Success.String() &&
					event.Object.Stage == audit.Stage_RequestProcessing.String()).Should(BeTrue())
				eventDetails := &audit.AuditEvent{}
				Eventually(func() error {
					return testutils.GetAuditEvent(ts.loggedInCtx, ts.tu.APIGwAddr, event.Object.UUID, eventDetails)
				}, 10, 1).Should(BeNil())
				Expect(eventDetails.ResponseObject == "").Should(BeTrue())
			}

		})
		It("check audit log file", func() {
			var buf bytes.Buffer
			for _, node := range ts.tu.QuorumNodes {
				ip := ts.tu.NameToIPMap[node]
				logs := ts.tu.CommandOutput(ip, "cat /var/log/pensando/audit.log")
				buf.WriteString(logs)
			}
			Expect(buf.String()).ShouldNot(BeEmpty())
		})
		It("bulk edit audit", func() {
			// Create staging buffer
			stagingBufferName := "E2ETestStagingBuffer"
			Eventually(func() error {
				_, err := ts.restSvc.StagingV1().Buffer().Create(ts.loggedInCtx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: stagingBufferName, Tenant: globals.DefaultTenant}})
				return err
			}, 10, 1).Should(BeNil())

			var items []*bulkedit.BulkEditItem
			numItems := 1001
			for i := 0; i < numItems; i++ {
				netw1 := network.Network{
					TypeMeta: api.TypeMeta{
						Kind:       "Network",
						APIVersion: "v1",
					},
					ObjectMeta: api.ObjectMeta{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Name:      fmt.Sprintf("TestStagingNetw%d", i),
					},
					Spec: network.NetworkSpec{
						Type:        network.NetworkType_Bridged.String(),
						IPv4Subnet:  "10.1.1.1/24",
						IPv4Gateway: "10.1.1.1",
						VlanID:      uint32(i + 100),
					},
				}

				n1, err := types.MarshalAny(&netw1)
				Expect(err).Should(BeNil())
				items = append(items, &bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_CREATE.String(),
					Object: &api.Any{Any: *n1},
				})
			}

			bulkEditReq := &staging.BulkEditAction{
				ObjectMeta: api.ObjectMeta{
					Name:      stagingBufferName,
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				},
				Spec: bulkedit.BulkEditActionSpec{
					Items: items,
				},
			}

			bEResp, err := ts.restSvc.StagingV1().Buffer().Bulkedit(ts.loggedInCtx, bulkEditReq)
			Expect(err).Should(BeNil())
			Expect(bEResp.Status.ValidationResult == "success").Should(BeTrue())

			ca := staging.CommitAction{}
			ca.ObjectMeta = bulkEditReq.ObjectMeta
			_, err = ts.restSvc.StagingV1().Buffer().Commit(ts.loggedInCtx, &ca)
			Expect(err).Should(BeNil())

			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{strings.Title(auth.Permission_Create.String())},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{audit.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string(network.KindNetwork)},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{audit.Stage_RequestProcessing.String()},
							},
						},
					},
				},
				From:       0,
				MaxResults: int32(numItems),
				Aggregate:  true,
			}
			var events []*testutils.AuditEntry
			Eventually(func() error {
				resp := testutils.AuditSearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits < int64(numItems) {
					return fmt.Errorf("expected [%d] audit logs for bulk edit of network create, got [%d]", numItems, resp.ActualHits)
				}
				events = resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
				return nil
			}, 30, 1).Should(BeNil())
			for _, event := range events {
				Expect(event.Object.Action == auth.Permission_Create.String() &&
					event.Object.Resource.Kind == string(network.KindNetwork) &&
					event.Object.Outcome == audit.Outcome_Success.String() &&
					event.Object.Stage == audit.Stage_RequestProcessing.String()).Should(BeTrue())
			}

			Eventually(func() error {
				_, err := ts.restSvc.StagingV1().Buffer().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: stagingBufferName, Tenant: globals.DefaultTenant})
				return err
			}, 10, 1).Should(BeNil())
			for i := 0; i < numItems; i++ {
				Eventually(func() error {
					_, err := ts.restSvc.NetworkV1().Network().Delete(ts.loggedInCtx, &api.ObjectMeta{Tenant: globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Name:      fmt.Sprintf("TestStagingNetw%d", i)})
					return err
				}, 10, 1).Should(BeNil())
			}
		})
	})
})
