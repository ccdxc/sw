package login

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
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
	_, token, err := UserLogin(apiGW, cred)
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
func UserLogin(apiGW string, in *auth.PasswordCredential) (*auth.User, string, error) {
	if !strings.HasPrefix(apiGW, "http") {
		apiGW = "http://" + apiGW
	}
	resp, err := login(apiGW, in)
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
func login(apiGW string, in *auth.PasswordCredential) (*http.Response, error) {
	req, err := netutils.CreateHTTPRequest(apiGW, in, "POST", LoginURLPath)
	if err != nil {
		return nil, err
	}
	client := http.DefaultClient
	ctx := context.Background()
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
	return auth.Permission{
		ResourceTenant:    tenant,
		ResourceGroup:     resourceGroup,
		ResourceKind:      resourceKind,
		ResourceNamespace: resourceNamespace,
		ResourceNames:     stringToSlice(resourceNames),
		Actions:           stringToSlice(actions),
	}
}

// NewRole is a helper to create new role
func NewRole(name, tenant string, permissions ...auth.Permission) *auth.Role {
	return &auth.Role{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRole)},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Spec: auth.RoleSpec{
			Permissions: permissions,
		},
	}
}

// NewRoleBinding is a helper to create new role binding
func NewRoleBinding(name, tenant, roleName, users, groups string) *auth.RoleBinding {
	return &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRoleBinding)},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Spec: auth.RoleBindingSpec{
			Users:      stringToSlice(users),
			UserGroups: stringToSlice(groups),
			Role:       roleName,
		},
	}
}

// NewClusterRole is a helper to create a role in default tenant
func NewClusterRole(name string, permissions ...auth.Permission) *auth.Role {
	return &auth.Role{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRole)},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: globals.DefaultTenant,
		},
		Spec: auth.RoleSpec{
			Permissions: permissions,
		},
	}
}

// NewClusterRoleBinding is a helper to create a role binding in default tenant
func NewClusterRoleBinding(name, roleName, users, groups string) *auth.RoleBinding {
	return &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRoleBinding)},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: globals.DefaultTenant,
		},
		Spec: auth.RoleBindingSpec{
			Users:      stringToSlice(users),
			UserGroups: stringToSlice(groups),
			Role:       roleName,
		},
	}
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
	case auth.Permission_Event.String(), auth.Permission_Search.String(), auth.Permission_MetricsQuery.String():
		if permission.ResourceGroup != "" {
			return fmt.Errorf("invalid API group, should be empty instead of [%q]", permission.ResourceGroup)
		}
	case auth.Permission_APIEndpoint.String():
		if permission.ResourceGroup != "" {
			return fmt.Errorf("invalid API group, should be empty instead of [%q]", permission.ResourceGroup)
		}
		if len(permission.ResourceNames) == 0 {
			return fmt.Errorf("missing API endpoint resource name")
		}
	default:
		if s.Kind2APIGroup(permission.ResourceKind) != permission.ResourceGroup {
			return fmt.Errorf("invalid resource kind [%q] and API group [%q]", permission.ResourceKind, permission.ResourceGroup)
		}
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

// PrintOperations creates a string out of operations for logging
func PrintOperations(operations []authz.Operation) string {
	var message string
	for _, oper := range operations {
		if oper != nil {
			message = message + fmt.Sprintf("%#v, action: %v; ", oper.GetResource(), oper.GetAction())
		}
	}
	return message
}
