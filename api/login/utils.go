package login

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api/generated/apiclient"

	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
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
	extReqID, ok := loginctx.ExtRequestIDHeaderFromContext(ctx)
	if ok {
		req.Header.Set("Pensando-Psm-External-Request-Id", extReqID)
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

// NewUserPreference is a helper to create new User Preference
func NewUserPreference(name, tenant, apiVersion string) *auth.UserPreference {
	userPref := &auth.UserPreference{}
	userPref.Defaults("all")
	userPref.Tenant = tenant
	userPref.Name = name
	userPref.UUID = uuid.NewV4().String()
	ts, _ := types.TimestampProto(time.Now())
	userPref.CreationTime.Timestamp = *ts
	userPref.ModTime.Timestamp = *ts
	userPref.GenerationID = "1"
	userPref.APIVersion = apiVersion
	userPref.SelfLink = userPref.MakeURI("configs", userPref.APIVersion, string(apiclient.GroupAuth))
	return userPref
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
