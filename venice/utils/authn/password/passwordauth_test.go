package password_test

import (
	"os"
	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	. "github.com/pensando/sw/venice/utils/authn/password"
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
)

// test user
var (
	user             *auth.User
	policy           *auth.AuthenticationPolicy
	apicl            apiclient.Services
	apiSrv           apiserver.Server
	apiSrvAddr       string
	testPasswordHash string // password hash for testPassword

	logger = log.WithContext("Pkg", "password_test")

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("passwordauth_test", logger))
)

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}

func setup() {
	var err error
	// api server
	apiSrv, apiSrvAddr, err = serviceutils.StartAPIServer(apisrvURL, "password_test", logger)
	if err != nil {
		panic("Unable to start API Server")
	}

	// api server client
	apicl, err = apiclient.NewGrpcAPIClient("password_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}

	// for tests in hasher_test.go
	cachePasswordHash()
}

func shutdown() {
	if apicl != nil {
		apicl.Close()
	}
	//stop api server
	apiSrv.Stop()
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

func setupAuth() {
	// Create Tenant
	MustCreateTenant(apicl, "default")
	//create test user
	user = MustCreateTestUser(apicl, testUser, testPassword, "default")
	// create auth policy
	policy = MustCreateAuthenticationPolicy(apicl, &auth.Local{}, nil, nil)
}

func cleanupAuth() {
	MustDeleteAuthenticationPolicy(apicl)
	MustDeleteUser(apicl, testUser, "default")
	MustDeleteTenant(apicl, "default")
}

func TestAuthenticate(t *testing.T) {
	setupAuth()
	defer cleanupAuth()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal(), logger)

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})

	Assert(t, ok, "Unsuccessful local user authentication")
	Assert(t, (autheduser.Name == user.Name && autheduser.Tenant == user.Tenant), "User returned by password auth module didn't match user being authenticated")
	AssertOk(t, err, "Error authenticating user")
}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	setupAuth()
	defer cleanupAuth()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal(), logger)

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Tenant: "default", Password: "wrongpassword"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, (autheduser == nil), "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned for incorrect password")
}

func TestIncorrectUserAuthentication(t *testing.T) {
	setupAuth()
	defer cleanupAuth()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal(), logger)

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: "test1", Tenant: "default", Password: "password"})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with incorrect username")
	Assert(t, err != nil, "No error returned for incorrect username")
	Assert(t, err == ErrInvalidCredential, "Incorrect error type returned")
}

func TestAPIServerDown(t *testing.T) {
	setupAuth()
	defer cleanupAuth()

	// create password authenticator
	authenticator := NewPasswordAuthenticator(nil, policy.Spec.Authenticators.GetLocal(), logger)

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with un-initialized API client")
	Assert(t, err == ErrAPIServerClientNotInitialized, "Incorrect error type returned")
	// stop API server
	apiSrv.Stop()
	defer setup()
	// create password authenticator
	authenticator = NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal(), logger)
	// authenticate
	autheduser, ok, err = authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Tenant: "default", Password: testPassword})

	Assert(t, !ok, "Successful local user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating when API server is down")
	Assert(t, err == ErrAPIServerUnavailable, "Incorrect error type returned")
}
