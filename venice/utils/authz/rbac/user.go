package rbac

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

type userPermissionChecker struct {
	perms []auth.Permission
}

func (pc *userPermissionChecker) checkPermissions(user *auth.User, operations []authz.Operation) (bool, error) {
	for _, operation := range operations {
		// check permissions
		if !permissionsAllow(pc.perms, operation) {
			return false, nil
		}
	}
	return true, nil
}

func (pc *userPermissionChecker) stop() {}

// NewUserAuthorizer returns an instance of user authorizer to check if an operation is allowed against his assigned permissions
func NewUserAuthorizer(perms []auth.Permission) authz.Authorizer {
	userAuthorizer := &authorizer{
		permissionChecker: &userPermissionChecker{perms: perms},
	}
	userAuthorizer.AbstractAuthorizer.Authorizer = userAuthorizer
	return userAuthorizer
}
