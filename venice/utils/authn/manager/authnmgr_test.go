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
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL    = "localhost:0"
	testUser     = "test"
	testPassword = "Pensandoo0$"
	tenant       = "default"
)

var expiration = 600 * time.Second

var (
	logger = log.WithContext("Pkg", "authnmgr_test")
)

var apicl apiclient.Services
var apiSrv apiserver.Server
var apiSrvAddr string
var secret []byte
var authnmgr *AuthenticationManager

// testJWTToken for benchmarking
var testHS512JWTToken string

func TestMain(m *testing.M) {
	recorder.Override(mockevtsrecorder.NewRecorder("authnmgr_test", logger))
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}

func setup() {
	var err error
	// api server
	apiSrv, apiSrvAddr, err = serviceutils.StartAPIServer(apisrvURL, "authnmgr_test", logger)
	if err != nil {
		panic("Unable to start API Server")
	}

	// api server client
	apicl, err = apiclient.NewGrpcAPIClient("authnmgr_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
	// create cluster
	MustCreateCluster(apicl)

	// create tenant
	MustCreateTenant(apicl, "default")

	// create test user
	MustCreateTestUser(apicl, testUser, testPassword, "default")

	// create secret for jwt tests
	secret, err = authn.CreateSecret(128)
	if err != nil {
		panic(fmt.Sprintf("Error generating secret: Err: %v", err))
	}
	testHS512JWTToken = createHeadlessToken(signatureAlgorithm, secret, expiration, issuerClaimValue)

	// create authentication manager
	authnmgr, err = NewAuthenticationManager("authnmgr_test", apiSrvAddr, nil, logger)
	if err != nil {
		panic("Error creating authentication manager")
	}
}

func shutdown() {
	// stop api server
	apiSrv.Stop()
	// stop AuthGetter
	authnmgr.GetAuthGetter().Stop()
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
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	policydata["LDAP disabled, Local enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               &auth.Ldap{},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
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
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	policydata["Local enabled, LDAP disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               &auth.Ldap{},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}

	return policydata
}

func createAuthenticationPolicy(t *testing.T, policy *auth.AuthenticationPolicy) *auth.AuthenticationPolicy {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := authGetter.GetAuthenticationPolicy()
		return err != nil, nil
	}, "failed to create AuthenticationPolicy")

	ret, err := CreateAuthenticationPolicyWithOrder(apicl,
		policy.Spec.Authenticators.Local,
		policy.Spec.Authenticators.Ldap,
		policy.Spec.Authenticators.Radius,
		policy.Spec.Authenticators.AuthenticatorOrder,
		policy.Spec.TokenExpiry)
	if err != nil {
		panic(fmt.Sprintf("CreateAuthenticationPolicyWithOrder failed with err %s", err))
	}

	AssertEventually(t, func() (bool, interface{}) {
		_, err := authGetter.GetAuthenticationPolicy()
		return err == nil, nil
	}, "Authentication policy not found after creation")
	return ret
}

func deleteAuthenticationPolicy(t *testing.T) {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := authGetter.GetAuthenticationPolicy()
		return err == nil, nil
	}, "did not find AuthenticationPolicy to delete")

	DeleteAuthenticationPolicy(apicl)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := authGetter.GetAuthenticationPolicy()
		return err != nil, nil
	}, "found AuthenticationPolicy after delete")
}

// TestAuthenticate tests successful authentication for various authentication policies with LDAP and Local Authenticator configured.
// This tests authentication for different order of the authenticators and if they are enabled or disabled.
func TestAuthenticate(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(t, policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertEventually(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(&auth.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})
			return ok, nil
		}, fmt.Sprintf("[%v] Unsuccessful local user authentication", testtype))

		Assert(t, autheduser.Name == testUser, fmt.Sprintf("[%v] User returned by authentication manager didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_Local.String(), fmt.Sprintf("[%v] User returned is not of type LOCAL", testtype))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))

		deleteAuthenticationPolicy(t)
	}
}

// TestIncorrectPasswordAuthentication tests failed authentication by all authenticators
func TestIncorrectPasswordAuthentication(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(t, policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertConsistently(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(&auth.PasswordCredential{Username: testUser, Password: "wrongpassword"})
			return !ok, nil
		}, fmt.Sprintf("[%v] Successful local user authentication", testtype), "100ms", "1s")

		Assert(t, autheduser == nil, fmt.Sprintf("[%v] User returned while authenticating with wrong password", testtype))
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned while authenticating with wrong password", testtype))

		deleteAuthenticationPolicy(t)
	}
}

// disabledLocalAuthenticatorPolicyData returns policy data where Local authenticator is disabled
func disabledLocalAuthenticatorPolicyData() map[string](*auth.AuthenticationPolicy) {
	policydata := make(map[string]*auth.AuthenticationPolicy)

	policydata["LDAP disabled, Local disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	return policydata
}

// TestAuthenticateWithDisabledAuthenticators test authentication for local user when local authenticator is disabled
func TestAuthenticateWithDisabledAuthenticators(t *testing.T) {
	for testtype, policy := range disabledLocalAuthenticatorPolicyData() {
		createAuthenticationPolicy(t, policy)

		// authenticate
		var autheduser *auth.User
		var ok bool
		var err error
		AssertEventually(t, func() (bool, interface{}) {
			autheduser, ok, err = authnmgr.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
			return !ok, nil
		}, fmt.Sprintf("[%v] local user authentication should fail", testtype))

		Assert(t, autheduser == nil, fmt.Sprintf("[%v] User returned with disabled authenticators", testtype))
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned with disabled local authenticator", testtype))
		deleteAuthenticationPolicy(t)
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
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	createdPolicy := createAuthenticationPolicy(t, policy)
	defer deleteAuthenticationPolicy(t)

	// create JWT token
	jwtTok := createHeadlessToken(signatureAlgorithm, createdPolicy.Spec.Secret, expiration, issuerClaimValue)

	var user *auth.User
	var ok bool
	var csrfTok string
	var err error
	AssertEventually(t, func() (bool, interface{}) {
		user, ok, csrfTok, err = authnmgr.ValidateToken(jwtTok)
		return ok, nil
	}, "token validation failed")
	AssertOk(t, err, "error validating token")
	Assert(t, csrfTok == testCsrfToken, "incorrect csrf token")
	Assert(t, user != nil && user.Name == testUser && user.Tenant == tenant, "incorrect user returned")
}

func TestValidateTokenErrors(t *testing.T) {
	mockAuthnmgr := NewMockAuthenticationManager()
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
			token:    NonExistentUserClaim,
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
