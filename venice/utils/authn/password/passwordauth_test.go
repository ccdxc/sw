package password_test

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
	. "github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks"
)

const (
	apisrvURL    = "localhost:0"
	testUser     = "test"
	testPassword = "pensandoo0"
)

// test user
var user *auth.User
var apicl apiclient.Services
var apiSrv apiserver.Server
var apiSrvAddr string

// password hash for testPassword
var testPasswordHash string

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
	logger := log.WithContext("Pkg", "password_test")
	apicl, err = apiclient.NewGrpcAPIClient(apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}

	//create test user
	user = createTestUser()

	//for tests in passwordhasher_test.go
	cachePasswordHash()
}

func shutdown() {
	//stop api server
	apiSrv.Stop()
}

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "password_test")

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

func cachePasswordHash() {
	// get password hasher
	hasher := GetPasswordHasher()
	// generate hash
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		panic("Error creating password hash")
	}
	//cache password for tests in passwordhasher_test.go
	testPasswordHash = passwdhash
}

// createTestUser creates a test user and caches its password in testPasswordHash global variable
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

// createDefaultAuthenticationPolicy creates an authentication policy
func createAuthenticationPolicy(enabled bool) *auth.AuthenticationPolicy {
	// authn policy object
	policy := auth.AuthenticationPolicy{
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
					Enabled: enabled,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}

	// create authentication policy object in api server
	_, err := apicl.AuthV1().AuthenticationPolicy().Create(context.Background(), &policy)
	if err != nil {
		panic("Error creating authentication policy")
	}
	return &policy
}

// deleteAuthenticationPolicy deletes an authentication policy
func deleteAuthenticationPolicy() {
	// delete authentication policy object in api server
	apicl.AuthV1().AuthenticationPolicy().Delete(context.Background(), &api.ObjectMeta{Name: "AuthenticationPolicy"})
}

func TestAuthenticate(t *testing.T) {
	policy := createAuthenticationPolicy(true)

	defer deleteAuthenticationPolicy()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(LocalUserPasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})

	Assert(t, ok, "Unsuccessful local user authentication")
	Assert(t, (autheduser.Name == user.Name && autheduser.Tenant == user.Tenant), "User returned by password auth module didn't match user being authenticated")
	AssertOk(t, err, "Error authenticating user")
}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	policy := createAuthenticationPolicy(true)

	defer deleteAuthenticationPolicy()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(LocalUserPasswordCredential{Username: testUser, Tenant: "default", Password: "wrongpassword"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, (autheduser == nil), "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned for incorrect password")
}

func TestIncorrectUserAuthentication(t *testing.T) {
	policy := createAuthenticationPolicy(true)

	defer deleteAuthenticationPolicy()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(LocalUserPasswordCredential{Username: "test1", Tenant: "default", Password: "password"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with incorrect username")
	Assert(t, err != nil, "No error returned for incorrect username")
	Assert(t, err == ErrInvalidCredential, "Incorrect error type returned")
}

func TestDisabledPasswordAuthenticator(t *testing.T) {
	policy := createAuthenticationPolicy(false)

	defer deleteAuthenticationPolicy()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(LocalUserPasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned with disabled password authenticator")
	AssertOk(t, err, "Error returned with disabled password authenticator")
}
