package orb

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/owner"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/resolver"
)

type authorizer struct {
	authz.AbstractAuthorizer
	owner authz.Authorizer
	rbac  authz.Authorizer
}

// NewORBAuthorizer checks for owner authorization before checking role-based authorization
func NewORBAuthorizer(name, apiServer string, rslver resolver.Interface) authz.Authorizer {
	orbAuthorizer := &authorizer{
		owner: owner.NewOwnerAuthorizer(),
		rbac:  rbac.NewRBACAuthorizer(name, apiServer, rslver),
	}
	orbAuthorizer.AbstractAuthorizer.Authorizer = orbAuthorizer
	return orbAuthorizer
}

func (a *authorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	ok, err := a.owner.IsAuthorized(user, operations...)
	if ok {
		return ok, err
	}
	return a.rbac.IsAuthorized(user, operations...)
}
