package manager

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// alwaysAllowAuthorizer implements Authorizer interface. It always allows an operation.
type alwaysAllowAuthorizer struct{}

func (a *alwaysAllowAuthorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	return true, nil
}

func newAlwaysAllowAuthorizer() authz.Authorizer {
	return &alwaysAllowAuthorizer{}
}

// alwaysDenyAuthorizer implements Authorizer interface. It always denies an operation.
type alwaysDenyAuthorizer struct{}

func (a *alwaysDenyAuthorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	return false, nil
}

func newAlwaysDenyAuthorizer() authz.Authorizer {
	return &alwaysDenyAuthorizer{}
}

func TestAuthorizationManager(t *testing.T) {
	tests := []struct {
		name        string
		authorizers []authz.Authorizer
		expected    bool
	}{
		{
			name:        "all authorizers allow",
			authorizers: []authz.Authorizer{newAlwaysAllowAuthorizer(), newAlwaysAllowAuthorizer()},
			expected:    true,
		},
		{
			name:        "first authorizer denies",
			authorizers: []authz.Authorizer{newAlwaysDenyAuthorizer(), newAlwaysAllowAuthorizer()},
			expected:    false,
		},
		{
			name:        "second authorizer denies",
			authorizers: []authz.Authorizer{newAlwaysAllowAuthorizer(), newAlwaysDenyAuthorizer()},
			expected:    false,
		},
		{
			name:        "all authorizers deny",
			authorizers: []authz.Authorizer{newAlwaysDenyAuthorizer(), newAlwaysDenyAuthorizer()},
			expected:    false,
		},
	}
	for _, test := range tests {
		authzMgr := &authorizationManager{
			authorizers: test.authorizers,
		}
		ok, _ := authzMgr.IsAuthorized(nil, nil)
		Assert(t, test.expected == ok, fmt.Sprintf("[%v] test failed", test.name))
	}

	authzMgr := NewAuthorizationManager("authzmgr_test", "", "").(*authorizationManager)
	Assert(t, len(authzMgr.authorizers) == 1, "authorization manager not initialized with an authorizer")
}
