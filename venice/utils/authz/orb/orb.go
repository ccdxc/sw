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
	var unauthorizedOps []authz.Operation
	for _, op := range operations {
		ok, _ := a.owner.IsAuthorized(user, op)
		if !ok {
			unauthorizedOps = append(unauthorizedOps, op)
		}
	}
	if len(unauthorizedOps) == 0 {
		return true, nil
	}
	return a.rbac.IsAuthorized(user, unauthorizedOps...)
}

func (a *authorizer) Stop() {
	a.rbac.Stop()
}
