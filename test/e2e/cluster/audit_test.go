package cluster

import (
	"bytes"
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/search"
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
	})
})
