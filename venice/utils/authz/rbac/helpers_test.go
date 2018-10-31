package rbac

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	. "github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestPermissionAllows(t *testing.T) {
	tests := []struct {
		name       string
		permission auth.Permission
		operation  Operation
		expected   bool
	}{
		{
			name: "matching resource by kind for all namespaces",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupNetwork),
				ResourceKind:      string(network.KindNetwork),
				ResourceNamespace: ResourceNamespaceAll,
				Actions:           []string{auth.Permission_AllActions.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "abc", "def"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "matching resource by api group for all namespaces",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupAuth),
				ResourceKind:      "",
				ResourceNamespace: ResourceNamespaceAll,
				Actions:           []string{auth.Permission_AllActions.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupAuth), string(auth.KindUser), "abc", "def"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "matching resource by kind and namespace",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupNetwork),
				ResourceKind:      string(network.KindNetwork),
				ResourceNamespace: "finance",
				Actions:           []string{auth.Permission_AllActions.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "def"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "matching resource by name",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupNetwork),
				ResourceKind:      string(network.KindNetwork),
				ResourceNamespace: "finance",
				ResourceNames:     []string{"pensando"},
				Actions:           []string{auth.Permission_Read.String(), auth.Permission_Update.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "pensando"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "matching all resources",
			permission: auth.Permission{
				ResourceTenant:    ResourceTenantAll,
				ResourceGroup:     ResourceGroupAll,
				ResourceKind:      ResourceKindAll,
				ResourceNamespace: ResourceNamespaceAll,
				Actions:           []string{auth.Permission_AllActions.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "pensando"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "matching arbitrary api endpoint",
			permission: auth.Permission{
				ResourceTenant:    "",
				ResourceGroup:     "",
				ResourceKind:      auth.Permission_APIEndpoint.String(),
				ResourceNamespace: "",
				ResourceNames:     []string{"/api/v1/search"},
				Actions:           []string{auth.Permission_AllActions.String()},
			},
			operation: NewOperation(
				NewResource("", "", auth.Permission_APIEndpoint.String(), "", "/api/v1/search"),
				auth.Permission_Read.String()),
			expected: true,
		},
		{
			name: "doesn't match resource by name",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupNetwork),
				ResourceKind:      string(network.KindNetwork),
				ResourceNamespace: "finance",
				ResourceNames:     []string{"pensando"},
				Actions:           []string{auth.Permission_Read.String(), auth.Permission_Update.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "xyz"),
				auth.Permission_Read.String()),
			expected: false,
		},
		{
			name: "doesn't match action",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     string(apiclient.GroupNetwork),
				ResourceKind:      string(network.KindNetwork),
				ResourceNamespace: "finance",
				Actions:           []string{auth.Permission_Read.String(), auth.Permission_Update.String()},
			},
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "xyz"),
				auth.Permission_Create.String()),
			expected: false,
		},
	}
	for _, test := range tests {
		Assert(t, test.expected == permissionAllows(test.permission, test.operation), fmt.Sprintf("[%v] test failed", test.name))
	}
}

func TestPermissionsAllow(t *testing.T) {
	tests := []struct {
		name        string
		operation   Operation
		permissions []auth.Permission
		expected    bool
	}{
		{
			name: "operation allowed",
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "xyz"),
				auth.Permission_Create.String()),
			permissions: []auth.Permission{
				{
					ResourceTenant:    "default",
					ResourceGroup:     string(apiclient.GroupNetwork),
					ResourceKind:      string(network.KindNetwork),
					ResourceNamespace: "finance",
					ResourceNames:     []string{"pensando"},
					Actions:           []string{auth.Permission_Read.String(), auth.Permission_Update.String()},
				},
				{
					ResourceTenant:    "default",
					ResourceGroup:     string(apiclient.GroupNetwork),
					ResourceKind:      string(network.KindNetwork),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_AllActions.String()},
				},
			},
			expected: true,
		},
		{
			name: "operation disallowed",
			operation: NewOperation(
				NewResource("default", string(apiclient.GroupNetwork), string(network.KindNetwork), "finance", "xyz"),
				auth.Permission_Create.String()),
			permissions: []auth.Permission{
				{
					ResourceTenant:    "default",
					ResourceGroup:     string(apiclient.GroupNetwork),
					ResourceKind:      string(network.KindNetwork),
					ResourceNamespace: "finance",
					ResourceNames:     []string{"pensando"},
					Actions:           []string{auth.Permission_Read.String(), auth.Permission_Update.String()},
				},
				{
					ResourceTenant:    "default",
					ResourceGroup:     string(apiclient.GroupSecurity),
					ResourceKind:      string(security.KindSecurityGroup),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_AllActions.String()},
				},
			},
			expected: false,
		},
	}

	for _, test := range tests {
		Assert(t, test.expected == permissionsAllow(test.permissions, test.operation), fmt.Sprintf("[%v] test failed", test.name))
	}
}
