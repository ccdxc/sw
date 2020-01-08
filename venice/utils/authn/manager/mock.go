package manager

import (
	"errors"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	invalidUserClaim   = "invalidUser"
	invalidTenantClaim = "invalidTenant"
	// NonExistentUserClaim simulates non existent user
	NonExistentUserClaim = "nonExistentUser"
	invalidToken         = "invalidToken"
)

type mockTokenManager struct{}

func (tm *mockTokenManager) ValidateToken(token string) (map[string]interface{}, bool, error) {
	claims := make(map[string]interface{})
	switch token {
	case invalidUserClaim:
		claims[SubClaim] = 0
	case invalidTenantClaim:
		claims[TenantClaim] = 0
	case NonExistentUserClaim:
		claims[SubClaim] = NonExistentUserClaim
	case invalidToken:
		return nil, false, nil
	default:
		claims[SubClaim] = "test"
		claims[TenantClaim] = "default"
	}
	return claims, true, nil
}

func (tm *mockTokenManager) CreateToken(*auth.User, map[string]interface{}) (string, time.Time, error) {
	return "", time.Time{}, nil
}

func (tm *mockTokenManager) Get(token, key string) (interface{}, bool, error) {
	return nil, false, nil
}

type mockAuthGetter struct {
	policy        *auth.AuthenticationPolicy
	simulateError bool
}

func (ag *mockAuthGetter) GetUser(name, tenant string) (*auth.User, bool) {
	switch name {
	case NonExistentUserClaim:
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
				Email:    name + "@pensando.io",
				Type:     auth.UserSpec_Local.String(),
			},
		}
		return user, true
	}

}

// GetAuthenticationPolicy returns authentication policy
func (ag *mockAuthGetter) GetAuthenticationPolicy() (*auth.AuthenticationPolicy, error) {
	if ag.simulateError {
		return nil, errors.New("authentication policy not found")
	}
	return ag.policy, nil
}

func (ag *mockAuthGetter) GetAuthenticators() ([]authn.Authenticator, error) {
	return nil, nil
}

func (ag *mockAuthGetter) GetTokenManager() (TokenManager, error) {
	return &mockTokenManager{}, nil
}

func (ag *mockAuthGetter) IsAuthBootstrapped() (bool, error) {
	return true, nil
}

func (ag *mockAuthGetter) Stop() {}

func (ag *mockAuthGetter) Start() {}

// NewMockAuthenticationManager creates a mock AuthenticationManager with a mock AuthGetter to return hardcoded user and
// authentication policy for testing
func NewMockAuthenticationManager() *AuthenticationManager {
	mockAuthnmgr := &AuthenticationManager{
		AuthGetter: NewMockAuthGetter(&auth.AuthenticationPolicy{
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
				Secret: nil,
			},
		}, false),
		logger: log.WithContext("Pkg", "authnmgr_mock"),
	}
	return mockAuthnmgr
}

// NewMockAuthGetter returns a mock auth getter
func NewMockAuthGetter(policy *auth.AuthenticationPolicy, simulateError bool) AuthGetter {
	return &mockAuthGetter{
		policy:        policy,
		simulateError: simulateError,
	}
}
