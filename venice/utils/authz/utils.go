package authz

import (
	"encoding/base64"
	"errors"
	"fmt"
	"reflect"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// UserTenantKey stores user tenant
	UserTenantKey = "pensando-venice-user-tenant-key"
	// UsernameKey stores username
	UsernameKey = "pensando-venice-user-key"
	// UserIsAdmin stores whether user is admin
	UserIsAdmin = "pensando-venice-user-is-admin"
	// PermsKey with -bin suffix tells grpc that value is binary. grpc auto base64 encodes and decodes it TODO see if we remove bin and manually base64 encode/decode
	PermsKey = "pensando-venice-perms-key-bin"
)

// AuthorizedOperations returns authorized operations for an user
func AuthorizedOperations(user *auth.User, authorizer Authorizer) []*auth.OperationStatus {
	var actions []auth.Permission_ActionType
	for _, action := range auth.Permission_ActionType_value {
		actions = append(actions, auth.Permission_ActionType(action))
	}
	operations := authorizer.AuthorizedOperations(user, user.Tenant, globals.DefaultNamespace, actions...)
	// check cluster scoped authorization for user belonging to default tenant
	if user.Tenant == globals.DefaultTenant {
		clusterOperations := authorizer.AuthorizedOperations(user, "", "", actions...)
		operations = append(operations, clusterOperations...)
	}
	var accessReview []*auth.OperationStatus
	for _, authzOp := range operations {
		authzRes := authzOp.GetResource()
		op := &auth.Operation{
			Resource: &auth.Resource{Tenant: authzRes.GetTenant(), Group: authzRes.GetGroup(), Kind: authzRes.GetKind(), Namespace: authzRes.GetNamespace()},
			Action:   authzOp.GetAction(),
		}
		opStatus := &auth.OperationStatus{
			Operation: op,
			Allowed:   true,
		}
		accessReview = append(accessReview, opStatus)
	}
	return accessReview
}

// GetOperationsFromPermissions constructs authz.Operation from auth.Permission
func GetOperationsFromPermissions(permissions []auth.Permission) []Operation {
	var operations []Operation
	for _, permission := range permissions {
		// if actions are defined at a kind, group or namespace level
		if len(permission.ResourceNames) == 0 {
			for _, action := range permission.Actions {
				resource := NewResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, permission.ResourceNamespace, "")
				operations = append(operations, NewOperation(resource, action))
			}
			return operations
		}
		for _, resourceName := range permission.ResourceNames {
			for _, action := range permission.Actions {
				resource := NewResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, permission.ResourceNamespace, resourceName)
				operations = append(operations, NewOperation(resource, action))
			}
		}
	}
	return operations
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

// ValidatePerm validates that resource kind and group is valid in permission
func ValidatePerm(permission auth.Permission) error {
	s := runtime.GetDefaultScheme()
	switch permission.ResourceKind {
	case "", ResourceKindAll:
		if permission.ResourceGroup != ResourceGroupAll {
			if _, ok := s.Kinds()[permission.ResourceGroup]; !ok {
				return fmt.Errorf("invalid API group [%q]", permission.ResourceGroup)
			}
		}
	case auth.Permission_APIEndpoint.String():
		if permission.ResourceGroup != "" {
			return fmt.Errorf("invalid API group, should be empty instead of [%q]", permission.ResourceGroup)
		}
		if len(permission.ResourceNames) == 0 {
			return fmt.Errorf("missing API endpoint resource name")
		}
		var errs []error
		for _, resourceName := range permission.ResourceNames {
			err := ValidateResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, resourceName)
			if err != nil {
				errs = append(errs, err)
			}
		}
		if err := k8serrors.NewAggregate(errs); err != nil {
			return err
		}
	default:
		if err := ValidateResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, ""); err != nil {
			return err
		}
	}
	var errs []error
	if len(permission.Actions) == 0 {
		// AllActions is the default action if not specified
		permission.Actions = append(permission.Actions, auth.Permission_AllActions.String())
	}
	for _, action := range permission.Actions {
		if err := ValidateAction(permission.ResourceGroup, permission.ResourceKind, action); err != nil {
			errs = append(errs, err)
		}
	}
	return k8serrors.NewAggregate(errs)
}

// ValidatePerms validates that resource kind and group is valid in permissions
func ValidatePerms(permissions []auth.Permission) error {
	var errs []error
	for _, perm := range permissions {
		if err := ValidatePerm(perm); err != nil {
			errs = append(errs, err)
		}
	}
	return k8serrors.NewAggregate(errs)
}

// ValidateOperation validates operation
func ValidateOperation(op *auth.Operation) (Operation, error) {
	// make sure interface type and value are not nil
	if op == nil || reflect.ValueOf(op).IsNil() {
		return nil, fmt.Errorf("operation not specified")
	}
	res := op.GetResource()
	if res == nil || reflect.ValueOf(res).IsNil() {
		return nil, fmt.Errorf("resource not specified")
	}
	if err := k8serrors.NewAggregate(op.Validate("all", "", true, false)); err != nil {
		return nil, err
	}
	if err := ValidateResource(res.Tenant, res.Group, res.Kind, res.Name); err != nil {
		return nil, err
	}
	if err := ValidateAction(res.Group, res.Kind, op.GetAction()); err != nil {
		return nil, err
	}
	return NewOperation(NewResource(res.Tenant, res.Group, res.Kind, res.Namespace, res.Name), op.Action), nil
}

// ValidateResource validates resource information
func ValidateResource(tenant, group, kind, name string) error {
	s := runtime.GetDefaultScheme()
	switch kind {
	case auth.Permission_Event.String(), auth.Permission_Search.String(), auth.Permission_MetricsQuery.String(), auth.Permission_FwLog.String(), auth.Permission_AuditEvent.String(), auth.Permission_TokenAuth.String():
		if group != "" {
			return fmt.Errorf("invalid API group, should be empty instead of [%q]", group)
		}
	case auth.Permission_APIEndpoint.String():
		if group != "" {
			return fmt.Errorf("invalid API group, should be empty instead of [%q]", group)
		}
		if name == "" {
			return fmt.Errorf("missing API endpoint resource name")
		}
	default:
		if s.Kind2APIGroup(kind) != group {
			return fmt.Errorf("invalid resource kind [%q] and API group [%q]", kind, group)
		}
		ok, err := s.IsClusterScoped(kind)
		if err != nil {
			log.Infof("unknown resource kind [%q], err: %v", kind, err)
		}
		if ok && (tenant != globals.DefaultTenant && tenant != "") {
			return fmt.Errorf("tenant should be empty or [%q] for cluster scoped resource kind [%q]", globals.DefaultTenant, kind)
		}
		ok, err = s.IsTenantScoped(kind)
		if err != nil {
			log.Infof("unknown resource kind [%q], err: %v", kind, err)
		}
		if ok && tenant == "" {
			return fmt.Errorf("tenant should not be empty for tenant scoped resource kind [%q]", kind)
		}
	}
	return nil
}

// ValidateAction validates if an action is valid for a kind
func ValidateAction(group, kind, action string) error {
	if action == "" {
		return fmt.Errorf("action cannot be empty")
	}
	switch kind {
	case string(staging.KindBuffer):
	case auth.Permission_AuditEvent.String(), auth.Permission_MetricsQuery.String(), auth.Permission_FwLog.String(), auth.Permission_Event.String(), auth.Permission_Search.String(), auth.Permission_TokenAuth.String():
		switch action {
		case auth.Permission_Read.String():
		default:
			return fmt.Errorf("invalid resource kind [%q] and action [%q]", kind, action)
		}
	default:
		switch action {
		case auth.Permission_Clear.String(), auth.Permission_Commit.String():
			return fmt.Errorf("invalid resource kind [%q] and action [%q]", kind, action)
		default:
		}
	}
	return nil
}

// PrintOperations creates a string out of operations for logging
func PrintOperations(operations []Operation) string {
	var message string
	for _, oper := range operations {
		if oper != nil {
			res := oper.GetResource()
			if res != nil {
				owner := res.GetOwner()
				var ownerTenant, ownerName string
				if owner != nil {
					ownerTenant = owner.Tenant
					ownerName = owner.Name
				}
				message = message + fmt.Sprintf("resource(tenant: %v, group: %v, kind: %v, namespace: %v, name: %v, owner: %v|%v), action: %v, id: %v; ",
					res.GetTenant(),
					res.GetGroup(),
					res.GetKind(),
					res.GetNamespace(),
					res.GetName(),
					ownerTenant,
					ownerName,
					oper.GetAction(),
					oper.GetID())
			}
		}
	}
	return message
}

// PopulateMapWithUserPerms puts user and its permissions in the provided map. Pass in nil perms/roles for only populating user info
func PopulateMapWithUserPerms(data map[string][]string, user *auth.User, isAdmin bool, perms []auth.Permission, encodePerms bool) error {
	// validate user obj
	if user == nil {
		return errors.New("no user specified")
	}
	if user.Tenant == "" {
		return errors.New("tenant not populated in user")
	}
	if user.Name == "" {
		return errors.New("username not populated in user object")
	}
	// set user info
	data[UserTenantKey] = []string{user.Tenant}
	data[UsernameKey] = []string{user.Name}
	// set role info
	data[UserIsAdmin] = []string{}
	if isAdmin {
		data[UserIsAdmin] = []string{"true"}
	}
	// set perms, reset existing value
	data[PermsKey] = []string{}
	for _, perm := range perms {
		permbytes, err := perm.Marshal()
		if err != nil {
			return err
		}
		var permstr string
		if encodePerms {
			permstr = base64.StdEncoding.EncodeToString(permbytes)
		} else {
			permstr = string(permbytes)
		}
		data[PermsKey] = append(data[PermsKey], permstr)
	}
	return nil
}

// RemoveUserPerms removes user and its permissions from the given map
func RemoveUserPerms(data map[string][]string) {
	delete(data, UserTenantKey)
	delete(data, UsernameKey)
	delete(data, UserIsAdmin)
	delete(data, PermsKey)
}

// UserMetaFromMap returns user meta info
func UserMetaFromMap(data map[string][]string) (*api.ObjectMeta, bool) {
	names := data[UsernameKey]
	tenants := data[UserTenantKey]
	if len(names) == 0 || len(tenants) == 0 {
		return nil, false
	}
	return &api.ObjectMeta{Name: names[0], Tenant: tenants[0]}, true
}

// UserIsAdminFromMap returens user roles
func UserIsAdminFromMap(data map[string][]string) (bool, bool) {
	if data[UserIsAdmin] == nil {
		return false, false
	}
	if len(data[UserIsAdmin]) == 0 {
		return false, true
	}
	return true, true
}

// PermsFromMap returns user permissions
func PermsFromMap(data map[string][]string, decodePerms bool) ([]auth.Permission, bool, error) {
	vals, ok := data[PermsKey]
	if !ok {
		return nil, ok, nil
	}
	var perms []auth.Permission
	for _, val := range vals {
		var data []byte
		var err error
		if decodePerms {
			data, err = base64.StdEncoding.DecodeString(val)
			if err != nil {
				return nil, false, err
			}
		} else {
			data = []byte(val)
		}
		perm := &auth.Permission{}
		if err := perm.Unmarshal(data); err != nil {
			return nil, false, err
		}
		perms = append(perms, *perm)
	}
	return perms, true, nil
}
