package authz

import (
	"github.com/pensando/sw/api/generated/auth"
)

// Authorizer represents an authorization module that makes authorization decision for a user to perform certain operations
// RBAC based authorization module will be one implementation.
type Authorizer interface {
	// IsAuthorized checks if a user is authorized for the specified operations
	// Returns true if user is authorized
	IsAuthorized(user *auth.User, operations ...Operation) (bool, error)

	// AllowedTenantKinds returns kinds in a tenant and namespace for which user has authorization for give action type.
	// If tenant is empty it assumes user tenant. If namespace is empty it assumes "default"
	AllowedTenantKinds(user *auth.User, tenant, namespace string, actionType auth.Permission_ActionType) ([]auth.ObjKind, error)

	// AllowedClusterKinds returns kinds in cluster scope for which user has authorization for give action type.
	AllowedClusterKinds(user *auth.User, actionType auth.Permission_ActionType) ([]auth.ObjKind, error)
}

// Operation represents an user operation on a resource about which an authorization decision has to be made
type Operation interface {
	// GetResource returns resource on which authorization for actions is to be checked
	GetResource() Resource

	// GetAction returns action that need to be performed on a resource group or resource type or a specific resource
	GetAction() string
}

// Resource represents an object or API endpoint resource on which authorization for an action needs to be checked
type Resource interface {
	// GetTenant returns tenant to which the resource belongs. For cluster-wide resource it returns empty string
	GetTenant() string

	// GetGroup returns resource group to which access is desired
	GetGroup() string

	// GetKind returns resource kind to which access is desired
	GetKind() string

	// GetNamespace returns resource namespace within which access to a resource is desired
	GetNamespace() string

	// GetName returns name of a specific resource to which access is desired
	GetName() string
}
