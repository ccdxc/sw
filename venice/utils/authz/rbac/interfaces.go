package rbac

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// permissionGetter abstracts out retrieval of roles and permissions for an user
type permissionGetter interface {
	// getPermissions retrieves permissions for user
	getPermissions(user *auth.User) []auth.Permission

	// getRoles retrieves roles for user
	getRoles(user *auth.User) []auth.Role

	// stop un-initializes permission getter
	stop()
}

// permissionChecker abstracts out permission checking computation.
type permissionChecker interface {
	// checkPermissions returns true if all requested operations are authorized
	checkPermissions(user *auth.User, requestedOperations []authz.Operation) (bool, error)
}
