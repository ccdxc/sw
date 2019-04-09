package login

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net/http"
	"reflect"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"
	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// LoginURLPath is where handler for login POST request is registered
	LoginURLPath = "/v1/login"

	// SessionID is cookie name storing JWT
	SessionID = "sid"
)

// NewLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT.
// Returns nil in case of error.
func NewLoggedInContext(ctx context.Context, apiGW string, cred *auth.PasswordCredential) (context.Context, error) {
	_, token, err := UserLogin(ctx, apiGW, cred)
	if err != nil {
		return nil, err
	}

	return loginctx.NewContextWithAuthzHeader(ctx, "Bearer "+token), nil
}

// GetTokenFromCookies returns session token from cookie
func GetTokenFromCookies(cookies []*http.Cookie) (string, error) {
	var token string
	for _, cookie := range cookies {
		if cookie.Name == SessionID {
			token = cookie.Value
			break
		}
	}
	if token == "" {
		return token, fmt.Errorf("cookie with name (%s) not found", SessionID)
	}
	return token, nil
}

// UserLogin sends a login request to API Gateway and returns authenticated user and session token upon success
func UserLogin(ctx context.Context, apiGW string, in *auth.PasswordCredential) (*auth.User, string, error) {
	if !strings.HasPrefix(apiGW, "https") {
		apiGW = "https://" + apiGW
	}
	resp, err := login(ctx, apiGW, in)
	if err != nil {
		return nil, "", err
	}
	if resp.StatusCode != http.StatusOK {
		return nil, "", fmt.Errorf("login failed, status code: %v", resp.StatusCode)
	}
	var user auth.User
	if err := json.NewDecoder(resp.Body).Decode(&user); err != nil {
		return nil, "", err
	}

	token, err := GetTokenFromCookies(resp.Cookies())
	if err != nil {
		return nil, token, err
	}
	return &user, token, nil
}

// login sends a login request to API Gateway and returns a *http.Response
func login(ctx context.Context, apiGW string, in *auth.PasswordCredential) (*http.Response, error) {
	req, err := netutils.CreateHTTPRequest(apiGW, in, "POST", LoginURLPath)
	if err != nil {
		return nil, err
	}
	client := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: true,
			},
		},
	}
	resp, err := client.Do(req.WithContext(ctx))
	return resp, err
}

func stringToSlice(val string) (ret []string) {
	if val == "" || val == "," {
		ret = nil
	} else {
		ret = strings.Split(val, ",")
	}
	return
}

// NewPermission is helper to create a permission for a role
func NewPermission(tenant, resourceGroup, resourceKind, resourceNamespace, resourceNames, actions string) auth.Permission {
	perm := auth.Permission{}
	perm.Defaults("all")
	perm.ResourceTenant = tenant
	perm.ResourceGroup = resourceGroup
	perm.ResourceKind = resourceKind
	if resourceNamespace != "" {
		perm.ResourceNamespace = resourceNamespace
	}
	perm.ResourceNames = stringToSlice(resourceNames)
	if actions != "" {
		perm.Actions = stringToSlice(actions)
	}
	return perm
}

// NewRole is a helper to create new role
func NewRole(name, tenant string, permissions ...auth.Permission) *auth.Role {
	role := &auth.Role{}
	role.Defaults("all")
	if tenant != "" {
		role.Tenant = tenant
	}
	role.Name = name
	role.Spec.Permissions = permissions
	role.UUID = uuid.NewV4().String()
	ts, _ := types.TimestampProto(time.Now())
	role.CreationTime.Timestamp = *ts
	role.ModTime.Timestamp = *ts
	role.GenerationID = "1"
	return role
}

// NewRoleBinding is a helper to create new role binding
func NewRoleBinding(name, tenant, roleName, users, groups string) *auth.RoleBinding {
	roleBinding := &auth.RoleBinding{}
	roleBinding.Defaults("all")
	if tenant != "" {
		roleBinding.Tenant = tenant
	}
	roleBinding.Name = name
	roleBinding.Spec.Role = roleName
	roleBinding.Spec.Users = stringToSlice(users)
	roleBinding.Spec.UserGroups = stringToSlice(groups)
	roleBinding.UUID = uuid.NewV4().String()
	ts, _ := types.TimestampProto(time.Now())
	roleBinding.CreationTime.Timestamp = *ts
	roleBinding.ModTime.Timestamp = *ts
	roleBinding.GenerationID = "1"
	return roleBinding
}

// NewClusterRole is a helper to create a role in default tenant
func NewClusterRole(name string, permissions ...auth.Permission) *auth.Role {
	return NewRole(name, globals.DefaultTenant, permissions...)
}

// NewClusterRoleBinding is a helper to create a role binding in default tenant
func NewClusterRoleBinding(name, roleName, users, groups string) *auth.RoleBinding {
	return NewRoleBinding(name, globals.DefaultTenant, roleName, users, groups)
}

// GetOperationsFromPermissions constructs authz.Operation from auth.Permission
func GetOperationsFromPermissions(permissions []auth.Permission) []authz.Operation {
	var operations []authz.Operation
	for _, permission := range permissions {
		// if actions are defined at a kind, group or namespace level
		if len(permission.ResourceNames) == 0 {
			for _, action := range permission.Actions {
				resource := authz.NewResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, permission.ResourceNamespace, "")
				operations = append(operations, authz.NewOperation(resource, action))
			}
			return operations
		}
		for _, resourceName := range permission.ResourceNames {
			for _, action := range permission.Actions {
				resource := authz.NewResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, permission.ResourceNamespace, resourceName)
				operations = append(operations, authz.NewOperation(resource, action))
			}
		}
	}
	return operations
}

// ValidatePerm validates that resource kind and group is valid in permission
func ValidatePerm(permission auth.Permission) error {
	s := runtime.GetDefaultScheme()
	switch permission.ResourceKind {
	case "", authz.ResourceKindAll:
		if permission.ResourceGroup != authz.ResourceGroupAll {
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
		return ValidateResource(permission.ResourceTenant, permission.ResourceGroup, permission.ResourceKind, "")
	}
	return nil
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
func ValidateOperation(op *auth.Operation) (authz.Operation, error) {
	// make sure interface type and value are not nil
	if op == nil || reflect.ValueOf(op).IsNil() {
		return nil, fmt.Errorf("operation not specified")
	}
	res := op.GetResource()
	if res == nil || reflect.ValueOf(res).IsNil() {
		return nil, fmt.Errorf("resource not specified")
	}
	if err := k8serrors.NewAggregate(op.Validate("all", "", true)); err != nil {
		return nil, err
	}
	if err := ValidateResource(res.Tenant, res.Group, res.Kind, res.Name); err != nil {
		return nil, err
	}
	return authz.NewOperation(authz.NewResource(res.Tenant, res.Group, res.Kind, res.Namespace, res.Name), op.Action), nil
}

// ValidateResource validates resource information
func ValidateResource(tenant, group, kind, name string) error {
	s := runtime.GetDefaultScheme()
	switch kind {
	case auth.Permission_Event.String(), auth.Permission_Search.String(), auth.Permission_MetricsQuery.String(), auth.Permission_FwlogsQuery.String(), auth.Permission_AuditEvent.String():
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
		if ok && tenant != "" {
			return fmt.Errorf("tenant should be empty for cluster scoped resource kind [%q]", kind)
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

// PrintOperations creates a string out of operations for logging
func PrintOperations(operations []authz.Operation) string {
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
				message = message + fmt.Sprintf("resource(tenant: %v, group: %v, kind: %v, namespace: %v, name: %v, owner: %v|%v), action: %v; ",
					res.GetTenant(),
					res.GetGroup(),
					res.GetKind(),
					res.GetNamespace(),
					res.GetName(),
					ownerTenant,
					ownerName,
					oper.GetAction())
			}
		}
	}
	return message
}
