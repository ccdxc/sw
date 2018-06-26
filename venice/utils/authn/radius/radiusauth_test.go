package radius

import (
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
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
	radiusURL    = "localhost:1812"
	nasID        = "Venice"
	nasSecret    = "testing123"
	testUser     = "test"
	testPassword = "password"
	testTenant   = "default"
	testGroup    = "NetworkAdmin"
)

var apicl apiclient.Services
var apiSrv apiserver.Server
var apiSrvAddr string

func TestMain(m *testing.M) {
	// run RADIUS tests only if RUN_RADIUS_TESTS env variable is set to true
	if os.Getenv("RUN_RADIUS_TESTS") == "true" {
		setup()
		code := m.Run()
		shutdown()
		os.Exit(code)
	}

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
	logger := log.WithContext("Pkg", "radius_test")
	apicl, err = apiclient.NewGrpcAPIClient("radius_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
}

func shutdown() {
	//stop api server
	apiSrv.Stop()
}

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "radius_test")

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

func createAuthenticationPolicy(radiusConf *auth.Radius) (*auth.AuthenticationPolicy, error) {
	return CreateAuthenticationPolicyWithOrder(apicl, nil, nil, radiusConf, []string{auth.Authenticators_RADIUS.String()}, nil)
}

func TestAuthenticator(t *testing.T) {
	tests := []struct {
		name     string
		user     string
		password string
		policy   *auth.Radius
		expected bool
	}{
		{
			name:     "successful radius auth",
			user:     testUser,
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: true,
		},
		{
			name:     "incorrect radius url",
			user:     testUser,
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        "localhost:1234",
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "incorrect secret",
			user:     testUser,
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     "incorrectSecret",
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "incorrect user",
			user:     "incorrectUser",
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "incorrect password",
			user:     testUser,
			password: "incorrectPassword",
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "username > 253",
			user:     CreateAlphabetString(254),
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "password > 128",
			user:     testUser,
			password: CreateAlphabetString(129),
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: nasID,
			},
			expected: false,
		},
		{
			name:     "NAS ID > 253",
			user:     testUser,
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        radiusURL,
					Secret:     nasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: CreateAlphabetString(254),
			},
			expected: false,
		},
		{
			name:     "multiple radius servers",
			user:     testUser,
			password: testPassword,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{
					{
						Url:        "localhost:1234",
						Secret:     nasSecret,
						AuthMethod: auth.Radius_PAP.String(),
					},
					{
						Url:        radiusURL,
						Secret:     nasSecret,
						AuthMethod: auth.Radius_PAP.String(),
					},
				},
				NasID: nasID,
			},
			expected: true,
		},
	}
	for _, test := range tests {
		policy, err := createAuthenticationPolicy(test.policy)
		if err != nil {
			t.Errorf("Error creating authentication policy: %v", err)
			return
		}
		// create password authenticator
		authenticator := NewRadiusAuthenticator("radius_test", apiSrvAddr, nil, policy.Spec.Authenticators.Radius)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: test.user, Password: test.password})
		DeleteAuthenticationPolicy(apicl)

		Assert(t, test.expected == ok, fmt.Sprintf("[%v] test failed", test.name))
		if test.expected {
			Assert(t, autheduser.Name == testUser, fmt.Sprintf("[%v] User returned by radius authenticator didn't match user being authenticated", test.name))
			Assert(t, autheduser.Tenant == testTenant, fmt.Sprintf("[%v] Incorrect tenant [%v] returned by radius authenticator", test.name, autheduser.Tenant))
			Assert(t, autheduser.Status.UserGroups[0] == testGroup,
				fmt.Sprintf("[%v] Incorrect user group [%v] returned by radius authenticator", test.name, autheduser.Status.UserGroups[0]))
			Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", test.name))
			AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", test.name))
		}
	}
}
