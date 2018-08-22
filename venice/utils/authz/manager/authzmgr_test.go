package manager

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAuthorizationManager(t *testing.T) {
	tests := []struct {
		name        string
		authorizers []authz.Authorizer
		expected    bool
	}{
		{
			name:        "all authorizers allow",
			authorizers: []authz.Authorizer{NewAlwaysAllowAuthorizer(), NewAlwaysAllowAuthorizer()},
			expected:    true,
		},
		{
			name:        "first authorizer denies",
			authorizers: []authz.Authorizer{NewAlwaysDenyAuthorizer(), NewAlwaysAllowAuthorizer()},
			expected:    false,
		},
		{
			name:        "second authorizer denies",
			authorizers: []authz.Authorizer{NewAlwaysAllowAuthorizer(), NewAlwaysDenyAuthorizer()},
			expected:    false,
		},
		{
			name:        "all authorizers deny",
			authorizers: []authz.Authorizer{NewAlwaysDenyAuthorizer(), NewAlwaysDenyAuthorizer()},
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

	authzMgr := NewAuthorizationManager("authzmgr_test", "", nil).(*authorizationManager)
	Assert(t, len(authzMgr.authorizers) == 1, "authorization manager not initialized with an authorizer")
}
