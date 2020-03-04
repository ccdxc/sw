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
		URL:        "10.11.100.102:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "radiususer",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{"Admin"},
	}
}

func getACSNoGroupUserConfig() *RadiusConfig {
	return &RadiusConfig{
		URL:        "10.11.100.102:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "nogroup",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{},
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
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: true,
		},
		{
			name:     "incorrect radius url",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        "localhost:1234",
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "incorrect secret",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     "incorrectSecret",
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "incorrect user",
			user:     "incorrectUser",
			password: config.Password,
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "incorrect password",
			user:     config.User,
			password: "incorrectPassword",
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "username > 253",
			user:     CreateAlphabetString(254),
			password: config.Password,
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "password > 128",
			user:     config.User,
			password: CreateAlphabetString(129),
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
						Servers: []*auth.RadiusServer{{
							Url:        config.URL,
							Secret:     config.NasSecret,
							AuthMethod: auth.Radius_PAP.String(),
						}},
						NasID: config.NasID,
					},
				},
			},
			expected: false,
		},
		{
			name:     "multiple radius servers",
			user:     config.User,
			password: config.Password,
			policy: &auth.Radius{
				Domains: []*auth.RadiusDomain{
					{
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
				},
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
	t.Skip()
	config := getACSConfig()
	testAuthenticator(t, config)
}

func testNoGroupUser(t *testing.T, config *RadiusConfig) {
	radiusConf := &auth.Radius{
		Domains: []*auth.RadiusDomain{
			{
				Servers: []*auth.RadiusServer{{
					Url:        config.URL,
					Secret:     config.NasSecret,
					AuthMethod: auth.Radius_PAP.String(),
				}},
				NasID: config.NasID,
			},
		},
	}
	policy, err := createAuthenticationPolicy(radiusConf)
	if err != nil {
		t.Errorf("Error creating authentication policy: %v", err)
		return
	}
	// create password authenticator
	authenticator := radius.NewRadiusAuthenticator(policy.Spec.Authenticators.Radius)

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.User, Password: config.Password})
	DeleteAuthenticationPolicy(tinfo.apicl)

	Assert(t, ok, "expected user without group to successfully authenticate")

	Assert(t, autheduser.Name == config.User, fmt.Sprintf("User [%s] returned by radius authenticator didn't match user [%s] being authenticated", autheduser.Name, config.User))
	Assert(t, autheduser.Tenant == config.Tenant, fmt.Sprintf("Incorrect tenant [%v] returned by radius authenticator", autheduser.Tenant))
	Assert(t, len(autheduser.Status.GetUserGroups()) == 0, fmt.Sprintf("expected no user groups, got [%#v]", autheduser.Status.GetUserGroups()))
	Assert(t, autheduser.Spec.GetType() == auth.UserSpec_External.String(), "User created is not of type EXTERNAL")
}

func TestACSNoUserGroup(t *testing.T) {
	t.Skip()
	testNoGroupUser(t, getACSNoGroupUserConfig())
}
