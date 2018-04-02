package manager

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
)

// AuthorizationManager authorizes user and returns authorization information
type authorizationManager struct {
	authorizers []authz.Authorizer
}

// NewAuthorizationManager returns an instance of AuthorizationManager
func NewAuthorizationManager(name, apiServer, resolverUrls string) authz.Authorizer {
	authorizers := make([]authz.Authorizer, 1)
	authorizers[0] = rbac.NewRBACAuthorizer(name, apiServer, resolverUrls)
	return &authorizationManager{
		authorizers: authorizers,
	}
}

// IsAuthorized checks if user is authorized for the given operations. If multiple authorizers are configured and enabled, it will execute them in the order specified.
// If any authorizer fails, access is denied and remaining authorizers are not tried.
// Returns
//   true if all authorizers succeed
func (authzmgr *authorizationManager) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	for _, authorizer := range authzmgr.authorizers {
		ok, err := authorizer.IsAuthorized(user, operations...)
		if !ok {
			// if any authorizer fails to authorize return false
			return false, err
		}
	}
	return true, nil
}
