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
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/events/generated/eventtypes"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/authz"
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
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{
							{
								Url:        ts.tu.Radius.URL,
								Secret:     ts.tu.Radius.NasSecret,
								AuthMethod: auth.Radius_PAP.String(),
							},
						},
						NasID: ts.tu.Radius.NasID,
					},
				},
			}
			authPolicy.Spec.Authenticators.AuthenticatorOrder = []string{"LOCAL", "RADIUS"}
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().AuthenticationPolicy().Update(ts.loggedInCtx, authPolicy)
				return err
			}, 10, 1).Should(BeNil())
			// updating auth policy re-generates JWT secret so login again to a get new JWT
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
			// TODO: Create a tenant if radius config specifies a non-default tenant
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().RoleBinding().Create(ts.loggedInCtx,
					login.NewRoleBinding("RadiusAdminRoleBinding", ts.tu.Radius.Tenant, globals.AdminRole, "", strings.Join(ts.tu.Radius.UserGroups, ",")))
				return err
			}, 10, 1).Should(BeNil())
		})
		It("check audit logs", func() {
			// search for successful login audit event
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
				Aggregate:  true,
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
				events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
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
			// search for auth policy update audit event
			stages := []string{audit.Stage_RequestAuthorization.String(), audit.Stage_RequestProcessing.String()}
			for _, stage := range stages {
				query := &search.SearchRequest{
					Query: &search.SearchQuery{
						Kinds: []string{auth.Permission_AuditEvent.String()},
						Fields: &fields.Selector{
							Requirements: []*fields.Requirement{
								{
									Key:      "action",
									Operator: "equals",
									Values:   []string{strings.Title(string(apiintf.UpdateOper))},
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
								{
									Key:      "stage",
									Operator: "equals",
									Values:   []string{stage},
								},
							},
						},
					},
					From:       0,
					MaxResults: 50,
					Aggregate:  true,
				}
				Eventually(func() error {
					resp := testutils.AuditSearchResponse{}
					err := ts.tu.Search(ts.loggedInCtx, query, &resp)
					if err != nil {
						return err
					}
					if resp.ActualHits == 0 {
						return fmt.Errorf("no audit logs for authentication policy update at stage %s", stage)
					}
					events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
					for _, event := range events {
						if (event.Object.Action == strings.Title(auth.Permission_Update.String())) &&
							(event.Object.Resource.Kind == string(auth.KindAuthenticationPolicy)) &&
							(event.Object.Outcome == audit.Outcome_Success.String()) &&
							(event.Object.Stage == stage) {
							return nil
						}
					}
					return fmt.Errorf("no audit logs for authentication policy update at stage %s", stage)
				}, 30, 1).Should(BeNil())
			}
			// query by category
			query = &search.SearchRequest{
				Query: &search.SearchQuery{
					Categories: []string{globals.Kind2Category("AuditEvent")},
				},
				From:       0,
				MaxResults: 50,
				Aggregate:  true,
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
				if len(resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries) == 0 {
					return fmt.Errorf("no audit events found for monitoring category search")
				}
				return err
			}, 30, 1).Should(BeNil())
		})
		It("successful radius auth", func() {
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
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
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
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
		})
		It("check expiry", func() {
			var err error
			time.Sleep(201 * time.Second)
			_, err = ts.restSvc.AuthV1().User().Get(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.User, Tenant: globals.DefaultTenant})
			Expect(err).Should(HaveOccurred())
		})
		AfterEach(func() {
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
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
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
		})
	})
	Context("auth events", func() {
		It("check login failure event", func() {
			_, err := login.NewLoggedInContext(context.TODO(), ts.tu.APIGwAddr, &auth.PasswordCredential{Username: ts.tu.User, Password: "incorrect", Tenant: globals.DefaultTenant})
			Expect(err).Should(HaveOccurred())
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Categories: []string{globals.Kind2Category("AuditEvent")},
					Kinds:      []string{auth.Permission_Event.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "type",
								Operator: "equals",
								Values:   []string{eventtypes.EventType_name[int32(eventtypes.LOGIN_FAILED)]},
							},
							{
								Key:      "meta.mod-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-3 * time.Second).Format(time.RFC3339Nano)},
							},
							{
								Key:      "meta.mod-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
				Aggregate:  true,
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
				events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_Event.String()].Entries

				for _, evt := range events {
					if strings.Contains(evt.Object.Message, fmt.Sprintf("%s|%s", globals.DefaultTenant, ts.tu.User)) {
						return nil
					}
				}
				return fmt.Errorf("no events found for failed login attempt in default tenant")
			}, 30, 1).Should(BeNil())
		})
	})
	Context("commit buffer", func() {
		It("check auth objects", func() {
			// create buffer
			const testbuffer = "TestBuffer"
			Eventually(func() error {
				_, err := ts.restSvc.StagingV1().Buffer().Create(ts.loggedInCtx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: testbuffer, Tenant: globals.DefaultTenant}})
				return err
			}, 10, 1).Should(BeNil())
			stagecl, err := apiclient.NewStagedRestAPIClient(ts.tu.APIGwAddr, testbuffer)
			Expect(err).ShouldNot(HaveOccurred())
			// create auth objects
			user := &auth.User{}
			user.Defaults("all")
			user.Name = "test2"
			user.Spec.Password = ts.tu.Password
			Eventually(func() error {
				user, err = stagecl.AuthV1().User().Create(ts.loggedInCtx, user)
				return err
			}, 10, 1).Should(BeNil())
			role := &auth.Role{}
			role.Defaults("all")
			role.Name = "NetworkAdminRole"
			role.Spec.Permissions = append(role.Spec.Permissions, login.NewPermission(
				globals.DefaultTenant,
				string(apiclient.GroupNetwork),
				string(network.KindNetwork),
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_AllActions.String()),
				login.NewPermission(
					globals.DefaultTenant,
					string(apiclient.GroupAuth),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()))
			Eventually(func() error {
				role, err = stagecl.AuthV1().Role().Create(ts.loggedInCtx, role)
				return err
			}, 10, 1).Should(BeNil())
			rb := &auth.RoleBinding{}
			rb.Defaults("all")
			rb.Name = "NetworkAdminRoleBinding"
			rb.Spec.Role = role.Name
			rb.Spec.Users = append(rb.Spec.Users, user.Name)
			Eventually(func() error {
				rb, err = stagecl.AuthV1().RoleBinding().Create(ts.loggedInCtx, rb)
				return err
			}, 10, 1).Should(BeNil())
			// commit buffer
			ca := &staging.CommitAction{}
			ca.Name = testbuffer
			ca.Tenant = globals.DefaultTenant
			Eventually(func() error {
				_, err = ts.restSvc.StagingV1().Buffer().Commit(ts.loggedInCtx, ca)
				return err
			}, 10, 1).Should(BeNil())
			// check auth objects got created
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().RoleBinding().Get(ts.loggedInCtx, rb.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().Role().Get(ts.loggedInCtx, role.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().User().Get(ts.loggedInCtx, user.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			// delete auth objects
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().RoleBinding().Delete(ts.loggedInCtx, rb.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().Role().Delete(ts.loggedInCtx, role.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().User().Delete(ts.loggedInCtx, user.GetObjectMeta())
				return err
			}, 10, 1).Should(BeNil())
			// delete buffer
			Eventually(func() error {
				_, err := ts.restSvc.StagingV1().Buffer().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: testbuffer, Tenant: globals.DefaultTenant})
				return err
			}, 10, 1).Should(BeNil())
			// search for commit audit event
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{auth.Permission_Commit.String()},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{audit.Outcome_Success.String()},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{audit.Stage_RequestProcessing.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string(staging.KindBuffer)},
							},
							{
								Key:      "resource.tenant",
								Operator: "equals",
								Values:   []string{globals.DefaultTenant},
							},
							{
								Key:      "resource.name",
								Operator: "equals",
								Values:   []string{testbuffer},
							},
						},
					},
				},
				From:       0,
				MaxResults: 50,
				Aggregate:  true,
			}
			Eventually(func() error {
				resp := testutils.AuditSearchResponse{}
				err := ts.tu.Search(ts.loggedInCtx, query, &resp)
				if err != nil {
					return err
				}
				if resp.ActualHits == 0 {
					return fmt.Errorf("no audit logs for successful staging buffer commit")
				}
				events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
				for _, event := range events {
					if (event.Object.Action == auth.Permission_Commit.String()) &&
						(event.Object.Outcome == audit.Outcome_Success.String()) &&
						(event.Object.User.Name == ts.tu.User) &&
						(event.Object.User.Tenant == globals.DefaultTenant) &&
						(event.Object.Resource.Name == "TestBuffer") &&
						(event.Object.Stage == audit.Stage_RequestProcessing.String()) {
						return nil
					}
				}
				return fmt.Errorf("no audit logs for successful commit")
			}, 30, 1).Should(BeNil())
		})
	})
	Context("user status", func() {
		It("check login timestamp", func() {
			var user *auth.User
			var err error
			Eventually(func() error {
				user, err = ts.restSvc.AuthV1().User().Get(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.User, Tenant: globals.DefaultTenant})
				return err
			}, 10, 1).Should(BeNil())
			loginTime, err := user.Status.LastLogin.Time()
			Expect(err).ShouldNot(HaveOccurred())
			ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.TODO())
			Eventually(func() error {
				user, err = ts.restSvc.AuthV1().User().Get(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.User, Tenant: globals.DefaultTenant})
				return err
			}, 10, 1).Should(BeNil())
			newLoginTime, err := user.Status.LastLogin.Time()
			Expect(err).ShouldNot(HaveOccurred())
			Expect(newLoginTime.After(loginTime)).Should(BeTrue())
		})
	})
})
