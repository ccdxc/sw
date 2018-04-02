package rbac

import (
	"reflect"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// TODO: Optimize. Benchmarking with 4 roles and 6 users per role in a tenant with 6 tenants in system I see ~2500ns for a permission check on mac
func permissionsAllow(permissions []auth.Permission, operation authz.Operation) bool {
	for _, permission := range permissions {
		if permissionAllows(permission, operation) {
			return true
		}
	}
	return false
}

func permissionAllows(permission auth.Permission, operation authz.Operation) bool {
	if !isValidOperationValue(operation) {
		return false
	}
	return resourceMatches(permission, operation.GetResource()) &&
		actionMatches(permission, operation.GetAction())
}

func actionMatches(permission auth.Permission, requestedAction string) bool {
	allowedActions := permission.GetActions()
	for _, action := range allowedActions {
		if action == auth.Permission_ALL_ACTIONS.String() {
			return true
		}
		if action == requestedAction {
			return true
		}
	}
	return false
}

func resourceMatches(permission auth.Permission, resource authz.Resource) bool {
	return resourceTenantMatches(permission, resource.GetTenant()) &&
		resourceGroupMatches(permission, resource.GetGroup()) &&
		resourceKindMatches(permission, resource.GetKind()) &&
		resourceNamespaceMatches(permission, resource.GetNamespace()) &&
		resourceNameMatches(permission, resource.GetName())
}

func resourceTenantMatches(permission auth.Permission, requestedResourceTenant string) bool {
	allowedTenant := permission.GetResourceTenant()
	if allowedTenant == authz.ResourceTenantAll {
		return true
	}
	return allowedTenant == requestedResourceTenant
}

func resourceNamespaceMatches(permission auth.Permission, requestedResourceNamespace string) bool {
	allowedNamespace := permission.GetResourceNamespace()
	if allowedNamespace == authz.ResourceNamespaceAll {
		return true
	}
	return allowedNamespace == requestedResourceNamespace
}

func resourceGroupMatches(permission auth.Permission, requestedResourceGroup string) bool {
	allowedResourceGroup := permission.GetResourceGroup()
	if allowedResourceGroup == authz.ResourceGroupAll {
		return true
	}
	return allowedResourceGroup == requestedResourceGroup

}

func resourceKindMatches(permission auth.Permission, requestedResourceKind string) bool {
	allowedResourceKind := permission.GetResourceKind()
	// no resource type is defined so permission is on resource group; check if resource type belongs to resource group
	// in permission
	if allowedResourceKind == "" {
		return resourceGroupContains(permission.GetResourceGroup(), requestedResourceKind)
	}
	// resource type is "all" in permission so it will match
	if allowedResourceKind == auth.Permission_ALL_RESOURCE_KINDS.String() {
		return true
	}

	return allowedResourceKind == requestedResourceKind
}

func resourceGroupContains(resourceGroup string, resourceType string) bool {
	// TODO: implement logic here
	return false
}

func resourceNameMatches(permission auth.Permission, requestedResourceName string) bool {
	allowedResourceNames := permission.GetResourceNames()
	// no resource names are defined in permission. It means all resources within a namespace
	if len(allowedResourceNames) == 0 {
		return true
	}

	for _, resourceName := range allowedResourceNames {
		if resourceName == requestedResourceName {
			return true
		}
	}

	return false
}

// isValidOperationValue validates operation interface value as it is an input coming from RBAC hooks in API Gateway
func isValidOperationValue(operation authz.Operation) bool {
	// make sure interface type and value are not nil
	if operation == nil || reflect.ValueOf(operation).IsNil() {
		return false
	}
	resource := operation.GetResource()
	if resource == nil || reflect.ValueOf(resource).IsNil() {
		return false
	}
	return true
}
