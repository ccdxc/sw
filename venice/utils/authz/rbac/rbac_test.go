package rbac

import (
	"fmt"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	. "github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
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
			Tenant: globals.DefaultTenant,
		},
		Status: auth.UserStatus{
			UserGroups: stringToSlice(groups),
		},
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
				NewClusterRole("SuperAdmin", NewPermission(
					ResourceTenantAll,
					ResourceGroupAll,
					ResourceKindAll,
					ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			},
			clusterRoleBindings: []*auth.RoleBinding{
				NewClusterRoleBinding("SuperAdminRB", "SuperAdmin", "Grace", "SuperAdmin"),
			},
			shouldPass: []userOps{
				{
					user: newClusterUser("Dorota", "SuperAdmin"),
					operation: NewOperation(
						NewResource(globals.DefaultTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
				},
				{
					user: newClusterUser("Grace", ""),
					operation: NewOperation(
						NewResource(globals.DefaultTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
				},
			},
			shouldFail: []userOps{
				{
					user: newUser("Shelly", globals.DefaultTenant, "NetworkAdmin"),
					operation: NewOperation(
						NewResource(globals.DefaultTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
				},
			},
		},
		{
			roles: []*auth.Role{
				NewRole("NetworkAdmin", testTenant, NewPermission(
					testTenant,
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			},
			roleBindings: []*auth.RoleBinding{
				NewRoleBinding("NetworkAdminRB", testTenant, "NetworkAdmin", "Shelly", ""),
			},
			clusterRoles: []*auth.Role{
				NewClusterRole("SuperAdmin", NewPermission(
					ResourceTenantAll,
					ResourceGroupAll,
					ResourceKindAll,
					ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			},
			clusterRoleBindings: []*auth.RoleBinding{
				NewClusterRoleBinding("SuperAdminRB", "SuperAdmin", "Grace", "SuperAdmin"),
			},
			shouldPass: []userOps{
				{
					user: newUser("Shelly", testTenant, ""),
					operation: NewOperation(
						NewResource(testTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""), auth.Permission_Create.String()),
				},
			},
			shouldFail: []userOps{
				{
					user: newUser("Shelly", "accounting", ""),
					operation: NewOperation(
						NewResource(globals.DefaultTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
				},
				{
					user: newUser("Shelly", "accounting", ""),
					operation: NewOperation(
						NewResource("accounting", string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
				},
				{
					user: nil,
					operation: NewOperation(
						NewResource(testTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", ""),
						auth.Permission_Create.String()),
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
		permGetter := NewMockPermissionGetter(test.roles, test.roleBindings, test.clusterRoles, test.clusterRoleBindings)
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
			NewRole("NetworkAdmin", tenant,
				NewPermission(tenant, string(apiclient.GroupNetwork), string(network.KindNetwork), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupNetwork), string(network.KindService), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupNetwork), string(network.KindLbPolicy), ResourceNamespaceAll, "", auth.Permission_AllActions.String())),
			NewRole("SecurityAdmin", tenant,
				NewPermission(tenant, string(apiclient.GroupSecurity), string(security.KindSecurityGroup), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupSecurity), string(security.KindSGPolicy), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupSecurity), string(security.KindApp), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupSecurity), string(security.KindTrafficEncryptionPolicy), ResourceNamespaceAll, "", auth.Permission_AllActions.String())),
			NewRole("TenantAdmin", tenant,
				NewPermission(tenant, string(apiclient.GroupCluster), string(cluster.KindTenant), ResourceNamespaceAll, "", auth.Permission_Read.String()+","+auth.Permission_Update.String()),
				NewPermission(tenant, string(apiclient.GroupSecurity), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupNetwork), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupAuth), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupMonitoring), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, "", auth.Permission_APIEndpoint.String(), ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
				NewPermission(tenant, string(apiclient.GroupWorkload), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String())),
			NewRole("Auditor", tenant,
				NewPermission(tenant, string(apiclient.GroupCluster), string(cluster.KindTenant), ResourceNamespaceAll, "", auth.Permission_Read.String()+","+auth.Permission_Update.String()),
				NewPermission(tenant, string(apiclient.GroupSecurity), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_Read.String()),
				NewPermission(tenant, string(apiclient.GroupNetwork), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_Read.String()),
				NewPermission(tenant, string(apiclient.GroupAuth), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_Read.String()),
				NewPermission(tenant, string(apiclient.GroupMonitoring), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_Read.String()),
				NewPermission(tenant, "", auth.Permission_APIEndpoint.String(), ResourceNamespaceAll, "", auth.Permission_Read.String()),
				NewPermission(tenant, string(apiclient.GroupWorkload), ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_Read.String())))

		for j := 0; j < numOfUsersPerRole; j++ {
			networkAdmin := fmt.Sprintf("Sally%d", j)
			securityAdmin := fmt.Sprintf("John%d", j)
			tenantAdmin := fmt.Sprintf("Deb%d", j)
			auditor := fmt.Sprintf("Sara%d", j)
			roleBindings = append(roleBindings, NewRoleBinding(networkAdmin, tenant, "NetworkAdmin", networkAdmin, ""),
				NewRoleBinding(securityAdmin, tenant, "SecurityAdmin", securityAdmin, ""),
				NewRoleBinding(tenantAdmin, tenant, "TenantAdmin", tenantAdmin, ""),
				NewRoleBinding(auditor, tenant, "Auditor", auditor, ""))
		}
	}
	clusterRoleBindings = append(clusterRoleBindings,
		NewClusterRoleBinding("SuperAdminRB", "SuperAdmin", "", "SuperAdmin"))

	clusterRoles = append(clusterRoles,
		NewClusterRole("SuperAdmin", NewPermission(ResourceTenantAll, ResourceGroupAll, ResourceKindAll, ResourceNamespaceAll, "", auth.Permission_AllActions.String())))

	return
}

func BenchmarkAuthorizer(b *testing.B) {
	permGetter := NewMockPermissionGetter(getPermissionDataForBenchmarking(6, 6))
	authorizer := &authorizer{
		permissionChecker: &defaultPermissionChecker{permissionGetter: permGetter},
	}
	requests := []userOps{
		{
			name: "allow create security group",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", string(apiclient.GroupSecurity), string(security.KindSecurityGroup), "prod", "sggrp1"), auth.Permission_Create.String()),
		},
		{
			name: "allow read security group",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", string(apiclient.GroupSecurity), string(security.KindSecurityGroup), "prod", "sggrp1"), auth.Permission_Read.String()),
		},
		{
			name: "deny read network",
			user: newUser("John0", "tenant0", ""),
			operation: NewOperation(
				NewResource("tenant0", string(apiclient.GroupNetwork), string(network.KindNetwork), "prod", "sggrp1"), auth.Permission_Read.String()),
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
