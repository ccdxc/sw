package rbac

import "github.com/pensando/sw/api/generated/auth"

// RoleManager provides role and permission information about a user
type RoleManager struct {
	permGetter permissionGetter
}

// NewRoleManager creates a new instance of RoleManager
func NewRoleManager(name, apiServer, resolverUrls string) *RoleManager {
	return &RoleManager{
		permGetter: getPermissionGetter(name, apiServer, resolverUrls),
	}
}

// GetPermissions retrieves permissions for an user
func (rolemgr *RoleManager) GetPermissions(user *auth.User) []auth.Permission {
	return rolemgr.permGetter.getPermissions(user)
}

// GetRoles retrieves roles for an user
func (rolemgr *RoleManager) GetRoles(user *auth.User) []auth.Role {
	return rolemgr.permGetter.getRoles(user)
}
