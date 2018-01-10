package manager

import (
	"os"
	"testing"
	"time"

	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	"fmt"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks"
)

const (
	apisrvURL    = "localhost:9192"
	testUser     = "test"
	testPassword = "pensandoo0"
)

var apicl apiclient.Services
var apiSrv apiserver.Server

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

	// api server client
	var err error
	logger := log.WithContext("Pkg", "ldap_test")
	apicl, err = apiclient.NewGrpcAPIClient(apisrvURL, logger)
	if err != nil {
		panic("Error creating api client")
	}

	// create test user
	createTestUser()
}

func shutdown() {
	//stop api server
	apiSrv.Stop()
}

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "ldap_test")

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

// createTestUser creates a test user
func createTestUser() *auth.User {
	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   testUser,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: testPassword,
			Email:    "testuser@pensandio.io",
			Type:     auth.UserSpec_LOCAL.String(),
		},
	}

	// create the user object in api server
	_, err := apicl.AuthV1().User().Create(context.Background(), &user)
	if err != nil {
		panic("Error creating user")
	}
	return &user
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
			Authenticators: &auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	policydata["LDAP disabled, Local enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: &auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	policydata["Local enabled, LDAP enabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: &auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
			},
		},
	}
	policydata["Local enabled, LDAP disabled"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: &auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
			},
		},
	}

	return policydata
}

func createAuthenticationPolicy(policy *auth.AuthenticationPolicy) *auth.AuthenticationPolicy {
	// create authentication policy object in api server
	createdPolicy, err := apicl.AuthV1().AuthenticationPolicy().Create(context.Background(), policy)
	if err != nil {
		panic("Error creating authentication policy")
	}
	return createdPolicy
}

// deleteAuthenticationPolicy deletes an authentication policy
func deleteAuthenticationPolicy() {
	// delete authentication policy object in api server
	apicl.AuthV1().AuthenticationPolicy().Delete(context.Background(), &api.ObjectMeta{Name: "AuthenticationPolicy"})
}

// TestAuthenticate tests successful authentication for various authentication policies with LDAP and Local Authenticator configured.
// This tests authentication for different order of the authenticators and if they are enabled or disabled.
func TestAuthenticate(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(policy)

		authnmgr, err := New(apisrvURL, "")
		if err != nil {
			panic("Error creating authentication manager")
		}

		// authenticate
		autheduser, ok, err := authnmgr.Authenticate(password.LocalUserPasswordCredential{Username: testUser, Password: testPassword})

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful local user authentication", testtype))
		Assert(t, autheduser.Name == testUser, fmt.Sprintf("[%v] User returned by authentication manager didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_LOCAL.String(), fmt.Sprintf("[%v] User returned is not of type LOCAL", testtype))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))

		deleteAuthenticationPolicy()
	}
}

// TestIncorrectPasswordAuthentication tests failed authentication by all authenticators
func TestIncorrectPasswordAuthentication(t *testing.T) {
	for testtype, policy := range authenticationPoliciesData() {
		createAuthenticationPolicy(policy)

		authnmgr, err := New(apisrvURL, "")
		if err != nil {
			panic("Error creating authentication manager")
		}

		// authenticate
		autheduser, ok, err := authnmgr.Authenticate(password.LocalUserPasswordCredential{Username: testUser, Password: "wrongpassword"})

		Assert(t, !ok, fmt.Sprintf("[%v] Successful local user authentication", testtype))
		Assert(t, autheduser == nil, fmt.Sprintf("[%v] User returned while authenticating with wrong password", testtype))
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned while authenticating with wrong password", testtype))

		deleteAuthenticationPolicy()
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
			Authenticators: &auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String(), auth.Authenticators_RADIUS.String()},
			},
		},
	}
	createAuthenticationPolicy(policy)

	authnmgr, err := New(apisrvURL, "")

	Assert(t, err != nil, "No error returned for un-implemented authenticator")
	Assert(t, authnmgr == nil, "An instance of AuthenticationManager returned when unimplemented authenticator is configured")

	deleteAuthenticationPolicy()
}
