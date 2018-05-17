package password_test

import (
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/password"
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
	apicl, err = apiclient.NewGrpcAPIClient("password_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}

	//create test user
	user = CreateTestUser(apicl, testUser, testPassword, "default")

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
	sch := runtime.GetDefaultScheme()
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

func TestAuthenticate(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})

	defer DeleteAuthenticationPolicy(apicl)

	// create password authenticator
	authenticator := NewPasswordAuthenticator("password_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&authn.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})

	Assert(t, ok, "Unsuccessful local user authentication")
	Assert(t, (autheduser.Name == user.Name && autheduser.Tenant == user.Tenant), "User returned by password auth module didn't match user being authenticated")
	AssertOk(t, err, "Error authenticating user")
}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})

	defer DeleteAuthenticationPolicy(apicl)

	// create password authenticator
	authenticator := NewPasswordAuthenticator("password_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&authn.PasswordCredential{Username: testUser, Tenant: "default", Password: "wrongpassword"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, (autheduser == nil), "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned for incorrect password")
}

func TestIncorrectUserAuthentication(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})

	defer DeleteAuthenticationPolicy(apicl)

	// create password authenticator
	authenticator := NewPasswordAuthenticator("password_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&authn.PasswordCredential{Username: "test1", Tenant: "default", Password: "password"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with incorrect username")
	Assert(t, err != nil, "No error returned for incorrect username")
	Assert(t, err == ErrInvalidCredential, "Incorrect error type returned")
}

func TestDisabledPasswordAuthenticator(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: false}, &auth.Ldap{Enabled: false})

	defer DeleteAuthenticationPolicy(apicl)

	// create password authenticator
	authenticator := NewPasswordAuthenticator("password_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLocal())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&authn.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned with disabled password authenticator")
	AssertOk(t, err, "Error returned with disabled password authenticator")
}
