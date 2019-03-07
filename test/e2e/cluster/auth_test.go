package cluster

import (
	"context"
	"fmt"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/login"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
)

var _ = Describe("auth tests", func() {
	Context("radius authentication ", func() {
		BeforeEach(func() {
			var authPolicy *auth.AuthenticationPolicy
			var err error
			Eventually(func() error {
				authPolicy, err = ts.restSvc.AuthV1().AuthenticationPolicy().Get(ts.loggedInCtx, &api.ObjectMeta{})
				return err
			}, 10, 1).Should(BeNil())
			// add radius config
			authPolicy.Spec.Authenticators.Radius = &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{
					{
						Url:        ts.tu.Radius.URL,
						Secret:     ts.tu.Radius.NasSecret,
						AuthMethod: auth.Radius_PAP.String(),
					},
				},
			}
			authPolicy.Spec.Authenticators.AuthenticatorOrder = []string{"LOCAL", "RADIUS"}
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().AuthenticationPolicy().Update(ts.loggedInCtx, authPolicy)
				return err
			}, 10, 1).Should(BeNil())
			// updating auth policy re-generates JWT secret so login again to a get new JWT
			ts.loggedInCtx = ts.tu.NewLoggedInContext(context.TODO())
			// TODO: Create a tenant if radius config specifies a non-default tenant
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().RoleBinding().Create(ts.loggedInCtx,
					login.NewRoleBinding("RadiusAdminRoleBinding", ts.tu.Radius.Tenant, globals.AdminRole, "", strings.Join(ts.tu.Radius.UserGroups, ",")))
				return err
			}, 10, 1).Should(BeNil())
		})
		It("check audit logs", func() {
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
				resp := search.SearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs for [%s|%s] successful login", globals.DefaultTenant, ts.tu.User)
				}
				return err
			}, 30, 1).Should(BeNil())
			query = &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{auth.Permission_Update.String()},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{audit.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string(auth.KindAuthenticationPolicy)},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
			}
			Eventually(func() error {
				resp := search.SearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs for authentication policy update")
				}
				return err
			}, 30, 1).Should(BeNil())
			// query by category
			query = &search.SearchRequest{
				Query: &search.SearchQuery{
					Categories: []string{search.Category_Monitoring.String()},
				},
				From:       0,
				MaxResults: 50,
			}
			Eventually(func() error {
				resp := search.SearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs retrieved for monitoring category query")
				}
				if len(resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[search.Category_Monitoring.String()].Kinds[auth.Permission_AuditEvent.String()].Entries) == 0 {
					return fmt.Errorf("no audit events found for monitoring category search")
				}
				return err
			}, 30, 1).Should(BeNil())
		})
		It("check login failure event", func() {
			_, err := login.NewLoggedInContext(context.TODO(), ts.tu.APIGwAddr, &auth.PasswordCredential{Username: ts.tu.User, Password: "incorrect", Tenant: globals.DefaultTenant})
			Expect(err).Should(HaveOccurred())
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Categories: []string{search.Category_Monitoring.String()},
					Kinds:      []string{auth.Permission_Event.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "type",
								Operator: "equals",
								Values:   []string{auth.LoginFailed},
							},
							{
								Key:      "meta.creation-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-3 * time.Second).Format(time.RFC3339Nano)},
							},
							{
								Key:      "meta.creation-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
			}
			Eventually(func() error {
				resp := testutils.EventSearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no events found for failed login attempt")
				}
				events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[search.Category_Monitoring.String()].Kinds[auth.Permission_Event.String()].Entries

				for _, evt := range events {
					if strings.Contains(evt.Object.Message, fmt.Sprintf("%s|%s", globals.DefaultTenant, ts.tu.User)) {
						return nil
					}
				}
				return fmt.Errorf("no events found for failed login attempt in default tenant")
			}, 30, 1).Should(BeNil())
		})
		It("successful radius auth", func() {
			// TODO: Enable this test once we can get ACS to accept connection from IP subnet of API Gw in CI. Currently ACS accepts 192.168.0.0/16, 10.3.0.0/16 but it doesn't seem to work for connections
			// coming from API Gw in CI. This test works on laptop.
			Skip("Skipping radius e2e")
			radiusCtx, err := authntestutils.NewLoggedInContext(context.TODO(), ts.tu.APIGwAddr, &auth.PasswordCredential{Username: ts.tu.Radius.User, Password: ts.tu.Radius.Password})
			Expect(err).ShouldNot(HaveOccurred())
			var user *auth.User
			Eventually(func() error {
				user, err = ts.restSvc.AuthV1().User().Get(radiusCtx, &api.ObjectMeta{Name: ts.tu.Radius.User, Tenant: ts.tu.Radius.Tenant})
				return err
			}, 10, 1).Should(BeNil())
			Expect(user.Name).Should(Equal(ts.tu.Radius.User))
			Expect(user.Tenant).Should(Equal(ts.tu.Radius.Tenant))
			Expect(user.Status.UserGroups).Should(Equal(ts.tu.Radius.UserGroups))
			Expect(user.Spec.Type).Should(Equal(auth.UserSpec_External.String()))
			logintime, err := user.Status.LastLogin.Time()
			Expect(err).ShouldNot(HaveOccurred())
			Expect(time.Now()).Should(BeTemporally("~", logintime, 15*time.Second))
			Expect(user.Status.Authenticators).Should(Equal([]string{auth.Authenticators_RADIUS.String()}))
			Eventually(func() error {
				_, err := ts.restSvc.AuthV1().User().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.Radius.User, Tenant: ts.tu.Radius.Tenant})
				return err
			}, 10, 1).Should(BeNil())
		})
		AfterEach(func() {
			Eventually(func() error {
				_, err := ts.restSvc.AuthV1().RoleBinding().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: "RadiusAdminRoleBinding", Tenant: ts.tu.Radius.Tenant})
				return err
			}, 10, 1).Should(BeNil())

			var authPolicy *auth.AuthenticationPolicy
			Eventually(func() error {
				var err error
				authPolicy, err = ts.restSvc.AuthV1().AuthenticationPolicy().Get(ts.loggedInCtx, &api.ObjectMeta{})
				return err
			}, 10, 1).Should(BeNil())
			authPolicy.Spec.Authenticators.Radius = &auth.Radius{}
			authPolicy.Spec.Authenticators.AuthenticatorOrder = []string{"LOCAL"}
			Eventually(func() error {
				_, err := ts.restSvc.AuthV1().AuthenticationPolicy().Update(ts.loggedInCtx, authPolicy)
				return err
			}, 10, 1).Should(BeNil())
			// updating auth policy re-generates JWT secret so login again to a get new JWT
			ts.loggedInCtx = ts.tu.NewLoggedInContext(context.TODO())
		})
	})
	Context("jwt", func() {
		BeforeEach(func() {
			var authPolicy *auth.AuthenticationPolicy
			var err error
			Eventually(func() error {
				authPolicy, err = ts.restSvc.AuthV1().AuthenticationPolicy().Get(ts.loggedInCtx, &api.ObjectMeta{})
				return err
			}, 10, 1).Should(BeNil())
			authPolicy.Spec.TokenExpiry = "2m"
			Eventually(func() error {
				authPolicy, err = ts.restSvc.AuthV1().AuthenticationPolicy().Update(ts.loggedInCtx, authPolicy)
				return err
			}, 10, 1).Should(BeNil())
			Expect(authPolicy.Spec.TokenExpiry).Should(Equal("2m"))
			time.Sleep(6 * time.Second)
			ts.loggedInCtx = ts.tu.NewLoggedInContext(context.TODO())
		})
		It("check expiry", func() {
			var err error
			time.Sleep(201 * time.Second)
			_, err = ts.restSvc.AuthV1().User().Get(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.User, Tenant: globals.DefaultTenant})
			Expect(err).Should(HaveOccurred())
		})
		AfterEach(func() {
			ts.loggedInCtx = ts.tu.NewLoggedInContext(context.TODO())
			var authPolicy *auth.AuthenticationPolicy
			var err error
			Eventually(func() error {
				authPolicy, err = ts.restSvc.AuthV1().AuthenticationPolicy().Get(ts.loggedInCtx, &api.ObjectMeta{})
				return err
			}, 10, 1).Should(BeNil())
			authPolicy.Spec.TokenExpiry = "144h"
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().AuthenticationPolicy().Update(ts.loggedInCtx, authPolicy)
				return err
			}, 10, 1).Should(BeNil())
			ts.loggedInCtx = ts.tu.NewLoggedInContext(context.TODO())
		})
	})
})
