package cluster

import (
	"fmt"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/search"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("diagnostics tests", func() {
	Context("spyglass logs", func() {
		var modObj *diagnostics.Module
		BeforeEach(func() {
			var err error
			var node string
			Eventually(func() error {
				node = ts.tu.GetNodeForService(globals.Spyglass)
				modObj, err = ts.restSvc.DiagnosticsV1().Module().Get(ts.loggedInCtx, &api.ObjectMeta{Name: fmt.Sprintf("%s-%s", node, globals.Spyglass)})
				return err
			}, 10, 1).Should(BeNil())
		})
		It("check log query", func() {
			modObj.Spec.LogLevel = diagnostics.ModuleSpec_Debug.String()
			var updatedModObj *diagnostics.Module
			var err error
			Eventually(func() error {
				updatedModObj, err = ts.restSvc.DiagnosticsV1().Module().Update(ts.loggedInCtx, modObj)
				return err
			}, 10, 1).Should(BeNil())
			Expect(modObj.Spec.LogLevel).Should(Equal(diagnostics.ModuleSpec_Debug.String()))
			// wait for spyglass to receive watch event
			time.Sleep(3 * time.Second)
			// create debug logs by searching for successful login audit event
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{svc.LoginAction},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{audit.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string(auth.KindUser)},
							},
							{
								Key:      "resource.tenant",
								Operator: "equals",
								Values:   []string{globals.DefaultTenant},
							},
							{
								Key:      "resource.name",
								Operator: "equals",
								Values:   []string{ts.tu.User},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
			}
			Eventually(func() error {
				resp := testutils.AuditSearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs for [%s|%s] successful login", globals.DefaultTenant, ts.tu.User)
				}
				events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[search.Category_Monitoring.String()].Kinds[auth.Permission_AuditEvent.String()].Entries
				for _, event := range events {
					if (event.Object.Action == svc.LoginAction) &&
						(event.Object.Outcome == audit.Outcome_Success.String()) &&
						(event.Object.User.Name == ts.tu.User) &&
						(event.Object.User.Tenant == globals.DefaultTenant) {
						return nil
					}
				}
				return fmt.Errorf("no audit logs for [%s|%s] successful login", globals.DefaultTenant, ts.tu.User)
			}, 30, 1).Should(BeNil())
			// query logs through Debug action
			Eventually(func() error {
				type debugResponse struct {
					Diagnostics map[string]interface{} `json:"diagnostics"`
				}
				resp := debugResponse{}
				var respStr string
				if respStr, err = ts.tu.Debug(ts.loggedInCtx, &diagnostics.DiagnosticsRequest{
					ObjectMeta: api.ObjectMeta{Name: updatedModObj.Name},
					Query:      diagnostics.DiagnosticsRequest_Log.String(),
				}, &resp); err != nil {
					return err
				}
				if !strings.Contains(respStr, "\"level\":\"debug\"") &&
					!strings.Contains(respStr, "\"level\":\"info\"") &&
					!strings.Contains(respStr, "\"level\":\"error\"") {
					return fmt.Errorf("no logs returned: {%v}", respStr)
				}
				return nil
			}, 30, 1).Should(BeNil())
			// restore info log level
			Eventually(func() error {
				updatedModObj.Spec.LogLevel = diagnostics.ModuleSpec_Info.String()
				modObj, err = ts.restSvc.DiagnosticsV1().Module().Update(ts.loggedInCtx, updatedModObj)
				return err
			}, 10, 1).Should(BeNil())
		})
	})
	Context("npm", func() {
		var modObj *diagnostics.Module
		BeforeEach(func() {
			var err error
			var node string
			Eventually(func() error {
				node = ts.tu.GetNodeForService(globals.Npm)
				modObj, err = ts.restSvc.DiagnosticsV1().Module().Get(ts.loggedInCtx, &api.ObjectMeta{Name: fmt.Sprintf("%s-%s", node, globals.Npm)})
				return err
			}, 10, 1).Should(BeNil())
		})
		It("check log query", func() {
			modObj.Spec.LogLevel = diagnostics.ModuleSpec_Debug.String()
			var updatedModObj *diagnostics.Module
			var err error
			Eventually(func() error {
				updatedModObj, err = ts.restSvc.DiagnosticsV1().Module().Update(ts.loggedInCtx, modObj)
				return err
			}, 10, 1).Should(BeNil())
			Expect(modObj.Spec.LogLevel).Should(Equal(diagnostics.ModuleSpec_Debug.String()))
			// query logs through Debug action
			Eventually(func() error {
				type debugResponse struct {
					Diagnostics map[string]interface{} `json:"diagnostics"`
				}
				resp := debugResponse{}
				var respStr string
				if respStr, err = ts.tu.Debug(ts.loggedInCtx, &diagnostics.DiagnosticsRequest{
					ObjectMeta: api.ObjectMeta{Name: updatedModObj.Name},
					Query:      diagnostics.DiagnosticsRequest_Log.String(),
				}, &resp); err != nil {
					return err
				}
				if !strings.Contains(respStr, "\"level\":\"debug\"") &&
					!strings.Contains(respStr, "\"level\":\"info\"") &&
					!strings.Contains(respStr, "\"level\":\"error\"") {
					return fmt.Errorf("no logs returned: {%v}", respStr)
				}
				return nil
			}, 30, 1).Should(BeNil())
			// restore info log level
			Eventually(func() error {
				updatedModObj.Spec.LogLevel = diagnostics.ModuleSpec_Info.String()
				modObj, err = ts.restSvc.DiagnosticsV1().Module().Update(ts.loggedInCtx, updatedModObj)
				return err
			}, 10, 1).Should(BeNil())
		})
		It("check stats query", func() {
			var err error
			// query stats through Debug action
			Eventually(func() error {
				type debugResponse struct {
					Diagnostics map[string]interface{} `json:"diagnostics"`
				}
				resp := debugResponse{}
				var respStr string
				if respStr, err = ts.tu.Debug(ts.loggedInCtx, &diagnostics.DiagnosticsRequest{
					ObjectMeta: api.ObjectMeta{Name: modObj.Name},
					Query:      diagnostics.DiagnosticsRequest_Stats.String(),
				}, &resp); err != nil {
					return err
				}
				if !strings.Contains(respStr, "\"cmdline\":") ||
					!strings.Contains(respStr, "\"cpustats\":") ||
					!strings.Contains(respStr, "\"memstats\":") {
					return fmt.Errorf("no stats returned: {%v}", respStr)
				}
				return nil
			}, 30, 1).Should(BeNil())
		})
	})
})
