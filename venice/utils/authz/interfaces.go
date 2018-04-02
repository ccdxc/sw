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

// operation implements authz.Operation interface
type operation struct {
	resource Resource
	action   string
}

func (op *operation) GetResource() Resource {
	return op.resource
}

func (op *operation) GetAction() string {
	return op.action
}

// NewOperation returns an instance of Operation
func NewOperation(resource Resource, action string) Operation {
	return &operation{
		resource: resource,
		action:   action,
	}
}

// resource implements Resource interface
type resource struct {
	tenant       string
	group        string
	resourceKind string
	namespace    string
	name         string
}

func (r *resource) GetTenant() string {
	return r.tenant
}

func (r *resource) GetGroup() string {
	return r.group
}

func (r *resource) GetKind() string {
	return r.resourceKind
}

func (r *resource) GetNamespace() string {
	return r.namespace
}

func (r *resource) GetName() string {
	return r.name
}

// NewResource returns an instance of Resource
func NewResource(tenant, group, resourceKind, namespace, name string) Resource {
	return &resource{
		tenant:       tenant,
		group:        group,
		resourceKind: resourceKind,
		namespace:    namespace,
		name:         name,
	}
}

const (
	// ResourceNamespaceAll is a keyword to match all namespaces
	ResourceNamespaceAll = "all"
	// ResourceGroupAll is a keyword to match all resource groups
	ResourceGroupAll = "all"
	// ResourceTenantAll is a keyword to match all tenants
	ResourceTenantAll = "all"
	// DefaultTenant is a tenant for defining cluster scoped permissions
	DefaultTenant = "default"
)
