package authz

import (
	"reflect"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// ResourceNamespaceAll is a keyword to match all namespaces
	ResourceNamespaceAll = "_All_"
	// ResourceGroupAll is a keyword to match all resource groups
	ResourceGroupAll = "_All_"
	// ResourceTenantAll is a keyword to match all tenants
	ResourceTenantAll = "_All_"
	// ResourceKindAll is a keyword to match all resource kinds
	ResourceKindAll = "_All_"
)

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

func getActionFromOper(in apiserver.APIOperType) string {
	switch in {
	case apiserver.CreateOper:
		return auth.Permission_Create.String()
	case apiserver.UpdateOper:
		return auth.Permission_Update.String()
	case apiserver.GetOper, apiserver.ListOper, apiserver.WatchOper:
		return auth.Permission_Read.String()
	case apiserver.DeleteOper:
		return auth.Permission_Delete.String()
	}
	return auth.Permission_AllActions.String()
}

// NewAPIServerOperation returns an instance of Operation given the APIServer Oper type
func NewAPIServerOperation(resource Resource, action apiserver.APIOperType) Operation {
	return &operation{
		resource: resource,
		action:   getActionFromOper(action),
	}
}

// IsValidOperationValue validates operation interface value as it is an input coming from authz hooks in API Gateway
func IsValidOperationValue(operation Operation) bool {
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

// resource implements Resource interface
type resource struct {
	tenant       string
	group        string
	resourceKind string
	namespace    string
	name         string
	owner        *auth.User
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

func (r *resource) GetOwner() *auth.User {
	return r.owner
}

func (r *resource) SetOwner(user *auth.User) {
	r.owner = user
}

// NewResource returns an instance of Resource
func NewResource(tenant, group, resourceKind, namespace, name string) Resource {
	return NewResourceWithOwner(tenant, group, resourceKind, namespace, name, nil)
}

// NewResourceWithOwner returns an instance of Resource
func NewResourceWithOwner(tenant, group, resourceKind, namespace, name string, owner *auth.User) Resource {
	return &resource{
		tenant:       tenant,
		group:        group,
		resourceKind: resourceKind,
		namespace:    namespace,
		name:         name,
		owner:        owner,
	}
}

// AbstractAuthorizer implements methods that other authorizers can inherit
type AbstractAuthorizer struct {
	Authorizer
}

// AllowedTenantKinds returns kinds in a tenant and namespace for which user has authorization for give action type.
// If tenant is empty it assumes user tenant. If namespace is empty it assumes "default"
func (a *AbstractAuthorizer) AllowedTenantKinds(user *auth.User, tenant, namespace string, actionType auth.Permission_ActionType) ([]auth.ObjKind, error) {
	// pick user tenant if no tenant is specified
	if tenant == "" {
		tenant = user.Tenant
	}
	// TODO: default or _All_ if no namespace specified?
	if namespace == "" {
		namespace = globals.DefaultNamespace
	}
	var allowedKinds []auth.ObjKind
	scheme := runtime.GetDefaultScheme()
	group2kinds := scheme.Kinds()
	for group, kinds := range group2kinds {
		for _, kind := range kinds {
			ok, err := scheme.IsTenantScoped(kind)
			if err != nil {
				return nil, err
			}
			if !ok {
				continue
			}
			resource := NewResource(tenant, group, kind, namespace, "")
			operation := NewOperation(resource, actionType.String())
			ok, err = a.IsAuthorized(user, operation)
			if err != nil {
				return nil, err
			}
			if !ok {
				continue
			}
			allowedKinds = append(allowedKinds, auth.ObjKind(kind))
		}
	}
	// add non-api server kinds like events
	kinds := []string{auth.Permission_Event.String()}
	for _, kind := range kinds {
		resource := NewResource(tenant, "", kind, namespace, "")
		operation := NewOperation(resource, actionType.String())
		ok, err := a.IsAuthorized(user, operation)
		if err != nil {
			return nil, err
		}
		if !ok {
			continue
		}
		allowedKinds = append(allowedKinds, auth.ObjKind(kind))
	}
	return allowedKinds, nil
}

// AllowedClusterKinds returns kinds in cluster scope for which user has authorization for give action type.
func (a *AbstractAuthorizer) AllowedClusterKinds(user *auth.User, actionType auth.Permission_ActionType) ([]auth.ObjKind, error) {
	if user.Tenant != globals.DefaultTenant {
		// only user in default tenant can have authorization to search across tenants
		return nil, nil
	}
	var allowedKinds []auth.ObjKind
	scheme := runtime.GetDefaultScheme()
	group2kinds := scheme.Kinds()
	for group, kinds := range group2kinds {
		for _, kind := range kinds {
			ok, err := scheme.IsClusterScoped(kind)
			if err != nil {
				return nil, err
			}
			if !ok {
				// not a cluster scoped kind
				continue
			}
			resource := NewResource("", group, kind, "", "")
			operation := NewOperation(resource, actionType.String())
			ok, err = a.IsAuthorized(user, operation)
			if err != nil {
				return nil, err
			}
			if !ok {
				// no authorization for this kind
				continue
			}
			allowedKinds = append(allowedKinds, auth.ObjKind(kind))
		}
	}
	return allowedKinds, nil
}
