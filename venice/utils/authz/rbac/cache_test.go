package rbac

import (
	"fmt"
	"reflect"
	"sort"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	. "github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func printPerms(name string, perms []auth.Permission) (message string) {
	message = fmt.Sprintln("-----------" + name + "------------")
	for _, perm := range perms {
		message = message + fmt.Sprintln(perm)
	}
	return
}

func sortPerms(perms []auth.Permission) {
	sort.Slice(perms, func(i, j int) bool {
		return perms[i].String() < perms[j].String()
	})
}

func arePermsEqual(expected []auth.Permission, returned []auth.Permission) bool {
	sortPerms(expected)
	sortPerms(returned)
	return reflect.DeepEqual(expected, returned)
}

func getUserPermissionTestData() (roleBindings []*auth.RoleBinding, roles []*auth.Role, clusterRoleBindings []*auth.RoleBinding, clusterRoles []*auth.Role) {
	roleBindings = []*auth.RoleBinding{
		NewRoleBinding("NetworkAdminRB", "testTenant", "NetworkAdmin", "Sally", "NetworkAdmin,TenantAdmin"),
		NewRoleBinding("SecurityAdminRB", "testTenant", "SecurityAdmin", "John", "SecurityAdmin,TenantAdmin"),
		NewRoleBinding("TenantAdminRB", "testTenant2", "TenantAdmin", "Deb,Sara", "TenantAdmin"),
		NewRoleBinding("SecurityAdminRBForJill", "testTenant", "SecurityAdmin", "Jill", ""),
	}
	roles = []*auth.Role{
		NewRole("NetworkAdmin", "testTenant",
			NewPermission("testTenant", "Network", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
		NewRole("SecurityAdmin", "testTenant",
			NewPermission("testTenant", "Security", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
		NewRole("TenantAdmin", "testTenant2",
			NewPermission("testTenant2", "Tenant", auth.Permission_Tenant.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()+","+auth.Permission_UPDATE.String()),
			NewPermission("testTenant2", "Security", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			NewPermission("testTenant2", "Network", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			NewPermission("testTenant2", "User", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			NewPermission("testTenant2", "Monitoring", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			NewPermission("testTenant2", "", auth.Permission_APIEndpoint.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			NewPermission("testTenant2", "Workload", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
	}
	clusterRoleBindings = []*auth.RoleBinding{
		NewClusterRoleBinding("SuperAdminRB", "SuperAdmin", "", "SuperAdmin"),
	}
	clusterRoles = []*auth.Role{
		NewClusterRole("SuperAdmin", NewPermission(ResourceTenantAll, ResourceGroupAll, auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
	}
	return
}

func initializeCache() *userPermissionsCache {
	roleBindings, roles, clusterRoleBindings, clusterRoles := getUserPermissionTestData()
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
	return cache
}

func TestGetPermissions(t *testing.T) {
	tests := []struct {
		name          string
		user          *auth.User
		expectedPerms []auth.Permission
	}{
		{
			name: "network admin",
			user: newUser("Sally", "testTenant", "NetworkAdmin"),
			expectedPerms: []auth.Permission{
				NewPermission("testTenant", "Network", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			},
		},
		{
			name: "security admin",
			user: newUser("John", "testTenant", "SecurityAdmin"),
			expectedPerms: []auth.Permission{
				NewPermission("testTenant", "Security", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			},
		},
		{
			name: "network and security admin",
			user: newUser("Tim", "testTenant", "NetworkAdmin,SecurityAdmin"),
			expectedPerms: []auth.Permission{
				NewPermission("testTenant", "Network", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				NewPermission("testTenant", "Security", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			},
		},
		{
			name: "security admin with role binding to an user",
			user: newUser("Jill", "testTenant", ""),
			expectedPerms: []auth.Permission{
				NewPermission("testTenant", "Security", auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			},
		},
	}

	cache := initializeCache()
	for _, test := range tests {
		perms := cache.getPermissions(test.user)
		Assert(t, len(perms) == len(test.expectedPerms), fmt.Sprintf("length unequal [%d] expected [%d] [%v] test failed", len(perms), len(test.expectedPerms), test.name))
		Assert(t, arePermsEqual(test.expectedPerms, perms),
			fmt.Sprintf("[%v] test failed\n%s\n%s", test.name, printPerms("Returned Perms", perms), printPerms("Expected Perms", test.expectedPerms)))
	}

}

func TestGetClusterPermissions(t *testing.T) {
	tests := []struct {
		name          string
		user          *auth.User
		expectedPerms []auth.Permission
	}{
		{
			name: "super admin",
			user: newClusterUser("Dorota", "SuperAdmin"),
			expectedPerms: []auth.Permission{
				NewPermission(ResourceTenantAll, ResourceGroupAll, auth.Permission_AllResourceKinds.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
			},
		},
	}
	cache := initializeCache()
	for _, test := range tests {
		perms := cache.getPermissions(test.user)
		Assert(t, arePermsEqual(test.expectedPerms, perms),
			fmt.Sprintf("[%v] test failed\n%s\n%s", test.name, printPerms("Returned Perms", perms), printPerms("Expected Perms", test.expectedPerms)))
	}
}

func TestResetCache(t *testing.T) {
	cache := initializeCache()
	Assert(t, len(cache.roleBindings["testTenant"]) == 3,
		fmt.Sprintf("role binding cache contains incorrect number of role bindings: %d", len(cache.roleBindings["testTenant"])))
	Assert(t, len(cache.roles["testTenant"]) == 2,
		fmt.Sprintf("role cache contains incorrect number of roles: %d", len(cache.roles["testTenant"])))
	cache.reset()
	Assert(t, len(cache.roleBindings) == 0, " role binding cache should be empty")
	Assert(t, len(cache.roles) == 0, " role cache should be empty")
}

func TestCacheOps(t *testing.T) {
	roleBindings, roles, clusterRoleBindings, clusterRoles := getUserPermissionTestData()
	cache := initializeCache()
	// testing add
	Assert(t, len(cache.roleBindings["testTenant"]) == 3,
		fmt.Sprintf("role binding cache contains incorrect number of role bindings: %d", len(cache.roleBindings["testTenant"])))
	Assert(t, len(cache.getRoleBindings("testTenant")) == 3,
		fmt.Sprintf("role binding cache contains incorrect number of role bindings: %d", len(cache.roleBindings["testTenant"])))
	Assert(t, len(cache.roles["testTenant"]) == 2,
		fmt.Sprintf("role cache contains incorrect number of roles: %d", len(cache.roles["testTenant"])))
	Assert(t, len(cache.getRoles("testTenant")) == 2,
		fmt.Sprintf("role cache contains incorrect number of roles: %d", len(cache.roles["testTenant"])))
	// testing delete
	cache.deleteRole(roles[0])
	Assert(t, len(cache.roles["testTenant"]) == 1,
		fmt.Sprintf("role cache contains incorrect number of roles after deleting one: %d", len(cache.roles["testTenant"])))

	_, ok := cache.getRole(roles[0].Name, roles[0].Tenant)
	Assert(t, !ok,
		fmt.Sprintf("role cache still contains role [%s] after deletion", roles[0].Name))

	cache.deleteRoleBinding(roleBindings[0])
	Assert(t, len(cache.roleBindings["testTenant"]) == 2,
		fmt.Sprintf("role binding cache contains incorrect number of role bindings after deleting one: %d", len(cache.roleBindings["testTenant"])))

	_, ok = cache.getRoleBinding(roleBindings[0].Name, roleBindings[0].Tenant)
	Assert(t, !ok,
		fmt.Sprintf("role binding cache still contains role binding [%s] after deletion", roleBindings[0].Name))

	Assert(t, len(cache.roleBindings[globals.DefaultTenant]) == 1,
		fmt.Sprintf("role binding cache contains incorrect number of cluster role bindings: %d", len(cache.roleBindings[globals.DefaultTenant])))
	Assert(t, len(cache.roles[globals.DefaultTenant]) == 1,
		fmt.Sprintf("role cache contains incorrect number of cluster roles: %d", len(cache.roles[globals.DefaultTenant])))
	cache.deleteRole(clusterRoles[0])
	Assert(t, len(cache.roles[globals.DefaultTenant]) == 0,
		fmt.Sprintf("cluster role cache contains incorrect number of roles after deleting one: %d", len(cache.roles[globals.DefaultTenant])))

	_, ok = cache.getRole(clusterRoles[0].Name, clusterRoles[0].Tenant)
	Assert(t, !ok,
		fmt.Sprintf("role cache still contains cluster role [%s] after deletion", clusterRoles[0].Name))

	cache.deleteRoleBinding(clusterRoleBindings[0])
	Assert(t, len(cache.roleBindings[globals.DefaultTenant]) == 0,
		fmt.Sprintf("role binding cache contains incorrect number of cluster role bindings after deleting one: %d", len(cache.roleBindings[globals.DefaultTenant])))

	_, ok = cache.getRoleBinding(clusterRoleBindings[0].Name, clusterRoleBindings[0].Tenant)
	Assert(t, !ok,
		fmt.Sprintf("role binding cache still contains cluster role binding [%s] after deletion", clusterRoleBindings[0].Name))

	cache.initializeCacheForTenant("testTenant3")
	Assert(t, len(cache.roleBindings) == 4, fmt.Sprintf("incorrect number of tenant role cache: %d", len(cache.roleBindings)))
	Assert(t, len(cache.roles) == 4, fmt.Sprintf("incorrect number of tenant role cache: %d", len(cache.roles)))
	cache.deleteCacheForTenant("testTenant2")
	Assert(t, len(cache.roleBindings) == 3, fmt.Sprintf("incorrect number of tenant role cache: %d", len(cache.roleBindings)))
	Assert(t, len(cache.roles) == 3, fmt.Sprintf("incorrect number of tenant role cache: %d", len(cache.roles)))
}

func TestHas(t *testing.T) {
	Assert(t, !has(strings.Split("", ","), ""), "should fail for empty string match")
	Assert(t, !has([]string{"a", "b"}, "d"), "should fail for element d")
	Assert(t, !has(strings.Split("", ","), "d"), "should fail for slice containing empty element")
	Assert(t, has([]string{"a", "b"}, "b"), "should succeed for element b")
}

func TestHasAny(t *testing.T) {
	Assert(t, !hasAny(strings.Split("", ","), strings.Split("", ",")), "empty strings matched")
	Assert(t, !hasAny(nil, nil), "nil slice matched")
	Assert(t, !hasAny(strings.Split("", ","), nil), "slice with empty string matched nil slice")
	Assert(t, !hasAny(nil, strings.Split("", ",")), "nil slice matched slice with empty string")
	Assert(t, !hasAny([]string{"a", "b"}, []string{""}), "expect slice not to match any element in the set")
	Assert(t, hasAny([]string{"a", "b"}, []string{"b"}), "expect slice to match element b in the set")
	Assert(t, hasAny([]string{"a", "b"}, []string{"c", "a"}), "expect slice to match element a in the set")
}
