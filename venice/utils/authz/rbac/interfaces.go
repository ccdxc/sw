package rbac

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// PermissionGetter abstracts out retrieval of roles and permissions for an user
type PermissionGetter interface {
	// GetPermissions retrieves permissions for user
	GetPermissions(user *auth.User) []auth.Permission

	// GetRolesForUser retrieves roles for user
	GetRolesForUser(user *auth.User) []auth.Role

	// GetRoles retrieves roles within a tenant
	GetRoles(tenant string) []auth.Role

	// GetRole returns role given the role and tenant name
	GetRole(name, tenant string) (auth.Role, bool)

	// GetRoleBindings retrieves role bindings within a given tenant
	GetRoleBindings(tenant string) []auth.RoleBinding

	// Start initializes/re-initializes PermissionGetter. It blocks if AuthGetter has not been un-initialized through Stop()
	Start()

	// Stop un-initializes PermissionGetter
	Stop()
}

// permissionChecker abstracts out permission checking computation.
type permissionChecker interface {
	// checkPermissions returns true if all requested operations are authorized
	checkPermissions(user *auth.User, requestedOperations []authz.Operation) (bool, error)

	// stops the permission checker
	stop()
}
