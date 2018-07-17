package auth

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestLogin(t *testing.T) {
	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", tinfo.apiServerAddr, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	MustCreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	MustCreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	in := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   "default",
	}
	var resp *http.Response
	var statusCode int
	AssertEventually(t, func() (bool, interface{}) {
		resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), in)
		if err == nil {
			statusCode = resp.StatusCode
		}
		return err == nil && statusCode == http.StatusOK, nil
	}, fmt.Sprintf("login request failed: Err: %v, Status code: %d", err, statusCode))

	// verify cookie
	cookies := resp.Cookies()
	Assert(t, len(cookies) == 1, "cookie not set in response")
	Assert(t, cookies[0].Name == "sid", "cookie not present")
	Assert(t, cookies[0].HttpOnly, "cookie is not http only")
	//Assert(t, cookies[0].Secure, "cookie is not secure") TODO:// Enable it once APIGW is TLS enabled
	Assert(t, cookies[0].MaxAge == apigw.TokenExpInDays*24*60*60, fmt.Sprintf("cookie max age is not 6 days, [%d]", cookies[0].MaxAge))
	Assert(t, cookies[0].Value != "", "session id value is not set")
	// verify user from response
	var user auth.User
	AssertOk(t, json.NewDecoder(resp.Body).Decode(&user), "unable to decode user from http response")
	Assert(t, user.Name == in.Username && user.Tenant == in.Tenant, fmt.Sprintf("incorrect user [%s] and tenant [%s] returned", user.Name, user.Tenant))
	Assert(t, user.Spec.Password == "", "password should be empty")
	// check CSRF token header is present
	Assert(t, resp.Header.Get(apigw.GrpcMDCsrfHeader) != "", "CSRF token not present")
}

func TestLoginFailures(t *testing.T) {
	tests := []struct {
		name     string
		cred     *auth.PasswordCredential
		expected int
	}{
		{
			name:     "non existent username",
			cred:     &auth.PasswordCredential{Username: "xxx", Password: "", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid password",
			cred:     &auth.PasswordCredential{Username: testUser, Password: "xxx", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid tenant",
			cred:     &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: "xxx"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username",
			cred:     &auth.PasswordCredential{Username: "", Password: testPassword, Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username and password",
			cred:     &auth.PasswordCredential{Username: "", Password: "", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty tenant",
			cred:     &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: ""},
			expected: http.StatusUnauthorized,
		},
	}

	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", tinfo.apiServerAddr, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	MustCreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	MustCreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	for _, test := range tests {
		var resp *http.Response
		var statusCode int
		AssertEventually(t, func() (bool, interface{}) {
			resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), test.cred)
			if err == nil {
				statusCode = resp.StatusCode
			}
			return err == nil && statusCode == test.expected, nil
		}, fmt.Sprintf("[%v] test failed, Err: %v, returned status code [%d], expected [%d]", test.name, err, statusCode, test.expected))

		cookies := resp.Cookies()
		Assert(t, len(cookies) == 0, fmt.Sprintf("[%v] test failed, cookie should not be set in response", test.name))
		Assert(t, resp.Header.Get(apigw.GrpcMDCsrfHeader) == "", fmt.Sprintf("[%v] test failed, CSRF token is present", test.name))
	}
}

func TestUserPasswordRemoval(t *testing.T) {
	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", tinfo.apiServerAddr, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	MustCreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	MustCreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}

	in := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   "default",
	}
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, in)
	AssertOk(t, err, "unable to get logged in context")
	// test GET user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: testUser, Tenant: "default"})
		return err == nil, nil
	}, "unable to fetch user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test LIST user
	var users []*auth.User
	AssertEventually(t, func() (bool, interface{}) {
		users, err = restcl.AuthV1().User().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}})
		return err == nil, nil
	}, "unable to fetch users")
	Assert(t, users[0].Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", users[0].Spec.Password))
	// test UPDATE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Update(ctx, &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "default",
				Name:   testUser,
			},
			Spec: auth.UserSpec{
				Fullname: "Test User Update",
				Email:    "testuser@pensandio.io",
				Type:     auth.UserSpec_LOCAL.String(),
				Password: testPassword,
			},
		})
		return err == nil, nil
	}, "unable to update user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test CREATE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Create(ctx, &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "default",
				Name:   "testUser2",
			},
			Spec: auth.UserSpec{
				Fullname: "Test User2",
				Email:    "testuser2@pensandio.io",
				Type:     auth.UserSpec_LOCAL.String(),
				Password: testPassword,
			},
		})
		return err == nil, nil
	}, "unable to create user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test DELETE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Delete(ctx, &api.ObjectMeta{Name: "testUser2", Tenant: "default"})
		return err == nil, nil
	}, "unable to delete user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
}

func TestAuthPolicy(t *testing.T) {
	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", tinfo.apiServerAddr, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	MustCreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	MustCreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}

	in := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   "default",
	}
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, in)
	AssertOk(t, err, "unable to get logged in context")
	// test GET AuthenticationPolicy
	var policy *auth.AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		return err == nil, nil
	}, "unable to fetch auth policy")
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	// test CREATE AuthenticationPolicy
	AssertConsistently(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Create(ctx, &auth.AuthenticationPolicy{
			TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name: "AuthenticationPolicy2",
			},
			Spec: auth.AuthenticationPolicySpec{
				Authenticators: auth.Authenticators{
					Ldap:               &auth.Ldap{Enabled: false},
					Local:              &auth.Local{Enabled: true},
					AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
				},
			},
		})
		return err != nil, nil

	}, "cannot create more than one auth policy")
	// test UPDATE AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Update(ctx, &auth.AuthenticationPolicy{
			TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name: "AuthenticationPolicy3",
			},
			Spec: auth.AuthenticationPolicySpec{
				Authenticators: auth.Authenticators{
					Ldap:               &auth.Ldap{Enabled: false},
					Local:              &auth.Local{Enabled: true},
					AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
				},
			},
		})
		return err == nil, nil
	}, "unable to update auth policy")
	Assert(t, policy.Name == "AuthenticationPolicy3", fmt.Sprintf("invalid auth policy name, [%s]", policy.Name))
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		return err != nil, nil
	}, "User should need to login again after updating authentication policy")
	// re-login as JWT secret is regenerated after update to authentication policy
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, in)
	AssertOk(t, err, "unable to get logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		return err == nil, nil
	}, "unable to fetch auth policy")
	Assert(t, policy.Name == "AuthenticationPolicy3", "invalid auth policy name")
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	// test DELETE AuthenticationPolicy
	AssertConsistently(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Delete(ctx, &api.ObjectMeta{Name: "AuthenticationPolicy3"})
		return err != nil, nil
	}, "AuthenticationPolicy can't be deleted")
}
