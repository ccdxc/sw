package cluster

import (
	"context"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
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
		It("successful radius auth", func() {
			// TODO: Enable this test once we can get ACS to accept connection from IP subnet of API Gw in CI. Currently ACS accepts 192.168.0.0/16, 10.3.0.0/16 but it doesn't seem to work for connections
			// coming from API Gw in CI. This test works on laptop.
			Skip("Skipping radius e2e")
			radiusCtx, err := testutils.NewLoggedInContext(context.TODO(), ts.tu.APIGwAddr, &auth.PasswordCredential{Username: ts.tu.Radius.User, Password: ts.tu.Radius.Password})
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
			logintime, err := user.Status.LastSuccessfulLogin.Time()
			Expect(err).ShouldNot(HaveOccurred())
			Expect(time.Now()).Should(BeTemporally("~", logintime, 15*time.Second))
			Expect(user.Status.Authenticators).Should(Equal([]string{auth.Authenticators_RADIUS.String()}))
		})
		AfterEach(func() {
			Eventually(func() error {
				_, err := ts.restSvc.AuthV1().RoleBinding().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: "RadiusAdminRoleBinding", Tenant: ts.tu.Radius.Tenant})
				return err
			}, 10, 1).Should(BeNil())
			Eventually(func() error {
				_, err := ts.restSvc.AuthV1().User().Delete(ts.loggedInCtx, &api.ObjectMeta{Name: ts.tu.Radius.User, Tenant: ts.tu.Radius.Tenant})
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
})
