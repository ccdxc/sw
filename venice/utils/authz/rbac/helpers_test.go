package rbac

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
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
				ResourceGroup:     "Network",
				ResourceKind:      auth.Permission_NETWORK.String(),
				ResourceNamespace: ResourceNamespaceAll,
				Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "abc", "def"),
				auth.Permission_READ.String()),
			expected: true,
		},
		{
			name: "matching resource by kind and namespace",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     "Network",
				ResourceKind:      auth.Permission_NETWORK.String(),
				ResourceNamespace: "finance",
				Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "def"),
				auth.Permission_READ.String()),
			expected: true,
		},
		{
			name: "matching resource by name",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     "Network",
				ResourceKind:      auth.Permission_NETWORK.String(),
				ResourceNamespace: "finance",
				ResourceNames:     []string{"pensando"},
				Actions:           []string{auth.Permission_READ.String(), auth.Permission_UPDATE.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "pensando"),
				auth.Permission_READ.String()),
			expected: true,
		},
		{
			name: "matching all resources",
			permission: auth.Permission{
				ResourceTenant:    ResourceTenantAll,
				ResourceGroup:     ResourceGroupAll,
				ResourceKind:      auth.Permission_ALL_RESOURCE_KINDS.String(),
				ResourceNamespace: ResourceNamespaceAll,
				Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "pensando"),
				auth.Permission_READ.String()),
			expected: true,
		},
		{
			name: "matching arbitrary api endpoint",
			permission: auth.Permission{
				ResourceTenant:    "",
				ResourceGroup:     "",
				ResourceKind:      auth.Permission_APIENDPOINT.String(),
				ResourceNamespace: "",
				ResourceNames:     []string{"/api/v1/search"},
				Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
			},
			operation: NewOperation(
				NewResource("", "", auth.Permission_APIENDPOINT.String(), "", "/api/v1/search"),
				auth.Permission_READ.String()),
			expected: true,
		},
		{
			name: "doesn't match resource by name",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     "Network",
				ResourceKind:      auth.Permission_NETWORK.String(),
				ResourceNamespace: "finance",
				ResourceNames:     []string{"pensando"},
				Actions:           []string{auth.Permission_READ.String(), auth.Permission_UPDATE.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "xyz"),
				auth.Permission_READ.String()),
			expected: false,
		},
		{
			name: "doesn't match action",
			permission: auth.Permission{
				ResourceTenant:    "default",
				ResourceGroup:     "Network",
				ResourceKind:      auth.Permission_NETWORK.String(),
				ResourceNamespace: "finance",
				Actions:           []string{auth.Permission_READ.String(), auth.Permission_UPDATE.String()},
			},
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "xyz"),
				auth.Permission_CREATE.String()),
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
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "xyz"),
				auth.Permission_CREATE.String()),
			permissions: []auth.Permission{
				{
					ResourceTenant:    "default",
					ResourceGroup:     "Network",
					ResourceKind:      auth.Permission_NETWORK.String(),
					ResourceNamespace: "finance",
					ResourceNames:     []string{"pensando"},
					Actions:           []string{auth.Permission_READ.String(), auth.Permission_UPDATE.String()},
				},
				{
					ResourceTenant:    "default",
					ResourceGroup:     "Network",
					ResourceKind:      auth.Permission_NETWORK.String(),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
				},
			},
			expected: true,
		},
		{
			name: "operation disallowed",
			operation: NewOperation(
				NewResource("default", "Network", auth.Permission_NETWORK.String(), "finance", "xyz"),
				auth.Permission_CREATE.String()),
			permissions: []auth.Permission{
				{
					ResourceTenant:    "default",
					ResourceGroup:     "Network",
					ResourceKind:      auth.Permission_NETWORK.String(),
					ResourceNamespace: "finance",
					ResourceNames:     []string{"pensando"},
					Actions:           []string{auth.Permission_READ.String(), auth.Permission_UPDATE.String()},
				},
				{
					ResourceTenant:    "default",
					ResourceGroup:     "Security",
					ResourceKind:      auth.Permission_SECURITYGROUP.String(),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
				},
			},
			expected: false,
		},
	}

	for _, test := range tests {
		Assert(t, test.expected == permissionsAllow(test.permissions, test.operation), fmt.Sprintf("[%v] test failed", test.name))
	}
}
