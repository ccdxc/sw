// Package rbac implements the authz.Authorizer interface using roles base access control.
package rbac

import (
	"sync"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// authorizer is RBAC authorizer that implements authz.Authorizer interface
type authorizer struct {
	permissionChecker permissionChecker
}

// NewRBACAuthorizer returns an instance of RBAC based Authorizer
func NewRBACAuthorizer(name, apiServer, resolverUrls string) authz.Authorizer {
	return &authorizer{
		permissionChecker: newDefaultPermissionChecker(name, apiServer, resolverUrls),
	}
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
	permissionGetter permissionGetter
}

func (pc *defaultPermissionChecker) checkPermissions(user *auth.User, operations []authz.Operation) (bool, error) {
	permissions := pc.permissionGetter.getPermissions(user)
	for _, operation := range operations {
		// check permissions
		if !permissionsAllow(permissions, operation) {
			return false, nil
		}
	}

	return true, nil
}

func newDefaultPermissionChecker(name, apiServer, resolverUrls string) permissionChecker {
	return &defaultPermissionChecker{
		permissionGetter: getPermissionGetter(name, apiServer, resolverUrls),
	}
}

// defaultPermissionGetter is a singleton that implements permissionGetter interface
var gPermGetter *defaultPermissionGetter
var once sync.Once

type defaultPermissionGetter struct {
	cache   *userPermissionsCache
	watcher *watcher
}

func (pg *defaultPermissionGetter) getPermissions(user *auth.User) []auth.Permission {
	return pg.cache.getPermissions(user)
}

func (pg *defaultPermissionGetter) getRoles(user *auth.User) []auth.Role {
	return pg.cache.getRoles(user)
}

func (pg *defaultPermissionGetter) stop() {
	pg.watcher.stop()
}

// getPermissionGetter returns a singleton implementation of permissionGetter
func getPermissionGetter(name, apiServer, resolverUrls string) permissionGetter {
	once.Do(func() {
		cache := newUserPermissionsCache()
		// start the watcher on api server
		watcher := newWatcher(cache, name, apiServer, resolverUrls)
		gPermGetter = &defaultPermissionGetter{
			cache:   cache,
			watcher: watcher,
		}
	})

	return gPermGetter
}
