package rbac

import "github.com/pensando/sw/api/generated/auth"

// NewMockPermissionGetter creates a mock permission getter
func NewMockPermissionGetter(roles []*auth.Role, roleBindings []*auth.RoleBinding, clusterRoles []*auth.Role, clusterRoleBindings []*auth.RoleBinding) PermissionGetter {
	cache := newUserPermissionsCache()
	for _, role := range roles {
		cache.addRole(role)
	}
	for _, roleBinding := range roleBindings {
		cache.addRoleBinding(roleBinding)
	}
	for _, role := range clusterRoles {
		cache.addRole(role)
	}
	for _, roleBinding := range clusterRoleBindings {
		cache.addRoleBinding(roleBinding)
	}

	return &defaultPermissionGetter{
		cache: cache,
	}
}
