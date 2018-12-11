// Package rbac implements the authz.Authorizer interface using roles base access control.
package rbac

import (
	"sync"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/resolver"
)

// authorizer is RBAC authorizer that implements authz.Authorizer interface
type authorizer struct {
	authz.AbstractAuthorizer
	permissionChecker permissionChecker
}

// NewRBACAuthorizer returns an instance of RBAC based Authorizer
func NewRBACAuthorizer(name, apiServer string, rslver resolver.Interface) authz.Authorizer {
	rbacAuthorizer := &authorizer{
		permissionChecker: newDefaultPermissionChecker(name, apiServer, rslver),
	}
	rbacAuthorizer.AbstractAuthorizer.Authorizer = rbacAuthorizer
	return rbacAuthorizer
}

func (a *authorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	ok, err := a.permissionChecker.checkPermissions(user, operations)
	if ok {
		return true, nil
	}
	return false, err
}

// defaultPermissionChecker implements permissionChecker interface
type defaultPermissionChecker struct {
	permissionGetter PermissionGetter
}

func (pc *defaultPermissionChecker) checkPermissions(user *auth.User, operations []authz.Operation) (bool, error) {
	permissions := pc.permissionGetter.GetPermissions(user)
	for _, operation := range operations {
		// check permissions
		if !permissionsAllow(permissions, operation) {
			return false, nil
		}
	}

	return true, nil
}

func newDefaultPermissionChecker(name, apiServer string, rslver resolver.Interface) permissionChecker {
	return &defaultPermissionChecker{
		permissionGetter: GetPermissionGetter(name, apiServer, rslver),
	}
}

// defaultPermissionGetter is a singleton that implements PermissionGetter interface
var gPermGetter *defaultPermissionGetter
var once sync.Once

type defaultPermissionGetter struct {
	cache   *userPermissionsCache
	watcher *watcher
}

func (pg *defaultPermissionGetter) GetPermissions(user *auth.User) []auth.Permission {
	return pg.cache.getPermissions(user)
}

func (pg *defaultPermissionGetter) GetRolesForUser(user *auth.User) []auth.Role {
	return pg.cache.getRolesForUser(user)
}

func (pg *defaultPermissionGetter) GetRoles(tenant string) []auth.Role {
	return pg.cache.getRoles(tenant)
}

func (pg *defaultPermissionGetter) GetRole(name, tenant string) (auth.Role, bool) {
	return pg.cache.getRole(name, tenant)
}

func (pg *defaultPermissionGetter) GetRoleBindings(tenant string) []auth.RoleBinding {
	return pg.cache.getRoleBindings(tenant)
}

func (pg *defaultPermissionGetter) Stop() {
	pg.watcher.stop()
}

// GetPermissionGetter returns a singleton implementation of PermissionGetter
func GetPermissionGetter(name, apiServer string, rslver resolver.Interface) PermissionGetter {
	once.Do(func() {
		cache := newUserPermissionsCache()
		// start the watcher on api server
		watcher := newWatcher(cache, name, apiServer, rslver)
		gPermGetter = &defaultPermissionGetter{
			cache:   cache,
			watcher: watcher,
		}
	})

	return gPermGetter
}
