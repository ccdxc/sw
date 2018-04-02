package rbac

import (
	"fmt"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	. "github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testTenant = "testTenant"
)

func stringToSlice(val string) (ret []string) {
	if val == "" || val == "," {
		ret = nil
	} else {
		ret = strings.Split(val, ",")
	}
	return
}

func newPermission(tenant, resourceGroup, resourceKind, resourceNamespace, resourceNames, actions string) auth.Permission {
	return auth.Permission{
		ResourceTenant:    tenant,
		ResourceGroup:     resourceGroup,
		ResourceKind:      resourceKind,
		ResourceNamespace: resourceNamespace,
		ResourceNames:     stringToSlice(resourceNames),
		Actions:           stringToSlice(actions),
	}
}

func newRole(name, tenant string, permissions ...auth.Permission) *auth.Role {
	return &auth.Role{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Spec: auth.RoleSpec{
			Permissions: permissions,
		},
	}
}

func newRoleBinding(name, tenant, roleName, users, groups string) *auth.RoleBinding {
	return &auth.RoleBinding{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Spec: auth.RoleBindingSpec{
			Users:      stringToSlice(users),
			UserGroups: stringToSlice(groups),
			Role:       roleName,
		},
	}
}

func newClusterRole(name string, permissions ...auth.Permission) *auth.Role {
	return &auth.Role{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: DefaultTenant,
		},
		Spec: auth.RoleSpec{
			Permissions: permissions,
		},
	}
}

func newClusterRoleBinding(name, roleName, users, groups string) *auth.RoleBinding {
	return &auth.RoleBinding{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: DefaultTenant,
		},
		Spec: auth.RoleBindingSpec{
			Users:      stringToSlice(users),
			UserGroups: stringToSlice(groups),
			Role:       roleName,
		},
	}
}

func newUser(name, tenant, groups string) *auth.User {
	return &auth.User{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Status: auth.UserStatus{
			UserGroups: stringToSlice(groups),
		},
	}
}

func newClusterUser(name, groups string) *auth.User {
	return &auth.User{
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: DefaultTenant,
		},
		Status: auth.UserStatus{
			UserGroups: stringToSlice(groups),
		},
	}
}

func getMockPermissionGetter(roles []*auth.Role, roleBindings []*auth.RoleBinding, clusterRoles []*auth.Role, clusterRoleBindings []*auth.RoleBinding) permissionGetter {
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

type userOps struct {
	name      string
	user      *auth.User
	operation Operation
}

func TestAuthorizer(t *testing.T) {
	tests := []struct {
		roles               []*auth.Role
		roleBindings        []*auth.RoleBinding
		clusterRoles        []*auth.Role
		clusterRoleBindings []*auth.RoleBinding

		shouldPass []userOps
		shouldFail []userOps
	}{
		{
			clusterRoles: []*auth.Role{
				newClusterRole("SuperAdmin", newPermission(
					ResourceTenantAll,
					ResourceGroupAll,
					auth.Permission_ALL_RESOURCE_KINDS.String(),
					ResourceNamespaceAll,
					"",
					auth.Permission_ALL_ACTIONS.String())),
			},
			clusterRoleBindings: []*auth.RoleBinding{
				newClusterRoleBinding("SuperAdminRB", "SuperAdmin", "Grace", "SuperAdmin"),
			},
			shouldPass: []userOps{
				{
					user: newClusterUser("Dorota", "SuperAdmin"),
					operation: NewOperation(
						NewResource(DefaultTenant, "Network", "Network", "prod", ""),
						auth.Permission_CREATE.String()),
				},
				{
					user: newClusterUser("Grace", ""),
					operation: NewOperation(
						NewResource(DefaultTenant, "Network", "Network", "prod", ""),
						auth.Permission_CREATE.String()),
				},
			},
			shouldFail: []userOps{
				{
					user: newUser("Shelly", DefaultTenant, "NetworkAdmin"),
					operation: NewOperation(
						NewResource(DefaultTenant, "Network", "Network", "prod", ""),
						auth.Permission_CREATE.String()),
				},
			},
		},
		{
			roles: []*auth.Role{
				newRole("NetworkAdmin", testTenant, newPermission(
					testTenant,
					"Network",
					auth.Permission_NETWORK.String(),
					ResourceNamespaceAll,
					"",
					auth.Permission_ALL_ACTIONS.String())),
			},
			roleBindings: []*auth.RoleBinding{
				newRoleBinding("NetworkAdminRB", testTenant, "NetworkAdmin", "Shelly", ""),
			},
			clusterRoles: []*auth.Role{
				newClusterRole("SuperAdmin", newPermission(
					ResourceTenantAll,
					ResourceGroupAll,
					auth.Permission_ALL_RESOURCE_KINDS.String(),
					ResourceNamespaceAll,
					"",
					auth.Permission_ALL_ACTIONS.String())),
			},
			clusterRoleBindings: []*auth.RoleBinding{
				newClusterRoleBinding("SuperAdminRB", "SuperAdmin", "Grace", "SuperAdmin"),
			},
			shouldPass: []userOps{
				{
					user: newUser("Shelly", testTenant, ""),
					operation: NewOperation(
						NewResource(testTenant, "Network", auth.Permission_NETWORK.String(), "prod", ""),
						auth.Permission_CREATE.String()),
				},
			},
			shouldFail: []userOps{
				{
					user: newUser("Shelly", "accounting", ""),
					operation: NewOperation(
						NewResource(DefaultTenant, "Network", auth.Permission_NETWORK.String(), "prod", ""),
						auth.Permission_CREATE.String()),
				},
				{
					user: newUser("Shelly", "accounting", ""),
					operation: NewOperation(
						NewResource("accounting", "Network", auth.Permission_NETWORK.String(), "prod", ""),
						auth.Permission_CREATE.String()),
				},
				{
					user: nil,
					operation: NewOperation(
						NewResource(testTenant, "Network", auth.Permission_NETWORK.String(), "prod", ""),
						auth.Permission_CREATE.String()),
				},
				{
					user:      newUser("Shelly", testTenant, ""),
					operation: nil,
				},
				{
					user:      newUser("Shelly", testTenant, ""),
					operation: Operation(nil),
				},
			},
		},
	}
	for i, test := range tests {
		permGetter := getMockPermissionGetter(test.roles, test.roleBindings, test.clusterRoles, test.clusterRoleBindings)
		authorizer := &authorizer{
			permissionChecker: &defaultPermissionChecker{permissionGetter: permGetter},
		}
		for _, pass := range test.shouldPass {
			ok, _ := authorizer.IsAuthorized(pass.user, pass.operation)
			Assert(t, ok, fmt.Sprintf("case %d: incorrectly restricted %s for user %s", i, pass.operation, pass.user.GetObjectMeta().GetName()))
		}
		for _, fail := range test.shouldFail {
			ok, _ := authorizer.IsAuthorized(fail.user, fail.operation)
			Assert(t, !ok, fmt.Sprintf("case %d: incorrectly allowed %s for user %s", i, fail.operation, func() string {
				if fail.user != nil {
					return fail.user.GetObjectMeta().GetName()
				}
				return ""
			}()))
		}
	}
}

func getPermissionDataForBenchmarking(numOfTenants, numOfUsersPerRole int) (roles []*auth.Role, roleBindings []*auth.RoleBinding, clusterRoles []*auth.Role, clusterRoleBindings []*auth.RoleBinding) {
	for i := 0; i < numOfTenants; i++ {
		tenant := fmt.Sprintf("tenant%d", i)
		roles = append(roles,
			newRole("NetworkAdmin", tenant,
				newPermission(tenant, "Network", auth.Permission_NETWORK.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Network", auth.Permission_SERVICE.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Network", auth.Permission_LBPOLICY.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
			newRole("SecurityAdmin", tenant,
				newPermission(tenant, "Security", auth.Permission_SECURITYGROUP.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Security", auth.Permission_SGPOLICY.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Security", auth.Permission_APP.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Security", auth.Permission_ENCRYPTIONPOLICY.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
			newRole("TenantAdmin", tenant,
				newPermission(tenant, "Tenant", auth.Permission_TENANT.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()+","+auth.Permission_UPDATE.String()),
				newPermission(tenant, "Security", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Network", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "User", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Monitoring", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "", auth.Permission_APIENDPOINT.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String()),
				newPermission(tenant, "Workload", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())),
			newRole("Auditor", tenant,
				newPermission(tenant, "Tenant", auth.Permission_TENANT.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()+","+auth.Permission_UPDATE.String()),
				newPermission(tenant, "Security", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()),
				newPermission(tenant, "Network", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()),
				newPermission(tenant, "User", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()),
				newPermission(tenant, "Monitoring", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()),
				newPermission(tenant, "", auth.Permission_APIENDPOINT.String(), ResourceNamespaceAll, "", auth.Permission_READ.String()),
				newPermission(tenant, "Workload", auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_READ.String())))

		for j := 0; j < numOfUsersPerRole; j++ {
			networkAdmin := fmt.Sprintf("Sally%d", j)
			securityAdmin := fmt.Sprintf("John%d", j)
			tenantAdmin := fmt.Sprintf("Deb%d", j)
			auditor := fmt.Sprintf("Sara%d", j)
			roleBindings = append(roleBindings, newRoleBinding(networkAdmin, tenant, "NetworkAdmin", networkAdmin, ""),
				newRoleBinding(securityAdmin, tenant, "SecurityAdmin", securityAdmin, ""),
				newRoleBinding(tenantAdmin, tenant, "TenantAdmin", tenantAdmin, ""),
				newRoleBinding(auditor, tenant, "Auditor", auditor, ""))
		}
	}
	clusterRoleBindings = append(clusterRoleBindings,
		newClusterRoleBinding("SuperAdminRB", "SuperAdmin", "", "SuperAdmin"))

	clusterRoles = append(clusterRoles,
		newClusterRole("SuperAdmin", newPermission(ResourceTenantAll, ResourceGroupAll, auth.Permission_ALL_RESOURCE_KINDS.String(), ResourceNamespaceAll, "", auth.Permission_ALL_ACTIONS.String())))

	return
}

func BenchmarkAuthorizer(b *testing.B) {
	permGetter := getMockPermissionGetter(getPermissionDataForBenchmarking(6, 6))
	authorizer := &authorizer{
		permissionChecker: &defaultPermissionChecker{permissionGetter: permGetter},
	}
	requests := []userOps{
		{
			name: "allow create security group",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", "Security", auth.Permission_SECURITYGROUP.String(), "prod", "sggrp1"), auth.Permission_CREATE.String()),
		},
		{
			name: "allow read security group",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", "Security", auth.Permission_SECURITYGROUP.String(), "prod", "sggrp1"), auth.Permission_READ.String()),
		},
		{
			name: "deny read network",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", "Network", auth.Permission_NETWORK.String(), "prod", "sggrp1"), auth.Permission_READ.String()),
		},
	}
	b.ResetTimer()
	for _, request := range requests {
		b.Run(request.name, func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				authorizer.IsAuthorized(request.user, request.operation)
			}
		})
	}
}
