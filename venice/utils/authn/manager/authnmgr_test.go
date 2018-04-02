package manager

import (
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL    = "localhost:0"
	testUser     = "test"
	testPassword = "pensandoo0"
	tenant       = "default"
	expiration   = 600
)

var apicl apiclient.Services
var apiSrv apiserver.Server
var apiSrvAddr string
var secret []byte
var authnmgr *AuthenticationManager

// testJWTToken for benchmarking
var testHS512JWTToken string

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}

func setup() {
	// api server
	apiSrv = createAPIServer(apisrvURL)
	if apiSrv == nil {
		panic("Unable to create API Server")
	}
	var err error
	apiSrvAddr, err = apiSrv.GetAddr()
	if err != nil {
		panic("Unable to get API Server address")
	}
	// api server client
	logger := log.WithContext("Pkg", "authnmgr_test")
	apicl, err = apiclient.NewGrpcAPIClient("authnmgr_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}

	// create test user
	CreateTestUser(apicl, testUser, testPassword, "default")

	// create secret for jwt tests
	secret = CreateSecret(128)
	testHS512JWTToken = createHeadlessToken(signatureAlgorithm, secret, time.Duration(expiration)*time.Second, issuerClaimValue)

	// create authentication manager
	authnmgr, err = NewAuthenticationManager("authnmgr_test", apiSrvAddr, nil, time.Duration(expiration))
	if err != nil {
		panic("Error creating authentication manager")
	}
}

func shutdown() {
	// stop api server
	apiSrv.Stop()
	// un-initialize authentication manager
	authnmgr.Uninitialize()
}

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "authnmgr_test")

	// api server config
	sch := runtime.NewScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: url,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}
	// create api server
	apiSrv := apisrvpkg.MustGetAPIServer()
	go apiSrv.Run(apisrvConfig)
	time.Sleep(time.Millisecond * 100)

	return apiSrv
}

// authenticationPoliciesData returns policies configured with Local and LDAP authenticators in different order
func authenticationPoliciesData() map[string]*auth.AuthenticationPolicy {
	policydata := make(map[string]*auth.AuthenticationPolicy)

	policydata["LDAP enabled, Local enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	policydata["LDAP disabled, Local enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	policydata["Local enabled, LDAP enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
			},
			Secret: secret,
		},
	}
	policydata["Local enabled, LDAP disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
			},
			Secret: secret,
		},
	}

	return policydata
}

func createAuthenticationPolicy(policy *auth.AuthenticationPolicy) *auth.AuthenticationPolicy {
	return CreateAuthenticationPolicyWithOrder(apicl,
		policy.Spec.Authenticators.Local,
		policy.Spec.Authenticators.Ldap,
		policy.Spec.Authenticators.Radius,
		policy.Spec.Authenticators.AuthenticatorOrder,
		secret)
}

// TestAuthenticate tests successful authentication for various authentication policies with LDAP and Local Authenticator configured.
// This tests authentication for different order of the authenticators and if they are enabled or disabled.
func TestAuthenticate(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertEventually(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(&authn.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})
			return ok, nil
		}, fmt.Sprintf("[%v] Unsuccessful local user authentication", testtype))

		Assert(t, autheduser.Name == testUser, fmt.Sprintf("[%v] User returned by authentication manager didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_LOCAL.String(), fmt.Sprintf("[%v] User returned is not of type LOCAL", testtype))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))

		DeleteAuthenticationPolicy(apicl)
	}
}

// TestIncorrectPasswordAuthentication tests failed authentication by all authenticators
func TestIncorrectPasswordAuthentication(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertConsistently(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(authn.PasswordCredential{Username: testUser, Password: "wrongpassword"})
			return !ok, nil
		}, fmt.Sprintf("[%v] Successful local user authentication", testtype))

		Assert(t, autheduser == nil, fmt.Sprintf("[%v] User returned while authenticating with wrong password", testtype))
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned while authenticating with wrong password", testtype))

		DeleteAuthenticationPolicy(apicl)
	}
}

// TestNotYetImplementedAuthenticator tests un-implemented authenticator
func TestNotYetImplementedAuthenticator(t *testing.T) {
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				Radius:             &auth.Radius{},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String(), auth.Authenticators_RADIUS.String()},
			},
			Secret: secret,
		},
	}
	createAuthenticationPolicy(policy)
	defer DeleteAuthenticationPolicy(apicl)

	var user *auth.User
	var ok bool
	var err error
	AssertConsistently(t, func() (bool, interface{}) {
		user, ok, err = authnmgr.Authenticate(&authn.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})
		return !ok, nil
	}, "User authenticated when unimplemented authenticator is configured")

	Assert(t, err != nil, "No error returned for un-implemented authenticator")
	Assert(t, user == nil, "User returned when unimplemented authenticator is configured")
}

// disabledLocalAuthenticatorPolicyData returns policy data where both LDAP and Local authenticators are disabled
func disabledLocalAuthenticatorPolicyData() map[string](*auth.AuthenticationPolicy) {
	policydata := make(map[string]*auth.AuthenticationPolicy)

	policydata["LDAP explicitly disabled, Local explicitly disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: false,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	policydata["LDAP implicitly disabled, Local implicitly disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               &auth.Ldap{},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}

	return policydata
}

// TestAuthenticateWithDisabledAuthenticators test authentication for local user when all authenticators are disabled
func TestAuthenticateWithDisabledAuthenticators(t *testing.T) {
	for testtype, policy := range disabledLocalAuthenticatorPolicyData() {
		createAuthenticationPolicy(policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertConsistently(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(authn.PasswordCredential{Username: testUser, Password: testPassword})
			return !ok, nil
		}, fmt.Sprintf("[%v] local user authentication should fail", testtype))

		Assert(t, autheduser == nil, fmt.Sprintf("[%v] User returned with disabled authenticators", testtype))
		AssertOk(t, err, fmt.Sprintf("[%v] No error should be returned with disabled authenticators", testtype))
		DeleteAuthenticationPolicy(apicl)
	}
}

func TestAuthnMgrValidateToken(t *testing.T) {
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	createAuthenticationPolicy(policy)
	defer DeleteAuthenticationPolicy(apicl)

	var user *auth.User
	var ok bool
	var csrfTok string
	var err error
	AssertEventually(t, func() (bool, interface{}) {
		user, ok, csrfTok, err = authnmgr.ValidateToken(testHS512JWTToken)
		return ok, nil
	}, "token validation failed")
	AssertOk(t, err, "error validating token")
	Assert(t, csrfTok == testCsrfToken, "incorrect csrf token")
	Assert(t, user != nil && user.Name == testUser && user.Tenant == tenant, "incorrect user returned")
}

const (
	invalidUserClaim     = "invalidUser"
	invalidTenantClaim   = "invalidTenant"
	nonExistentUserClaim = "nonExistentUser"
)

type mockTokenManager struct{}

func (tm *mockTokenManager) ValidateToken(token string) (map[string]interface{}, bool, error) {
	claims := make(map[string]interface{})
	switch token {
	case invalidUserClaim:
		claims[SubClaim] = 0
	case invalidTenantClaim:
		claims[TenantClaim] = 0
	case nonExistentUserClaim:
		claims[SubClaim] = nonExistentUserClaim
	}
	return claims, true, nil
}

func (tm *mockTokenManager) CreateToken(*auth.User, map[string]interface{}) (string, error) {
	return "", nil
}

func (tm *mockTokenManager) Get(token, key string) (interface{}, bool, error) {
	return nil, false, nil
}

type mockAuthGetter struct{}

func (ag *mockAuthGetter) GetUser(name, tenant string) (*auth.User, bool) {
	switch name {
	case nonExistentUserClaim:
		return nil, false
	default:
		// user object
		user := &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: tenant,
				Name:   name,
			},
			Spec: auth.UserSpec{
				Fullname: "Test User" + name,
				Password: "",
				Email:    name + "@pensandio.io",
				Type:     auth.UserSpec_LOCAL.String(),
			},
		}
		return user, true
	}

}

// GetAuthenticationPolicy returns authentication policy
func (ag *mockAuthGetter) GetAuthenticationPolicy() (*auth.AuthenticationPolicy, error) {
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	return policy, nil
}

func (ag *mockAuthGetter) GetAuthenticators() ([]authn.Authenticator, error) {
	return nil, nil
}

func (ag *mockAuthGetter) GetTokenManager() (TokenManager, error) {
	return &mockTokenManager{}, nil
}

// Stop un-initializes AuthGetter
func (ag *mockAuthGetter) Stop() {}

// Start re-initializes AuthGetter. It blocks if AuthGetter has not been un-initialized through Stop()
func (ag *mockAuthGetter) Start() {}

func TestValidateTokenErrors(t *testing.T) {
	mockAuthnmgr := &AuthenticationManager{
		authGetter: &mockAuthGetter{},
	}
	tests := []struct {
		name     string
		token    string
		expected error
	}{
		{
			name:     "invalid user claim",
			token:    invalidUserClaim,
			expected: ErrUserNotFound,
		},
		{
			name:     "invalid tenant claim",
			token:    invalidTenantClaim,
			expected: ErrUserNotFound,
		},
		{
			name:     "non existent user",
			token:    nonExistentUserClaim,
			expected: ErrUserNotFound,
		},
	}
	for _, test := range tests {
		user, ok, csrfTok, err := mockAuthnmgr.ValidateToken(test.token)
		Assert(t, err == test.expected, fmt.Sprintf("[%v]  test failed", test.name))
		Assert(t, !ok, fmt.Sprintf("[%v]  test failed", test.name))
		Assert(t, user == nil, fmt.Sprintf("[%v]  test failed", test.name))
		Assert(t, csrfTok == "", fmt.Sprintf("[%v]  test failed", test.name))
	}
}
