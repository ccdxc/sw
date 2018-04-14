package auth

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestLogin(t *testing.T) {
	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", "localhost:"+tinfo.apiserverport, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	CreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	in := &authn.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   "default",
	}
	var resp *http.Response
	var statusCode int
	AssertEventually(t, func() (bool, interface{}) {
		resp, err = Login("http://localhost:"+tinfo.apigwport, in)
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
	Assert(t, cookies[0].Secure, "cookie is not secure")
	Assert(t, cookies[0].MaxAge == int((svc.TokenExpInDays*24*time.Hour).Seconds()), "cookie max age is not 6 days")
	Assert(t, cookies[0].Value != "", "session id value is not set")
	// verify user from response
	var user auth.User
	AssertOk(t, json.NewDecoder(resp.Body).Decode(&user), "unable to decode user from http response")
	Assert(t, user.Name == in.Username && user.Tenant == in.Tenant, fmt.Sprintf("incorrect user [%s] and tenant [%s] returned", user.Name, user.Tenant))
	Assert(t, user.Spec.Password == "", "password should be empty")
	// check CSRF token header is present
	Assert(t, resp.Header.Get(svc.CsrfHeader) != "", "CSRF token not present")
}

func TestLoginFailures(t *testing.T) {
	tests := []struct {
		name     string
		cred     *authn.PasswordCredential
		expected int
	}{
		{
			name:     "non existent username",
			cred:     &authn.PasswordCredential{Username: "xxx", Password: "", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid password",
			cred:     &authn.PasswordCredential{Username: testUser, Password: "xxx", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid tenant",
			cred:     &authn.PasswordCredential{Username: testUser, Password: testPassword, Tenant: "xxx"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username",
			cred:     &authn.PasswordCredential{Username: "", Password: testPassword, Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username and password",
			cred:     &authn.PasswordCredential{Username: "", Password: "", Tenant: "default"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty tenant",
			cred:     &authn.PasswordCredential{Username: testUser, Password: testPassword, Tenant: ""},
			expected: http.StatusUnauthorized,
		},
	}

	// api server client
	logger := log.WithContext("Pkg", "auth integration tests")
	apicl, err := apiclient.NewGrpcAPIClient("login_integ_test", "localhost:"+tinfo.apiserverport, logger)
	if err != nil {
		panic("error creating api client")
	}
	// create authentication policy with local auth enabled
	CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	defer DeleteAuthenticationPolicy(apicl)
	// create local user
	CreateTestUser(apicl, testUser, testPassword, "default")
	defer DeleteUser(apicl, testUser, "default")

	for _, test := range tests {
		var resp *http.Response
		var statusCode int
		AssertEventually(t, func() (bool, interface{}) {
			resp, err = Login("http://localhost:"+tinfo.apigwport, test.cred)
			if err == nil {
				statusCode = resp.StatusCode
			}
			return err == nil && statusCode == test.expected, nil
		}, fmt.Sprintf("[%v] test failed, Err: %v, returned status code [%d], expected [%d]", test.name, err, statusCode, test.expected))

		cookies := resp.Cookies()
		Assert(t, len(cookies) == 0, fmt.Sprintf("[%v] test failed, cookie should not be set in response", test.name))
		Assert(t, resp.Header.Get(svc.CsrfHeader) == "", fmt.Sprintf("[%v] test failed, CSRF token is present", test.name))
	}
}
