package auth

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn/radius"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func getACSConfig() *RadiusConfig {
	return &RadiusConfig{
		URL:        "10.11.100.101:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "radiususer",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{"Admin"},
	}
}

func createAuthenticationPolicy(radiusConf *auth.Radius) (*auth.AuthenticationPolicy, error) {
	return CreateAuthenticationPolicyWithOrder(tinfo.apicl, nil, nil, radiusConf, []string{auth.Authenticators_RADIUS.String()}, ExpiryDuration)
}

func testAuthenticator(t *testing.T, config *RadiusConfig) {
	tests := []struct {
		name     string
		user     string
		password string
		policy   *auth.Radius
		expected bool
	}{
		{
			name:     "successful radius auth",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: true,
		},
		{
			name:     "incorrect radius url",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        "localhost:1234",
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "incorrect secret",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     "incorrectSecret",
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "incorrect user",
			user:     "incorrectUser",
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "incorrect password",
			user:     config.User,
			password: "incorrectPassword",
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "username > 253",
			user:     CreateAlphabetString(254),
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "password > 128",
			user:     config.User,
			password: CreateAlphabetString(129),
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
			expected: false,
		},
		{
			name:     "NAS ID > 253",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: CreateAlphabetString(254),
			},
			expected: false,
		},
		{
			name:     "multiple radius servers",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Enabled: true,
				Servers: []*auth.RadiusServer{
					{
						Url:        "localhost:1234",
						Secret:     config.NasSecret,
						AuthMethod: auth.Radius_PAP.String(),
					},
					{
						Url:        config.URL,
						Secret:     config.NasSecret,
						AuthMethod: auth.Radius_PAP.String(),
					},
				},
				NasID: config.NasID,
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
		authenticator := radius.NewRadiusAuthenticator(policy.Spec.Authenticators.Radius)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: test.user, Password: test.password})
		DeleteAuthenticationPolicy(tinfo.apicl)

		Assert(t, test.expected == ok, fmt.Sprintf("[%v] test failed", test.name))
		if test.expected {
			Assert(t, autheduser.Name == config.User, fmt.Sprintf("[%v] User returned by radius authenticator didn't match user being authenticated", test.name))
			Assert(t, autheduser.Tenant == config.Tenant, fmt.Sprintf("[%v] Incorrect tenant [%v] returned by radius authenticator", test.name, autheduser.Tenant))
			userGroups := autheduser.Status.GetUserGroups()
			sort.Strings(userGroups)
			sort.Strings(config.UserGroups)
			Assert(t, reflect.DeepEqual(userGroups, config.UserGroups), fmt.Sprintf("[%v] Incorrect user group returned, expected [%v], got [%v]", test, config.UserGroups, userGroups))
			Assert(t, autheduser.Spec.GetType() == auth.UserSpec_External.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", test.name))
			AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", test.name))
		}
	}
}

func TestACSAuthentication(t *testing.T) {
	config := getACSConfig()
	testAuthenticator(t, config)
}
