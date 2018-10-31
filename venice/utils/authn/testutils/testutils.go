package testutils

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils"
)

// RadiusConfig to define test config for ACS or OpenRadius
type RadiusConfig struct {
	// URL is the host:port of the radius server
	URL string `json:",omitempty"`
	// NasID of venice
	NasID string `json:",omitempty"`
	// NasSecret is the shared secret between venice and radius server
	NasSecret string `json:",omitempty"`
	// User is the radius username
	User string `json:",omitempty"`
	// Password is the radius user password
	Password string `json:",omitempty"`
	// UserGroups contain groups to which user belongs
	UserGroups []string `json:",omitempty"`
	// Tenant to which user belongs
	Tenant string `json:",omitempty"`
}

func encodeHTTPRequest(req *http.Request, request interface{}) error {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(request)
	if err != nil {
		return err
	}
	req.Body = ioutil.NopCloser(&buf)
	return nil
}

// getHTTPRequest creates a http request
func getHTTPRequest(instance string, in interface{}, method, path string) (*http.Request, error) {
	target, err := url.Parse(instance)
	if err != nil {
		return nil, fmt.Errorf("invalid instance %s", instance)
	}
	target.Path = path
	req, err := http.NewRequest(method, target.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("could not create request (%s)", err)
	}
	if err = encodeHTTPRequest(req, in); err != nil {
		return nil, fmt.Errorf("could not encode request (%s)", err)
	}
	return req, nil
}

// Login sends a login request to API Gateway and returns a *http.Response
func Login(apiGW string, in *auth.PasswordCredential) (*http.Response, error) {
	path := login.LoginURLPath
	req, err := getHTTPRequest(apiGW, in, "POST", path)
	if err != nil {
		return nil, err
	}
	client := http.DefaultClient
	ctx := context.Background()
	resp, err := client.Do(req.WithContext(ctx))
	return resp, err
}

// NewLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT.
// Returns original context in case of error.
func NewLoggedInContext(ctx context.Context, apiGW string, in *auth.PasswordCredential) (context.Context, error) {
	var err error
	var nctx context.Context
	if testutils.CheckEventually(func() (bool, interface{}) {
		nctx, err = login.NewLoggedInContext(ctx, apiGW, in)
		if err == nil {
			return true, nil
		}
		log.Errorf("unable to get logged in context (%v)", err)
		return false, nil
	}) {
		return nctx, err
	}

	return ctx, err
}

// CreateTestUser creates a test user
func CreateTestUser(apicl apiclient.Services, username, password, tenant string) (*auth.User, error) {
	// user object
	user := &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   username,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: password,
			Email:    "testuser@pensandio.io",
			Type:     auth.UserSpec_Local.String(),
		},
	}

	// create the user object in api server
	var err error
	var createdUser *auth.User
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdUser, err = apicl.AuthV1().User().Create(context.Background(), user)
		if err == nil || strings.HasPrefix(err.Error(), "Status:(409)") {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating user, Err: %v", err)
		return nil, err
	}
	return createdUser, err
}

// MustCreateTestUser creates testuser and panics if fails
func MustCreateTestUser(apicl apiclient.Services, username, password, tenant string) *auth.User {
	u, err := CreateTestUser(apicl, username, password, tenant)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateTestUser", err))
	}
	return u
}

// DeleteUser deletes an user
func DeleteUser(apicl apiclient.Services, username, tenant string) error {
	// delete user object in api server
	_, err := apicl.AuthV1().User().Delete(context.Background(), &api.ObjectMeta{Name: username, Tenant: tenant})
	return err
}

// MustDeleteUser deletes an user and throws a panic in case of error
func MustDeleteUser(apicl apiclient.Services, username, tenant string) {
	if err := DeleteUser(apicl, username, tenant); err != nil {
		panic(fmt.Sprintf("DeleteUser failed with err: %v", err))
	}
}

// CreateAuthenticationPolicy creates an authentication policy with local and ldap auth config. secret and radius config is set to nil in the policy
func CreateAuthenticationPolicy(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap) (*auth.AuthenticationPolicy, error) {
	return CreateAuthenticationPolicyWithOrder(apicl, local, ldap, nil, []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()})
}

// MustCreateAuthenticationPolicy creates an authentication policy with local, ldap and radius auth config
func MustCreateAuthenticationPolicy(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap, radius *auth.Radius) *auth.AuthenticationPolicy {
	pol, err := CreateAuthenticationPolicyWithOrder(apicl, local, ldap, radius, []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String(), auth.Authenticators_RADIUS.String()})
	if err != nil {
		panic(fmt.Sprintf("CreateAuthenticationPolicy failed with err %s", err))
	}
	return pol
}

// CreateAuthenticationPolicyWithOrder creates an authentication policy
func CreateAuthenticationPolicyWithOrder(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap, radius *auth.Radius, order []string) (*auth.AuthenticationPolicy, error) {
	// authn policy object
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               ldap,
				Local:              local,
				Radius:             radius,
				AuthenticatorOrder: order,
			},
		},
	}

	// create authentication policy object in api server
	var err error
	var createdPolicy *auth.AuthenticationPolicy
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdPolicy, err = apicl.AuthV1().AuthenticationPolicy().Create(context.Background(), policy)
		if err == nil || strings.HasPrefix(err.Error(), "Status:(409)") {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating authentication policy, Err: %v", err)
		return nil, err
	}
	return createdPolicy, err
}

// DeleteAuthenticationPolicy deletes an authentication policy
func DeleteAuthenticationPolicy(apicl apiclient.Services) error {
	// delete authentication policy object in api server
	_, err := apicl.AuthV1().AuthenticationPolicy().Delete(context.Background(), &api.ObjectMeta{Name: "AuthenticationPolicy"})
	if err != nil {
		log.Errorf("Error deleting AuthenticationPolicy: %v", err)
		return err
	}
	return nil
}

// MustDeleteAuthenticationPolicy deletes an authentication policy and throws a panic in case of error
func MustDeleteAuthenticationPolicy(apicl apiclient.Services) {
	if err := DeleteAuthenticationPolicy(apicl); err != nil {
		panic(fmt.Sprintf("DeleteAuthenticationPolicy failed with err: %v", err))
	}
}

// CreateTenant creates a tenant
func CreateTenant(apicl apiclient.Services, name string) (*cluster.Tenant, error) {
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.TenantSpec{},
	}
	var err error
	var createdTenant *cluster.Tenant
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdTenant, err = apicl.ClusterV1().Tenant().Create(context.Background(), tenant)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "500ms", "90s") {
		log.Errorf("Error creating tenant, Err: %v", err)
		return nil, err
	}
	return createdTenant, err
}

// MustCreateTenant creates a tenant and panics if fails
func MustCreateTenant(apicl apiclient.Services, name string) *cluster.Tenant {
	tenant, err := CreateTenant(apicl, name)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateTenant", err))
	}
	return tenant
}

// DeleteTenant deletes a tenant
func DeleteTenant(apicl apiclient.Services, tenant string) error {
	// delete tenant object in api server
	_, err := apicl.ClusterV1().Tenant().Delete(context.Background(), &api.ObjectMeta{Name: tenant})
	if err != nil {
		return err
	}
	deletedTenant, err := apicl.ClusterV1().Tenant().Get(context.Background(), &api.ObjectMeta{Name: tenant})
	if err == nil {
		log.Errorf("Error deleting tenant, found [%#v]", deletedTenant)
		return fmt.Errorf("deleted tenant still exists")
	}

	return nil
}

// MustDeleteTenant deletes a tenant
func MustDeleteTenant(apicl apiclient.Services, tenant string) {
	if err := DeleteTenant(apicl, tenant); err != nil {
		panic(fmt.Sprintf("DeleteTenant failed with err: %v", err))
	}
}

// CreateRole creates a role
func CreateRole(apicl apiclient.Services, name, tenant string, permissions ...auth.Permission) (*auth.Role, error) {
	role := login.NewRole(name, tenant, permissions...)
	var err error
	var createdRole *auth.Role
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdRole, err = apicl.AuthV1().Role().Create(context.Background(), role)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating role, Err: %v", err)
		return nil, err
	}
	return createdRole, err
}

// MustCreateRole creates a role and panics if fails
func MustCreateRole(apicl apiclient.Services, name, tenant string, permissions ...auth.Permission) *auth.Role {
	role, err := CreateRole(apicl, name, tenant, permissions...)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRole", err))
	}
	return role
}

// DeleteRole deletes a role
func DeleteRole(apicl apiclient.Services, name, tenant string) error {
	// delete role binding object in api server
	_, err := apicl.AuthV1().Role().Delete(context.Background(), &api.ObjectMeta{Name: name, Tenant: tenant})
	return err
}

// MustDeleteRole deletes a role
func MustDeleteRole(apicl apiclient.Services, name, tenant string) {
	if err := DeleteRole(apicl, name, tenant); err != nil {
		panic(fmt.Sprintf("DeleteRole failed with err: %v", err))
	}
}

// CreateRoleBinding creates a role binding
func CreateRoleBinding(apicl apiclient.Services, name, tenant, roleName string, users, groups []string) (*auth.RoleBinding, error) {
	// TODO: use func from rbac utils
	roleBinding := &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRoleBinding)},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: tenant,
		},
		Spec: auth.RoleBindingSpec{
			Users:      users,
			UserGroups: groups,
			Role:       roleName,
		},
	}
	var err error
	var createdRoleBinding *auth.RoleBinding
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdRoleBinding, err = apicl.AuthV1().RoleBinding().Create(context.Background(), roleBinding)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating role binding, Err: %v", err)
		return nil, err
	}
	return createdRoleBinding, err
}

// MustCreateRoleBinding creates a role binding and panics if fails
func MustCreateRoleBinding(apicl apiclient.Services, name, tenant, roleName string, users, groups []string) *auth.RoleBinding {
	roleBinding, err := CreateRoleBinding(apicl, name, tenant, roleName, users, groups)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRoleBinding", err))
	}
	return roleBinding
}

// DeleteRoleBinding deletes a role binding
func DeleteRoleBinding(apicl apiclient.Services, name, tenant string) error {
	// delete role binding object in api server
	_, err := apicl.AuthV1().RoleBinding().Delete(context.Background(), &api.ObjectMeta{Name: name, Tenant: tenant})
	return err
}

// MustDeleteRoleBinding deletes a role binding
func MustDeleteRoleBinding(apicl apiclient.Services, name, tenant string) {
	if err := DeleteRoleBinding(apicl, name, tenant); err != nil {
		panic(fmt.Sprintf("DeleteRoleBinding failed with err: %v", err))
	}
}

// SetAuthBootstrapFlag sets bootstrap flag in the cluster object
func SetAuthBootstrapFlag(apicl apiclient.Services) (*cluster.Cluster, error) {
	var clusterObj *cluster.Cluster
	var err error
	if !testutils.CheckEventually(func() (bool, interface{}) {
		clusterObj, err = apicl.ClusterV1().Cluster().AuthBootstrapComplete(context.Background(), &cluster.ClusterAuthBootstrapRequest{})
		return err == nil, err
	}, "100ms", "20s") {
		log.Errorf("Error setting bootstrap flag: %v", err)
		return nil, err
	}
	return clusterObj, err
}

// MustSetAuthBootstrapFlag sets bootstrap flag in the cluster object
func MustSetAuthBootstrapFlag(apicl apiclient.Services) *cluster.Cluster {
	clusterObj, err := SetAuthBootstrapFlag(apicl)
	if err != nil {
		panic(fmt.Sprintf("error %v in SetAuthBootstrapFlag", err))
	}
	return clusterObj
}

// CreateCluster creates a test cluster object
func CreateCluster(apicl apiclient.Services) (*cluster.Cluster, error) {
	clusterObj := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "Cluster",
		},
	}
	var err error
	var createdCluster *cluster.Cluster
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdCluster, err = apicl.ClusterV1().Cluster().Create(context.Background(), clusterObj)
		if (err == nil) || strings.Contains(err.Error(), "AlreadyExists") {
			return true, nil
		}
		return false, err
	}, "500ms", "90s") {
		log.Errorf("Error creating cluster, Err: %v", err)
		return nil, err
	}
	return createdCluster, nil
}

// MustCreateCluster creates a cluster and panics if fails
func MustCreateCluster(apicl apiclient.Services) *cluster.Cluster {
	cluster, err := CreateCluster(apicl)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateCluster", err))
	}
	return cluster
}

// DeleteCluster deletes a cluster object
func DeleteCluster(apicl apiclient.Services) error {
	// delete cluster object in api server
	_, err := apicl.ClusterV1().Cluster().Delete(context.Background(), &api.ObjectMeta{})
	if err != nil {
		return err
	}
	deletedCluster, err := apicl.ClusterV1().Cluster().Get(context.Background(), &api.ObjectMeta{})
	if err == nil {
		log.Errorf("Error deleting cluster, found [%#v]", deletedCluster)
		return fmt.Errorf("deleted cluster still exists")
	}

	return nil
}

// MustDeleteCluster deletes a cluster
func MustDeleteCluster(apicl apiclient.Services) {
	if err := DeleteCluster(apicl); err != nil {
		panic(fmt.Sprintf("DeleteCluster failed with err: %v", err))
	}
}
