package rbac

import (
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
)

// userPermissionsCache is a cache of roles and role bindings. It implements permissionCache interface.
type userPermissionsCache struct {
	// mutex to synchronize access to roles and role bindings cache
	sync.RWMutex
	// cache roles with tenant as the key
	roles map[string]map[string]*auth.Role
	// cache role bindings with tenant as the key
	roleBindings map[string]map[string]*auth.RoleBinding
}

func (c *userPermissionsCache) reset() {
	defer c.Unlock()
	c.Lock()
	// reset cache
	c.roles = make(map[string]map[string]*auth.Role)
	c.roleBindings = make(map[string]map[string]*auth.RoleBinding)
}

func (c *userPermissionsCache) getPermissions(user *auth.User) []auth.Permission {
	var permissions []auth.Permission
	roles := c.getRoles(user)
	for _, role := range roles {
		permissions = append(permissions, role.Spec.GetPermissions()...)
	}
	return permissions
}

func (c *userPermissionsCache) getRoles(user *auth.User) []auth.Role {
	var roles []auth.Role

	defer c.RUnlock()
	c.RLock()
	if user == nil {
		return nil
	}
	tenant := user.GetTenant()
	roleBindings := c.roleBindings[tenant]
	// check which role bindings apply to user and accumulate roles
	for _, roleBinding := range roleBindings {
		if has(roleBinding.Spec.GetUsers(), user.GetName()) ||
			hasAny(roleBinding.Spec.GetUserGroups(), user.Status.GetUserGroups()) {
			role := c.roles[tenant][getKey(tenant, roleBinding.Spec.GetRole())]
			// accumulate role
			roles = append(roles, *role)
		}
	}
	return roles
}

func (c *userPermissionsCache) addRole(role *auth.Role) {
	defer c.Unlock()
	c.Lock()
	// create role cache for tenant if it doesn't exist
	_, ok := c.roles[role.GetTenant()]
	if !ok {
		c.roles[role.GetTenant()] = make(map[string]*auth.Role)
	}
	c.roles[role.GetTenant()][getKey(role.GetTenant(), role.GetName())] = role
}

func (c *userPermissionsCache) deleteRole(role *auth.Role) {
	defer c.Unlock()
	c.Lock()
	delete(c.roles[role.GetTenant()], getKey(role.GetTenant(), role.GetName()))
}

func (c *userPermissionsCache) getRole(ometa *api.ObjectMeta) *auth.Role {
	defer c.Unlock()
	c.Lock()
	return c.roles[ometa.GetTenant()][getKey(ometa.GetTenant(), ometa.GetName())]
}

func (c *userPermissionsCache) addRoleBinding(roleBinding *auth.RoleBinding) {
	defer c.Unlock()
	c.Lock()
	// create role binding cache for tenant if it doesn't exist
	_, ok := c.roleBindings[roleBinding.GetTenant()]
	if !ok {
		c.roleBindings[roleBinding.GetTenant()] = make(map[string]*auth.RoleBinding)
	}
	c.roleBindings[roleBinding.GetTenant()][getKey(roleBinding.GetTenant(), roleBinding.GetName())] = roleBinding
}

func (c *userPermissionsCache) deleteRoleBinding(roleBinding *auth.RoleBinding) {
	defer c.Unlock()
	c.Lock()
	delete(c.roleBindings[roleBinding.GetTenant()], getKey(roleBinding.GetTenant(), roleBinding.GetName()))
}

func (c *userPermissionsCache) getRoleBinding(ometa *api.ObjectMeta) *auth.RoleBinding {
	defer c.RUnlock()
	c.RLock()
	return c.roleBindings[ometa.GetTenant()][getKey(ometa.GetTenant(), ometa.GetName())]
}

func (c *userPermissionsCache) initializeCacheForTenant(tenant string) {
	defer c.Unlock()
	c.Lock()
	// create role and role binding cache for tenant
	c.roles[tenant] = make(map[string]*auth.Role)
	c.roleBindings[tenant] = make(map[string]*auth.RoleBinding)
}

func (c *userPermissionsCache) deleteCacheForTenant(tenant string) {
	defer c.Unlock()
	c.Lock()
	// delete role and role binding cache for tenant
	delete(c.roles, tenant)
	delete(c.roleBindings, tenant)
}

// for testing
func (c *userPermissionsCache) getRoleCache() map[string]map[string]*auth.Role {
	return c.roles
}

// for testing
func (c *userPermissionsCache) getRoleBindingCache() map[string]map[string]*auth.RoleBinding {
	return c.roleBindings
}

// newUserPermissionsCache returns an instance of userPermissionsCache
func newUserPermissionsCache() *userPermissionsCache {
	return &userPermissionsCache{
		roles:        make(map[string]map[string]*auth.Role),
		roleBindings: make(map[string]map[string]*auth.RoleBinding),
	}
}

func getKey(tenant string, name string) string {
	return tenant + "/" + name
}

func has(set []string, ele string) bool {
	// do not match empty string
	if ele == "" {
		return false
	}
	for _, s := range set {
		if s == ele {
			return true
		}
	}
	return false
}

func hasAny(set, contains []string) bool {
	owning := make(map[string]struct{}, len(set))
	for _, ele := range set {
		// do not add empty string
		if ele != "" {
			owning[ele] = struct{}{}
		}
	}
	for _, ele := range contains {
		if _, ok := owning[ele]; ok {
			return true
		}
	}
	return false
}
