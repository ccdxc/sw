package authz

import (
	"fmt"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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
	resource    Resource
	action      string
	auditAction string
	id          string
}

func (op *operation) GetResource() Resource {
	return op.resource
}

func (op *operation) GetAction() string {
	return op.action
}

func (op *operation) GetAuditAction() string {
	return op.auditAction
}

func (op *operation) GetID() string {
	return op.id
}

// NewOperation returns an instance of Operation
func NewOperation(resource Resource, action string) Operation {
	return &operation{
		resource:    resource,
		action:      action,
		auditAction: action,
		id:          uuid.NewV4().String(),
	}
}

func getActionFromOper(in apiintf.APIOperType) string {
	switch in {
	case apiintf.CreateOper:
		return auth.Permission_Create.String()
	case apiintf.UpdateOper:
		return auth.Permission_Update.String()
	case apiintf.GetOper, apiintf.ListOper, apiintf.WatchOper:
		return auth.Permission_Read.String()
	case apiintf.DeleteOper:
		return auth.Permission_Delete.String()
	}
	return auth.Permission_AllActions.String()
}

// NewAPIServerOperation returns an instance of Operation given the APIServer Oper type
func NewAPIServerOperation(resource Resource, action apiintf.APIOperType, auditAction string) Operation {
	pAction := getActionFromOper(action)
	if auditAction == "" {
		auditAction = string(action)
	}
	return &operation{
		resource:    resource,
		action:      pAction,
		auditAction: auditAction,
		id:          uuid.NewV4().String(),
	}
}

// NewAuditOperation returns an instance of Operation with potentially a different action string for auditing
func NewAuditOperation(resource Resource, action, auditAction string) Operation {
	return &operation{
		resource:    resource,
		action:      action,
		auditAction: auditAction,
		id:          uuid.NewV4().String(),
	}
}

// NewOperationWithID creates an operation with a given ID
func NewOperationWithID(resource Resource, action, auditAction, id string) Operation {
	return &operation{
		resource:    resource,
		action:      action,
		auditAction: auditAction,
		id:          id,
	}
}

// resource implements Resource interface
type resource struct {
	auth.Resource
	owner *auth.User
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
	res := auth.Resource{}
	res.Defaults("all")
	res.Tenant = tenant
	res.Group = group
	res.Kind = resourceKind
	res.Namespace = namespace
	res.Name = name
	return &resource{
		Resource: res,
		owner:    owner,
	}
}

// AbstractAuthorizer implements methods that other authorizers can inherit
type AbstractAuthorizer struct {
	Authorizer
}

// AuthorizedOperations returns authorized operations for a user for various kinds and given actions
func (a *AbstractAuthorizer) AuthorizedOperations(user *auth.User, tenant, namespace string, actionTypes ...auth.Permission_ActionType) []Operation {
	// assume cluster scoped kinds for empty tenant
	if tenant == "" {
		if user.Tenant != globals.DefaultTenant {
			// only user in default tenant can have authorization to search across tenants
			return nil
		}
		namespace = ""
	}
	// TODO: default or _All_ if no namespace specified?
	if tenant != "" && namespace == "" {
		namespace = globals.DefaultNamespace
	}
	var allowedOperations []Operation
	scheme := runtime.GetDefaultScheme()
	group2kinds := scheme.Kinds()
	for group, kinds := range group2kinds {
		for _, kind := range kinds {
			switch kind {
			case string(bookstore.KindBook), string(bookstore.KindCustomer), string(bookstore.KindOrder), string(bookstore.KindPublisher), string(bookstore.KindStore), "Coupon":
				// skip example kinds
			case string(staging.KindClearAction), string(staging.KindCommitAction):
				// skip these as we have commit and clear action for staging buffer
			default:
				if tenant != "" {
					ok, err := scheme.IsTenantScoped(kind)
					if err != nil {
						// this should not occur
						panic(fmt.Sprintf("unexpected error in determining scope of known kind: %v", err))
					}
					if !ok {
						// not a tenant scoped kind
						continue
					}
				} else {
					ok, err := scheme.IsClusterScoped(kind)
					if err != nil {
						// this should not occur
						panic(fmt.Sprintf("unexpected error in determining scope of known kind: %v", err))
					}
					if !ok {
						// not a cluster scoped kind
						continue
					}
				}
				resource := NewResource(tenant, group, kind, namespace, "")
				// check authorization for all action if present
				ok, op := a.checkAllAction(user, resource, actionTypes...)
				// if all actions for a kind are allowed then don't need to check other actions
				if ok {
					allowedOperations = append(allowedOperations, op)
					continue
				}
				ops := a.getAllowedOperations(user, resource, actionTypes...)
				allowedOperations = append(allowedOperations, ops...)
			}
		}
	}
	// add non-api server kinds like events etc
	if tenant != "" {
		kinds := []string{auth.Permission_AuditEvent.String(),
			auth.Permission_FwLog.String(),
			auth.Permission_Event.String(),
			auth.Permission_TokenAuth.String(),
			auth.Permission_APIEndpoint.String()}
		for _, kind := range kinds {
			resource := NewResource(tenant, "", kind, namespace, "")
			ok, op := a.checkAllAction(user, resource, actionTypes...)
			// if all actions for a kind are allowed then don't need to check other actions
			if ok {
				allowedOperations = append(allowedOperations, op)
				continue
			}
			ops := a.getAllowedOperations(user, resource, actionTypes...)
			allowedOperations = append(allowedOperations, ops...)
		}
	}
	return allowedOperations
}

func (a *AbstractAuthorizer) checkAllAction(user *auth.User, resource Resource, actionTypes ...auth.Permission_ActionType) (bool, Operation) {
	for _, action := range actionTypes {
		if action == auth.Permission_AllActions {
			op := NewOperation(resource, auth.Permission_AllActions.String())
			ok, err := a.IsAuthorized(user, op)
			if err != nil {
				log.ErrorLog("method", "checkAllAction", "msg", fmt.Sprintf("error authorizing operation %v", PrintOperations([]Operation{op})), "error", err)
				break
			}
			if ok {
				return ok, op
			}
			break
		}
	}
	return false, nil
}

func (a *AbstractAuthorizer) getAllowedOperations(user *auth.User, resource Resource, actionTypes ...auth.Permission_ActionType) []Operation {
	var allowedOperations []Operation
	for _, action := range actionTypes {
		if action != auth.Permission_AllActions {
			// check if action is valid for a kind
			if err := ValidateAction(resource.GetGroup(), resource.GetKind(), action.String()); err != nil {
				continue
			}
			op := NewOperation(resource, action.String())
			ok, err := a.IsAuthorized(user, op)
			if err != nil {
				log.ErrorLog("method", "getAllowedOperations", "msg", fmt.Sprintf("error authorizing operation %v", PrintOperations([]Operation{op})), "error", err)
				continue
			}
			if !ok {
				continue
			}
			allowedOperations = append(allowedOperations, op)
		}
	}
	return allowedOperations
}
